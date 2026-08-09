#include "HUD.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

HUD::HUD() {}

void HUD::drawScenery() const {
    // 1. Grass Background (Lush dark green)
    setfillstyle(SOLID_FILL, COLOR(34, 76, 42));
    bar(0, 0, HUD_X_START - 5, SCREEN_HEIGHT);

    // 2. Asphalt Roads (Dark slate)
    setfillstyle(SOLID_FILL, COLOR(45, 52, 60));
    // Vertical Road
    bar(ROAD_LEFT, 0, ROAD_RIGHT, SCREEN_HEIGHT);
    // Horizontal Road
    bar(0, ROAD_TOP, HUD_X_START - 5, ROAD_BOTTOM);

    // Intersection Box Area
    bar(ROAD_LEFT, ROAD_TOP, ROAD_RIGHT, ROAD_BOTTOM);

    // 3. Lane Center Markings (Dashed White & Double Yellow)
    // Double Yellow Center Lines - Vertical
    setcolor(YELLOW);
    line(CENTER_X - 1, 0, CENTER_X - 1, ROAD_TOP);
    line(CENTER_X + 1, 0, CENTER_X + 1, ROAD_TOP);
    line(CENTER_X - 1, ROAD_BOTTOM, CENTER_X - 1, SCREEN_HEIGHT);
    line(CENTER_X + 1, ROAD_BOTTOM, CENTER_X + 1, SCREEN_HEIGHT);

    // Double Yellow Center Lines - Horizontal
    line(0, CENTER_Y - 1, ROAD_LEFT, CENTER_Y - 1);
    line(0, CENTER_Y + 1, ROAD_LEFT, CENTER_Y + 1);
    line(ROAD_RIGHT, CENTER_Y - 1, HUD_X_START - 5, CENTER_Y - 1);
    line(ROAD_RIGHT, CENTER_Y + 1, HUD_X_START - 5, CENTER_Y + 1);

    // Zebra Crosswalks at Stop Lines
    setfillstyle(SOLID_FILL, WHITE);
    // South Approach Stop Line Zebra
    for (int x = ROAD_LEFT + 10; x < ROAD_RIGHT - 10; x += 20) {
        bar(x, STOP_LINE_SOUTH - 12, x + 10, STOP_LINE_SOUTH - 2);
    }
    // North Approach Stop Line Zebra
    for (int x = ROAD_LEFT + 10; x < ROAD_RIGHT - 10; x += 20) {
        bar(x, STOP_LINE_NORTH + 2, x + 10, STOP_LINE_NORTH + 12);
    }
    // East Approach Stop Line Zebra
    for (int y = ROAD_TOP + 10; y < ROAD_BOTTOM - 10; y += 20) {
        bar(STOP_LINE_EAST - 12, y, STOP_LINE_EAST - 2, y + 10);
    }
    // West Approach Stop Line Zebra
    for (int y = ROAD_TOP + 10; y < ROAD_BOTTOM - 10; y += 20) {
        bar(STOP_LINE_WEST + 2, y, STOP_LINE_WEST + 12, y + 10);
    }
}

void HUD::drawDetectionZones(const IntersectionManager& manager) const {
    setcolor(COLOR(0, 200, 255));

    // Southbound Detection Zone (Top)
    int s_left = ROAD_LEFT + 5;
    int s_top = STOP_LINE_SOUTH - DETECTION_ZONE_LENGTH;
    int s_right = CENTER_X - 5;
    int s_bottom = STOP_LINE_SOUTH - 14;
    rectangle(s_left, s_top, s_right, s_bottom);

    // Northbound Detection Zone (Bottom)
    int n_left = CENTER_X + 5;
    int n_top = STOP_LINE_NORTH + 14;
    int n_right = ROAD_RIGHT - 5;
    int n_bottom = STOP_LINE_NORTH + DETECTION_ZONE_LENGTH;
    rectangle(n_left, n_top, n_right, n_bottom);

    // Eastbound Detection Zone (Left)
    int e_left = STOP_LINE_EAST - DETECTION_ZONE_LENGTH;
    int e_top = ROAD_TOP + 5;
    int e_right = STOP_LINE_EAST - 14;
    int e_bottom = CENTER_Y - 5;
    rectangle(e_left, e_top, e_right, e_bottom);

    // Westbound Detection Zone (Right)
    int w_left = STOP_LINE_WEST + 14;
    int w_top = CENTER_Y + 5;
    int w_right = STOP_LINE_WEST + DETECTION_ZONE_LENGTH;
    int w_bottom = ROAD_BOTTOM - 5;
    rectangle(w_left, w_top, w_right, w_bottom);
}

