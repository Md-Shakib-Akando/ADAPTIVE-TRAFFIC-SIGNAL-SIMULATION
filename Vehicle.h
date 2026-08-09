#ifndef VEHICLE_H
#define VEHICLE_H

#include "Config.h"

class Vehicle {
public:
    int id;
    Direction dir;
    VehicleType type;
    float x, y;
    int length;
    int width;
    float speed;
    float targetSpeed;
    float maxSpeed;
    float accelRate;
    float decelRate;
    int color;
    int secondaryColor;
    int accentColor;

    double spawnTime;
    double totalWaitTime;
    float boxWaitTime;
    bool isStopped;
    bool hasPassedStopLine;
    bool hasCleared;
    bool inDetectionZone;
    bool inDeadlock;

    Vehicle(int vehicleId, Direction direction, double currentTime);

    void update(float dt, float distToAhead, float distToStopLine, bool isGreen, double currentTime);
    void draw() const;

    // Helper geometry getters
    float getFrontBumperPos() const;
    float getRearBumperPos() const;
    bool checkInDetectionZone() const;
    bool checkPassedStopLine() const;
    bool checkClearedIntersection() const;
    bool isInIntersectionBox() const;

private:
    void drawSedan(int left, int top, int right, int bottom, bool vertical) const;
    void drawSUV(int left, int top, int right, int bottom, bool vertical) const;
    void drawBus(int left, int top, int right, int bottom, bool vertical) const;
    void drawBike(int left, int top, int right, int bottom, bool vertical) const;
    void drawTruck(int left, int top, int right, int bottom, bool vertical) const;
    void drawTaxi(int left, int top, int right, int bottom, bool vertical) const;
    void drawAmbulance(int left, int top, int right, int bottom, bool vertical) const;
};

#endif // VEHICLE_H
