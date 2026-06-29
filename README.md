# R6_Drone
This repo is a documentation of the ongoing succusses and failures with building a drone inspired by Tom Clancys Rainbow Six Seige. As a disclaimer, because I was doing this project alone, I have used AI to generate part of the code specifically the front end, however the back end, the motor control files are written by me. I am also not an everything engineer as I spent most of my time researching the electronics for this project and had to supplement a lot of the other challenges to AI.


# Theoretical analysis
  This drone is composed of a cylindrical chassis with two mecanum wheels attached at both ends. In the game, it is able to move in all directions and even jump, however, the jumping is above my paygrade and a few concessions must be done in order to get the omnimovement working. I will refer to these concessions in the Experimentation section.
  The main challenge with this setup is the coaxle mecanum setup. Usually a typical mecanum wheel setup contains 4 mecanum wheels with 2 one 1 axis and the other 2 on the other axis. This setup allows diagonal forces to sufficently cancel and move omnidirectionally by controlling each wheel independently. For example if you want to go forward all wheels go forward, but sideways diagonal wheels must be going in the same direction. However, it gets trickier beacuse we only have one axis and two wheels. Looking at the drone form the game there is a tail (commonly called the antennae) on the back of the drone. It creates an important 3rd point of contact so that it can actually move forward. If there was no tail, the motors would just spin the chassis.
  I plan to use using an Esp32-S3 with a Camera module, a L298N motor driver, 12V 200RPM motors, a Buck converter for the MCU and a 12V battery. The chassis I designed in Fusion and the Axle I edited an existing model. I will 3d print all the parts and assemble them with nuts and bolts where needed.

# Experimentation and analysis
In practice, I was able to get the drone to move forward, however there were a few problems with the idea of the drone as a whole.
# 1) Whenever I tried to strafe the drone would just spin
Whenever strafing, the tail would lose contact and the chasis rotates centerpoint because both wheels are spinning in opposite directions. Because there is no third point of contact, the wheels couldn't slip and generate enough diagonal force to strafe. If strafing wasn't a dealbreaker this concept actually would work quite well, however I already spent like 200$ on mecanum wheels so I stuck with high standards. 
# 2 Backward movement
It's impossible for the drone to move backwards in its current state. What would happen is the chassis would completelty flip until the drone was upside down and the tail made contact with the ground again. It would still work it would just be upside down and take a couple seconds to flup cover completely before changing directions.


# Potential Fixes

1) Integrated Flywheel:
   I'm no mecanical engineer, however a wheel on the inside that spins may be able to resist the torque of the motors that allow for short bursts of sideward movement. However, this would need a complete redesign and a PCB with feedback sensors, which honestly isn't worth testing.
2) Skids:
  Probably the most feasible of the solutions, adding skids to the front of the chassis would limit its ability to rotate past a certain point, and with this may allow for some slipping and strafing may be possible.

   