void HUD::drawTrafficLightBox(int x, int y, bool isRed, bool isYellow, bool isGreen) const {
    // Housing box
    setfillstyle(SOLID_FILL, COLOR(20, 24, 30));
    setcolor(COLOR(100, 110, 120));
    bar(x, y, x + 24, y + 64);
    rectangle(x, y, x + 24, y + 64);

    // Red light
    setfillstyle(SOLID_FILL, isRed ? COLOR(255, 40, 40) : COLOR(70, 15, 15));
    fillellipse(x + 12, y + 12, 7, 7);

    // Yellow light
    setfillstyle(SOLID_FILL, isYellow ? COLOR(255, 210, 20) : COLOR(70, 60, 10));
    fillellipse(x + 12, y + 32, 7, 7);

    // Green light
    setfillstyle(SOLID_FILL, isGreen ? COLOR(30, 240, 80) : COLOR(10, 60, 20));
    fillellipse(x + 12, y + 52, 7, 7);
}

void HUD::drawTrafficLights(const TrafficLightController& lightController) const {
    bool nsGreen  = lightController.isNorthSouthGreen();
    bool nsYellow = lightController.isNorthSouthYellow();
    bool nsRed    = !nsGreen && !nsYellow;

    bool ewGreen  = lightController.isEastWestGreen();
    bool ewYellow = lightController.isEastWestYellow();
    bool ewRed    = !ewGreen && !ewYellow;

    // South approach light (Top Left corner of intersection)
    drawTrafficLightBox(ROAD_LEFT - 32, ROAD_TOP - 70, nsRed, nsYellow, nsGreen);

    // North approach light (Bottom Right corner of intersection)
    drawTrafficLightBox(ROAD_RIGHT + 8, ROAD_BOTTOM + 6, nsRed, nsYellow, nsGreen);

    // East approach light (Bottom Left corner of intersection)
    drawTrafficLightBox(ROAD_LEFT - 32, ROAD_BOTTOM + 6, ewRed, ewYellow, ewGreen);

    // West approach light (Top Right corner of intersection)
    drawTrafficLightBox(ROAD_RIGHT + 8, ROAD_TOP - 70, ewRed, ewYellow, ewGreen);
}

void HUD::drawDeadlockOverlay(const IntersectionManager& manager) const {
    if (manager.isDeadlocked) {
        // Flashing Warning Border around the intersection box
        setcolor(COLOR(255, 40, 40));
        rectangle(ROAD_LEFT - 2, ROAD_TOP - 2, ROAD_RIGHT + 2, ROAD_BOTTOM + 2);
        rectangle(ROAD_LEFT - 4, ROAD_TOP - 4, ROAD_RIGHT + 4, ROAD_BOTTOM + 4);
        setcolor(COLOR(255, 220, 0));
        rectangle(ROAD_LEFT - 6, ROAD_TOP - 6, ROAD_RIGHT + 6, ROAD_BOTTOM + 6);

        // Center Gridlock Alert Label
        setfillstyle(SOLID_FILL, COLOR(200, 20, 20));
        bar(CENTER_X - 60, CENTER_Y - 12, CENTER_X + 60, CENTER_Y + 12);
        setcolor(WHITE);
        rectangle(CENTER_X - 60, CENTER_Y - 12, CENTER_X + 60, CENTER_Y + 12);
        setbkcolor(COLOR(200, 20, 20));
        outtextxy(CENTER_X - 52, CENTER_Y - 6, (char*)"GRIDLOCK STALL");
        setbkcolor(COLOR(15, 22, 32));
    }
}

