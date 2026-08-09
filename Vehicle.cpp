#include "Vehicle.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>

Vehicle::Vehicle(int vehicleId, Direction direction, double currentTime)
    : id(vehicleId), dir(direction), spawnTime(currentTime), totalWaitTime(0.0), boxWaitTime(0.0f),
      isStopped(false), hasPassedStopLine(false), hasCleared(false), inDetectionZone(false), inDeadlock(false) {

    // 1. High-Contrast Vibrant Color Palette (No dark asphalt blends)
    int vibrantColors[] = {
        COLOR(235, 50, 50),   // Vivid Crimson Red
        COLOR(30, 130, 240),  // Bright Royal Blue
        COLOR(250, 180, 20),  // Bright Amber Gold
        COLOR(35, 195, 100),  // Vibrant Emerald Green
        COLOR(175, 65, 230),  // Electric Purple
        COLOR(245, 245, 250), // Pure Pearl White
        COLOR(255, 115, 30),  // Sunset Orange
        COLOR(20, 210, 235),  // Vivid Cyan / Sky Blue
        COLOR(205, 215, 225)  // Platinum Silver
    };
    int numColors = 9;

    // 2. Weighted Vehicle Type Distribution:
    // 35% Sedan, 15% SUV, 15% Bus, 15% Bike, 10% Truck, 5% Taxi, 5% Ambulance
    int r = rand() % 100;
    if (r < 35) {
        type = VEHICLE_SEDAN;
    } else if (r < 50) {
        type = VEHICLE_SUV;
    } else if (r < 65) {
        type = VEHICLE_BUS;
    } else if (r < 80) {
        type = VEHICLE_BIKE;
    } else if (r < 90) {
        type = VEHICLE_TRUCK;
    } else if (r < 95) {
        type = VEHICLE_TAXI;
    } else {
        type = VEHICLE_AMBULANCE;
    }

    // 3. Initialize Dimensions, Physics & Styling per Vehicle Type
    switch (type) {
        case VEHICLE_SEDAN:
            length = 36;
            width = 20;
            maxSpeed = 3.2f;
            accelRate = 0.08f;
            decelRate = 0.16f;
            color = vibrantColors[rand() % numColors];
            secondaryColor = COLOR(30, 45, 60); // Tinted glass
            accentColor = WHITE;
            break;

        case VEHICLE_SUV:
            length = 40;
            width = 22;
            maxSpeed = 3.0f;
            accelRate = 0.075f;
            decelRate = 0.16f;
            color = vibrantColors[rand() % numColors];
            secondaryColor = COLOR(30, 45, 60);
            accentColor = COLOR(90, 105, 120); // Roof rack
            break;

        case VEHICLE_BUS: {
            length = 62;
            width = 24;
            maxSpeed = 2.5f;
            accelRate = 0.045f;
            decelRate = 0.13f;
            int busColors[] = {
                COLOR(225, 45, 45),  // City Red
                COLOR(30, 125, 235), // Metro Blue
                COLOR(35, 185, 90),  // Eco Green
                COLOR(250, 175, 25)  // Golden Transit
            };
            color = busColors[rand() % 4];
            secondaryColor = COLOR(240, 245, 250); // White roof
            accentColor = COLOR(25, 35, 50);       // Windows
            break;
        }

        case VEHICLE_BIKE:
            length = 22;
            width = 12;
            maxSpeed = 3.6f;
            accelRate = 0.12f;
            decelRate = 0.18f;
            color = vibrantColors[rand() % numColors];
            secondaryColor = COLOR(20, 25, 30); // Chassis/Tires
            accentColor = vibrantColors[rand() % numColors]; // Rider Helmet
            break;

        case VEHICLE_TRUCK:
            length = 50;
            width = 23;
            maxSpeed = 2.6f;
            accelRate = 0.05f;
            decelRate = 0.14f;
            color = vibrantColors[rand() % numColors]; // Cab
            secondaryColor = COLOR(225, 235, 245);    // Cargo Box
            accentColor = COLOR(110, 125, 140);       // Box Trim
            break;

        case VEHICLE_TAXI:
            length = 36;
            width = 20;
            maxSpeed = 3.3f;
            accelRate = 0.085f;
            decelRate = 0.16f;
            color = COLOR(255, 205, 10); // Bright Taxi Yellow
            secondaryColor = COLOR(25, 25, 25); // Checker pattern
            accentColor = COLOR(255, 255, 255); // Taxi Roof Sign
            break;

        case VEHICLE_AMBULANCE:
            length = 42;
            width = 22;
            maxSpeed = 3.5f;
            accelRate = 0.095f;
            decelRate = 0.17f;
            color = COLOR(250, 250, 255);       // Hospital White
            secondaryColor = COLOR(235, 35, 35); // Emergency Red Stripe/Cross
            accentColor = COLOR(30, 130, 255);   // Blue Beacon
            break;

        default:
            length = 36;
            width = 20;
            maxSpeed = 3.2f;
            accelRate = 0.08f;
            decelRate = 0.16f;
            color = vibrantColors[0];
            secondaryColor = COLOR(30, 45, 60);
            accentColor = WHITE;
            break;
    }

    speed = maxSpeed;
    targetSpeed = maxSpeed;

    // Initial position based on spawn lane
    switch (dir) {
        case DIR_SOUTH:
            x = LANE_SOUTH_X;
            y = -length / 2.0f;
            break;
        case DIR_NORTH:
            x = LANE_NORTH_X;
            y = SCREEN_HEIGHT + length / 2.0f;
            break;
        case DIR_EAST:
            x = -length / 2.0f;
            y = LANE_EAST_Y;
            break;
        case DIR_WEST:
            x = SCREEN_WIDTH + length / 2.0f;
            y = LANE_WEST_Y;
            break;
    }
}

