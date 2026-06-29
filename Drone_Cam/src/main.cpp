#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include "Joystick.h"
#include <WebSocketsServer.h>
#include "MotorControl.h"

/*
  Drone_Cam - main.cpp

  Purpose:
  - Initialize the OV3660 camera on the XIAO ESP32S3 Sense board,
    start a WiFi access point, and serve a simple web UI.
  - Two endpoints are provided:
    - "/capture": returns a single JPEG image (one-shot capture).
    - "/stream": returns a multipart MJPEG stream for live preview.

  This file focuses on clarity: each section below includes explanatory
  comments describing what the code does and why.
*/

// WiFi Access Point credentials
// The ESP32 will create a soft AP so other devices (phone/browser)
// can connect directly to the camera without an external router.
const char* ap_ssid = "XIAO-CAM";       // SSID clients will see
const char* ap_password = "12345678";   // Simple password for AP mode

MotorControl motor;

// Lightweight HTTP server listening on port 80.
// We use the Arduino `WebServer` class to handle basic request routing.
WebServer server(80);

// XIAO ESP32S3 Sense OV3660 pin map
// Camera pin and configuration structure for esp_camera
// This maps the OV3660 sensor pins to the XIAO ESP32S3 Sense board
// and defines image format, frame size, and other runtime settings.
camera_config_t camera_config = {
  .pin_pwdn = -1,
  .pin_reset = -1,
  .pin_xclk = 10,
  .pin_sccb_sda = 40,
  .pin_sccb_scl = 39,

  .pin_d7 = 48,
  .pin_d6 = 11,
  .pin_d5 = 12,
  .pin_d4 = 14,
  .pin_d3 = 16,
  .pin_d2 = 18,
  .pin_d1 = 17,
  .pin_d0 = 15,

  .pin_vsync = 38,
  .pin_href = 47,
  .pin_pclk = 13,

  .xclk_freq_hz = 20000000,
  .ledc_timer = LEDC_TIMER_0,
  .ledc_channel = LEDC_CHANNEL_0,

  .pixel_format = PIXFORMAT_JPEG,

  // Frame size: QVGA (320x240) keeps the data small for web serving.
  // Change to a larger size only if the board has enough RAM and bandwidth.
  .frame_size = FRAMESIZE_QVGA,

  // JPEG quality: lower is better compression (smaller data), range 0-63.
  // 12 is a reasonable trade-off for preview + capture on constrained boards.
  .jpeg_quality = 12,

  // Number of frame buffers. 1 uses least memory, but prevents parallel
  // processing. For streaming on low-RAM boards, keep this at 1.
  .fb_count = 1
};

// Very small HTML front-end served at the root path.
// It contains an <img> tag pointing to the streaming endpoint for live preview
// and a link to capture a single still image.
const char* html = R"rawliteral(
<html>
<head>
<title>XIAO Camera</title>
</head>
<body>
<h2>XIAO ESP32S3 Camera</h2>
<img src="/stream">
<br><br>
<a href="/capture">Capture Image</a>
<br>
<a href="/joystick_ui">Joystick</a> | <a href="/control">Control (Video + Joystick)</a>
</body>
</html>
)rawliteral";

void handleRoot() {
  // Serve the simple HTML page defined above. This is a synchronous
  // response that returns the full HTML document with content-type text/html.
  server.send(200, "text/html", html);
}

