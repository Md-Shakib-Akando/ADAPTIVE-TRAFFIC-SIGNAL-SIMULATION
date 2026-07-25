#include "Vehicle.h"
#include <cmath>
#include <algorithm>

Vehicle::Vehicle(int vehicleId, Direction direction, double currentTime)
    : id(vehicleId), dir(direction), spawnTime(currentTime), totalWaitTime(0.0),
      isStopped(false), hasPassedStopLine(false), hasCleared(false), inDetectionZone(false) {

    speed = MAX_CAR_SPEED;
    targetSpeed = MAX_CAR_SPEED;
    vehicleType = rand() % 3;

    // Palette of vibrant, distinct car colors
    int colorPalette[] = {
        COLOR(220, 50, 50),   // Crimson Red
        COLOR(40, 120, 220),  // Deep Blue
        COLOR(240, 160, 20),  // Gold/Amber
        COLOR(40, 180, 90),   // Emerald Green
        COLOR(160, 60, 200),  // Purple
        COLOR(230, 230, 230), // Silver White
        COLOR(50, 50, 50)     // Dark Charcoal
    };
    color = colorPalette[rand() % 7];

    // Initial position based on spawn lane
    switch (dir) {
        case DIR_SOUTH:
            x = LANE_SOUTH_X;
            y = -CAR_LENGTH;
            break;
        case DIR_NORTH:
            x = LANE_NORTH_X;
            y = SCREEN_HEIGHT + CAR_LENGTH;
            break;
        case DIR_EAST:
            x = -CAR_LENGTH;
            y = LANE_EAST_Y;
            break;
        case DIR_WEST:
            x = SCREEN_WIDTH + CAR_LENGTH;
            y = LANE_WEST_Y;
            break;
    }
}

float Vehicle::getFrontBumperPos() const {
    switch (dir) {
        case DIR_SOUTH: return y + CAR_LENGTH / 2.0f;
        case DIR_NORTH: return y - CAR_LENGTH / 2.0f;
        case DIR_EAST:  return x + CAR_LENGTH / 2.0f;
        case DIR_WEST:  return x - CAR_LENGTH / 2.0f;
    }
    return 0.0f;
}

float Vehicle::getRearBumperPos() const {
    switch (dir) {
        case DIR_SOUTH: return y - CAR_LENGTH / 2.0f;
        case DIR_NORTH: return y + CAR_LENGTH / 2.0f;
        case DIR_EAST:  return x - CAR_LENGTH / 2.0f;
        case DIR_WEST:  return x + CAR_LENGTH / 2.0f;
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

    // Default target speed is max speed
    float desiredSpeed = MAX_CAR_SPEED;

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
        speed = std::min(targetSpeed, speed + ACCEL_RATE * dt * 60.0f);
    } else if (speed > targetSpeed) {
        speed = std::max(targetSpeed, speed - DECEL_RATE * dt * 60.0f);
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

void Vehicle::draw() const {
    int curColor = color;
    setcolor(curColor);
    setfillstyle(SOLID_FILL, curColor);

    int l = CAR_LENGTH;
    int w = CAR_WIDTH;

    if (dir == DIR_SOUTH || dir == DIR_NORTH) {
        int left = (int)(x - w / 2);
        int top = (int)(y - l / 2);
        int right = (int)(x + w / 2);
        int bottom = (int)(y + l / 2);

        bar(left, top, right, bottom);

        // Roof / Windshield detail
        setfillstyle(SOLID_FILL, COLOR(30, 40, 50));
        bar(left + 3, top + 8, right - 3, bottom - 8);

        // Headlights
        setcolor(YELLOW);
        setfillstyle(SOLID_FILL, YELLOW);
        if (dir == DIR_SOUTH) {
            fillellipse(left + 4, bottom - 2, 2, 2);
            fillellipse(right - 4, bottom - 2, 2, 2);
            // Taillights
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 4, top + 2, 2, 2);
            fillellipse(right - 4, top + 2, 2, 2);
        } else {
            fillellipse(left + 4, top + 2, 2, 2);
            fillellipse(right - 4, top + 2, 2, 2);
            // Taillights
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 4, bottom - 2, 2, 2);
            fillellipse(right - 4, bottom - 2, 2, 2);
        }
    } else {
        int left = (int)(x - l / 2);
        int top = (int)(y - w / 2);
        int right = (int)(x + l / 2);
        int bottom = (int)(y + w / 2);

        bar(left, top, right, bottom);

        // Roof / Windshield detail
        setfillstyle(SOLID_FILL, COLOR(30, 40, 50));
        bar(left + 8, top + 3, right - 8, bottom - 3);

        // Headlights & Taillights
        setcolor(YELLOW); setfillstyle(SOLID_FILL, YELLOW);
        if (dir == DIR_EAST) {
            fillellipse(right - 2, top + 4, 2, 2);
            fillellipse(right - 2, bottom - 4, 2, 2);
            // Taillights
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(left + 2, top + 4, 2, 2);
            fillellipse(left + 2, bottom - 4, 2, 2);
        } else {
            fillellipse(left + 2, top + 4, 2, 2);
            fillellipse(left + 2, bottom - 4, 2, 2);
            // Taillights
            setcolor(RED); setfillstyle(SOLID_FILL, RED);
            fillellipse(right - 2, top + 4, 2, 2);
            fillellipse(right - 2, bottom - 4, 2, 2);
        }
    }
}
