#include "Joystick.h"
#include <Arduino.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

static volatile float joystick_x = 0.0f;
static volatile float joystick_y = 0.0f;
static volatile unsigned long s_last = 0;

static WebServer *s_server = nullptr;
static WebSocketsServer s_webSocket = WebSocketsServer(81); // websocket on port 81

// ------------------------------
// Web pages
// ------------------------------

static const char* joystickPage = R"rawliteral(
<!doctype html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
  body{font-family:sans-serif;text-align:center;margin:0;padding:0}
  #pad{width:320px;height:320px;background:#222;margin:20px auto;border-radius:8px;touch-action:none;position:relative}
  #knob{width:80px;height:80px;background:#0af;border-radius:50%;position:absolute;left:160px;top:160px;transform:translate(-50%,-50%)}
  #status{margin-top:10px;font-size:14px}
</style>
</head>
<body>
<h3>Virtual Joystick (WebSocket)</h3>
<div id="pad"><div id="knob"></div></div>
<div id="status">Connecting...</div>

<script>
const pad = document.getElementById('pad');
const knob = document.getElementById('knob');
const statusEl = document.getElementById('status');

let dragging = false;
let ws = null;
let holdX = 0;
let holdY = 0;
let lastSend = 0;

function connectWS() {
  const proto = location.protocol === 'https:' ? 'wss' : 'ws';
  ws = new WebSocket(`${proto}://${location.hostname}:81/`);

  ws.onopen = () => {
    statusEl.textContent = 'WebSocket connected';
  };

  ws.onclose = () => {
    statusEl.textContent = 'WebSocket disconnected, retrying...';
    setTimeout(connectWS, 1000);
  };

  ws.onerror = () => {
    statusEl.textContent = 'WebSocket error';
  };
}

function sendXY(x, y) {
  if (!ws || ws.readyState !== WebSocket.OPEN) return;
  ws.send(`${x.toFixed(3)},${y.toFixed(3)}`);
}

function throttledSend(x, y) {
  const now = performance.now();
  if (now - lastSend >= 20) {   // about 50 Hz max
    lastSend = now;
    sendXY(x, y);
  }
}

function updateFromPoint(px, py) {
  const r = pad.getBoundingClientRect();
  const cx = r.left + r.width / 2;
  const cy = r.top + r.height / 2;
  const max = r.width / 2;

  let dx = px - cx;
  let dy = cy - py; // up = positive

  dx = Math.max(-max, Math.min(max, dx));
  dy = Math.max(-max, Math.min(max, dy));

  const nx = dx / max;
  const ny = dy / max;

  holdX = nx;
  holdY = ny;

  knob.style.left = `${(r.width / 2) + dx}px`;
  knob.style.top  = `${(r.height / 2) - dy}px`;

  throttledSend(nx, ny);
}

function resetKnob() {
  knob.style.left = '160px';
  knob.style.top = '160px';
  holdX = 0;
  holdY = 0;
  sendXY(0, 0);
}

// Keepalive while held so joystick stays fresh
setInterval(() => {
  if (dragging) sendXY(holdX, holdY);
}, 100);

pad.addEventListener('pointerdown', (e) => {
  dragging = true;
  pad.setPointerCapture(e.pointerId);
  updateFromPoint(e.clientX, e.clientY);
});

pad.addEventListener('pointermove', (e) => {
  if (dragging) updateFromPoint(e.clientX, e.clientY);
});

pad.addEventListener('pointerup', () => {
  dragging = false;
  resetKnob();
});

pad.addEventListener('pointercancel', () => {
  dragging = false;
  resetKnob();
});

connectWS();
</script>
</body>
</html>
)rawliteral";

