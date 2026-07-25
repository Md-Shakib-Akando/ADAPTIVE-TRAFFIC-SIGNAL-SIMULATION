#include "Intersection.h"
#include <algorithm>
#include <cmath>

IntersectionManager::IntersectionManager()
    : nextVehicleId(1), countSouth(0), countNorth(0), countEast(0), countWest(0),
      totalClearedVehicles(0), totalAccumulatedWaitTime(0.0), autoSpawnEnabled(true) {

    for (int i = 0; i < 4; ++i) {
        spawnTimers[i] = 0.0f;
        spawnIntervals[i] = 2.5f + (rand() % 20) / 10.0f; // 2.5s - 4.5s random interval
    }
}

float IntersectionManager::getAverageWaitTime() const {
    if (totalClearedVehicles == 0) return 0.0f;
    return static_cast<float>(totalAccumulatedWaitTime / totalClearedVehicles);
}

float IntersectionManager::calculateDistanceToAhead(const Vehicle& curr, const Vehicle& ahead) const {
    switch (curr.dir) {
        case DIR_SOUTH: {
            float aheadRear = ahead.y - CAR_LENGTH / 2.0f;
            float currFront = curr.y + CAR_LENGTH / 2.0f;
            return aheadRear - currFront;
        }
        case DIR_NORTH: {
            float aheadRear = ahead.y + CAR_LENGTH / 2.0f;
            float currFront = curr.y - CAR_LENGTH / 2.0f;
            return currFront - aheadRear;
        }
        case DIR_EAST: {
            float aheadRear = ahead.x - CAR_LENGTH / 2.0f;
            float currFront = curr.x + CAR_LENGTH / 2.0f;
            return aheadRear - currFront;
        }
        case DIR_WEST: {
            float aheadRear = ahead.x + CAR_LENGTH / 2.0f;
            float currFront = curr.x - CAR_LENGTH / 2.0f;
            return currFront - aheadRear;
        }
    }
    return 9999.0f;
}

float IntersectionManager::calculateDistanceToStopLine(const Vehicle& curr) const {
    float front = curr.getFrontBumperPos();
    switch (curr.dir) {
        case DIR_SOUTH: return STOP_LINE_SOUTH - front;
        case DIR_NORTH: return front - STOP_LINE_NORTH;
        case DIR_EAST:  return STOP_LINE_EAST - front;
        case DIR_WEST:  return front - STOP_LINE_WEST;
    }
    return 0.0f;
}

bool IntersectionManager::spawnVehicle(Direction dir, double currentTime) {
    // Check if spawn zone is clear
    for (const auto& v : vehicles) {
        if (v.dir == dir) {
            float rear = v.getRearBumperPos();
            if (dir == DIR_SOUTH && rear < (CAR_LENGTH + 20.0f)) return false;
            if (dir == DIR_NORTH && rear > (SCREEN_HEIGHT - CAR_LENGTH - 20.0f)) return false;
            if (dir == DIR_EAST  && rear < (CAR_LENGTH + 20.0f)) return false;
            if (dir == DIR_WEST  && rear > (SCREEN_WIDTH - CAR_LENGTH - 20.0f)) return false;
        }
    }

    vehicles.emplace_back(nextVehicleId++, dir, currentTime);
    return true;
}

void IntersectionManager::triggerBurstSpawn(double currentTime) {
    spawnVehicle(DIR_SOUTH, currentTime);
    spawnVehicle(DIR_NORTH, currentTime);
    spawnVehicle(DIR_EAST,  currentTime);
    spawnVehicle(DIR_WEST,  currentTime);
}

void IntersectionManager::clearAllVehicles() {
    vehicles.clear();
    countSouth = countNorth = countEast = countWest = 0;
}

bool IntersectionManager::handleMouseClick(int mx, int my, double currentTime) {
    if (mx >= HUD_X_START) return false; // Clicked on HUD panel

    if (mx >= ROAD_LEFT && mx <= CENTER_X && my <= ROAD_TOP) {
        return spawnVehicle(DIR_SOUTH, currentTime);
    }
    if (mx >= CENTER_X && mx <= ROAD_RIGHT && my >= ROAD_BOTTOM) {
        return spawnVehicle(DIR_NORTH, currentTime);
    }
    if (mx <= ROAD_LEFT && my >= ROAD_TOP && my <= CENTER_Y) {
        return spawnVehicle(DIR_EAST, currentTime);
    }
    if (mx >= ROAD_RIGHT && my >= CENTER_Y && my <= ROAD_BOTTOM) {
        return spawnVehicle(DIR_WEST, currentTime);
    }
    return false;
}

