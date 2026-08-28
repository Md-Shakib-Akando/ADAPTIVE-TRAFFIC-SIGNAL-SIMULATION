#include "HUD.h"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cmath>

HUD::HUD() {}

void HUD::drawTree(int x, int y, int radius) const {
    // Tree Trunk
    setfillstyle(SOLID_FILL, COLOR(85, 55, 30));
    bar(x - 3, y + radius - 4, x + 3, y + radius + 10);

    // Foliage outer shadow
    setfillstyle(SOLID_FILL, COLOR(22, 60, 28));
    fillellipse(x, y + 2, radius, radius);

    // Foliage main canopy
    setfillstyle(SOLID_FILL, COLOR(34, 110, 48));
    fillellipse(x, y, radius - 2, radius - 2);

    // Foliage highlight
    setfillstyle(SOLID_FILL, COLOR(55, 145, 65));
    fillellipse(x - radius / 3, y - radius / 3, radius / 2, radius / 2);
}

void HUD::drawStreetLight(int x, int y, bool onLeft) const {
    // Base & Pole
    setcolor(COLOR(70, 75, 85));
    setfillstyle(SOLID_FILL, COLOR(50, 55, 65));
    fillellipse(x, y, 4, 4);

    // Lamp Head
    int headX = onLeft ? x + 10 : x - 10;
    line(x, y, headX, y - 6);
    setfillstyle(SOLID_FILL, COLOR(250, 230, 140));
    fillellipse(headX, y - 6, 3, 3);
}

void HUD::drawDirectionArrow(int x, int y, Direction dir) const {
    setcolor(WHITE);
    setfillstyle(SOLID_FILL, WHITE);

    // Draw painted white lane arrows on asphalt
    switch (dir) {
        case DIR_SOUTH: { // Arrow pointing DOWN (Top approach)
            line(x, y - 14, x, y + 10);
            line(x - 1, y - 14, x - 1, y + 10);
            line(x + 1, y - 14, x + 1, y + 10);
            int pts[6] = { x, y + 16, x - 6, y + 6, x + 6, y + 6 };
            fillpoly(3, pts);
            break;
        }
        case DIR_NORTH: { // Arrow pointing UP (Bottom approach)
            line(x, y + 14, x, y - 10);
            line(x - 1, y + 14, x - 1, y - 10);
            line(x + 1, y + 14, x + 1, y - 10);
            int pts[6] = { x, y - 16, x - 6, y - 6, x + 6, y - 6 };
            fillpoly(3, pts);
            break;
        }
        case DIR_EAST: { // Arrow pointing RIGHT (Left approach)
            line(x - 14, y, x + 10, y);
            line(x - 14, y - 1, x + 10, y - 1);
            line(x - 14, y + 1, x + 10, y + 1);
            int pts[6] = { x + 16, y, x + 6, y - 6, x + 6, y + 6 };
            fillpoly(3, pts);
            break;
        }
        case DIR_WEST: { // Arrow pointing LEFT (Right approach)
            line(x + 14, y, x - 10, y);
            line(x + 14, y - 1, x - 10, y - 1);
            line(x + 14, y + 1, x - 10, y + 1);
            int pts[6] = { x - 16, y, x - 6, y - 6, x - 6, y + 6 };
            fillpoly(3, pts);
            break;
        }
    }
}

