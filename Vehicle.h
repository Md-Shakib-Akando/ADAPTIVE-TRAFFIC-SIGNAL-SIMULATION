#ifndef VEHICLE_H
#define VEHICLE_H

#include "Config.h"

class Vehicle {
public:
    int id;
    Direction dir;
    float x, y;
    float speed;
    float targetSpeed;
    int color;
    int vehicleType; // 0: Sedan, 1: SUV, 2: Bus/Truck

    double spawnTime;
    double totalWaitTime;
    bool isStopped;
    bool hasPassedStopLine;
    bool hasCleared;
    bool inDetectionZone;

    Vehicle(int vehicleId, Direction direction, double currentTime);

    void update(float dt, float distToAhead, float distToStopLine, bool isGreen, double currentTime);
    void draw() const;

    // Helper geometry getters
    float getFrontBumperPos() const;
    float getRearBumperPos() const;
    bool checkInDetectionZone() const;
    bool checkPassedStopLine() const;
    bool checkClearedIntersection() const;
};

#endif // VEHICLE_H
