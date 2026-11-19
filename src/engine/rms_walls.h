// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace swgb {
struct PathMapFootprint {
    int left, top, right, bottom; // Exclusive right and bottom, in original map axes.
};
inline PathMapFootprint pathMapFootprint(float x,float y,float radiusX,float radiusY) {
    return {int(double(x)-radiusX),int(double(y)-radiusY),
            int(double(x)+radiusX),int(double(y)+radiusY)};
}
struct RmsWallObject {
    int owner = 0;
    float x = 0, y = 0, radiusX = 0, radiusY = 0;
    int obstructionType = 0, obstructionClass = 0, unitClass = 0;
};
struct RmsWallStart {
    int owner = 0;
    float x = 0, y = 0, radiusX = 0, radiusY = 0;
};
struct RmsWallInput {
    int width = 0, height = 0;
    std::vector<uint8_t> terrain; // Row-major, after starting-object foundation callbacks.
    std::vector<float> restrictions;
    std::vector<RmsWallObject> objects; // Creation order, before any wall/gate objects.
    std::vector<RmsWallStart> commandCenters;
    int minimumDistance = 16, maximumDistance = 20;
};
struct RmsWallSegment {
    int owner = 0, fromX = 0, fromY = 0, toX = 0, toY = 0, length = 0;
    bool active = false;
};
// Retained intermediate grids support differential checks against the original controller.
struct RmsWallBoundary {
    std::vector<std::vector<RmsWallSegment>> segments; // Owner index; original prepended-list order.
    std::vector<uint32_t> obstruction; // Row-major.
    std::vector<uint8_t> directions; // X-major; only the byte consumed by this controller.
    std::vector<std::vector<uint8_t>> routeBoundaries; // Eight successive route stages.
    std::vector<uint8_t> reachableScratch, reachableBoundary;
    std::vector<uint8_t> prunedScratch, prunedBoundary, segmentScratch;
};
// Verified startup cache reconstruction; later terrain edits/removals need ordered update history.
// Leaves output unchanged on failure; reports bounded-profile rejection through error.
bool generateRmsWallBoundary(const RmsWallInput& input, RmsWallBoundary& output, std::string* error = nullptr);
}