void handleCapture() {

  // One-shot capture handler. This endpoint grabs a single frame from
  // the camera and returns it as a JPEG image with proper HTTP headers.
  // Steps:
  //  1) Acquire a frame buffer from the camera driver (blocking call).
  //  2) If acquisition fails, return HTTP 500.
  //  3) Write raw HTTP response headers and the JPEG payload to the
  //     client socket, then release the frame buffer back to the driver.

  camera_fb_t * fb = esp_camera_fb_get(); // get a frame buffer

  if (!fb) {
    // If the camera driver returned null, something went wrong with capture.
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }

  // Use the raw client socket so we can write binary JPEG data directly.
  WiFiClient client = server.client();

  // Manually write the HTTP response headers for an image payload.
  client.printf("HTTP/1.1 200 OK\r\n");
  client.printf("Content-Type: image/jpeg\r\n");
  client.printf("Content-Length: %u\r\n\r\n", fb->len);
  client.write(fb->buf, fb->len); // write the JPEG bytes

  // Return the buffer to the driver to free associated memory.
  esp_camera_fb_return(fb);
}

void handleStream() {

  // MJPEG streaming handler. The response uses the multipart content-type
  // `multipart/x-mixed-replace` with a boundary string. Each loop iteration
  // captures a frame and writes one MIME part containing a JPEG image.
  // Clients (browsers) will render this as a continuous stream.

  WiFiClient client = server.client();

  // Initial HTTP response headers for multipart stream. Note the boundary
  // used later to separate individual JPEG frames.
  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n"
  );

  // Loop while the client connection is open. Each iteration grabs a new
  // frame and sends it as a MIME part. We include short delays to avoid
  // saturating the CPU and allow other tasks to run.
  while (client.connected()) {

    camera_fb_t * fb = esp_camera_fb_get();

    if (!fb) {
      // If capture failed, wait briefly and retry. This prevents a tight
      // busy-loop when the camera temporarily cannot produce frames.
      delay(100);
      continue;
    }

    // Write the multipart boundary and per-part headers, then the JPEG
    // payload and a trailing CRLF to separate parts.
    client.printf("--frame\r\n");
    client.printf("Content-Type: image/jpeg\r\n");
    client.printf("Content-Length: %u\r\n\r\n", fb->len);
    client.write(fb->buf, fb->len);
    client.print("\r\n");

    // Release the frame buffer back to the driver.
    esp_camera_fb_return(fb);

    if (!client.connected()) break; // stop if client disconnected

    // Small delay to control frame rate and yield to the scheduler.
    delay(30);
  }
}

void setup() {

  setupMotorControl();

  // Initialize serial for debug output. Useful to see boot messages and
  // error diagnostics during development.
  Serial.begin(115200);
  delay(2000);

  Serial.println("Starting camera...");

  // Initialize the camera driver with our pin and format config above.
  // This will allocate frame buffers according to `.fb_count` and prepare
  // the sensor for capture.
  esp_err_t err = esp_camera_init(&camera_config);

  if (err != ESP_OK) {
    // If initialization fails, print the error code so users can debug
    // hardware pin mappings, power, or sensor issues.
    Serial.printf("Camera init failed: 0x%x\n", err);
  } else {
    Serial.println("Camera init OK");
  }

  // Start the ESP32 in Access Point mode so phones or browsers can
  // connect directly to the board's WiFi and access the web UI.
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);

  Serial.println("WiFi Access Point started");
  Serial.print("Connect to: ");
  Serial.println(ap_ssid);

  // Print the IP address for convenience. For softAP, this is usually
  // 192.168.4.1 unless changed by the network stack.
  Serial.print("Open browser at: http://");
  Serial.println(WiFi.softAPIP());

  // Register HTTP endpoint handlers. The server will call the provided
  // functions when a request arrives for the matching path.
  server.on("/", handleRoot);
  server.on("/capture", handleCapture);
  server.on("/stream", handleStream);

  // Register joystick UI and command endpoints (non-blocking handlers).
  setupJoystickRoutes(server);

  // Start the HTTP server and begin listening for connections.
  server.begin();
}

void loop() {
  // Handle incoming HTTP clients. This should be called frequently; it
  // processes pending client connections and dispatches registered handlers.
  server.handleClient();
  updateJoystickWebSocket();
  motor.WriteMotors();
  Serial.print(getJoystickX());
  Serial.print(", ");
  Serial.println(getJoystickY());
}