void IntersectionManager::update(float dt, double currentTime) {
    // 1. Automatic Vehicle Spawning
    if (autoSpawnEnabled) {
        for (int d = 0; d < 4; ++d) {
            spawnTimers[d] += dt;
            if (spawnTimers[d] >= spawnIntervals[d]) {
                if (spawnVehicle(static_cast<Direction>(d), currentTime)) {
                    spawnTimers[d] = 0.0f;
                    spawnIntervals[d] = 2.0f + (rand() % 35) / 10.0f; // 2.0s - 5.5s
                }
            }
        }
    }

    // 2. Sort vehicles per approach direction (furthest along first)
    std::vector<Vehicle*> dirVehicles[4];
    for (auto& v : vehicles) {
        dirVehicles[v.dir].push_back(&v);
    }

    std::sort(dirVehicles[DIR_SOUTH].begin(), dirVehicles[DIR_SOUTH].end(),
              [](const Vehicle* a, const Vehicle* b) { return a->y > b->y; });

    std::sort(dirVehicles[DIR_NORTH].begin(), dirVehicles[DIR_NORTH].end(),
              [](const Vehicle* a, const Vehicle* b) { return a->y < b->y; });

    std::sort(dirVehicles[DIR_EAST].begin(), dirVehicles[DIR_EAST].end(),
              [](const Vehicle* a, const Vehicle* b) { return a->x > b->x; });

    std::sort(dirVehicles[DIR_WEST].begin(), dirVehicles[DIR_WEST].end(),
              [](const Vehicle* a, const Vehicle* b) { return a->x < b->x; });

    // Reset detection counts
    countSouth = countNorth = countEast = countWest = 0;

    // 3. Update physics for vehicles in each direction
    for (int d = 0; d < 4; ++d) {
        Direction dir = static_cast<Direction>(d);
        bool isGreen = false;
        if (dir == DIR_SOUTH || dir == DIR_NORTH) {
            isGreen = lightController.isNorthSouthGreen();
        } else {
            isGreen = lightController.isEastWestGreen();
        }

        auto& list = dirVehicles[d];
        for (size_t i = 0; i < list.size(); ++i) {
            Vehicle* v = list[i];
            float distAhead = 9999.0f;
            if (i > 0) {
                distAhead = calculateDistanceToAhead(*v, *list[i - 1]);
            }
            float distStop = calculateDistanceToStopLine(*v);

            v->update(dt, distAhead, distStop, isGreen, currentTime);

            if (v->inDetectionZone) {
                if (dir == DIR_SOUTH) countSouth++;
                else if (dir == DIR_NORTH) countNorth++;
                else if (dir == DIR_EAST) countEast++;
                else if (dir == DIR_WEST) countWest++;
            }
        }
    }

    // 4. Update Traffic Light Controller with live counts
    lightController.update(dt, countNorth, countSouth, countEast, countWest);

    // 5. Clean up vehicles that have left the screen & accumulate metrics
    for (auto it = vehicles.begin(); it != vehicles.end(); ) {
        bool offScreen = false;
        if (it->dir == DIR_SOUTH && it->y > (SCREEN_HEIGHT + CAR_LENGTH * 2)) offScreen = true;
        if (it->dir == DIR_NORTH && it->y < (-CAR_LENGTH * 2)) offScreen = true;
        if (it->dir == DIR_EAST  && it->x > (SCREEN_WIDTH + CAR_LENGTH * 2)) offScreen = true;
        if (it->dir == DIR_WEST  && it->x < (-CAR_LENGTH * 2)) offScreen = true;

        if (offScreen) {
            totalClearedVehicles++;
            totalAccumulatedWaitTime += it->totalWaitTime;
            it = vehicles.erase(it);
        } else {
            ++it;
        }
    }
}

void IntersectionManager::drawVehicles() const {
    for (const auto& v : vehicles) {
        v.draw();
    }
}
