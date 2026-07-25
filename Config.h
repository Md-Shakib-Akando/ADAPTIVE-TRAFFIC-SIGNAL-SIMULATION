#ifndef CONFIG_H
#define CONFIG_H

#include <graphics.h>

// Screen Dimensions
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 750;
const int HUD_X_START = 710;

// Intersection Geometry
const int CENTER_X = 355;
const int CENTER_Y = 375;
const int ROAD_WIDTH = 160; // 2 lanes (80px per lane)
const int LANE_WIDTH = 80;

// Road boundaries
const int ROAD_LEFT   = CENTER_X - ROAD_WIDTH / 2; // 275
const int ROAD_RIGHT  = CENTER_X + ROAD_WIDTH / 2; // 435
const int ROAD_TOP    = CENTER_Y - ROAD_WIDTH / 2; // 295
const int ROAD_BOTTOM = CENTER_Y + ROAD_WIDTH / 2; // 455

// Lane Centerlines
const int LANE_SOUTH_X = CENTER_X - LANE_WIDTH / 2; // 315 (Top approach, moving down)
const int LANE_NORTH_X = CENTER_X + LANE_WIDTH / 2; // 395 (Bottom approach, moving up)
const int LANE_EAST_Y  = CENTER_Y - LANE_WIDTH / 2; // 335 (Left approach, moving right)
const int LANE_WEST_Y  = CENTER_Y + LANE_WIDTH / 2; // 415 (Right approach, moving left)

// Stop Lines (where vehicles stop when signal is red)
const int STOP_LINE_SOUTH = ROAD_TOP;    // Y = 295
const int STOP_LINE_NORTH = ROAD_BOTTOM; // Y = 455
const int STOP_LINE_EAST  = ROAD_LEFT;   // X = 275
const int STOP_LINE_WEST  = ROAD_RIGHT;  // X = 435

// Detection Zone length (pixels back from stop line)
const int DETECTION_ZONE_LENGTH = 180;

// Vehicle Dimensions & Physics
const int CAR_LENGTH = 36;
const int CAR_WIDTH  = 22;
const float SAFE_FOLLOW_DISTANCE = 16.0f; // Extra gap between front bumper and rear bumper
const float MAX_CAR_SPEED = 3.2f;
const float ACCEL_RATE = 0.08f;
const float DECEL_RATE = 0.16f;

// Traffic Light Timing Constants (Seconds)
const float BASE_GREEN_TIME = 6.0f;
const float MIN_GREEN_TIME  = 4.0f;
const float MAX_GREEN_TIME  = 20.0f;
const float YELLOW_TIME     = 3.0f;
const float TIME_PER_CAR    = 1.5f; // Extra green seconds per waiting car in adaptive mode
const float FIXED_GREEN_TIME = 8.0f;

enum Direction {
    DIR_SOUTH = 0, // Moving from top to bottom
    DIR_NORTH = 1, // Moving from bottom to top
    DIR_EAST  = 2, // Moving from left to right
    DIR_WEST  = 3  // Moving from right to left
};

enum SignalPhase {
    PHASE_NS_GREEN  = 0,
    PHASE_NS_YELLOW = 1,
    PHASE_EW_GREEN  = 2,
    PHASE_EW_YELLOW = 3
};

enum ControlMode {
    MODE_ADAPTIVE = 0,
    MODE_FIXED    = 1
};

#endif // CONFIG_H