void HUD::drawScenery() const {
    // ================= 1. GRASS & SCENERY BACKGROUND =================
    setfillstyle(SOLID_FILL, COLOR(32, 75, 40));
    bar(0, 0, HUD_X_START - 5, SCREEN_HEIGHT);

    // Subtle lawn texture lines
    setcolor(COLOR(28, 68, 35));
    for (int gy = 20; gy < SCREEN_HEIGHT; gy += 45) {
        line(0, gy, HUD_X_START - 5, gy);
    }
    for (int gx = 20; gx < HUD_X_START - 5; gx += 45) {
        line(gx, 0, gx, SCREEN_HEIGHT);
    }

    // Landscaped Trees positioned away from traffic signal corners
    // Top-Left Quadrant
    drawTree(55, 55, 18);
    drawTree(145, 65, 20);
    drawTree(60, 160, 18);

    // Bottom-Left Quadrant
    drawTree(55, 660, 20);
    drawTree(145, 670, 18);
    drawTree(55, 570, 18);

    // Top-Right Quadrant
    drawTree(590, 60, 20);
    drawTree(650, 70, 18);
    drawTree(640, 160, 18);

    // Bottom-Right Quadrant
    drawTree(590, 670, 20);
    drawTree(650, 660, 18);
    drawTree(640, 570, 18);

    // ================= 2. CONCRETE SIDEWALKS & CURBS =================
    const int SW_WIDTH = 20;
    setfillstyle(SOLID_FILL, COLOR(105, 112, 122)); // Concrete sidewalk gray

    // Vertical sidewalks
    bar(ROAD_LEFT - SW_WIDTH, 0, ROAD_LEFT, SCREEN_HEIGHT);
    bar(ROAD_RIGHT, 0, ROAD_RIGHT + SW_WIDTH, SCREEN_HEIGHT);

    // Horizontal sidewalks
    bar(0, ROAD_TOP - SW_WIDTH, HUD_X_START - 5, ROAD_TOP);
    bar(0, ROAD_BOTTOM, HUD_X_START - 5, ROAD_BOTTOM + SW_WIDTH);

    // Curb edge borders
    setcolor(COLOR(175, 180, 190));
    line(ROAD_LEFT - SW_WIDTH, 0, ROAD_LEFT - SW_WIDTH, SCREEN_HEIGHT);
    line(ROAD_RIGHT + SW_WIDTH, 0, ROAD_RIGHT + SW_WIDTH, SCREEN_HEIGHT);
    line(0, ROAD_TOP - SW_WIDTH, HUD_X_START - 5, ROAD_TOP - SW_WIDTH);
    line(0, ROAD_BOTTOM + SW_WIDTH, HUD_X_START - 5, ROAD_BOTTOM + SW_WIDTH);

    // Sidewalk tiles
    setcolor(COLOR(85, 92, 100));
    for (int y = 15; y < SCREEN_HEIGHT; y += 30) {
        if (y < ROAD_TOP - SW_WIDTH || y > ROAD_BOTTOM + SW_WIDTH) {
            line(ROAD_LEFT - SW_WIDTH, y, ROAD_LEFT, y);
            line(ROAD_RIGHT, y, ROAD_RIGHT + SW_WIDTH, y);
        }
    }
    for (int x = 15; x < HUD_X_START - 5; x += 30) {
        if (x < ROAD_LEFT - SW_WIDTH || x > ROAD_RIGHT + SW_WIDTH) {
            line(x, ROAD_TOP - SW_WIDTH, x, ROAD_TOP);
            line(x, ROAD_BOTTOM, x, ROAD_BOTTOM + SW_WIDTH);
        }
    }

    // Street Lamps on sidewalk corners
    drawStreetLight(ROAD_LEFT - 8, ROAD_TOP - 8, true);
    drawStreetLight(ROAD_RIGHT + 8, ROAD_TOP - 8, false);
    drawStreetLight(ROAD_LEFT - 8, ROAD_BOTTOM + 8, true);
    drawStreetLight(ROAD_RIGHT + 8, ROAD_BOTTOM + 8, false);

    // ================= 3. ASPHALT ROADWAY =================
    setfillstyle(SOLID_FILL, COLOR(38, 44, 52)); // Deep asphalt slate
    bar(ROAD_LEFT, 0, ROAD_RIGHT, SCREEN_HEIGHT);
    bar(0, ROAD_TOP, HUD_X_START - 5, ROAD_BOTTOM);
    bar(ROAD_LEFT, ROAD_TOP, ROAD_RIGHT, ROAD_BOTTOM); // Intersection center

    // Road Edge White Boundary Lines
    setcolor(COLOR(230, 235, 240));
    line(ROAD_LEFT, 0, ROAD_LEFT, ROAD_TOP);
    line(ROAD_LEFT, ROAD_BOTTOM, ROAD_LEFT, SCREEN_HEIGHT);
    line(ROAD_RIGHT, 0, ROAD_RIGHT, ROAD_TOP);
    line(ROAD_RIGHT, ROAD_BOTTOM, ROAD_RIGHT, SCREEN_HEIGHT);

    line(0, ROAD_TOP, ROAD_LEFT, ROAD_TOP);
    line(ROAD_RIGHT, ROAD_TOP, HUD_X_START - 5, ROAD_TOP);
    line(0, ROAD_BOTTOM, ROAD_LEFT, ROAD_BOTTOM);
    line(ROAD_RIGHT, ROAD_BOTTOM, HUD_X_START - 5, ROAD_BOTTOM);

    // ================= 4. REAL-WORLD OPEN INTERSECTION CROSSROAD =================
    // Clean seamless dark asphalt junction (authentic real-world crossroad)
    setfillstyle(SOLID_FILL, COLOR(38, 44, 52));
    bar(ROAD_LEFT, ROAD_TOP, ROAD_RIGHT, ROAD_BOTTOM);

    // ================= 5. DOUBLE YELLOW CENTERLINES =================
    setcolor(COLOR(255, 205, 10));
    // Vertical Centerlines (Terminate cleanly before crosswalk)
    line(CENTER_X - 2, 0, CENTER_X - 2, STOP_LINE_SOUTH - 18);
    line(CENTER_X + 2, 0, CENTER_X + 2, STOP_LINE_SOUTH - 18);
    line(CENTER_X - 2, STOP_LINE_NORTH + 18, CENTER_X - 2, SCREEN_HEIGHT);
    line(CENTER_X + 2, STOP_LINE_NORTH + 18, CENTER_X + 2, SCREEN_HEIGHT);

    // Horizontal Centerlines (Terminate cleanly before crosswalk)
    line(0, CENTER_Y - 2, STOP_LINE_EAST - 18, CENTER_Y - 2);
    line(0, CENTER_Y + 2, STOP_LINE_EAST - 18, CENTER_Y + 2);
    line(STOP_LINE_WEST + 18, CENTER_Y - 2, HUD_X_START - 5, CENTER_Y - 2);
    line(STOP_LINE_WEST + 18, CENTER_Y + 2, HUD_X_START - 5, CENTER_Y + 2);

    // ================= 6. ZEBRA PEDESTRIAN CROSSINGS =================
    setfillstyle(SOLID_FILL, COLOR(245, 245, 245));
    // South Approach Zebra (Top)
    for (int x = ROAD_LEFT + 6; x < ROAD_RIGHT - 6; x += 18) {
        bar(x, STOP_LINE_SOUTH - 14, x + 10, STOP_LINE_SOUTH - 2);
    }
    // North Approach Zebra (Bottom)
    for (int x = ROAD_LEFT + 6; x < ROAD_RIGHT - 6; x += 18) {
        bar(x, STOP_LINE_NORTH + 2, x + 10, STOP_LINE_NORTH + 14);
    }
    // East Approach Zebra (Left)
    for (int y = ROAD_TOP + 6; y < ROAD_BOTTOM - 6; y += 18) {
        bar(STOP_LINE_EAST - 14, y, STOP_LINE_EAST - 2, y + 10);
    }
    // West Approach Zebra (Right)
    for (int y = ROAD_TOP + 6; y < ROAD_BOTTOM - 6; y += 18) {
        bar(STOP_LINE_WEST + 2, y, STOP_LINE_WEST + 14, y + 10);
    }

    // ================= 7. SOLID STOP LINES =================
    setcolor(WHITE);
    // South Stop Line
    for (int t = 0; t < 3; ++t) line(ROAD_LEFT, STOP_LINE_SOUTH - 15 - t, CENTER_X, STOP_LINE_SOUTH - 15 - t);
    // North Stop Line
    for (int t = 0; t < 3; ++t) line(CENTER_X, STOP_LINE_NORTH + 15 + t, ROAD_RIGHT, STOP_LINE_NORTH + 15 + t);
    // East Stop Line (Left road, Bottom lane)
    for (int t = 0; t < 3; ++t) line(STOP_LINE_EAST - 15 - t, CENTER_Y, STOP_LINE_EAST - 15 - t, ROAD_BOTTOM);
    // West Stop Line (Right road, Top lane)
    for (int t = 0; t < 3; ++t) line(STOP_LINE_WEST + 15 + t, ROAD_TOP, STOP_LINE_WEST + 15 + t, CENTER_Y);

    // ================= 8. LANE DIRECTIONAL ARROWS ON ALL ROADS =================
    // 1. Approach Lanes (Placed safely behind the induction sensor box)
    drawDirectionArrow(LANE_SOUTH_X, 55, DIR_SOUTH);  // Top approach (Behind sensor)
    drawDirectionArrow(LANE_NORTH_X, 695, DIR_NORTH); // Bottom approach (Behind sensor)
    drawDirectionArrow(45, LANE_EAST_Y, DIR_EAST);    // Left approach (Behind sensor)
    drawDirectionArrow(665, LANE_WEST_Y, DIR_WEST);   // Right approach (Behind sensor)

    // 2. Departure / Exit Lanes (Indicating outgoing traffic flow)
    drawDirectionArrow(LANE_NORTH_X, 130, DIR_NORTH); // Top exit lane
    drawDirectionArrow(LANE_SOUTH_X, 620, DIR_SOUTH); // Bottom exit lane
    drawDirectionArrow(130, LANE_WEST_Y, DIR_WEST);   // Left exit lane
    drawDirectionArrow(580, LANE_EAST_Y, DIR_EAST);   // Right exit lane

    // ================= 9. ROADWAY ORIENTATION LABELS =================
    setbkcolor(COLOR(32, 75, 40));
    setcolor(COLOR(220, 240, 255));
    outtextxy(ROAD_LEFT - 140, 15, (char*)"[TOP APPROACH]");
    outtextxy(ROAD_LEFT - 140, 30, (char*)"SOUTHBOUND v");

    outtextxy(ROAD_RIGHT + 30, SCREEN_HEIGHT - 40, (char*)"[BOT APPROACH]");
    outtextxy(ROAD_RIGHT + 30, SCREEN_HEIGHT - 25, (char*)"NORTHBOUND ^");

    outtextxy(15, ROAD_BOTTOM + 25, (char*)"[LEFT APPROACH] EASTBOUND >");
    outtextxy(HUD_X_START - 215, ROAD_TOP - 40, (char*)"[RIGHT APPROACH] WESTBOUND <");
}

