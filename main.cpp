#include <graphics.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <cctype>

#include "Config.h"
#include "Vehicle.h"
#include "TrafficLight.h"
#include "Intersection.h"
#include "HUD.h"

int main() {
    // Initialize WinBGIm Graphics Window
    initwindow(SCREEN_WIDTH, SCREEN_HEIGHT, (char*)"Adaptive Traffic Signal Simulation - Smart Density Control");

    IntersectionManager intersection;
    HUD hud;

    int page = 0;
    bool isPaused = false;

    auto lastTime = std::chrono::high_resolution_clock::now();
    double simulationTime = 0.0;

    float frameCounter = 0;
    float fpsTimer = 0.0f;
    float currentFPS = 60.0f;

    // Main Simulation Loop
    while (true) {
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = now - lastTime;
        lastTime = now;

        float dt = elapsed.count();
        if (dt > 0.05f) dt = 0.05f; // Clamp delta time to avoid frame skips

        fpsTimer += dt;
        frameCounter++;
        if (fpsTimer >= 0.5f) {
            currentFPS = frameCounter / fpsTimer;
            frameCounter = 0;
            fpsTimer = 0.0f;
        }

        // 1. Keyboard Input Handling
        if (kbhit()) {
            char key = getch();
            char uKey = std::toupper(key);

            if (key == 27) { // ESC key
                break;
            } else if (uKey == 'N') {
                intersection.spawnVehicle(DIR_SOUTH, simulationTime);
            } else if (uKey == 'S') {
                intersection.spawnVehicle(DIR_NORTH, simulationTime);
            } else if (uKey == 'E') {
                intersection.spawnVehicle(DIR_EAST, simulationTime);
            } else if (uKey == 'W') {
                intersection.spawnVehicle(DIR_WEST, simulationTime);
            } else if (uKey == 'M') {
                intersection.lightController.toggleMode();
            } else if (uKey == 'B') {
                intersection.triggerBurstSpawn(simulationTime);
            } else if (uKey == 'C') {
                intersection.clearAllVehicles();
            } else if (uKey == 'P') {
                isPaused = !isPaused;
            }
        }

        // 2. Mouse Click Input Handling
        if (ismouseclick(WM_LBUTTONDOWN)) {
            int mx, my;
            getmouseclick(WM_LBUTTONDOWN, mx, my);
            intersection.handleMouseClick(mx, my, simulationTime);
        }

        // 3. Update Physics & Light Timing (if not paused)
        if (!isPaused) {
            simulationTime += dt;
            intersection.update(dt, simulationTime);
        }

        // 4. Double Buffered Render
        setactivepage(page);
        cleardevice();

        hud.drawScenery();
        hud.drawDetectionZones(intersection);
        intersection.drawVehicles();
        hud.drawTrafficLights(intersection.lightController);
        hud.drawDashboard(intersection, currentFPS, isPaused);

        setvisualpage(page);
        page = 1 - page;

        // Frame rate limiter (~60 FPS)
        delay(14);
    }

    closegraph();
    return 0;
}
