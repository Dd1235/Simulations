# Cloth Simulation with Verlet Integration (Work in Progress, Mass-Spring Model is almost done)

This project is a real-time cloth simulation implemented in C++ using SFML (Simple and Fast Multimedia Library). It showcases a physically-based cloth model using Verlet integration as an integrator, and simulates cloth using a simplified mass-spring model, providing interactive features such as tearing and pinning. The simulation demonstrates fundamental concepts in physics and computer graphics.

## Table of Contents

- [Features](#features)
- [How to Use](#how-to-use)
- [Detailed Explanation](#detailed-explanation)
  - [Physics Behind the Simulation](#physics-behind-the-simulation)
    - [Particles and Constraints](#particles-and-constraints)
    - [Verlet Integration](#verlet-integration)
    - [Constraint Satisfaction](#constraint-satisfaction)
    - [Collision Handling](#collision-handling)
    - [Tearing Mechanism](#tearing-mechanism)
    - [Pinning Mechanism](#pinning-mechanism)
  - [Code Structure](#code-structure)
    - [Classes Overview](#classes-overview)
    - [Main Function Workflow](#main-function-workflow)
  - [Time Step and Accumulator](#time-step-and-accumulator)
- [Code](#code)

## Features

- **Realistic Cloth Simulation**: Simulates cloth behavior using particles connected by constraints.
- **Verlet Integration**: Employs Verlet integration for stable and accurate motion simulation.
- **Interactive Tearing**: Click and drag to tear the cloth along a drawn path.
- **Dynamic Pinning**: Switch to pinning mode to add or remove pins on the cloth, fixing particles in place.
- **Collision Detection**: Cloth particles interact with the ground, preventing them from falling through.
- **Adjustable Parameters**: Modify cloth resolution, stiffness, and other parameters for different effects.
- **Visual Indicators**: Pinned particles are highlighted, and modes are displayed for user guidance.

## How to Use

1. **Setup**:

   - Ensure you have SFML installed on your system.
   - Compile the code using a C++ compiler with SFML linked.

2. **Running the Simulation**:

   - Execute the compiled program.
   - A window will open displaying the cloth simulation.

3. **Interaction**:

   - **Normal Mode**:
     - **Tearing**: Click and drag the left mouse button over the cloth to draw a red line. Release the mouse button to tear the cloth along the drawn path.
   - **Pinning Mode**:
     - Press the **'P'** key to toggle pinning mode.
     - **Adding/Removing Pins**: Click on the cloth to pin or unpin particles. Pinned particles are shown in blue.
   - The current mode is displayed at the top-left corner of the window.

4. **Exiting**:
   - Close the window or press the close button to exit the simulation.

## Detailed Explanation

### Physics Behind the Simulation

#### Particles and Constraints

- **Particles**: Represented as points with position, previous position, and acceleration.
- **Constraints**: Connections between particles that maintain a specific rest length, simulating the structural integrity of the cloth.

#### Verlet Integration

- **Concept**: A numerical method used to integrate Newton's equations of motion.
- **Implementation**:
  - **Position Update**:
    \[ \text{new position} = \text{current position} + (\text{current position} - \text{previous position}) + \text{acceleration} \times (\text{time step})^2 \]
  - **Advantages**:
    - No need to compute velocities explicitly.
    - Provides stability for systems with constraints.

#### Constraint Satisfaction

- **Purpose**: Ensures particles remain at their rest distances, preserving the cloth's shape.
- **Process**:
  - Compute the difference between the current distance and the rest length.
  - Adjust particle positions to satisfy the constraints.
  - Repeat over multiple iterations for better accuracy.

#### Collision Handling

- **Ground Collision**:
  - Particles are prevented from falling below the ground level.
  - If a particle collides with the ground, its position is adjusted, and its vertical velocity is inverted to simulate a bounce.

#### Tearing Mechanism

- **Interaction**:
  - Users can draw a line over the cloth to indicate where it should tear.
- **Process**:
  - Detect intersections between the tear line and cloth constraints.
  - Deactivate intersected constraints, allowing the cloth to tear along the path.

#### Pinning Mechanism

- **Interaction**:
  - Users can add or remove pins in pinning mode.
- **Process**:
  - Clicking on a particle toggles its pinned state.
  - Pinned particles remain fixed in space, affecting the cloth's movement.

### Code Structure

#### Classes Overview

1. **Particle**

   - **Attributes**:
     - `position`: Current position of the particle.
     - `previousPosition`: Position in the previous time step.
     - `acceleration`: Current acceleration acting on the particle.
     - `isPinned`: Indicates if the particle is fixed in space.
   - **Methods**:
     - `applyForce()`: Adds force to the particle's acceleration.
     - `update()`: Updates the particle's position using Verlet integration.
     - `applyDamping()`: Applies damping to simulate energy loss.
     - `handleGroundCollision()`: Checks and handles collision with the ground.

2. **Constraint**

   - **Attributes**:
     - `particle1` and `particle2`: The two particles connected by the constraint.
     - `restLength`: The original length of the constraint.
     - `isActive`: Indicates if the constraint is active.
   - **Methods**:
     - `satisfy()`: Adjusts particle positions to satisfy the constraint.
     - `deactivate()`: Deactivates the constraint, simulating a tear.

3. **InputHandler**
   - **Static Attributes**:
     - `isDragging`: Tracks if the user is currently dragging the mouse.
     - `isPinMode`: Indicates if the simulation is in pinning mode.
     - `dragPath`: Stores the path of the mouse during dragging.
   - **Static Methods**:
     - `handleEvents()`: Processes user input and updates the simulation accordingly.
     - `drawOverlay()`: Draws additional visuals like the tear line or pin cursor.
     - `handleTearing()`: Manages tearing interactions.
     - `handlePinning()`: Manages pinning interactions.
     - `togglePin()`: Adds or removes a pin on a particle.
     - `processTear()`: Deactivates constraints intersected by the tear line.
     - `lineIntersectsLine()`: Checks if two line segments intersect.

#### Main Function Workflow

1. **Initialization**:

   - Create the simulation window.
   - Initialize particles and constraints to form the cloth mesh.
   - Set up structural, shear, and bend constraints for realistic behavior.

2. **Simulation Loop**:
   - **Event Handling**: Capture and process user inputs (tearing, pinning, mode switching).
   - **Physics Update**:
     - Apply forces (gravity) to particles.
     - Update particle positions using Verlet integration.
     - Apply damping and collision handling.
     - Satisfy constraints over multiple iterations.
   - **Rendering**:
     - Clear the window.
     - Draw constraints and particles.
     - Draw overlays (tear line, pin cursor).
     - Display the current mode.
     - Display the updated frame.

### Time Step and Accumulator

- **Fixed Time Step**:
  - The simulation uses a fixed time step (`TIME_STEP = 0.016f` seconds) to ensure consistent physics updates regardless of frame rate.
- **Accumulator**:
  - **Purpose**: Handles situations where the frame rendering time doesn't align perfectly with the fixed time step.
  - **Process**:
    - Accumulate elapsed time.
    - Update the simulation in fixed increments (`TIME_STEP`) as long as the accumulator allows.
    - This ensures that the physics simulation runs smoothly and accurately over time.

## To do

1. Also Implement using

- Finite element method
- Position based dynamics

2. Collision detecting with other objects
3. Optimize processTear() and togglePin()

## Some resources I found helpful

1. [Short video explaining Verlet Integration](https://youtu.be/g55QvpAev0I?si=JdR67iT-l5GhM6Vh)
2. [Advanced Character Physics paper](https://www.cs.cmu.edu/afs/cs/academic/class/15462-s13/www/lec_slides/Jakobsen.pdf)
3. [Notes on Mass-Spring modeling](https://ocw.mit.edu/courses/6-837-computer-graphics-fall-2012/resources/mit6_837f12_lec08/)

## Also look into

1. [Large Steps in Cloth Simulation](https://www.cs.cmu.edu/~baraff/papers/sig98.pdf)
2. [Integration Methods](https://cseweb.ucsd.edu/classes/sp16/cse169-a/slides/CSE169_11.pdf)
3. [Position Based Dynamics](https://matthias-research.github.io/pages/publications/posBasedDyn.pdf)
4. [Line Segment Intersection](https://www.cs.umd.edu/class/spring2020/cmsc754/Lects/lect04-intersection.pdf)