void HUD::drawDetectionZones(const IntersectionManager& manager) const {
    // ================= REALISTIC ROAD INDUCTION SENSOR LOOPS =================
    // Realistic saw-cut inductive loops embedded into the asphalt with chamfered corners
    
    char sTag[64];

    // Helper lambda to draw a realistic pavement-embedded induction loop
    auto drawLoop = [](int x1, int y1, int x2, int y2, int count) {
        int loopColor = (count > 0) ? COLOR(0, 225, 255) : COLOR(58, 68, 80);
        setcolor(loopColor);

        const int c = 8; // 45-degree chamfer cut at corners

        // Outer chamfered saw-cut rectangle
        line(x1 + c, y1, x2 - c, y1);
        line(x2, y1 + c, x2, y2 - c);
        line(x2 - c, y2, x1 + c, y2);
        line(x1, y2 - c, x1, y1 + c);

        // Corner diagonal cuts
        line(x1, y1 + c, x1 + c, y1);
        line(x2 - c, y1, x2, y1 + c);
        line(x2, y2 - c, x2 - c, y2);
        line(x1 + c, y2, x1, y2 - c);

        // Inner secondary coil wire
        const int m = 4;
        line(x1 + c + m, y1 + m, x2 - c - m, y1 + m);
        line(x2 - m, y1 + c + m, x2 - m, y2 - c - m);
        line(x2 - c - m, y2 - m, x1 + c + m, y2 - m);
        line(x1 + m, y2 - c - m, x1 + m, y1 + c + m);
    };

    // 1. Southbound Approach Sensor (Top Road, Left Lane)
    int s_x1 = ROAD_LEFT + 6, s_y1 = STOP_LINE_SOUTH - DETECTION_ZONE_LENGTH;
    int s_x2 = CENTER_X - 6,  s_y2 = STOP_LINE_SOUTH - 22;
    drawLoop(s_x1, s_y1, s_x2, s_y2, manager.countSouth);

    // 2. Northbound Approach Sensor (Bottom Road, Right Lane)
    int n_x1 = CENTER_X + 6,  n_y1 = STOP_LINE_NORTH + 22;
    int n_x2 = ROAD_RIGHT - 6, n_y2 = STOP_LINE_NORTH + DETECTION_ZONE_LENGTH;
    drawLoop(n_x1, n_y1, n_x2, n_y2, manager.countNorth);

    // 3. Eastbound Approach Sensor (Left Road, Bottom Lane)
    int e_x1 = STOP_LINE_EAST - DETECTION_ZONE_LENGTH, e_y1 = CENTER_Y + 6;
    int e_x2 = STOP_LINE_EAST - 22,                   e_y2 = ROAD_BOTTOM - 6;
    drawLoop(e_x1, e_y1, e_x2, e_y2, manager.countEast);

    // 4. Westbound Approach Sensor (Right Road, Top Lane)
    int w_x1 = STOP_LINE_WEST + 22,                   w_y1 = ROAD_TOP + 6;
    int w_x2 = STOP_LINE_WEST + DETECTION_ZONE_LENGTH, w_y2 = CENTER_Y - 6;
    drawLoop(w_x1, w_y1, w_x2, w_y2, manager.countWest);

    // Pavement-edge smart sensor status tags
    setbkcolor(COLOR(22, 34, 48));

    // South Tag
    setcolor(manager.countSouth > 0 ? COLOR(0, 240, 255) : COLOR(140, 165, 190));
    std::snprintf(sTag, sizeof(sTag), "SENSOR: %d", manager.countSouth);
    outtextxy(s_x1 + 4, s_y1 + 4, sTag);

    // North Tag
    setcolor(manager.countNorth > 0 ? COLOR(0, 240, 255) : COLOR(140, 165, 190));
    std::snprintf(sTag, sizeof(sTag), "SENSOR: %d", manager.countNorth);
    outtextxy(n_x1 + 4, n_y2 - 16, sTag);

    // East Tag
    setcolor(manager.countEast > 0 ? COLOR(0, 240, 255) : COLOR(140, 165, 190));
    std::snprintf(sTag, sizeof(sTag), "SENSOR: %d", manager.countEast);
    outtextxy(e_x1 + 4, e_y1 + 4, sTag);

    // West Tag
    setcolor(manager.countWest > 0 ? COLOR(0, 240, 255) : COLOR(140, 165, 190));
    std::snprintf(sTag, sizeof(sTag), "SENSOR: %d", manager.countWest);
    outtextxy(w_x2 - 76, w_y1 + 4, sTag);
}

