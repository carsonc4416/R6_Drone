#include "MotorControl.h"
#include <Arduino.h>
#include "Joystick.h"
// Placeholder for motor control implementation.
// This file will contain functions to control the drone's motors based on
// virtual joystick inputs. For now, it's an empty stub to be filled in later.

const int PWM_PIN_1 = D0; // Example PWM pin for motor 1
const int pwmChannel_1 = 0;
const int pwmFreq_1 = 10000;     // 5 kHz
const int pwmResolution_1 = 8;  // 8-bit (0–255)
const int PWM_PIN_2 = D1; // Example PWM pin for motor 2
const int pwmChannel_2 = 1;
const int PWM_PIN_3 = D2; // Example PWM pin for motor 3
const int pwmChannel_3 = 2;
const int PWM_PIN_4 = D3; // Example PWM pin for motor 4
const int pwmChannel_4 = 3;




void setupMotorControl()
{
ledcSetup(pwmChannel_1, pwmFreq_1, pwmResolution_1);
ledcAttachPin(PWM_PIN_1, pwmChannel_1);
ledcSetup(pwmChannel_2, pwmFreq_1, pwmResolution_1);
ledcAttachPin(PWM_PIN_2, pwmChannel_2);
ledcSetup(pwmChannel_3, pwmFreq_1, pwmResolution_1);
ledcAttachPin(PWM_PIN_3, pwmChannel_3);
ledcSetup(pwmChannel_4, pwmFreq_1, pwmResolution_1);
ledcAttachPin(PWM_PIN_4, pwmChannel_4);
}

void MotorControl::WriteMotors() 
{
  float x = getJoystickX(); // Get the current X value from the joystick
  float y = getJoystickY(); // Get the current Y value from the joystick

  setMotorControl(x, y);
}

void MotorControl::setMotorControl(float x, float y)
{
 float motor1 = y + x;
 float motor2 = y - x;

 motor1 = constrain(motor1, -1.0f, 1.0f);
 motor2 = constrain(motor2, -1.0f, 1.0f);

int MapMotor1 = constrain((int)(fabs(motor1) * 255.0f), 0, 255);
int MapMotor2 = constrain((int)(fabs(motor2) * 255.0f), 0, 255);

if (motor1 > 0) {
  ledcWrite(pwmChannel_1, MapMotor1); // Forward
  ledcWrite(pwmChannel_2, 0); // Stop
} else if (motor1 < 0) {
  ledcWrite(pwmChannel_1, 0); // Stop
  ledcWrite(pwmChannel_2, MapMotor1); // Stop
} else {
  ledcWrite(pwmChannel_1, 0); // Stop
  ledcWrite(pwmChannel_2, 0); // Stop

}

if (motor2 > 0) {
  ledcWrite(pwmChannel_3, MapMotor2); // Forward
ledcWrite(pwmChannel_4, 0); // Forward
} else if (motor2 < 0) {
 ledcWrite(pwmChannel_3, 0);
 ledcWrite(pwmChannel_4, MapMotor2); // Stop
} else {
 ledcWrite(pwmChannel_3, 0); // Stop
 ledcWrite(pwmChannel_4, 0); // Stop
}
}