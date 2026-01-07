# AVT_25-26 Grupo 13 - Drone Delivery Simulation

## Made By IST Students
- Ramiro Moldes
- João Santos
- David Quintino

## Report
[📄 Project Report](Report.pdf)
[📄 Project Trailer Download link](https://drive.tecnico.ulisboa.pt/download/851498741624655)

## Project Description

This is an OpenGL-based 3D drone delivery simulation developed for the Advanced Visualization Techniques (AVT) course. Players control a drone navigating through a city environment to pick up and deliver packages to designated destinations while managing battery life and avoiding obstacles.

## Features

### Core Gameplay
- **Drone Control System**: Full 3D movement with physics-based flight mechanics
- **Package Delivery**: Pick up packages and deliver them to marked destinations
- **Battery Management**: Monitor and manage battery consumption during flight
- **Collision Detection**: Real-time collision system for obstacles and interaction objects
- **Score System**: Track delivery performance and earn points

### Graphics & Visual Effects
- **Advanced Lighting**: Phong shading with multiple light sources including drone headlights
- **Dynamic Skybox**: Day and night modes with environment cube mapping
- **Shadow Mapping**: Real-time shadows for objects and billboards
- **Particle System**: Fireworks particle effects with up to 1500 particles
- **2D Lens Flare**: Dynamic lens flare effects for light sources
- **Billboard Rendering**: grass and tree billboards for environment detail
- **Fog Effects**: Atmospheric fog for enhanced depth perception
- **Normal Mapping**: Enhanced surface detail on terrain

### Camera System
- **Multiple Camera Modes**: Switch between orthographic and perspective projections
- **Top-Down View**: Orthographic camera for strategic navigation
- **Perspective Views**: Third-person and first-person camera options
- **Rear-View Mirror**: Real-time rear-view display using stencil buffer

### Technical Features
- **Model Loading**: Support for OBJ models via Assimp library
- **Texture Mapping**: Multiple texture modes and texture support (TGA, PNG)
- **TrueType Font Rendering**: On-screen text rendering for UI elements
- **Environment Mapping**: Reflection effects on selected objects
- **Shader-Based Rendering**: Modern OpenGL with custom vertex and fragment shaders

### Controls

#### Drone Movement
- **W** - Move forward
- **S** - Move backward
- **A** - Move left
- **D** - Move right
- **Arrow Up** - Ascend
- **Arrow Down** - Descend
- **Arrow Left** - Rotate left
- **Arrow Right** - Rotate right

#### Camera Controls
- **1** - Activate Camera 1 (Top-down orthographic view)
- **2** - Activate Camera 2 (Top perspective view)
- **3** - Activate Camera 3 (Third-person view with mouse control)
- **Left Mouse Button + Drag** - Rotate camera (Camera 3 only)

#### Lighting Controls
- **N** - Toggle day/night (directional light)
- **C** - Toggle point lights
- **H** - Toggle spotlights (drone headlights)

#### Game Controls
- **P** - Pause/unpause game
- **R** - Restart game (reset drone and delivery mission)
- **F** - Toggle fog effects
- **K** - Toggle debug mode (show light positions and hitboxes)
- **I** - Toggle keybinds display

#### Special
- **ESC** - Exit application

## How to Run
Open **AVT_Project.sln** in **Microsoft Visual Studio** and build/run the solution (F5 or Ctrl+F5).

---

**Built with**: OpenGL 3.3+, GLEW, FreeGLUT, Assimp, DevIL, STB TrueType
