// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_start_placement_policy.h"
#include <cassert>

// Original calls recorded in rms-empty-owner-{cc,worker}-policy-oracle-2026-11-18.json.
static void checkStartPlacementPolicy() {
    using namespace swgb;
    GenieStartObstacle o;o.id=83;o.type=70;o.unitClass=58;o.owner=2;
    o.halfX=o.halfY=.2f;o.height=2;o.speed=.8f;
    assert(genieStartCenterObstacle(o,1,false,0));
    assert(!genieStartCenterObstacle(o,1,true,0));
    assert(genieStartCenterObstacle(o,1,true,6));
    o.owner=0;assert(!genieStartCenterObstacle(o,1,false,0));
    o.owner=1;assert(!genieStartCenterObstacle(o,1,false,0));
    o.owner=2;assert(genieStartWorkerObstacle(o));
    o.type=80;o.unitClass=3;o.speed=0;o.pendingBuilding=true;
    assert(!genieStartCenterObstacle(o,1,false,0));
    assert(genieStartCenterObstacle(o,1,true,0));
    assert(!genieStartWorkerObstacle(o));
    o.pendingBuilding=false;o.height=0;
    assert(genieStartWorkerObstacle(o));
    o.type=10;o.unitClass=31;
    assert(!genieStartWorkerObstacle(o));
    o.height=2;o.state=3;o.hasTile=true;
    for (int graphic=9;graphic<=22;++graphic) {
        o.terrainId=graphic;
        const bool expected=graphic==10 || graphic==13 || (graphic>=17 && graphic<=21);
        assert(genieStartCenterObstacle(o,1,false,0)==expected);
    }
    o.state=7;assert(!genieStartWorkerObstacle(o));
    assert(!genieStartCenterObstacle(o,1,false,0));
    o.state=2;o.canBeBuiltOn=true;
    assert(!genieStartWorkerObstacle(o));
    assert(!genieStartCenterObstacle(o,1,false,0));
    o.canBeBuiltOn=false;o.type=70;o.unitClass=27;o.height=0;
    assert(genieStartWorkerObstacle(o));
}
