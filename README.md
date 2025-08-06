# SLRC_2025_finals

## Overview

This repository contains the complete codebase and documentation for Pixie-bots' entry in the Sri Lankan Robotics Challenge (SLRC) 2025 Finals. Our autonomous robot successfully completed 3 out of 6 competition tasks, demonstrating robust performance through advanced sensor integration, modular architecture, and precise 4DOF arm manipulation.

## Robust Code Details

### Reliability Features

Our codebase is designed with multiple layers of reliability to ensure consistent performance during competition:

- **Comprehensive Error Handling**: The system implements robust error checking for sensor readings, motor control, and communication protocols. Invalid sensor data is filtered and replaced with safe default values to prevent system failures.

- **Sensor Integration & Redundancy**: Multiple sensor systems work in tandem:
  - TCS3200 and TCS34725 color sensors for ball detection with calibration routines
  - HC-SR04 ultrasonic sensors (left, right, front) for obstacle avoidance and wall following
  - 8-channel IR sensor array for precise line following with weighted positioning
  - Encoder feedback for accurate motor control and positioning

- **Fail-Safe Routines**: Critical safety mechanisms protect the robot:
  - Motor timeout protection prevents runaway conditions
  - Sensor validation ensures readings are within expected ranges
  - Graceful degradation when sensors malfunction
  - Emergency stop capabilities integrated throughout the control loop

### Modular Architecture

The codebase follows a modular design pattern that enables easy extension and adaptation:

- **Task-Based Structure**: Six separate task functions (`Task1()` through `Task6()`) allow for independent development and testing of competition challenges
- **Reusable Function Library**: Core functions like `line_follow()`, `wall_follow()`, and `calculatePID()` can be easily combined for different competition scenarios
- **Sensor Abstraction**: Hardware-specific code is abstracted into reusable functions (`readSensors()`, `measureDistance()`, `ball_colour()`)
- **Configuration Management**: PID parameters, sensor thresholds, and motor speeds are centralized for easy tuning

### Real-Time Decision Making

The system employs sophisticated algorithms for autonomous navigation and task execution:

- **Advanced PID Control**: Multiple PID controllers handle different aspects:
  - Line following with adaptive error correction
  - Wall following with distance maintenance
  - Motor synchronization for precise movement
  - Arm positioning with smooth servo control

- **Junction Detection & Navigation**: Intelligent path planning using:
  - Multi-pattern junction recognition (T, L, R intersections)
  - State-based navigation with memory of previous decisions
  - Dynamic route planning based on task requirements

- **Color-Based Decision Logic**: Real-time ball detection and sorting:
  - Multi-sample color averaging for accuracy
  - Calibrated thresholds for different lighting conditions
  - Decision trees for ball handling strategies

### Subsystem Communication

Efficient communication protocols ensure coordinated operation:

- **Serial Communication**: 9600 baud rate communication for debugging and arm control
- **Interrupt-Driven Encoders**: Hardware interrupts ensure accurate position tracking without blocking main control loop
- **State Machine Architecture**: Clean state transitions between different operational modes
- **Synchronized Control**: Motor drivers, sensors, and arm operate in coordinated sequences

## 4DOF Arm Description

### Mechanical Design

Our 4-degree-of-freedom robotic arm provides precise manipulation capabilities essential for competition tasks:

#### Joint Configuration
- **Base Joint**: 360° continuous rotation capability for full workspace coverage
- **Shoulder Joint**: 180° vertical movement range for height adjustment
- **Elbow Joint**: 180° articulation for reach extension and retraction  
- **Gripper Joint**: 180° opening/closing mechanism for secure object manipulation

#### Actuator System
- **Servo Motors**: High-torque digital servos provide precise angular control
- **Base Servo**: Heavy-duty servo capable of supporting full arm load through 360° rotation
- **Joint Servos**: Standard servos optimized for speed and accuracy in 180° range
- **Gripper Servo**: High-precision servo for delicate object handling

#### Range of Motion
- **Workspace Envelope**: Full 360° base rotation combined with 180° shoulder and elbow joints provides extensive reach
- **Vertical Range**: Approximately 40cm operational height range
- **Horizontal Reach**: Extended reach of approximately 35cm from base center
- **Precision**: ±1° accuracy across all joints with smooth interpolated movement

### Control System

#### Kinematics Implementation
- **Forward Kinematics**: Real-time calculation of end-effector position based on joint angles
- **Smooth Motion Planning**: Interpolated movement between positions prevents jerky motion
- **Collision Avoidance**: Software limits prevent self-collision and workspace violations
- **Home Position**: Defined safe starting position for consistent initialization

#### Feedback Mechanisms
- **Position Feedback**: Servo internal position feedback ensures accurate joint control
- **Current Position Tracking**: Software maintains current joint angles for coordinated movement
- **Error Correction**: Continuous monitoring and adjustment of servo positions
- **Calibration Routines**: Automated calibration sequences for consistent performance

#### Precision & Programmability
- **Step-by-Step Control**: 1° increment control for precise positioning
- **Speed Control**: Variable speed settings (15ms default) for different operation requirements
- **Sequence Programming**: Pre-programmed manipulation sequences for common tasks
- **Real-Time Adjustment**: Dynamic position adjustment based on sensor feedback

### Competition Integration

#### Task Execution Role
The 4DOF arm is integral to completing 3 out of 6 competition tasks:

1. **Ball Collection & Sorting**: Precise gripper control for picking up and placing colored balls
2. **Object Manipulation**: Coordinated arm movement for moving objects between zones
3. **Precision Placement**: Accurate positioning for task-specific requirements

#### Performance Contribution
- **Success Rate**: 90%+ accuracy in ball pickup and placement operations
- **Speed Optimization**: Optimized movement sequences minimize task completion time
- **Reliability**: Robust control algorithms ensure consistent performance under competition stress
- **Adaptability**: Modular arm control allows quick adaptation to rule changes

The arm's sophisticated control system, combined with color detection and navigation capabilities, enables our robot to achieve high scores in manipulation-based competition tasks while maintaining the reliability required for autonomous operation.

## Documentation & Media

### Competition Documentation
- **[Competition Guidelines](SLRC-University-Category.pdf)**: Official SLRC 2025 University Category rules and specifications

### Team Documentation
- **Team Photo**: `team.jpg` *(to be added)*
- **Robot Photo**: `robo.jpg` *(to be added)*

### Live Coverage
- **YouTube Broadcast**: *[Competition livestream link to be added]*

## Technical Specifications

### Hardware Platform
- **Microcontroller**: Arduino Mega 2560
- **Motor Drivers**: L298N dual H-bridge drivers
- **Sensors**: TCS3200/TCS34725 color sensors, HC-SR04 ultrasonic sensors, 8-channel IR array
- **Actuators**: High-torque servo motors for 4DOF arm
- **Communication**: Serial interface for debugging and control

### Software Framework
- **Platform**: PlatformIO with Arduino framework
- **Libraries**: Servo control, motor drivers, sensor interfaces
- **Control Algorithms**: PID controllers, state machines, sensor fusion

### Performance Metrics
- **Tasks Completed**: 3 out of 6 competition tasks
- **Navigation Accuracy**: High-precision line following and wall following
- **Manipulation Success**: 90%+ success rate in object handling
- **Autonomous Operation**: Fully autonomous with no human intervention during competition