void HUD::drawDashboard(const IntersectionManager& manager, float fps, bool isPaused) const {
    int panelLeft   = HUD_X_START;
    int panelTop    = 10;
    int panelRight  = SCREEN_WIDTH - 10;
    int panelBottom = SCREEN_HEIGHT - 10;

    // Panel Outer Background
    setfillstyle(SOLID_FILL, COLOR(15, 22, 32));
    bar(panelLeft, panelTop, panelRight, panelBottom);

    // Border Accent
    setcolor(manager.isDeadlocked ? COLOR(240, 60, 60) : COLOR(50, 140, 220));
    rectangle(panelLeft, panelTop, panelRight, panelBottom);
    rectangle(panelLeft + 2, panelTop + 2, panelRight - 2, panelBottom - 2);

    setbkcolor(COLOR(15, 22, 32));
    char buf[128];

    // Header Title
    setcolor(COLOR(240, 245, 255));
    outtextxy(panelLeft + 15, panelTop + 12, (char*)"SMART TRAFFIC CONTROL");

    setcolor(COLOR(120, 160, 200));
    outtextxy(panelLeft + 15, panelTop + 28, (char*)"Adaptive 2D Density Simulation");

    setcolor(COLOR(60, 80, 100));
    line(panelLeft + 10, panelTop + 46, panelRight - 10, panelTop + 46);

    // 1. SYSTEM CONTROL MODE & DEADLOCK STATUS BADGES
    int modeY = panelTop + 54;
    if (manager.lightController.mode == MODE_ADAPTIVE) {
        setfillstyle(SOLID_FILL, COLOR(20, 100, 50));
        bar(panelLeft + 15, modeY, panelRight - 15, modeY + 24);
        setcolor(COLOR(100, 255, 150));
        outtextxy(panelLeft + 30, modeY + 4, (char*)"[ MODE: ADAPTIVE SMART ]");
    } else {
        setfillstyle(SOLID_FILL, COLOR(120, 70, 10));
        bar(panelLeft + 15, modeY, panelRight - 15, modeY + 24);
        setcolor(COLOR(255, 200, 100));
        outtextxy(panelLeft + 35, modeY + 4, (char*)"[ MODE: FIXED TIMER ]");
    }

    // Deadlock Status Banner
    int deadY = modeY + 30;
    if (manager.isDeadlocked) {
        setfillstyle(SOLID_FILL, COLOR(180, 25, 25));
        bar(panelLeft + 15, deadY, panelRight - 15, deadY + 26);
        setcolor(COLOR(255, 255, 100));
        outtextxy(panelLeft + 22, deadY + 5, (char*)"[ ! DEADLOCK DETECTED ! ]");
    } else if (manager.resolvingDeadlock) {
        setfillstyle(SOLID_FILL, COLOR(25, 110, 160));
        bar(panelLeft + 15, deadY, panelRight - 15, deadY + 26);
        setcolor(COLOR(160, 240, 255));
        outtextxy(panelLeft + 20, deadY + 5, (char*)"[ RESOLVING: PRIORITY WAVE ]");
    } else if (manager.starvationDetected) {
        setfillstyle(SOLID_FILL, COLOR(150, 90, 15));
        bar(panelLeft + 15, deadY, panelRight - 15, deadY + 26);
        setcolor(COLOR(255, 220, 120));
        outtextxy(panelLeft + 20, deadY + 5, (char*)"[ WARNING: STARVATION WAIT ]");
    } else {
        setfillstyle(SOLID_FILL, COLOR(25, 45, 70));
        bar(panelLeft + 15, deadY, panelRight - 15, deadY + 26);
        setcolor(COLOR(120, 200, 255));
        outtextxy(panelLeft + 32, deadY + 5, (char*)"[ STATUS: TRAFFIC FLOWING ]");
    }

    // 2. SIGNAL PHASE & COUNTDOWN TIMER
    int phaseY = deadY + 34;
    setcolor(COLOR(200, 210, 225));
    outtextxy(panelLeft + 15, phaseY, (char*)"ACTIVE PHASE:");

    const char* phaseStr = "";
    int phaseColor = WHITE;
    if (manager.lightController.isNorthSouthGreen())  { phaseStr = "NORTH-SOUTH GREEN"; phaseColor = COLOR(50, 240, 100); }
    if (manager.lightController.isNorthSouthYellow()) { phaseStr = "NORTH-SOUTH YELLOW"; phaseColor = YELLOW; }
    if (manager.lightController.isEastWestGreen())    { phaseStr = "EAST-WEST GREEN"; phaseColor = COLOR(50, 240, 100); }
    if (manager.lightController.isEastWestYellow())   { phaseStr = "EAST-WEST YELLOW"; phaseColor = YELLOW; }

    setcolor(phaseColor);
    outtextxy(panelLeft + 15, phaseY + 16, (char*)phaseStr);

    // Countdown bar
    float rem = manager.lightController.timeRemaining;
    float dur = manager.lightController.phaseDuration;
    float pct = std::max(0.0f, std::min(1.0f, rem / (dur > 0.0f ? dur : 1.0f)));

    int barX = panelLeft + 15;
    int barY = phaseY + 34;
    int barW = panelRight - panelLeft - 30;
    int barH = 12;

    setfillstyle(SOLID_FILL, COLOR(30, 40, 55));
    bar(barX, barY, barX + barW, barY + barH);

    setfillstyle(SOLID_FILL, phaseColor);
    bar(barX, barY, barX + static_cast<int>(barW * pct), barY + barH);

    setcolor(COLOR(240, 240, 240));
    std::snprintf(buf, sizeof(buf), "%.1f sec remaining", rem);
    outtextxy(panelLeft + 15, barY + 15, buf);

    setcolor(COLOR(60, 80, 100));
    line(panelLeft + 10, barY + 34, panelRight - 10, barY + 34);

    // 3. REAL-TIME LANE DETECTION COUNTS
    int countY = barY + 42;
    setcolor(COLOR(200, 210, 225));
    outtextxy(panelLeft + 15, countY, (char*)"LANE DETECTION COUNTS:");

    struct ApproachInfo {
        const char* name;
        int count;
    } approaches[4] = {
        { "Southbound (Top):", manager.countSouth },
        { "Northbound (Bot):", manager.countNorth },
        { "Eastbound  (Left):", manager.countEast },
        { "Westbound (Right):", manager.countWest }
    };

    int appY = countY + 18;
    for (int i = 0; i < 4; ++i) {
        setcolor(COLOR(170, 185, 205));
        std::snprintf(buf, sizeof(buf), "%-18s %d cars", approaches[i].name, approaches[i].count);
        outtextxy(panelLeft + 15, appY, buf);

        int dBarX = panelLeft + 185;
        int dBarW = 65;
        int dBarH = 9;
        int filledW = std::min(dBarW, approaches[i].count * 13);

        setfillstyle(SOLID_FILL, COLOR(30, 40, 50));
        bar(dBarX, appY + 3, dBarX + dBarW, appY + 3 + dBarH);

        int densityColor = (approaches[i].count >= 4) ? RED : ((approaches[i].count >= 2) ? YELLOW : GREEN);
        setfillstyle(SOLID_FILL, densityColor);
        bar(dBarX, appY + 3, dBarX + filledW, appY + 3 + dBarH);

        appY += 19;
    }

    setcolor(COLOR(60, 80, 100));
    line(panelLeft + 10, appY + 4, panelRight - 10, appY + 4);

    // 4. PERFORMANCE & DEADLOCK METRICS
    int metricY = appY + 12;
    setcolor(COLOR(200, 210, 225));
    outtextxy(panelLeft + 15, metricY, (char*)"SYSTEM & DEADLOCK METRICS:");

    std::snprintf(buf, sizeof(buf), "Cleared: %d | Wait: %.1fs", manager.totalClearedVehicles, manager.getAverageWaitTime());
    setcolor(COLOR(170, 185, 205));
    outtextxy(panelLeft + 15, metricY + 17, buf);

    std::snprintf(buf, sizeof(buf), "Deadlocks: %d Det / %d Res", manager.deadlocksDetectedCount, manager.deadlocksResolvedCount);
    setcolor(manager.isDeadlocked ? COLOR(255, 100, 100) : COLOR(170, 185, 205));
    outtextxy(panelLeft + 15, metricY + 33, buf);

    std::snprintf(buf, sizeof(buf), "Box Occupants: %d  |  FPS: %.0f", manager.boxOccupantCount, fps);
    setcolor(COLOR(170, 185, 205));
    outtextxy(panelLeft + 15, metricY + 49, buf);

    // Anti-Deadlock Guard Indicator
    std::snprintf(buf, sizeof(buf), "Anti-Deadlock Guard: %s", manager.antiDeadlockGuard ? "[ ACTIVE ]" : "[ DISABLED ]");
    setcolor(manager.antiDeadlockGuard ? COLOR(100, 240, 150) : COLOR(255, 180, 50));
    outtextxy(panelLeft + 15, metricY + 65, buf);

    setcolor(COLOR(60, 80, 100));
    line(panelLeft + 10, metricY + 83, panelRight - 10, metricY + 83);

    // 5. USER CONTROLS LEGEND
    int ctrlY = metricY + 91;
    setcolor(COLOR(240, 200, 80));
    outtextxy(panelLeft + 15, ctrlY, (char*)"INTERACTIVE CONTROLS:");

    setcolor(COLOR(140, 160, 185));
    outtextxy(panelLeft + 15, ctrlY + 16, (char*)"[N/S/E/W] Spawn Mixed Vehicle");
    outtextxy(panelLeft + 15, ctrlY + 30, (char*)"[D]       Force Deadlock Demo");
    outtextxy(panelLeft + 15, ctrlY + 44, (char*)"[R]       Resolve Deadlock");
    outtextxy(panelLeft + 15, ctrlY + 58, (char*)"[X]       Toggle Anti-Deadlock");
    outtextxy(panelLeft + 15, ctrlY + 72, (char*)"[M]       Toggle Mode (Adap/Fix)");
    outtextxy(panelLeft + 15, ctrlY + 86, (char*)"[B]       Burst Spike | [C] Clear");
    outtextxy(panelLeft + 15, ctrlY + 100, (char*)"[P]       Pause / Resume");

    if (isPaused) {
        setfillstyle(SOLID_FILL, RED);
        bar(panelLeft + 15, panelBottom - 30, panelRight - 15, panelBottom - 8);
        setcolor(WHITE);
        outtextxy(panelLeft + 70, panelBottom - 24, (char*)"*** SIMULATION PAUSED ***");
    }
}
