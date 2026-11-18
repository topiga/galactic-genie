// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "genie_ai_timing.h"
#include <cstdint>
namespace swgb {
struct GeniePlayerConstructorMetadata {
    uint32_t infoRevisitTime=0,infoFoodRefreshTime=0,infoKnownRefreshTime=0;
};
// Entry is the full CRT state at594f00, not the outer saved15-bit draw.
// Original583a26/583a59/583a89 supply the three constructor-prefix draws.
inline GeniePlayerConstructorMetadata genieInfoConstructorMetadata(uint32_t state) {
    const uint32_t wrapped=genieRandomDraw(state)*150000u;
    const int64_t signedProduct=wrapped<0x80000000u?int64_t(wrapped):int64_t(wrapped)-0x100000000ll;
    const uint32_t known=uint32_t(signedProduct/32767);
    const uint32_t revisit=uint32_t(uint64_t(genieRandomDraw(state))*60000u/32767u);
    const uint32_t food=uint32_t(uint64_t(genieRandomDraw(state))*7500u/32767u);
    return {revisit,food,known};
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
// Actual61e007 startup prefix before computer owner2: fresh Gaia, human owner1,
// controller mode0/default shared PER. Proven with 2–8 players and CC civs1..8.
// Continue with geniePlayerConstructorWithMetadata once per computer in owner order.
// This seed is independent of the RMS seed.
inline uint32_t genieNormalSessionComputerConstructorState(uint32_t sessionSeed) {
    uint32_t state=sessionSeed;
    genieRandomDraw(state); //61e057 initial checkpoint
    genieRandomDraw(state); //61e0a2 after fresh map allocation (no RNG)
    geniePlayerConstructorRandom(state); //Gaia common constructor
    genieRandomDraw(state); //5d5be9 Gaia-specific timer
    genieRandomDraw(state); //61e0e5 after Gaia checkpoint
    geniePlayerConstructorRandom(state); //human owner1 has no InfoAI
    // Shared PER setup has no RNG; the next owner's outer draw precedes594f00.
    return state;
}
} // namespace swgb
