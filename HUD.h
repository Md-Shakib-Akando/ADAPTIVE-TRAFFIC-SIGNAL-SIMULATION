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
    void drawTrafficLightBox(int x, int y, bool isRed, bool isYellow, bool isGreen) const;
};

#endif // HUD_H
