// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "core/genie_start_placement.h"
#include <limits>

// Original5c92e0/48e180 controls in private
// rms-empty-owner-geometry-oracle-2026-11-18.json. Loaded original worker83
// master, synthetic32-square map; these verify primitives, not full gate policy.
static void checkStartPlacement() {
    using namespace swgb;
    GenieStartFootprint footprint;
    check(genieStartFootprint(16,16,.2f,.2f,32,32,footprint),"Starting footprint accepts an interior point");
    check(footprint.minX==15 && footprint.minY==15 &&
          footprint.maxX==16 && footprint.maxY==16,"Starting footprint covers inclusive terrain tiles");
    struct Bound {float x;bool accepted;};
    const Bound bounds[]={{.25f,true},{.2499999850988388f,false},
        {31.75f,true},{31.750999450683594f,false},{31.751001358032227f,false}};
    for (const auto &b:bounds)
        check(genieStartFootprint(b.x,16,.25f,.25f,32,32,footprint)==b.accepted,"Original float footprint boundary");
    check(!genieStartTerrainPassable(0),"Zero terrain multiplier rejects");
    check(!genieStartTerrainPassable(.05f),"Original terrain threshold is strict");
    check(genieStartTerrainPassable(.05000000447034836f),"Next float above terrain threshold accepts");
    struct Contact {float x,y;bool overlaps;};
    const Contact contacts[]={{16,16,true},{16.5f,16,false},
        {16.499998092651367f,16,true},{16.500001907348633f,16,false},
        {16,16.5f,false},{16.5f,16.5f,false},
        {16.499998092651367f,16.499998092651367f,true}};
    for (const auto &c:contacts)
        check(genieStartRectanglesOverlap(c.x,c.y,.25f,.25f,16,16,.25f,.25f)==c.overlaps,"Original strict footprint overlap");
    check(!genieStartFootprint(std::numeric_limits<float>::infinity(),16,.25f,.25f,32,32,footprint),"Nonfinite placement rejects before conversion");
    check(!genieStartFootprint(16,16,-.25f,.25f,32,32,footprint),"Negative footprint rejects");
}