void HUD::drawTrafficLightPost(int poleX, int poleY, int boxX, int boxY,
                              bool isRed, bool isYellow, bool isGreen,
                              const char* directionLabel, float timeRemaining, bool isHorizontal) const {
    // 1. Steel Support Pole & Cantilever Arm
    setcolor(COLOR(100, 108, 120));
    setfillstyle(SOLID_FILL, COLOR(70, 76, 85));
    fillellipse(poleX, poleY, 5, 5); // Pole Base

    int armTargetX = isHorizontal ? (boxX + 34) : (boxX + 13);
    int armTargetY = isHorizontal ? (boxY + 13) : (boxY + 34);
    line(poleX, poleY, armTargetX, armTargetY); // Arm

    // 2. High-Visibility Signal Housing Box
    const int boxW = isHorizontal ? 68 : 26;
    const int boxH = isHorizontal ? 26 : 68;

    // Outer Yellow Backplate / Border
    setfillstyle(SOLID_FILL, COLOR(255, 204, 0));
    bar(boxX - 3, boxY - 3, boxX + boxW + 3, boxY + boxH + 3);

    // Inner Dark Housing
    setfillstyle(SOLID_FILL, COLOR(18, 22, 28));
    bar(boxX, boxY, boxX + boxW, boxY + boxH);
    setcolor(COLOR(120, 130, 145));
    rectangle(boxX, boxY, boxX + boxW, boxY + boxH);

    // 3. Lamps (Red, Yellow, Green)
    int rX, rY, yX, yY, gX, gY;
    if (isHorizontal) {
        rX = boxX + 13; rY = boxY + 13;
        yX = boxX + 34; yY = boxY + 13;
        gX = boxX + 55; gY = boxY + 13;
    } else {
        rX = boxX + 13; rY = boxY + 13;
        yX = boxX + 13; yY = boxY + 34;
        gX = boxX + 13; gY = boxY + 55;
    }

    const int LAMP_RAD = 7;

    // --- RED LAMP ---
    if (isRed) {
        setfillstyle(SOLID_FILL, COLOR(120, 20, 20));
        fillellipse(rX, rY, LAMP_RAD + 2, LAMP_RAD + 2);
        setfillstyle(SOLID_FILL, COLOR(255, 30, 30));
        fillellipse(rX, rY, LAMP_RAD, LAMP_RAD);
        setfillstyle(SOLID_FILL, COLOR(255, 170, 170));
        fillellipse(rX - 2, rY - 2, 2, 2);
    } else {
        setfillstyle(SOLID_FILL, COLOR(65, 15, 15));
        fillellipse(rX, rY, LAMP_RAD, LAMP_RAD);
    }

    // --- YELLOW LAMP ---
    if (isYellow) {
        setfillstyle(SOLID_FILL, COLOR(130, 110, 15));
        fillellipse(yX, yY, LAMP_RAD + 2, LAMP_RAD + 2);
        setfillstyle(SOLID_FILL, COLOR(255, 215, 10));
        fillellipse(yX, yY, LAMP_RAD, LAMP_RAD);
        setfillstyle(SOLID_FILL, COLOR(255, 245, 170));
        fillellipse(yX - 2, yY - 2, 2, 2);
    } else {
        setfillstyle(SOLID_FILL, COLOR(65, 55, 10));
        fillellipse(yX, yY, LAMP_RAD, LAMP_RAD);
    }

    // --- GREEN LAMP ---
    if (isGreen) {
        setfillstyle(SOLID_FILL, COLOR(15, 120, 45));
        fillellipse(gX, gY, LAMP_RAD + 2, LAMP_RAD + 2);
        setfillstyle(SOLID_FILL, COLOR(30, 245, 90));
        fillellipse(gX, gY, LAMP_RAD, LAMP_RAD);
        setfillstyle(SOLID_FILL, COLOR(180, 255, 205));
        fillellipse(gX - 2, gY - 2, 2, 2);
    } else {
        setfillstyle(SOLID_FILL, COLOR(10, 60, 25));
        fillellipse(gX, gY, LAMP_RAD, LAMP_RAD);
    }

    // 4. Direction & Action Badge (Placed right below the housing)
    int badgeW = 86;
    int badgeH = 22;
    int badgeX = boxX + (boxW / 2) - (badgeW / 2);
    int badgeY = boxY + boxH + 4;
    if (badgeX < 5) badgeX = 5;

    int badgeBg = isRed ? COLOR(160, 20, 20) : (isYellow ? COLOR(160, 130, 10) : COLOR(20, 140, 50));
    setfillstyle(SOLID_FILL, badgeBg);
    bar(badgeX, badgeY, badgeX + badgeW, badgeY + badgeH);
    setcolor(WHITE);
    rectangle(badgeX, badgeY, badgeX + badgeW, badgeY + badgeH);

    setbkcolor(badgeBg);
    setcolor(WHITE);
    char bText[64];
    if (isRed) {
        std::snprintf(bText, sizeof(bText), "%s:STOP", directionLabel);
    } else if (isYellow) {
        std::snprintf(bText, sizeof(bText), "%s:WAIT", directionLabel);
    } else {
        std::snprintf(bText, sizeof(bText), "%s:GO %.0fs", directionLabel, std::max(0.0f, timeRemaining));
    }
    outtextxy(badgeX + 3, badgeY + 4, bText);
}

