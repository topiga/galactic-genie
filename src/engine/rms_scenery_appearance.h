// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>

namespace swgb {
struct RmsCarbonAppearance {
    uint8_t facet;
    float headingRadians;
};
// Original fresh type30 carbon deposits217/1341 only, after successful placement.
// Requires validated graphics5140/5147: flags6, one frame, twelve angles, no sounds.
// Consumes base facet, sprite frame, then heading draws. Does not create a world object.
RmsCarbonAppearance drawRmsCarbonAppearance(uint32_t &randomState);
}
