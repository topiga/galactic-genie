// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "genie_random.h"
#include <cstdint>
namespace swgb {
struct GeniePlayerConstructorMetadata {
    uint32_t infoRevisitTime=0,infoFoodRefreshTime=0;
};
// Entry is the full CRT state at594f00, not the outer saved15-bit draw.
// Original583a26/583a59/583a89 supply the three constructor-prefix draws.
inline GeniePlayerConstructorMetadata genieInfoConstructorMetadata(uint32_t state) {
    genieRandomDraw(state);
    const uint32_t revisit=uint32_t(uint64_t(genieRandomDraw(state))*60000u/32767u);
    const uint32_t food=uint32_t(uint64_t(genieRandomDraw(state))*7500u/32767u);
    return {revisit,food};
}
inline uint32_t genieInfoConstructorRevisitTime(uint32_t state) {
    return genieInfoConstructorMetadata(state).infoRevisitTime;
}
// Retain nested metadata without changing ambientgeniePlayerConstructorRandom:
// the AI constructor runs after the outer draw, then5cde79 restores that draw.
inline GeniePlayerConstructorMetadata geniePlayerConstructorWithMetadata(uint32_t &state) {
    const uint32_t saved=genieRandomDraw(state);
    const auto result=genieInfoConstructorMetadata(state);
    state=saved;
    genieRandomDraw(state);
    return result;
}
} // namespace swgb
