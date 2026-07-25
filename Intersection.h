#ifndef INTERSECTION_H
#define INTERSECTION_H

#include <vector>
#include "Vehicle.h"
#include "TrafficLight.h"

class IntersectionManager {
public:
    std::vector<Vehicle> vehicles;
    TrafficLightController lightController;

    int nextVehicleId;
    float spawnTimers[4];
    float spawnIntervals[4];

    // Detection Zone Live Vehicle Counts
    int countSouth; // Vehicles heading South in top detection zone
    int countNorth; // Vehicles heading North in bottom detection zone
    int countEast;  // Vehicles heading East in left detection zone
    int countWest;  // Vehicles heading West in right detection zone

    // Performance Metrics
    int totalClearedVehicles;
    double totalAccumulatedWaitTime;

    bool autoSpawnEnabled;

    IntersectionManager();

    void update(float dt, double currentTime);
    void drawVehicles() const;

    bool spawnVehicle(Direction dir, double currentTime);
    void triggerBurstSpawn(double currentTime);
    void clearAllVehicles();
    bool handleMouseClick(int mx, int my, double currentTime);

    float getAverageWaitTime() const;

private:
    float calculateDistanceToAhead(const Vehicle& curr, const Vehicle& ahead) const;
    float calculateDistanceToStopLine(const Vehicle& curr) const;
};

#endif // INTERSECTION_H