float Vehicle::getFrontBumperPos() const {
    switch (dir) {
        case DIR_SOUTH: return y + length / 2.0f;
        case DIR_NORTH: return y - length / 2.0f;
        case DIR_EAST:  return x + length / 2.0f;
        case DIR_WEST:  return x - length / 2.0f;
    }
    return 0.0f;
}

float Vehicle::getRearBumperPos() const {
    switch (dir) {
        case DIR_SOUTH: return y - length / 2.0f;
        case DIR_NORTH: return y + length / 2.0f;
        case DIR_EAST:  return x - length / 2.0f;
        case DIR_WEST:  return x + length / 2.0f;
    }
    return 0.0f;
}

bool Vehicle::checkPassedStopLine() const {
    float bumper = getFrontBumperPos();
    switch (dir) {
        case DIR_SOUTH: return bumper >= STOP_LINE_SOUTH;
        case DIR_NORTH: return bumper <= STOP_LINE_NORTH;
        case DIR_EAST:  return bumper >= STOP_LINE_EAST;
        case DIR_WEST:  return bumper <= STOP_LINE_WEST;
    }
    return false;
}

bool Vehicle::checkClearedIntersection() const {
    float rear = getRearBumperPos();
    switch (dir) {
        case DIR_SOUTH: return rear > ROAD_BOTTOM;
        case DIR_NORTH: return rear < ROAD_TOP;
        case DIR_EAST:  return rear > ROAD_RIGHT;
        case DIR_WEST:  return rear < ROAD_LEFT;
    }
    return false;
}

bool Vehicle::isInIntersectionBox() const {
    float front = getFrontBumperPos();
    float rear = getRearBumperPos();
    float minP = std::min(front, rear);
    float maxP = std::max(front, rear);

    if (dir == DIR_SOUTH || dir == DIR_NORTH) {
        return (maxP > ROAD_TOP && minP < ROAD_BOTTOM);
    } else {
        return (maxP > ROAD_LEFT && minP < ROAD_RIGHT);
    }
}

bool Vehicle::checkInDetectionZone() const {
    float front = getFrontBumperPos();
    switch (dir) {
        case DIR_SOUTH:
            return (front >= (STOP_LINE_SOUTH - DETECTION_ZONE_LENGTH)) && (front <= STOP_LINE_SOUTH);
        case DIR_NORTH:
            return (front <= (STOP_LINE_NORTH + DETECTION_ZONE_LENGTH)) && (front >= STOP_LINE_NORTH);
        case DIR_EAST:
            return (front >= (STOP_LINE_EAST - DETECTION_ZONE_LENGTH)) && (front <= STOP_LINE_EAST);
        case DIR_WEST:
            return (front <= (STOP_LINE_WEST + DETECTION_ZONE_LENGTH)) && (front >= STOP_LINE_WEST);
    }
    return false;
}

