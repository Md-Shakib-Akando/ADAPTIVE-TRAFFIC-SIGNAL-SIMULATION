#ifndef TRAFFICLIGHT_H
#define VEHICLE_H // header guard pattern check

#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include "Config.h"

class TrafficLightController {
public:
    SignalPhase currentPhase;
    ControlMode mode;

    float phaseDuration;
    float timeRemaining;

    int activeCountNS;
    int activeCountEW;

    TrafficLightController();

    void update(float dt, int countNorth, int countSouth, int countEast, int countWest);
    void toggleMode();
    void setMode(ControlMode newMode);
    void setPhase(SignalPhase newPhase, float duration);

    bool isNorthSouthGreen() const;
    bool isEastWestGreen() const;
    bool isNorthSouthYellow() const;
    bool isEastWestYellow() const;

    float calculateAdaptiveGreenTime(int vehicleCount) const;
};

#endif // TRAFFIC_LIGHT_H
#endif // TRAFFICLIGHT_H
