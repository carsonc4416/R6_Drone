#ifndef JOYSTICK_H
#define JOYSTICK_H
#include <WebServer.h>
void setupJoystickRoutes(WebServer &server);
void updateJoystickWebSocket();

bool joystickIsRecent(unsigned long timeoutMs);
void getJoystickSnapshot(float &outX, float &outY, unsigned long &outLastMillis);
float getJoystickX();
float getJoystickY();
unsigned long joystickLastUpdateMillis();
void clearJoystick();

#endif // JOYSTICK_H