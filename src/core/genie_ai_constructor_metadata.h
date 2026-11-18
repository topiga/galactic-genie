// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "genie_random.h"
#include <cstdint>
namespace swgb {
// Entry state is the full CRT state at594f00, not the15-bit outer saved draw.
// Actual583a26/583a59 are the first two draws in this constructor prefix.
inline uint32_t genieInfoConstructorRevisitTime(uint32_t constructorState) {
    genieRandomDraw(constructorState);
    return uint32_t(uint64_t(genieRandomDraw(constructorState))*60000u/32767u);
}
struct GeniePlayerConstructorMetadata {
    uint32_t infoRevisitTime=0;
};
// Same ambient result asgeniePlayerConstructorRandom. The optional AI constructor
// runs on the state after the outer draw, then5cde79 restores the saved draw.
inline GeniePlayerConstructorMetadata geniePlayerConstructorWithMetadata(uint32_t &state) {
    const uint32_t saved=genieRandomDraw(state);
    const GeniePlayerConstructorMetadata result{genieInfoConstructorRevisitTime(state)};
    state=saved;
    genieRandomDraw(state);
    return result;
}
} // namespace swgb