void Vehicle::update(float dt, float distToAhead, float distToStopLine, bool isGreen, double currentTime) {
    if (!hasPassedStopLine && checkPassedStopLine()) {
        hasPassedStopLine = true;
    }
    if (!hasCleared && checkClearedIntersection()) {
        hasCleared = true;
    }

    inDetectionZone = checkInDetectionZone();

    // Track stationary time inside intersection box for deadlock detection
    if (isInIntersectionBox() && isStopped) {
        boxWaitTime += dt;
    } else {
        boxWaitTime = 0.0f;
    }

    // Default target speed is max speed
    float desiredSpeed = maxSpeed;

    // 1. Safe Following Distance Logic (Collision Avoidance)
    if (distToAhead < 999.0f) {
        float effectiveGap = distToAhead - SAFE_FOLLOW_DISTANCE;
        if (effectiveGap <= 0.0f) {
            desiredSpeed = 0.0f;
        } else {
            float followingSpeed = std::max(0.0f, effectiveGap / 10.0f);
            desiredSpeed = std::min(desiredSpeed, followingSpeed);
        }
    }

    // 2. Traffic Light & Stop Line Behavior
    if (!hasPassedStopLine && !isGreen) {
        if (distToStopLine > 0.0f && distToStopLine < 160.0f) {
            float stopSpeed = std::max(0.0f, (distToStopLine - 4.0f) / 12.0f);
            desiredSpeed = std::min(desiredSpeed, stopSpeed);
        } else if (distToStopLine <= 0.0f) {
            desiredSpeed = 0.0f;
        }
    }

    // Smooth Acceleration / Deceleration
    targetSpeed = desiredSpeed;
    if (speed < targetSpeed) {
        speed = std::min(targetSpeed, speed + accelRate * dt * 60.0f);
    } else if (speed > targetSpeed) {
        speed = std::max(targetSpeed, speed - decelRate * dt * 60.0f);
    }

    if (speed < 0.05f) {
        speed = 0.0f;
        isStopped = true;
        totalWaitTime += dt;
    } else {
        isStopped = false;
    }

    // Update Position
    float step = speed * (dt * 60.0f);
    switch (dir) {
        case DIR_SOUTH: y += step; break;
        case DIR_NORTH: y -= step; break;
        case DIR_EAST:  x += step; break;
        case DIR_WEST:  x -= step; break;
    }
}

// ---------------------- 2D Vehicle Rendering ----------------------

