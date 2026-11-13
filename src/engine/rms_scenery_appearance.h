// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>

namespace swgb {
struct RmsCarbonAppearance {
    uint8_t facet;
    float headingRadians;
};
// Original fresh type30 deposits217/1341/66/102 after successful placement.
// Requires validated graphics5140/5147/5134/5232: flags6, one frame, ten or twelve angles, no sounds.
// Consumes base facet, sprite frame, then heading draws. Does not create a world object.
RmsCarbonAppearance drawRmsCarbonAppearance(uint32_t &randomState, uint8_t angleCount=12);
}