void HUD::drawTrafficLights(const TrafficLightController& lightController) const {
    bool nsGreen  = lightController.isNorthSouthGreen();
    bool nsYellow = lightController.isNorthSouthYellow();
    bool nsRed    = !nsGreen && !nsYellow;

    bool ewGreen  = lightController.isEastWestGreen();
    bool ewYellow = lightController.isEastWestYellow();
    bool ewRed    = !ewGreen && !ewYellow;

    float remTime = lightController.timeRemaining;

    // =========================================================================
    // 4 SEPARATE CORNERS - SPECIFIC HORIZONTAL / VERTICAL ORIENTATIONS:
    // South: Horizontal | North: Horizontal | East: Vertical | West: Vertical
    // =========================================================================

    // 1. TOP-LEFT CORNER: SOUTHBOUND Signal (Horizontal)
    drawTrafficLightPost(ROAD_LEFT - 12, STOP_LINE_SOUTH - 20,
                         ROAD_LEFT - 85, STOP_LINE_SOUTH - 65,
                         nsRed, nsYellow, nsGreen, "SOUTH", remTime, true);

    // 2. BOTTOM-LEFT CORNER: EASTBOUND Signal (Vertical)
    drawTrafficLightPost(STOP_LINE_EAST - 20, ROAD_BOTTOM + 12,
                         STOP_LINE_EAST - 70, ROAD_BOTTOM + 15,
                         ewRed, ewYellow, ewGreen, "EAST", remTime, false);

    // 3. BOTTOM-RIGHT CORNER: NORTHBOUND Signal (Horizontal)
    drawTrafficLightPost(ROAD_RIGHT + 12, STOP_LINE_NORTH + 20,
                         ROAD_RIGHT + 18, STOP_LINE_NORTH + 25,
                         nsRed, nsYellow, nsGreen, "NORTH", remTime, true);

    // 4. TOP-RIGHT CORNER: WESTBOUND Signal (Vertical)
    drawTrafficLightPost(STOP_LINE_WEST + 20, ROAD_TOP - 12,
                         STOP_LINE_WEST + 25, ROAD_TOP - 100,
                         ewRed, ewYellow, ewGreen, "WEST", remTime, false);
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
        bar(CENTER_X - 65, CENTER_Y - 14, CENTER_X + 65, CENTER_Y + 14);
        setcolor(WHITE);
        rectangle(CENTER_X - 65, CENTER_Y - 14, CENTER_X + 65, CENTER_Y + 14);
        setbkcolor(COLOR(200, 20, 20));
        outtextxy(CENTER_X - 58, CENTER_Y - 6, (char*)"! GRIDLOCK DETECTED !");
        setbkcolor(COLOR(15, 22, 32));
    }
}

