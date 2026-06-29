#ifndef MotorControl_h
#define MotorControl_h

// Todo: Implement motor control functions for the drone.
// This will include functions to set motor speeds, directions,
// in accordance with virtual analog joystick values.


class MotorControl {
public:

void WriteMotors();
void setMotorControl(float, float);
};

void setupMotorControl();





#endif