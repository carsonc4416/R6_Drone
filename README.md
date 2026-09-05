# R6_Drone

This repo is a documentation of the ongoing successes and failures with building a drone inspired by Tom Clancy's Rainbow Six Siege.

![Drone_2 assembly, isometric view](https://github.com/user-attachments/assets/b9bdab08-6a4e-4bd2-bc8c-9937ed43940b)

*The assembly in Fusion: cylindrical chassis, one mecanum wheel at each end, tail arm off the back.*

**Contents**

- [Theoretical analysis](#theoretical-analysis)
- [Experimentation and analysis](#experimentation-and-analysis)
- [Potential fixes](#potential-fixes)

---

## Theoretical analysis

This drone is composed of a cylindrical chassis with two mecanum wheels attached at both ends. In the game, it is able to move in all directions and even jump, however, the jumping is above my paygrade and a few concessions must be done in order to get the omnimovement working. I will refer to these concessions in the Experimentation section.

The main challenge with this setup is the coaxial mecanum setup. Usually a typical mecanum wheel setup contains 4 mecanum wheels with 2 on 1 axis and the other 2 on the other axis. This setup allows diagonal forces to sufficiently cancel and move omnidirectionally by controlling each wheel independently. For example if you want to go forward all wheels go forward, but sideways diagonal wheels must be going in the same direction. However, it gets trickier because we only have one axis and two wheels. In control terms the platform is underactuated, with two actuators against three degrees of freedom, so lateral velocity isn't independently commandable no matter how the wheels are driven.

![Side view of the chassis](https://github.com/user-attachments/assets/ff475d51-10b8-4f09-9370-d1c6fa50b9f0)

*Side view. Both mecanum wheels share a single axis, which is the whole problem.*

Looking at the drone from the game there is a tail (commonly called the antennae) on the back of the drone. It creates an important 3rd point of contact so that it can actually move forward. If there was no tail, the motors would just spin the chassis.

I plan to use an ESP32-S3 with a Camera module, a L298N motor driver, 12V 200RPM motors, a Buck converter for the MCU and a 12V battery. The chassis I designed in Fusion and the axle I edited an existing model. I will 3D print all the parts and assemble them with nuts and bolts where needed.

## Experimentation and analysis

In practice, I was able to get the drone to move forward, however there were a few problems with the idea of the drone as a whole.

**1) Whenever I tried to strafe the drone would just spin**

Whenever strafing, the tail would lose contact and the chassis rotates about its centerpoint because both wheels are spinning in opposite directions. Because there is no third point of contact, the wheels couldn't slip and generate enough diagonal force to strafe. If strafing wasn't a dealbreaker this concept actually would work quite well, however I already spent like 200$ on mecanum wheels so I stuck with high standards.

![Top view of the chassis](https://github.com/user-attachments/assets/a857c751-db1c-4695-94b3-6269468a6b17)

*Top view. With two contact points on one axis, driving the wheels in opposite directions rotates the chassis about its centerpoint instead of translating it sideways.*

**2) Backward movement**

It's impossible for the drone to move backwards in its current state. What would happen is the chassis would completely flip until the drone was upside down and the tail made contact with the ground again. It would still work it would just be upside down and take a couple seconds to flip over completely before changing directions.

## Potential fixes

1. **Integrated flywheel:** I'm no mechanical engineer, however a wheel on the inside that spins may be able to resist the torque of the motors that allow for short bursts of sideward movement. However, this would need a complete redesign and a PCB with feedback sensors, which honestly isn't worth testing.
2. **Skids:** Probably the most feasible of the solutions, adding skids to the front of the chassis would limit its ability to rotate past a certain point, and with this may allow for some slipping and strafing may be possible.

---

Front-end UI generated with AI assistance; motor control and drive logic written by me.
