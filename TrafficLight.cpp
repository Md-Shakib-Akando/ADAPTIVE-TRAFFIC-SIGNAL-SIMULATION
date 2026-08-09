#include "TrafficLight.h"
#include <algorithm>

TrafficLightController::TrafficLightController() {
    currentPhase = PHASE_NS_GREEN;
    mode = MODE_ADAPTIVE;
    activeCountNS = 0;
    activeCountEW = 0;
    phaseDuration = BASE_GREEN_TIME;
    timeRemaining = phaseDuration;
}

float TrafficLightController::calculateAdaptiveGreenTime(int vehicleCount) const {
    if (mode == MODE_FIXED) {
        return FIXED_GREEN_TIME;
    }
    float dynamicTime = BASE_GREEN_TIME + (vehicleCount * TIME_PER_CAR);
    return std::max(MIN_GREEN_TIME, std::min(MAX_GREEN_TIME, dynamicTime));
}

void TrafficLightController::toggleMode() {
    if (mode == MODE_ADAPTIVE) {
        mode = MODE_FIXED;
    } else {
        mode = MODE_ADAPTIVE;
    }
}

void TrafficLightController::setMode(ControlMode newMode) {
    mode = newMode;
}

void TrafficLightController::setPhase(SignalPhase newPhase, float duration) {
    currentPhase = newPhase;
    phaseDuration = duration;
    timeRemaining = duration;
}

bool TrafficLightController::isNorthSouthGreen() const {
    return (currentPhase == PHASE_NS_GREEN);
}

bool TrafficLightController::isEastWestGreen() const {
    return (currentPhase == PHASE_EW_GREEN);
}

bool TrafficLightController::isNorthSouthYellow() const {
    return (currentPhase == PHASE_NS_YELLOW);
}

bool TrafficLightController::isEastWestYellow() const {
    return (currentPhase == PHASE_EW_YELLOW);
}

void TrafficLightController::update(float dt, int countNorth, int countSouth, int countEast, int countWest) {
    activeCountNS = countNorth + countSouth;
    activeCountEW = countEast + countWest;

    timeRemaining -= dt;

    if (timeRemaining <= 0.0f) {
        switch (currentPhase) {
            case PHASE_NS_GREEN:
                currentPhase = PHASE_NS_YELLOW;
                phaseDuration = YELLOW_TIME;
                timeRemaining = phaseDuration;
                break;

            case PHASE_NS_YELLOW:
                currentPhase = PHASE_EW_GREEN;
                phaseDuration = calculateAdaptiveGreenTime(activeCountEW);
                timeRemaining = phaseDuration;
                break;

            case PHASE_EW_GREEN:
                currentPhase = PHASE_EW_YELLOW;
                phaseDuration = YELLOW_TIME;
                timeRemaining = phaseDuration;
                break;

            case PHASE_EW_YELLOW:
                currentPhase = PHASE_NS_GREEN;
                phaseDuration = calculateAdaptiveGreenTime(activeCountNS);
                timeRemaining = phaseDuration;
                break;
        }
    }
}