void Vehicle::drawSedan(int left, int top, int right, int bottom, bool vertical) const {
    // Body
    setfillstyle(SOLID_FILL, color);
    bar(left, top, right, bottom);

    if (vertical) {
        // Windshields & Roof
        setfillstyle(SOLID_FILL, secondaryColor);
        bar(left + 2, top + 6, right - 2, bottom - 6);
        // Roof top in body color
        setfillstyle(SOLID_FILL, color);
        bar(left + 3, top + 11, right - 3, bottom - 11);

        // Headlights & Taillights
        setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
        if (dir == DIR_SOUTH) {
            fillellipse(left + 4, bottom - 2, 2, 2);
            fillellipse(right - 4, bottom - 2, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 4, top + 2, 2, 2);
            fillellipse(right - 4, top + 2, 2, 2);
        } else {
            fillellipse(left + 4, top + 2, 2, 2);
            fillellipse(right - 4, top + 2, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 4, bottom - 2, 2, 2);
            fillellipse(right - 4, bottom - 2, 2, 2);
        }
    } else {
        // Windshields & Roof
        setfillstyle(SOLID_FILL, secondaryColor);
        bar(left + 6, top + 2, right - 6, bottom - 2);
        // Roof top in body color
        setfillstyle(SOLID_FILL, color);
        bar(left + 11, top + 3, right - 11, bottom - 3);

        // Headlights & Taillights
        setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
        if (dir == DIR_EAST) {
            fillellipse(right - 2, top + 4, 2, 2);
            fillellipse(right - 2, bottom - 4, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 2, top + 4, 2, 2);
            fillellipse(left + 2, bottom - 4, 2, 2);
        } else {
            fillellipse(left + 2, top + 4, 2, 2);
            fillellipse(left + 2, bottom - 4, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(right - 2, top + 4, 2, 2);
            fillellipse(right - 2, bottom - 4, 2, 2);
        }
    }
}

void Vehicle::drawSUV(int left, int top, int right, int bottom, bool vertical) const {
    // Sturdy SUV Body
    setfillstyle(SOLID_FILL, color);
    bar(left, top, right, bottom);

    if (vertical) {
        // Tinted Glass area
        setfillstyle(SOLID_FILL, secondaryColor);
        bar(left + 2, top + 6, right - 2, bottom - 6);

        // Roof
        setfillstyle(SOLID_FILL, color);
        bar(left + 3, top + 10, right - 3, bottom - 10);

        // Roof rack rails
        setcolor(accentColor);
        line(left + 4, top + 12, left + 4, bottom - 12);
        line(right - 4, top + 12, right - 4, bottom - 12);
        line(left + 4, top + 15, right - 4, top + 15);
        line(left + 4, bottom - 15, right - 4, bottom - 15);

        // Spare Tire & Lights
        if (dir == DIR_SOUTH) {
            // Spare tire on top (rear)
            setfillstyle(SOLID_FILL, COLOR(20, 20, 25));
            fillellipse((left + right) / 2, top + 2, 4, 2);
            setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
            fillellipse(left + 4, bottom - 2, 2, 2);
            fillellipse(right - 4, bottom - 2, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 3, top + 3, 2, 2);
            fillellipse(right - 3, top + 3, 2, 2);
        } else {
            // Spare tire on bottom (rear)
            setfillstyle(SOLID_FILL, COLOR(20, 20, 25));
            fillellipse((left + right) / 2, bottom - 2, 4, 2);
            setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
            fillellipse(left + 4, top + 2, 2, 2);
            fillellipse(right - 4, top + 2, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 3, bottom - 3, 2, 2);
            fillellipse(right - 3, bottom - 3, 2, 2);
        }
    } else {
        // Tinted Glass area
        setfillstyle(SOLID_FILL, secondaryColor);
        bar(left + 6, top + 2, right - 6, bottom - 2);

        // Roof
        setfillstyle(SOLID_FILL, color);
        bar(left + 10, top + 3, right - 10, bottom - 3);

        // Roof rack rails
        setcolor(accentColor);
        line(left + 12, top + 4, right - 12, top + 4);
        line(left + 12, bottom - 4, right - 12, bottom - 4);
        line(left + 15, top + 4, left + 15, bottom - 4);
        line(right - 15, top + 4, right - 15, bottom - 4);

        if (dir == DIR_EAST) {
            // Spare tire on left (rear)
            setfillstyle(SOLID_FILL, COLOR(20, 20, 25));
            fillellipse(left + 2, (top + bottom) / 2, 2, 4);
            setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
            fillellipse(right - 2, top + 4, 2, 2);
            fillellipse(right - 2, bottom - 4, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 3, top + 3, 2, 2);
            fillellipse(left + 3, bottom - 3, 2, 2);
        } else {
            // Spare tire on right (rear)
            setfillstyle(SOLID_FILL, COLOR(20, 20, 25));
            fillellipse(right - 2, (top + bottom) / 2, 2, 4);
            setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
            fillellipse(left + 2, top + 4, 2, 2);
            fillellipse(left + 2, bottom - 4, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(right - 3, top + 3, 2, 2);
            fillellipse(right - 3, bottom - 3, 2, 2);
        }
    }
}

void Vehicle::drawBus(int left, int top, int right, int bottom, bool vertical) const {
    // Bus Base
    setfillstyle(SOLID_FILL, color);
    bar(left, top, right, bottom);

    if (vertical) {
        // White / Light Roof Center
        setfillstyle(SOLID_FILL, secondaryColor);
        bar(left + 3, top + 8, right - 3, bottom - 8);

        // AC Units on roof
        setfillstyle(SOLID_FILL, COLOR(120, 130, 140));
        bar(left + 6, top + 18, right - 6, top + 26);
        bar(left + 6, bottom - 26, right - 6, bottom - 18);

        // Windows along sides
        setfillstyle(SOLID_FILL, accentColor);
        for (int ypos = top + 10; ypos < bottom - 10; ypos += 9) {
            bar(left + 1, ypos, left + 3, ypos + 6);
            bar(right - 3, ypos, right - 1, ypos + 6);
        }

        if (dir == DIR_SOUTH) {
            // Front Windshield (Bottom)
            setfillstyle(SOLID_FILL, COLOR(30, 45, 60));
            bar(left + 2, bottom - 7, right - 2, bottom - 2);
            // Destination sign
            setfillstyle(SOLID_FILL, COLOR(255, 180, 20));
            bar(left + 6, bottom - 5, right - 6, bottom - 3);

            // Headlights & Taillights
            setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
            fillellipse(left + 3, bottom - 1, 2, 2);
            fillellipse(right - 3, bottom - 1, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 3, top + 2, 2, 2);
            fillellipse(right - 3, top + 2, 2, 2);
        } else {
            // Front Windshield (Top)
            setfillstyle(SOLID_FILL, COLOR(30, 45, 60));
            bar(left + 2, top + 2, right - 2, top + 7);
            // Destination sign
            setfillstyle(SOLID_FILL, COLOR(255, 180, 20));
            bar(left + 6, top + 3, right - 6, top + 5);

            // Headlights & Taillights
            setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
            fillellipse(left + 3, top + 1, 2, 2);
            fillellipse(right - 3, top + 1, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 3, bottom - 2, 2, 2);
            fillellipse(right - 3, bottom - 2, 2, 2);
        }
    } else {
        // White / Light Roof Center
        setfillstyle(SOLID_FILL, secondaryColor);
        bar(left + 8, top + 3, right - 8, bottom - 3);

        // AC Units on roof
        setfillstyle(SOLID_FILL, COLOR(120, 130, 140));
        bar(left + 18, top + 6, left + 26, bottom - 6);
        bar(right - 26, top + 6, right - 18, bottom - 6);

        // Windows along sides
        setfillstyle(SOLID_FILL, accentColor);
        for (int xpos = left + 10; xpos < right - 10; xpos += 9) {
            bar(xpos, top + 1, xpos + 6, top + 3);
            bar(xpos, bottom - 3, xpos + 6, bottom - 1);
        }

        if (dir == DIR_EAST) {
            // Front Windshield (Right)
            setfillstyle(SOLID_FILL, COLOR(30, 45, 60));
            bar(right - 7, top + 2, right - 2, bottom - 2);
            // Destination sign
            setfillstyle(SOLID_FILL, COLOR(255, 180, 20));
            bar(right - 5, top + 6, right - 3, bottom - 6);

            // Headlights & Taillights
            setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
            fillellipse(right - 1, top + 3, 2, 2);
            fillellipse(right - 1, bottom - 3, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 2, top + 3, 2, 2);
            fillellipse(left + 2, bottom - 3, 2, 2);
        } else {
            // Front Windshield (Left)
            setfillstyle(SOLID_FILL, COLOR(30, 45, 60));
            bar(left + 2, top + 2, left + 7, bottom - 2);
            // Destination sign
            setfillstyle(SOLID_FILL, COLOR(255, 180, 20));
            bar(left + 3, top + 6, left + 5, bottom - 6);

            // Headlights & Taillights
            setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
            fillellipse(left + 1, top + 3, 2, 2);
            fillellipse(left + 1, bottom - 3, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(right - 2, top + 3, 2, 2);
            fillellipse(right - 2, bottom - 3, 2, 2);
        }
    }
}

void Vehicle::drawBike(int left, int top, int right, int bottom, bool vertical) const {
    int cx = (left + right) / 2;
    int cy = (top + bottom) / 2;

    if (vertical) {
        // Front & Rear Tires
        setfillstyle(SOLID_FILL, COLOR(20, 20, 25));
        bar(cx - 2, top, cx + 2, top + 4);
        bar(cx - 2, bottom - 4, cx + 2, bottom);

        // Bike Body Frame
        setfillstyle(SOLID_FILL, color);
        bar(cx - 3, top + 4, cx + 3, bottom - 4);

        // Handlebars
        setcolor(COLOR(200, 200, 210));
        line(cx - 5, (dir == DIR_SOUTH) ? bottom - 5 : top + 5,
             cx + 5, (dir == DIR_SOUTH) ? bottom - 5 : top + 5);

        // Rider Torso (Jacket)
        setfillstyle(SOLID_FILL, COLOR(40, 50, 60));
        fillellipse(cx, cy, 4, 3);

        // Rider Helmet
        setfillstyle(SOLID_FILL, accentColor);
        fillellipse(cx, cy, 3, 3);

        // Headlight & Taillight
        if (dir == DIR_SOUTH) {
            setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
            fillellipse(cx, bottom - 1, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(cx, top + 1, 2, 2);
        } else {
            setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
            fillellipse(cx, top + 1, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(cx, bottom - 1, 2, 2);
        }
    } else {
        // Front & Rear Tires
        setfillstyle(SOLID_FILL, COLOR(20, 20, 25));
        bar(left, cy - 2, left + 4, cy + 2);
        bar(right - 4, cy - 2, right, cy + 2);

        // Bike Body Frame
        setfillstyle(SOLID_FILL, color);
        bar(left + 4, cy - 3, right - 4, cy + 3);

        // Handlebars
        setcolor(COLOR(200, 200, 210));
        line((dir == DIR_EAST) ? right - 5 : left + 5, cy - 5,
             (dir == DIR_EAST) ? right - 5 : left + 5, cy + 5);

        // Rider Torso
        setfillstyle(SOLID_FILL, COLOR(40, 50, 60));
        fillellipse(cx, cy, 3, 4);

        // Rider Helmet
        setfillstyle(SOLID_FILL, accentColor);
        fillellipse(cx, cy, 3, 3);

        // Headlight & Taillight
        if (dir == DIR_EAST) {
            setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
            fillellipse(right - 1, cy, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 1, cy, 2, 2);
        } else {
            setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
            fillellipse(left + 1, cy, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(right - 1, cy, 2, 2);
        }
    }
}

void Vehicle::drawTruck(int left, int top, int right, int bottom, bool vertical) const {
    if (vertical) {
        if (dir == DIR_SOUTH) {
            // Cargo Box (Top 65%)
            setfillstyle(SOLID_FILL, secondaryColor);
            bar(left, top, right, bottom - 16);
            setcolor(accentColor);
            rectangle(left, top, right, bottom - 16);
            line((left + right) / 2, top, (left + right) / 2, bottom - 16);

            // Cab Gap
            setfillstyle(SOLID_FILL, COLOR(30, 35, 40));
            bar(left + 2, bottom - 16, right - 2, bottom - 13);

            // Front Cab (Bottom 35%)
            setfillstyle(SOLID_FILL, color);
            bar(left + 1, bottom - 13, right - 1, bottom);
            // Windshield
            setfillstyle(SOLID_FILL, COLOR(30, 45, 60));
            bar(left + 3, bottom - 6, right - 3, bottom - 2);

            // Lights
            setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
            fillellipse(left + 3, bottom - 1, 2, 2);
            fillellipse(right - 3, bottom - 1, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 2, top + 2, 2, 2);
            fillellipse(right - 2, top + 2, 2, 2);
        } else {
            // Cargo Box (Bottom 65%)
            setfillstyle(SOLID_FILL, secondaryColor);
            bar(left, top + 16, right, bottom);
            setcolor(accentColor);
            rectangle(left, top + 16, right, bottom);
            line((left + right) / 2, top + 16, (left + right) / 2, bottom);

            // Cab Gap
            setfillstyle(SOLID_FILL, COLOR(30, 35, 40));
            bar(left + 2, top + 13, right - 2, top + 16);

            // Front Cab (Top 35%)
            setfillstyle(SOLID_FILL, color);
            bar(left + 1, top, right - 1, top + 13);
            // Windshield
            setfillstyle(SOLID_FILL, COLOR(30, 45, 60));
            bar(left + 3, top + 2, right - 3, top + 6);

            // Lights
            setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
            fillellipse(left + 3, top + 1, 2, 2);
            fillellipse(right - 3, top + 1, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 2, bottom - 2, 2, 2);
            fillellipse(right - 2, bottom - 2, 2, 2);
        }
    } else {
        if (dir == DIR_EAST) {
            // Cargo Box (Left 65%)
            setfillstyle(SOLID_FILL, secondaryColor);
            bar(left, top, right - 16, bottom);
            setcolor(accentColor);
            rectangle(left, top, right - 16, bottom);
            line(left, (top + bottom) / 2, right - 16, (top + bottom) / 2);

            // Cab Gap
            setfillstyle(SOLID_FILL, COLOR(30, 35, 40));
            bar(right - 16, top + 2, right - 13, bottom - 2);

            // Front Cab (Right 35%)
            setfillstyle(SOLID_FILL, color);
            bar(right - 13, top + 1, right, bottom - 1);
            // Windshield
            setfillstyle(SOLID_FILL, COLOR(30, 45, 60));
            bar(right - 6, top + 3, right - 2, bottom - 3);

            // Lights
            setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
            fillellipse(right - 1, top + 3, 2, 2);
            fillellipse(right - 1, bottom - 3, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 2, top + 2, 2, 2);
            fillellipse(left + 2, bottom - 2, 2, 2);
        } else {
            // Cargo Box (Right 65%)
            setfillstyle(SOLID_FILL, secondaryColor);
            bar(left + 16, top, right, bottom);
            setcolor(accentColor);
            rectangle(left + 16, top, right, bottom);
            line(left + 16, (top + bottom) / 2, right, (top + bottom) / 2);

            // Cab Gap
            setfillstyle(SOLID_FILL, COLOR(30, 35, 40));
            bar(left + 13, top + 2, left + 16, bottom - 2);

            // Front Cab (Left 35%)
            setfillstyle(SOLID_FILL, color);
            bar(left, top + 1, left + 13, bottom - 1);
            // Windshield
            setfillstyle(SOLID_FILL, COLOR(30, 45, 60));
            bar(left + 2, top + 3, left + 6, bottom - 3);

            // Lights
            setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
            fillellipse(left + 1, top + 3, 2, 2);
            fillellipse(left + 1, bottom - 3, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(right - 2, top + 2, 2, 2);
            fillellipse(right - 2, bottom - 2, 2, 2);
        }
    }
}

void Vehicle::drawTaxi(int left, int top, int right, int bottom, bool vertical) const {
    // Yellow Taxi Body
    setfillstyle(SOLID_FILL, color);
    bar(left, top, right, bottom);

    int cx = (left + right) / 2;
    int cy = (top + bottom) / 2;

    if (vertical) {
        // Windshields
        setfillstyle(SOLID_FILL, COLOR(30, 45, 60));
        bar(left + 2, top + 6, right - 2, bottom - 6);
        // Roof
        setfillstyle(SOLID_FILL, color);
        bar(left + 3, top + 11, right - 3, bottom - 11);

        // Checker Pattern Stripe
        setfillstyle(SOLID_FILL, secondaryColor);
        bar(left + 4, cy - 2, left + 8, cy + 2);
        bar(right - 8, cy - 2, right - 4, cy + 2);

        // TAXI Roof Light Sign
        setfillstyle(SOLID_FILL, accentColor);
        bar(cx - 5, cy - 3, cx + 5, cy + 3);
        setcolor(COLOR(255, 160, 0));
        rectangle(cx - 5, cy - 3, cx + 5, cy + 3);

        // Lights
        setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
        if (dir == DIR_SOUTH) {
            fillellipse(left + 4, bottom - 2, 2, 2);
            fillellipse(right - 4, bottom - 2, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 4, top + 2, 2, 2);
            fillellipse(right - 4, top + 2, 2, 2);
        } else {
            fillellipse(left + 4, top + 2, 2, 2);
            fillellipse(right - 4, top + 2, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 4, bottom - 2, 2, 2);
            fillellipse(right - 4, bottom - 2, 2, 2);
        }
    } else {
        // Windshields
        setfillstyle(SOLID_FILL, COLOR(30, 45, 60));
        bar(left + 6, top + 2, right - 6, bottom - 2);
        // Roof
        setfillstyle(SOLID_FILL, color);
        bar(left + 11, top + 3, right - 11, bottom - 3);

        // Checker Pattern Stripe
        setfillstyle(SOLID_FILL, secondaryColor);
        bar(cx - 2, top + 4, cx + 2, top + 8);
        bar(cx - 2, bottom - 8, cx + 2, bottom - 4);

        // TAXI Roof Light Sign
        setfillstyle(SOLID_FILL, accentColor);
        bar(cx - 3, cy - 5, cx + 3, cy + 5);
        setcolor(COLOR(255, 160, 0));
        rectangle(cx - 3, cy - 5, cx + 3, cy + 5);

        // Lights
        setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
        if (dir == DIR_EAST) {
            fillellipse(right - 2, top + 4, 2, 2);
            fillellipse(right - 2, bottom - 4, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 2, top + 4, 2, 2);
            fillellipse(left + 2, bottom - 4, 2, 2);
        } else {
            fillellipse(left + 2, top + 4, 2, 2);
            fillellipse(left + 2, bottom - 4, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(right - 2, top + 4, 2, 2);
            fillellipse(right - 2, bottom - 4, 2, 2);
        }
    }
}

void Vehicle::drawAmbulance(int left, int top, int right, int bottom, bool vertical) const {
    // Pure White Body
    setfillstyle(SOLID_FILL, color);
    bar(left, top, right, bottom);

    int cx = (left + right) / 2;
    int cy = (top + bottom) / 2;

    if (vertical) {
        // Red Side Stripes
        setfillstyle(SOLID_FILL, secondaryColor);
        bar(left, top + 8, left + 2, bottom - 8);
        bar(right - 2, top + 8, right, bottom - 8);

        // Front / Rear Windows
        setfillstyle(SOLID_FILL, COLOR(30, 45, 60));
        if (dir == DIR_SOUTH) {
            bar(left + 3, bottom - 8, right - 3, bottom - 4);
            bar(left + 3, top + 4, right - 3, top + 7);
        } else {
            bar(left + 3, top + 4, right - 3, top + 8);
            bar(left + 3, bottom - 7, right - 3, bottom - 4);
        }

        // Red Medical Cross (+) on Roof
        setfillstyle(SOLID_FILL, secondaryColor);
        bar(cx - 5, cy - 2, cx + 5, cy + 2);
        bar(cx - 2, cy - 5, cx + 2, cy + 5);

        // Dual Emergency Beacon Bar (Red + Blue)
        int beaconY = (dir == DIR_SOUTH) ? bottom - 11 : top + 11;
        setfillstyle(SOLID_FILL, RED);
        bar(cx - 5, beaconY - 2, cx - 1, beaconY + 2);
        setfillstyle(SOLID_FILL, accentColor);
        bar(cx + 1, beaconY - 2, cx + 5, beaconY + 2);

        // Lights
        setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
        if (dir == DIR_SOUTH) {
            fillellipse(left + 4, bottom - 2, 2, 2);
            fillellipse(right - 4, bottom - 2, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 4, top + 2, 2, 2);
            fillellipse(right - 4, top + 2, 2, 2);
        } else {
            fillellipse(left + 4, top + 2, 2, 2);
            fillellipse(right - 4, top + 2, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 4, bottom - 2, 2, 2);
            fillellipse(right - 4, bottom - 2, 2, 2);
        }
    } else {
        // Red Side Stripes
        setfillstyle(SOLID_FILL, secondaryColor);
        bar(left + 8, top, right - 8, top + 2);
        bar(left + 8, bottom - 2, right - 8, bottom);

        // Front / Rear Windows
        setfillstyle(SOLID_FILL, COLOR(30, 45, 60));
        if (dir == DIR_EAST) {
            bar(right - 8, top + 3, right - 4, bottom - 3);
            bar(left + 4, top + 3, left + 7, bottom - 3);
        } else {
            bar(left + 4, top + 3, left + 8, bottom - 3);
            bar(right - 7, top + 3, right - 4, bottom - 3);
        }

        // Red Medical Cross (+) on Roof
        setfillstyle(SOLID_FILL, secondaryColor);
        bar(cx - 2, cy - 5, cx + 2, cy + 5);
        bar(cx - 5, cy - 2, cx + 5, cy + 2);

        // Dual Emergency Beacon Bar (Red + Blue)
        int beaconX = (dir == DIR_EAST) ? right - 11 : left + 11;
        setfillstyle(SOLID_FILL, RED);
        bar(beaconX - 2, cy - 5, beaconX + 2, cy - 1);
        setfillstyle(SOLID_FILL, accentColor);
        bar(beaconX - 2, cy + 1, beaconX + 2, cy + 5);

        // Lights
        setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
        if (dir == DIR_EAST) {
            fillellipse(right - 2, top + 4, 2, 2);
            fillellipse(right - 2, bottom - 4, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 2, top + 4, 2, 2);
            fillellipse(left + 2, bottom - 4, 2, 2);
        } else {
            fillellipse(left + 2, top + 4, 2, 2);
            fillellipse(left + 2, bottom - 4, 2, 2);
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(right - 2, top + 4, 2, 2);
            fillellipse(right - 2, bottom - 4, 2, 2);
        }
    }
}

void Vehicle::draw() const {
    int l = length;
    int w = width;

    if (dir == DIR_SOUTH || dir == DIR_NORTH) {
        int left = static_cast<int>(x - w / 2);
        int top = static_cast<int>(y - l / 2);
        int right = static_cast<int>(x + w / 2);
        int bottom = static_cast<int>(y + l / 2);

        switch (type) {
            case VEHICLE_SEDAN:     drawSedan(left, top, right, bottom, true); break;
            case VEHICLE_SUV:       drawSUV(left, top, right, bottom, true); break;
            case VEHICLE_BUS:       drawBus(left, top, right, bottom, true); break;
            case VEHICLE_BIKE:      drawBike(left, top, right, bottom, true); break;
            case VEHICLE_TRUCK:     drawTruck(left, top, right, bottom, true); break;
            case VEHICLE_TAXI:      drawTaxi(left, top, right, bottom, true); break;
            case VEHICLE_AMBULANCE: drawAmbulance(left, top, right, bottom, true); break;
            default:                drawSedan(left, top, right, bottom, true); break;
        }
    } else {
        int left = static_cast<int>(x - l / 2);
        int top = static_cast<int>(y - w / 2);
        int right = static_cast<int>(x + l / 2);
        int bottom = static_cast<int>(y + w / 2);

        switch (type) {
            case VEHICLE_SEDAN:     drawSedan(left, top, right, bottom, false); break;
            case VEHICLE_SUV:       drawSUV(left, top, right, bottom, false); break;
            case VEHICLE_BUS:       drawBus(left, top, right, bottom, false); break;
            case VEHICLE_BIKE:      drawBike(left, top, right, bottom, false); break;
            case VEHICLE_TRUCK:     drawTruck(left, top, right, bottom, false); break;
            case VEHICLE_TAXI:      drawTaxi(left, top, right, bottom, false); break;
            case VEHICLE_AMBULANCE: drawAmbulance(left, top, right, bottom, false); break;
            default:                drawSedan(left, top, right, bottom, false); break;
        }
    }

    // Visual highlight when vehicle is trapped in deadlock
    if (inDeadlock) {
        int left = static_cast<int>(x - (dir == DIR_SOUTH || dir == DIR_NORTH ? width : length) / 2) - 3;
        int top = static_cast<int>(y - (dir == DIR_SOUTH || dir == DIR_NORTH ? length : width) / 2) - 3;
        int right = static_cast<int>(x + (dir == DIR_SOUTH || dir == DIR_NORTH ? width : length) / 2) + 3;
        int bottom = static_cast<int>(y + (dir == DIR_SOUTH || dir == DIR_NORTH ? length : width) / 2) + 3;

        setcolor(COLOR(255, 40, 40));
        rectangle(left, top, right, bottom);
        setcolor(COLOR(255, 220, 0));
        rectangle(left - 1, top - 1, right + 1, bottom + 1);
    }
}

