#ifndef HUD_H
#define HUD_H

#include "Config.h"
#include "Intersection.h"

class HUD {
public:
    HUD();

    void drawScenery() const;
    void drawDetectionZones(const IntersectionManager& manager) const;
    void drawTrafficLights(const TrafficLightController& lightController) const;
    void drawDashboard(const IntersectionManager& manager, float fps, bool isPaused) const;
    void drawDeadlockOverlay(const IntersectionManager& manager) const;

private:
    void drawTrafficLightPost(int poleX, int poleY, int boxX, int boxY,
                              bool isRed, bool isYellow, bool isGreen,
                              const char* directionLabel, float timeRemaining, bool isHorizontal = false) const;
    void drawTree(int x, int y, int radius) const;
    void drawStreetLight(int x, int y, bool onLeft) const;
    void drawDirectionArrow(int x, int y, Direction dir) const;
};

#endif // HUD_H

