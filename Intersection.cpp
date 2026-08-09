#include "Intersection.h"
#include <algorithm>
#include <cmath>

IntersectionManager::IntersectionManager()
    : nextVehicleId(1), countSouth(0), countNorth(0), countEast(0), countWest(0),
      totalClearedVehicles(0), totalAccumulatedWaitTime(0.0), autoSpawnEnabled(true),
      isDeadlocked(false), deadlockDuration(0.0f), deadlocksDetectedCount(0),
      deadlocksResolvedCount(0), boxOccupantCount(0), antiDeadlockGuard(true),
      resolvingDeadlock(false), resolveTimer(0.0f), resolveDirection(0), starvationDetected(false) {

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
            float aheadRear = ahead.getRearBumperPos();
            float currFront = curr.getFrontBumperPos();
            return aheadRear - currFront;
        }
        case DIR_NORTH: {
            float aheadRear = ahead.getRearBumperPos();
            float currFront = curr.getFrontBumperPos();
            return currFront - aheadRear;
        }
        case DIR_EAST: {
            float aheadRear = ahead.getRearBumperPos();
            float currFront = curr.getFrontBumperPos();
            return aheadRear - currFront;
        }
        case DIR_WEST: {
            float aheadRear = ahead.getRearBumperPos();
            float currFront = curr.getFrontBumperPos();
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
    // Check if spawn zone is clear (ensure enough room for largest vehicle + safe gap)
    const float minSpawnClearance = 70.0f;
    for (const auto& v : vehicles) {
        if (v.dir == dir) {
            float rear = v.getRearBumperPos();
            if (dir == DIR_SOUTH && rear < minSpawnClearance) return false;
            if (dir == DIR_NORTH && rear > (SCREEN_HEIGHT - minSpawnClearance)) return false;
            if (dir == DIR_EAST  && rear < minSpawnClearance) return false;
            if (dir == DIR_WEST  && rear > (SCREEN_WIDTH - minSpawnClearance)) return false;
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
    isDeadlocked = false;
    resolvingDeadlock = false;
    deadlockDuration = 0.0f;
    boxOccupantCount = 0;
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

// ---------------------- Deadlock System Methods ----------------------

bool IntersectionManager::isBoxExitClear(Direction dir, float vehicleLength) const {
    if (!antiDeadlockGuard) return true; // Prevention guard disabled

    for (const auto& v : vehicles) {
        if (v.dir == dir) {
            float rear = v.getRearBumperPos();
            switch (dir) {
                case DIR_SOUTH:
                    if (rear > ROAD_BOTTOM && rear < (ROAD_BOTTOM + vehicleLength + 15.0f) && v.speed < 0.2f) {
                        return false;
                    }
                    break;
                case DIR_NORTH:
                    if (rear < ROAD_TOP && rear > (ROAD_TOP - vehicleLength - 15.0f) && v.speed < 0.2f) {
                        return false;
                    }
                    break;
                case DIR_EAST:
                    if (rear > ROAD_RIGHT && rear < (ROAD_RIGHT + vehicleLength + 15.0f) && v.speed < 0.2f) {
                        return false;
                    }
                    break;
                case DIR_WEST:
                    if (rear < ROAD_LEFT && rear > (ROAD_LEFT - vehicleLength - 15.0f) && v.speed < 0.2f) {
                        return false;
                    }
                    break;
            }
        }
    }
    return true;
}

void IntersectionManager::checkDeadlock(float dt) {
    boxOccupantCount = 0;
    int stoppedInBoxNS = 0;
    int stoppedInBoxEW = 0;
    starvationDetected = false;

    for (auto& v : vehicles) {
        if (v.isInIntersectionBox()) {
            boxOccupantCount++;
            if (v.boxWaitTime >= DEADLOCK_WAIT_THRESHOLD) {
                if (v.dir == DIR_SOUTH || v.dir == DIR_NORTH) stoppedInBoxNS++;
                else stoppedInBoxEW++;
            }
        }

        if (v.isStopped && v.totalWaitTime >= STARVATION_THRESHOLD) {
            starvationDetected = true;
        }
    }

    // Deadlock condition: conflicting vehicles immobilized inside the intersection box
    bool currentlyStuck = (stoppedInBoxNS > 0 && stoppedInBoxEW > 0) ||
                          (boxOccupantCount >= 3 && (stoppedInBoxNS + stoppedInBoxEW >= 2));

    if (currentlyStuck) {
        if (!isDeadlocked) {
            isDeadlocked = true;
            deadlockDuration = 0.0f;
            deadlocksDetectedCount++;
        }
        deadlockDuration += dt;

        // Highlight trapped vehicles
        for (auto& v : vehicles) {
            if (v.isInIntersectionBox() && v.boxWaitTime >= 0.5f) {
                v.inDeadlock = true;
            }
        }

        // Auto-resolve after threshold
        if (deadlockDuration >= DEADLOCK_AUTO_RESOLVE_DELAY && !resolvingDeadlock) {
            resolveDeadlock();
        }
    } else {
        if (boxOccupantCount == 0 || (stoppedInBoxNS == 0 && stoppedInBoxEW == 0)) {
            if (isDeadlocked && resolvingDeadlock) {
                deadlocksResolvedCount++;
            }
            isDeadlocked = false;
            resolvingDeadlock = false;
            deadlockDuration = 0.0f;
            for (auto& v : vehicles) {
                v.inDeadlock = false;
            }
        }
    }
}

void IntersectionManager::resolveDeadlock() {
    resolvingDeadlock = true;
    resolveTimer = 0.0f;

    // Prioritize clearing whichever direction has stuck vehicles
    bool hasNS = false, hasEW = false;
    for (const auto& v : vehicles) {
        if (v.isInIntersectionBox()) {
            if (v.dir == DIR_SOUTH || v.dir == DIR_NORTH) hasNS = true;
            if (v.dir == DIR_EAST || v.dir == DIR_WEST) hasEW = true;
        }
    }

    if (hasNS) {
        lightController.setPhase(PHASE_NS_GREEN, 8.0f);
    } else if (hasEW) {
        lightController.setPhase(PHASE_EW_GREEN, 8.0f);
    } else {
        lightController.setPhase(PHASE_NS_GREEN, 6.0f);
    }

    for (auto& v : vehicles) {
        v.inDeadlock = false;
    }
}

void IntersectionManager::forceDeadlockScenario(double currentTime) {
    // 4-Way Gridlock Simulation Demonstration
    clearAllVehicles();

    // 1. Heading South
    Vehicle vSouth(nextVehicleId++, DIR_SOUTH, currentTime);
    vSouth.x = LANE_SOUTH_X;
    vSouth.y = ROAD_TOP + 40.0f;
    vSouth.speed = 0.0f;
    vSouth.targetSpeed = 0.0f;
    vSouth.boxWaitTime = DEADLOCK_WAIT_THRESHOLD + 0.5f;
    vSouth.isStopped = true;
    vSouth.hasPassedStopLine = true;
    vSouth.inDeadlock = true;
    vehicles.push_back(vSouth);

    // 2. Heading East
    Vehicle vEast(nextVehicleId++, DIR_EAST, currentTime);
    vEast.x = ROAD_LEFT + 40.0f;
    vEast.y = LANE_EAST_Y;
    vEast.speed = 0.0f;
    vEast.targetSpeed = 0.0f;
    vEast.boxWaitTime = DEADLOCK_WAIT_THRESHOLD + 0.5f;
    vEast.isStopped = true;
    vEast.hasPassedStopLine = true;
    vEast.inDeadlock = true;
    vehicles.push_back(vEast);

    // 3. Heading North
    Vehicle vNorth(nextVehicleId++, DIR_NORTH, currentTime);
    vNorth.x = LANE_NORTH_X;
    vNorth.y = ROAD_BOTTOM - 40.0f;
    vNorth.speed = 0.0f;
    vNorth.targetSpeed = 0.0f;
    vNorth.boxWaitTime = DEADLOCK_WAIT_THRESHOLD + 0.5f;
    vNorth.isStopped = true;
    vNorth.hasPassedStopLine = true;
    vNorth.inDeadlock = true;
    vehicles.push_back(vNorth);

    // 4. Heading West
    Vehicle vWest(nextVehicleId++, DIR_WEST, currentTime);
    vWest.x = ROAD_RIGHT - 40.0f;
    vWest.y = LANE_WEST_Y;
    vWest.speed = 0.0f;
    vWest.targetSpeed = 0.0f;
    vWest.boxWaitTime = DEADLOCK_WAIT_THRESHOLD + 0.5f;
    vWest.isStopped = true;
    vWest.hasPassedStopLine = true;
    vWest.inDeadlock = true;
    vehicles.push_back(vWest);

    isDeadlocked = true;
    deadlockDuration = 0.0f;
    deadlocksDetectedCount++;
}

void IntersectionManager::toggleAntiDeadlockGuard() {
    antiDeadlockGuard = !antiDeadlockGuard;
}

// ---------------------- Main Update Cycle ----------------------

void IntersectionManager::update(float dt, double currentTime) {
    // 1. Automatic Vehicle Spawning
    if (autoSpawnEnabled && !isDeadlocked) {
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

            // Anti-Deadlock "Don't Block the Box" Guard:
            // If green but receiving exit across intersection is blocked, treat light as red at stop line
            bool effectiveGreen = isGreen;
            if (isGreen && !v->hasPassedStopLine && antiDeadlockGuard) {
                if (!isBoxExitClear(v->dir, static_cast<float>(v->length))) {
                    effectiveGreen = false; // Hold at stop line to prevent box gridlock
                }
            }

            v->update(dt, distAhead, distStop, effectiveGreen, currentTime);

            if (v->inDetectionZone) {
                if (dir == DIR_SOUTH) countSouth++;
                else if (dir == DIR_NORTH) countNorth++;
                else if (dir == DIR_EAST) countEast++;
                else if (dir == DIR_WEST) countWest++;
            }
        }
    }

    // 4. Update Traffic Light Controller with live counts (unless resolving deadlock priority)
    if (!resolvingDeadlock) {
        lightController.update(dt, countNorth, countSouth, countEast, countWest);
    } else {
        lightController.timeRemaining -= dt;
        if (lightController.timeRemaining <= 0.0f) {
            resolvingDeadlock = false;
        }
    }

    // 5. Run Deadlock Detection & Starvation monitor
    checkDeadlock(dt);

    // 6. Clean up vehicles that have left the screen & accumulate metrics
    for (auto it = vehicles.begin(); it != vehicles.end(); ) {
        bool offScreen = false;
        if (it->dir == DIR_SOUTH && it->y > (SCREEN_HEIGHT + it->length * 2)) offScreen = true;
        if (it->dir == DIR_NORTH && it->y < (-it->length * 2)) offScreen = true;
        if (it->dir == DIR_EAST  && it->x > (SCREEN_WIDTH + it->length * 2)) offScreen = true;
        if (it->dir == DIR_WEST  && it->x < (-it->length * 2)) offScreen = true;

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

