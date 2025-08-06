
---

# SLRC 2025 Finals – Pixie-bots

Welcome to the official repository for the Pixie-bots team's robot developed for the Sri Lankan Robotics Challenge (SLRC) 2025 University Category!

## Overview

This robot was designed to compete in the SLRC 2025 University Category, which consists of 6 challenging tasks as outlined in the official [SLRC University Category PDF](https://ent.uom.lk/wp-content/uploads/2025/02/SLRC-University-Category.pdf). Our robot successfully completed 3 out of the 6 tasks at the grand finale.

## Competition Details

- **Event:** SLRC 2025 University Category Robotics Competition
- **Organizer:** Electronic Club, University of Moratuwa
- **Tasks:** 6 tasks ([PDF](https://ent.uom.lk/wp-content/uploads/2025/02/SLRC-University-Category.pdf))
- **Achievement:** 3 tasks completed successfully

## Grand Finale

The robot's performance at the grand finale was broadcast live on YouTube.  
Watch our robot in action from **5:56:00 to 6:12:00** in the stream:  
[SLRC 2025 Finals Live Broadcast](https://m.youtube.com/watch?v=aplZ_r-cd3Y)

## Images

![Team Pixie-bots](team.jpg)  
*Our dedicated team behind the project*

![Competition Robot](robo.jpg)  
*The robot that competed at SLRC 2025 Finals*

---

## Robust Code Architecture

Our robot’s performance relied on a highly robust and modular codebase, designed to maximize reliability and flexibility in a demanding competition environment.

### Reliability Features

- **Comprehensive Error Handling:** All critical subsystems feature layered error detection and graceful recovery for hardware faults, sensor anomalies, and communication failures.
- **Sensor Integration:** Real-time fusion of sensor data (IMU, proximity, encoders) ensures accurate state estimation and reactive control.
- **Fail-Safe Routines:** The code includes watchdog timers and safety interlocks, guaranteeing that the robot enters a safe state in the event of unexpected behavior.

### Modularity & Extensibility

- **Task-Based Modules:** Each competition task is encapsulated in its own code module, allowing rapid adaptation or replacement before and during the event.
- **Subsystem Abstraction:** High-level orchestration communicates with independent modules for locomotion, manipulation, and sensing, allowing parallel development and debugging.

### Real-Time Decision Making

- **Event-Driven Architecture:** The robot employs asynchronous event loops to respond instantly to environmental changes and judge commands.
- **Inter-Subsystem Communication:** Efficient message passing ensures coordinated action between mobility, arm control, and sensor feedback, supporting complex multi-step tasks.

---

## 4DOF Robotic Arm

A key innovation in our robot is its custom-designed 4 Degrees of Freedom (4DOF) robotic arm, which enabled successful completion of manipulation tasks.

### Mechanical Design

- **Joints:** Shoulder rotation, elbow flexion, wrist rotation, and gripper actuation.
- **Actuators:** Precision digital servos provide smooth, accurate movement across all axes.
- **Range of Motion:** The arm’s kinematic chain allows a workspace optimized for object pickup, placement, and interaction with competition elements.

### Control System

- **Kinematics:** Forward and inverse kinematics enable precise end-effector positioning for complex tasks.
- **Feedback:** Integrated encoders and current sensors deliver closed-loop feedback for position and force control.
- **Precision & Programmability:** The arm supports programmable trajectories and variable-speed movement for delicate or rapid operations.

### Role in Competition Tasks

- **Task Execution:** The 4DOF arm was essential for tasks requiring object manipulation, including picking, sorting, and placing components.
- **Contribution to Success:** Its reliability and precision were pivotal in completing 3 out of 6 tasks, handling varied objects and interfaces under time pressure.

---

## Repository Contents

- Source code for robust robot control and task automation
- Documentation and setup instructions
- Technical details and schematics of the 4DOF arm
- Media from the grand finale

## References

- [SLRC University Category 2025 Tasks (PDF)](https://ent.uom.lk/wp-content/uploads/2025/02/SLRC-University-Category.pdf)

---

For more information or to contribute, feel free to open an issue or pull request!

---