static const char* combinedPage = R"rawliteral(
<!doctype html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
  body{font-family:sans-serif;text-align:center;margin:0;padding:0}
  #container{display:flex;flex-direction:column;align-items:center}
  #stream{max-width:320px;border:1px solid #444;margin:10px}
  #pad{width:320px;height:320px;background:#222;margin:10px;border-radius:8px;touch-action:none;position:relative}
  #knob{width:80px;height:80px;background:#0af;border-radius:50%;position:absolute;left:160px;top:160px;transform:translate(-50%,-50%)}
  #status{margin-top:8px;font-size:14px}
  @media(min-width:700px){#container{flex-direction:row;justify-content:center} #stream{margin-right:20px}}
</style>
</head>
<body>
<h3>Live Preview + Joystick (WebSocket)</h3>
<div id="container">
  <div>
    <img id="stream" src="/capture" alt="stream">
    <div id="status">Connecting...</div>
  </div>
  <div id="pad"><div id="knob"></div></div>
</div>

<script>
const streamImg = document.getElementById('stream');
const statusEl = document.getElementById('status');

function refreshFrame() {
  streamImg.src = '/capture?ts=' + Date.now();
}
setInterval(refreshFrame, 200);

const pad = document.getElementById('pad');
const knob = document.getElementById('knob');

let dragging = false;
let ws = null;
let holdX = 0;
let holdY = 0;
let lastSend = 0;

function connectWS() {
  const proto = location.protocol === 'https:' ? 'wss' : 'ws';
  ws = new WebSocket(`${proto}://${location.hostname}:81/`);

  ws.onopen = () => {
    statusEl.textContent = 'WebSocket connected';
  };

  ws.onclose = () => {
    statusEl.textContent = 'WebSocket disconnected, retrying...';
    setTimeout(connectWS, 1000);
  };

  ws.onerror = () => {
    statusEl.textContent = 'WebSocket error';
  };
}

function sendXY(x, y) {
  if (!ws || ws.readyState !== WebSocket.OPEN) return;
  ws.send(`${x.toFixed(3)},${y.toFixed(3)}`);
}

function throttledSend(x, y) {
  const now = performance.now();
  if (now - lastSend >= 20) {   // about 50 Hz max
    lastSend = now;
    sendXY(x, y);
  }
}

function updateFromPoint(px, py) {
  const r = pad.getBoundingClientRect();
  const cx = r.left + r.width / 2;
  const cy = r.top + r.height / 2;
  const max = r.width / 2;

  let dx = px - cx;
  let dy = cy - py; // up = positive

  dx = Math.max(-max, Math.min(max, dx));
  dy = Math.max(-max, Math.min(max, dy));

  const nx = dx / max;
  const ny = dy / max;

  holdX = nx;
  holdY = ny;

  knob.style.left = `${(r.width / 2) + dx}px`;
  knob.style.top  = `${(r.height / 2) - dy}px`;

  throttledSend(nx, ny);
}

function resetKnob() {
  knob.style.left = '160px';
  knob.style.top = '160px';
  holdX = 0;
  holdY = 0;
  sendXY(0, 0);
}

// Keepalive while held
setInterval(() => {
  if (dragging) sendXY(holdX, holdY);
}, 100);

pad.addEventListener('pointerdown', (e) => {
  dragging = true;
  pad.setPointerCapture(e.pointerId);
  updateFromPoint(e.clientX, e.clientY);
});

pad.addEventListener('pointermove', (e) => {
  if (dragging) updateFromPoint(e.clientX, e.clientY);
});

pad.addEventListener('pointerup', () => {
  dragging = false;
  resetKnob();
});

pad.addEventListener('pointercancel', () => {
  dragging = false;
  resetKnob();
});

connectWS();
</script>
</body>
</html>
)rawliteral";

// ------------------------------
// HTTP handlers
// ------------------------------

static void handleJoystickPage() {
  s_server->send(200, "text/html", joystickPage);
}

static void handleCombinedPage() {
  s_server->send(200, "text/html", combinedPage);
}

static void handleJoystickState() {
  char buf[128];
  snprintf(buf, sizeof(buf),
           "{\"x\":%.3f,\"y\":%.3f,\"last\":%lu}",
           joystick_x, joystick_y, s_last);
  s_server->send(200, "application/json", buf);
}

// ------------------------------
// WebSocket handler
// ------------------------------

static void handleWebSocketMessage(uint8_t num, uint8_t *payload, size_t length) {
  if (length == 0) return;

  String msg;
  msg.reserve(length);
  for (size_t i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  int comma = msg.indexOf(',');
  if (comma < 0) return;

  float fx = msg.substring(0, comma).toFloat();
  float fy = msg.substring(comma + 1).toFloat();

  fx = constrain(fx, -1.0f, 1.0f);
  fy = constrain(fy, -1.0f, 1.0f);

  const float deadzone = 0.03f;
  if (fabs(fx) < deadzone) fx = 0.0f;
  if (fabs(fy) < deadzone) fy = 0.0f;

  joystick_x = fx;
  joystick_y = fy;
  s_last = millis();

  // Optional debug
  // Serial.print("WS x=");
  // Serial.print(joystick_x, 3);
  // Serial.print(" y=");
  // Serial.println(joystick_y, 3);

  // Optional acknowledgement back to browser
  // s_webSocket.sendTXT(num, "ok");
}

static void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      break;

    case WStype_CONNECTED:
      s_webSocket.sendTXT(num, "connected");
      break;

    case WStype_TEXT:
      handleWebSocketMessage(num, payload, length);
      break;

    default:
      break;
  }
}

// ------------------------------
// Public setup/update functions
// ------------------------------

void setupJoystickRoutes(WebServer &server) {
  s_server = &server;

  server.on("/joystick_ui", HTTP_GET, handleJoystickPage);
  server.on("/control", HTTP_GET, handleCombinedPage);
  server.on("/joystick_state", HTTP_GET, handleJoystickState);

  s_webSocket.begin();
  s_webSocket.onEvent(webSocketEvent);
}

void updateJoystickWebSocket() {
  s_webSocket.loop();
}

bool joystickIsRecent(unsigned long timeoutMs) {
  unsigned long now = millis();
  return (now - s_last) <= timeoutMs;
}

void getJoystickSnapshot(float &outX, float &outY, unsigned long &outLastMillis) {
  outX = joystick_x;
  outY = joystick_y;
  outLastMillis = s_last;
}

float getJoystickX() {
  const unsigned long recentTimeout = 1000;
  if (!joystickIsRecent(recentTimeout)) return 0.0f;
  return joystick_x;
}

float getJoystickY() {
  const unsigned long recentTimeout = 1000;
  if (!joystickIsRecent(recentTimeout)) return 0.0f;
  return joystick_y;
}

unsigned long joystickLastUpdateMillis() {
  return s_last;
}

void clearJoystick() {
  joystick_x = 0.0f;
  joystick_y = 0.0f;
  s_last = 0;
}