void HUD::drawDashboard(const IntersectionManager& manager, float fps, bool isPaused) const {
    int panelLeft   = HUD_X_START;
    int panelTop    = 10;
    int panelRight  = SCREEN_WIDTH - 10;
    int panelBottom = SCREEN_HEIGHT - 10;

    // Panel Outer Background (Dark modern glassmorphic card)
    setfillstyle(SOLID_FILL, COLOR(15, 22, 32));
    bar(panelLeft, panelTop, panelRight, panelBottom);

    // Border Accent
    int borderColor = manager.isDeadlocked ? COLOR(240, 60, 60) : COLOR(45, 130, 215);
    setcolor(borderColor);
    rectangle(panelLeft, panelTop, panelRight, panelBottom);
    rectangle(panelLeft + 2, panelTop + 2, panelRight - 2, panelBottom - 2);

    setbkcolor(COLOR(15, 22, 32));
    char buf[128];

    // ================= HEADER TITLE =================
    setcolor(COLOR(240, 245, 255));
    outtextxy(panelLeft + 14, panelTop + 14, (char*)"SMART TRAFFIC CONTROLLER");

    setcolor(COLOR(120, 165, 210));
    outtextxy(panelLeft + 14, panelTop + 32, (char*)"Adaptive Density & Deadlock AI");

    setcolor(COLOR(55, 75, 95));
    line(panelLeft + 10, panelTop + 54, panelRight - 10, panelTop + 54);

    // ================= 1. SYSTEM CONTROL MODE & DEADLOCK STATUS =================
    int modeY = panelTop + 62;
    if (manager.lightController.mode == MODE_ADAPTIVE) {
        setfillstyle(SOLID_FILL, COLOR(18, 100, 48));
        bar(panelLeft + 12, modeY, panelRight - 12, modeY + 26);
        setcolor(COLOR(110, 255, 160));
        outtextxy(panelLeft + 25, modeY + 5, (char*)"[ MODE: ADAPTIVE DENSITY AI ]");
    } else {
        setfillstyle(SOLID_FILL, COLOR(130, 75, 12));
        bar(panelLeft + 12, modeY, panelRight - 12, modeY + 26);
        setcolor(COLOR(255, 205, 110));
        outtextxy(panelLeft + 30, modeY + 5, (char*)"[ MODE: FIXED TIMER (8.0s) ]");
    }

    // Deadlock Status Banner
    int deadY = modeY + 34;
    if (manager.isDeadlocked) {
        setfillstyle(SOLID_FILL, COLOR(180, 25, 25));
        bar(panelLeft + 12, deadY, panelRight - 12, deadY + 26);
        setcolor(COLOR(255, 255, 100));
        outtextxy(panelLeft + 20, deadY + 5, (char*)"[ ! DEADLOCK DETECTED ! ]");
    } else if (manager.resolvingDeadlock) {
        setfillstyle(SOLID_FILL, COLOR(25, 110, 165));
        bar(panelLeft + 12, deadY, panelRight - 12, deadY + 26);
        setcolor(COLOR(160, 240, 255));
        outtextxy(panelLeft + 18, deadY + 5, (char*)"[ RESOLVING: PRIORITY WAVE ]");
    } else if (manager.starvationDetected) {
        setfillstyle(SOLID_FILL, COLOR(155, 95, 15));
        bar(panelLeft + 12, deadY, panelRight - 12, deadY + 26);
        setcolor(COLOR(255, 220, 120));
        outtextxy(panelLeft + 18, deadY + 5, (char*)"[ WARNING: STARVATION QUEUE ]");
    } else {
        setfillstyle(SOLID_FILL, COLOR(24, 48, 75));
        bar(panelLeft + 12, deadY, panelRight - 12, deadY + 26);
        setcolor(COLOR(120, 205, 255));
        outtextxy(panelLeft + 28, deadY + 5, (char*)"[ STATUS: TRAFFIC FLOWING ]");
    }

    setcolor(COLOR(55, 75, 95));
    line(panelLeft + 10, deadY + 36, panelRight - 10, deadY + 36);

    // ================= 2. ACTIVE PHASE & HIGH-VISIBILITY TIMER =================
    int phaseY = deadY + 44;
    setcolor(COLOR(200, 215, 230));
    outtextxy(panelLeft + 12, phaseY, (char*)"ACTIVE SIGNAL PHASE:");

    const char* phaseStr = "";
    int phaseColor = WHITE;
    if (manager.lightController.isNorthSouthGreen())  { phaseStr = "NORTH - SOUTH : [ GREEN ]"; phaseColor = COLOR(40, 245, 100); }
    if (manager.lightController.isNorthSouthYellow()) { phaseStr = "NORTH - SOUTH : [ YELLOW ]"; phaseColor = COLOR(255, 215, 0); }
    if (manager.lightController.isEastWestGreen())    { phaseStr = "EAST - WEST   : [ GREEN ]"; phaseColor = COLOR(40, 245, 100); }
    if (manager.lightController.isEastWestYellow())   { phaseStr = "EAST - WEST   : [ YELLOW ]"; phaseColor = COLOR(255, 215, 0); }

    setcolor(phaseColor);
    outtextxy(panelLeft + 12, phaseY + 18, (char*)phaseStr);

    // Animated Phase Countdown Bar
    float rem = manager.lightController.timeRemaining;
    float dur = manager.lightController.phaseDuration;
    float pct = std::max(0.0f, std::min(1.0f, rem / (dur > 0.0f ? dur : 1.0f)));

    int barX = panelLeft + 12;
    int barY = phaseY + 38;
    int barW = panelRight - panelLeft - 24;
    int barH = 15;

    // Background track
    setfillstyle(SOLID_FILL, COLOR(30, 42, 58));
    bar(barX, barY, barX + barW, barY + barH);
    setcolor(COLOR(65, 85, 110));
    rectangle(barX, barY, barX + barW, barY + barH);

    // Active fill
    setfillstyle(SOLID_FILL, phaseColor);
    bar(barX + 1, barY + 1, barX + static_cast<int>((barW - 2) * pct), barY + barH - 1);

    setcolor(COLOR(240, 240, 240));
    std::snprintf(buf, sizeof(buf), "%.1f sec remaining (Total %.1fs)", rem, dur);
    outtextxy(panelLeft + 12, barY + 18, buf);

    setcolor(COLOR(55, 75, 95));
    line(panelLeft + 10, barY + 40, panelRight - 10, barY + 40);

    // ================= 3. 4-WAY APPROACH REAL-TIME STATUS CARDS =================
    int countY = barY + 48;
    setcolor(COLOR(200, 215, 230));
    outtextxy(panelLeft + 12, countY, (char*)"ROAD APPROACH MONITOR:");

    struct ApproachRow {
        const char* name;
        int count;
        bool isGreen;
        bool isYellow;
    } approaches[4] = {
        { "South (Top):", manager.countSouth, manager.lightController.isNorthSouthGreen(), manager.lightController.isNorthSouthYellow() },
        { "North (Bot):", manager.countNorth, manager.lightController.isNorthSouthGreen(), manager.lightController.isNorthSouthYellow() },
        { "East  (Lft):", manager.countEast,  manager.lightController.isEastWestGreen(),   manager.lightController.isEastWestYellow() },
        { "West  (Rgt):", manager.countWest,  manager.lightController.isEastWestGreen(),   manager.lightController.isEastWestYellow() }
    };

    int rowY = countY + 22;
    for (int i = 0; i < 4; ++i) {
        // Signal pill
        int sigColor = approaches[i].isGreen ? COLOR(40, 240, 90) : (approaches[i].isYellow ? COLOR(255, 210, 10) : COLOR(255, 50, 50));
        const char* sigText = approaches[i].isGreen ? "GO " : (approaches[i].isYellow ? "YEL" : "RED");

        setfillstyle(SOLID_FILL, sigColor);
        bar(panelLeft + 12, rowY + 1, panelLeft + 44, rowY + 16);
        setcolor(BLACK);
        setbkcolor(sigColor);
        outtextxy(panelLeft + 15, rowY + 2, (char*)sigText);

        setbkcolor(COLOR(15, 22, 32));
        setcolor(COLOR(180, 195, 215));
        std::snprintf(buf, sizeof(buf), "%-12s %d cars", approaches[i].name, approaches[i].count);
        outtextxy(panelLeft + 50, rowY + 2, buf);

        // Density meter bar
        int dBarX = panelLeft + 195;
        int dBarW = 60;
        int dBarH = 12;
        int filledW = std::min(dBarW, approaches[i].count * 12);

        setfillstyle(SOLID_FILL, COLOR(28, 38, 50));
        bar(dBarX, rowY + 2, dBarX + dBarW, rowY + 2 + dBarH);

        int densityColor = (approaches[i].count >= 4) ? COLOR(255, 60, 60) : ((approaches[i].count >= 2) ? COLOR(255, 210, 20) : COLOR(40, 210, 100));
        setfillstyle(SOLID_FILL, densityColor);
        bar(dBarX, rowY + 2, dBarX + filledW, rowY + 2 + dBarH);

        rowY += 24;
    }

    setcolor(COLOR(55, 75, 95));
    line(panelLeft + 10, rowY + 6, panelRight - 10, rowY + 6);

    // ================= 4. PERFORMANCE & DEADLOCK METRICS =================
    int metricY = rowY + 14;
    setcolor(COLOR(200, 215, 230));
    outtextxy(panelLeft + 12, metricY, (char*)"SYSTEM PERFORMANCE & STATS:");

    std::snprintf(buf, sizeof(buf), "Cleared: %d | Avg Wait: %.1fs", manager.totalClearedVehicles, manager.getAverageWaitTime());
    setcolor(COLOR(175, 190, 210));
    outtextxy(panelLeft + 12, metricY + 20, buf);

    std::snprintf(buf, sizeof(buf), "Deadlocks: %d Det / %d Res", manager.deadlocksDetectedCount, manager.deadlocksResolvedCount);
    setcolor(manager.isDeadlocked ? COLOR(255, 100, 100) : COLOR(175, 190, 210));
    outtextxy(panelLeft + 12, metricY + 38, buf);

    std::snprintf(buf, sizeof(buf), "Box Occupants: %d  |  FPS: %.0f", manager.boxOccupantCount, fps);
    setcolor(COLOR(175, 190, 210));
    outtextxy(panelLeft + 12, metricY + 56, buf);

    // Anti-Deadlock Guard Indicator
    std::snprintf(buf, sizeof(buf), "Box Guard: %s", manager.antiDeadlockGuard ? "[ ACTIVE (ON) ]" : "[ DISABLED (OFF) ]");
    setcolor(manager.antiDeadlockGuard ? COLOR(100, 245, 155) : COLOR(255, 175, 50));
    outtextxy(panelLeft + 12, metricY + 74, buf);

    setcolor(COLOR(55, 75, 95));
    line(panelLeft + 10, metricY + 94, panelRight - 10, metricY + 94);

    // ================= 5. INTERACTIVE HOTKEYS LEGEND =================
    int ctrlY = metricY + 98;
    setcolor(COLOR(250, 210, 80));
    outtextxy(panelLeft + 12, ctrlY, (char*)"KEYBOARD CONTROLS:");

    setcolor(COLOR(145, 165, 190));
    outtextxy(panelLeft + 12, ctrlY + 20, (char*)"[N / S / E / W]  Spawn Vehicles");
    outtextxy(panelLeft + 12, ctrlY + 40, (char*)"[D]              Force Deadlock Demo");
    outtextxy(panelLeft + 12, ctrlY + 60, (char*)"[R]              Resolve Deadlock Now");
    outtextxy(panelLeft + 12, ctrlY + 80, (char*)"[X]              Toggle Anti-Deadlock");
    outtextxy(panelLeft + 12, ctrlY + 100, (char*)"[M]              Toggle Mode (Adap/Fix)");
    outtextxy(panelLeft + 12, ctrlY + 120, (char*)"[B]              Burst Spike Traffic");
    outtextxy(panelLeft + 12, ctrlY + 140, (char*)"[C]              Clear All Vehicles");
    outtextxy(panelLeft + 12, ctrlY + 160, (char*)"[P]              Pause / Resume Sim");

    if (isPaused) {
        setfillstyle(SOLID_FILL, COLOR(210, 30, 30));
        bar(panelLeft + 12, panelBottom - 30, panelRight - 12, panelBottom - 8);
        setcolor(WHITE);
        setbkcolor(COLOR(210, 30, 30));
        outtextxy(panelLeft + 60, panelBottom - 23, (char*)"*** SIMULATION PAUSED ***");
    }
}
