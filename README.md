# Adaptive Traffic Signal Simulation 🚦
### Smart Density-Sensing 2D Traffic Control System

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg?style=for-the-badge&logo=cplusplus)](https://en.cppreference.com/w/cpp/17)
[![Graphics](https://img.shields.io/badge/Graphics-WinBGIm-green.svg?style=for-the-badge)](https://en.wikipedia.org/wiki/Borland_Graphics_Interface)
[![Platform](https://img.shields.io/badge/Platform-Windows-0078D6.svg?style=for-the-badge&logo=windows)](https://microsoft.com)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)](LICENSE)

---

## 📌 Project Overview

Traditional traffic signals rely on fixed-duration timers, which frequently cause unnecessary congestion: green lights stay active for empty lanes while vehicles queue up on opposing approaches. 

This project delivers a real-time **2D Adaptive Smart Traffic Control System** built in **C++17** using **WinBGIm Graphics**. The system continuously monitors active vehicle density inside virtual detection zones and dynamically adjusts green light durations in real time to optimize traffic throughput and minimize average wait times.

---

## ✨ Key Features & Functional Modules

- 🚘 **Vehicle Dynamics & Physics**:
  - **Car-Following Model**: Vehicles maintain safe distance gaps to avoid rear-end collisions.
  - **Smooth Braking & Stop-Line Behavior**: Cars decelerate smoothly and halt precisely at stop lines when signals are red or yellow.
  - **Smooth Acceleration**: Rapid acceleration to target cruising speed when signals turn green.
- 📡 **Real-Time Smart Density Counter**:
  - Monitors virtual detection zones (180px back from stop lines) across all 4 approaches (Northbound, Southbound, Eastbound, Westbound).
  - Continuously counts vehicles entering, waiting, or clearing the detection zones.
- 🧠 **Adaptive Light Controller**:
  - **Dynamic Extension Algorithm**: Automatically extends green light duration during high traffic conditions ($T_{green} = \text{clamp}(T_{base} + k \cdot N_{cars}, T_{min}, T_{max})$).
  - **Dual Mode Toggle**: Live side-by-side comparison between **Adaptive Smart Mode** and **Fixed-Timer Baseline Mode**.
- 📊 **On-Screen Dashboard (HUD)**:
  - Active signal phase display and progress countdown bar.
  - Real-time vehicle counts per lane with visual density level indicators (Low / Medium / High).
  - Live system metrics: Total Cleared Vehicles, Average Vehicle Wait Time (sec), and Simulation FPS counter.
- 🎮 **Interactive User Stress-Testing**:
  - Manual vehicle spawning per approach via keyboard hotkeys or mouse clicks on roads.
  - Traffic spike burst mode to stress-test adaptive signal response under heavy congestion.
  - Pause/Resume and Scene Clear controls.

---

## 📐 Adaptive Algorithm Model

The adaptive signal controller calculates target green light duration ($T_{green}$) at each phase transition based on the real-time vehicle count ($N_{cars}$) inside active detection zones:

$$T_{green} = \max\left(T_{min}, \min\left(T_{max}, T_{base} + N_{cars} \times \Delta t_{car}\right)\right)$$

Where:
- $T_{base} = 6.0\text{ seconds}$ (Baseline green time)
- $\Delta t_{car} = 1.5\text{ seconds}$ (Extension time per waiting vehicle)
- $T_{min} = 4.0\text{ seconds}$ (Minimum green threshold)
- $T_{max} = 20.0\text{ seconds}$ (Maximum green cap)

---

## 🎮 Interactive Controls

| Hotkey / Input | Action |
| :--- | :--- |
| `N` | Spawn vehicle on **Southbound** approach (Top) |
| `S` | Spawn vehicle on **Northbound** approach (Bottom) |
| `E` | Spawn vehicle on **Eastbound** approach (Left) |
| `W` | Spawn vehicle on **Westbound** approach (Right) |
| **Mouse Left Click** | Click on any road lane to spawn a vehicle |
| `M` | Toggle between **Adaptive Smart Mode** and **Fixed-Timer Mode** |
| `B` | Trigger **Burst Traffic Spike** (Spawns cars on all 4 lanes) |
| `C` | **Clear** all vehicles from the scene |
| `P` | **Pause / Resume** simulation |
| `ESC` | Exit simulation |

---

## 📁 Project Structure

```
ADAPTIVE-TRAFFIC-SIGNAL-SIMULATION/
├── Config.h            # Simulation constants, geometry, timing & physics rules
├── Vehicle.h/.cpp      # Vehicle class (position, physics, braking, drawing)
├── TrafficLight.h/.cpp # Signal state machine, timer, and adaptive algorithm
├── Intersection.h/.cpp # Multi-lane traffic manager, spawner & detection zones
├── HUD.h/.cpp          # Scenery, signals, detection zones & dashboard HUD rendering
├── main.cpp            # 60 FPS double-buffered main render loop & input handler
└── .vscode/
    └── tasks.json      # MinGW g++ build task configuration
```

---

## 🛠️ Prerequisites & Installation

### Requirements
- **OS**: Windows (10/11)
- **Compiler**: GCC 9.2.0+ (MinGW-w64) supporting C++17
- **Graphics Library**: WinBGIm (`graphics.h` and Windows GDI libraries)

### Build & Run Instructions

#### Option 1: Command Line (GCC)
Open terminal or MinGW shell in the project root directory and run:

```bash
# Compile all source files
g++ -std=c++17 main.cpp Vehicle.cpp TrafficLight.cpp Intersection.cpp HUD.cpp -o traffic_sim.exe -lbgi -lgdi32 -lcomdlg32 -luuid -loleaut32 -lole32

# Execute simulation
./traffic_sim.exe
```

#### Option 2: Visual Studio Code
1. Open the project folder in VS Code.
2. Press `Ctrl + Shift + B` to trigger the build task (`Build Traffic Simulation`).
3. Run `traffic_sim.exe` from terminal or via Code Runner.

---

## 📈 Performance & Results

- **Reduced Wait Times**: Under uneven traffic conditions, Adaptive Smart Mode significantly reduces average vehicle wait time compared to Fixed-Timer Mode.
- **Dynamic Throughput**: Green signals automatically shorten for empty lanes and extend up to 20 seconds during traffic spikes to clear congestion rapidly.
- **60 FPS Animation**: Double-buffering prevents screen flickering and ensures smooth vehicle animation.

---

## 📜 License

This project is open source and available under the [MIT License](LICENSE).
