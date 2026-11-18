// SPDX-License-Identifier: GPL-3.0-or-later
// Original58ef40/58f100 controls: build-placement-position-registry-controls-
// 2026-11-18.json. Query branches:58934a/58947e,58f190,589cf6.
#include "../src/engine/ai_build_positions.h"
#include <cassert>
#include <cstdio>
#include <utility>

int main() {
    swgb::AiBuildPositions positions;
    assert(positions.slots().size()==200);
    for(const auto &s:positions.slots())assert(s.prototype==-1 && !s.status && !s.x && !s.y);
    positions.upsert(562,12,19,1);
    positions.upsert(562,12,19,2);
    positions.upsert(584,12,19,1);
    assert(positions.slots()[0].prototype==562 && positions.slots()[0].status==2);
    assert(positions.slots()[1].prototype==584 && positions.slots()[1].status==1);
    assert(positions.rejected(562,268,275));
    assert(!positions.rejected(584,12,19));
    assert(!positions.updateExisting(562,13,19,2));
    assert(positions.updateExisting(562,268,275,0));
    auto preset=positions.consumePreset(562);
    assert(preset && preset->x==12 && preset->y==19 && preset->status==1);
    assert(!positions.consumePreset(562));
    assert(positions.plannedNearby(562,22,29,10));
    assert(positions.plannedNearby(562,258,265,10));
    assert(!positions.plannedNearby(562,23,29,10));
    assert(!positions.plannedNearby(562,22,30,10));
    assert(!positions.plannedNearby(563,12,19,10));
    assert(positions.updateExisting(562,12,19,3));
    const auto &empty=positions.slots()[0];
    assert(empty.prototype==-1 && empty.status==3 && !empty.x && !empty.y);
    positions.upsert(562,268,275,257);
    assert(positions.slots()[0].prototype==562 && positions.slots()[0].status==1);
    assert(positions.slots()[0].x==12 && positions.slots()[0].y==19);
    // Original growth controls use two slots; fill the actual constructor's
    // 200 slots to exercise the same doubling/copy/initialization operations.
    for(int i=2;i<200;++i)positions.upsert(1000+i,i,0,1);
    positions.upsert(3000,7,8,2);
    assert(positions.slots().size()==400);
    assert(positions.slots()[0].prototype==562 && positions.slots()[1].prototype==584);
    assert(positions.slots()[199].prototype==1199);
    assert(positions.slots()[200].prototype==3000 && positions.slots()[200].status==2);
    for(size_t i=201;i<400;++i) {
        const auto &s=positions.slots()[i];
        assert(s.prototype==-1 && s.status==2 && !s.x && !s.y);
    }
    // Full-width IDs are retained by registry primitives. Production master
    // IDs are sign-extended shorts by the caller, not truncated by this class.
    positions.upsert(0x10001,30,30,0);
    assert(!positions.consumePreset(1));
    positions.upsert(0x10001,31,31,0);
    preset=positions.consumePreset(0x10001);
    assert(preset && preset->x==30);
    preset=positions.consumePreset(0x10001);
    assert(preset && preset->x==31);
    auto transferred=std::move(positions);
    assert(transferred.slots().size()==400 && transferred.rejected(3000,7,8));
    positions.upsert(562,12,19,1);
    assert(positions.slots().size()>=200 && positions.plannedNearby(562,12,19,0));
    swgb::AiBuildPositions destination;
    destination=std::move(positions);
    positions.upsert(584,20,21,2);
    assert(destination.plannedNearby(562,12,19,0));
    assert(positions.rejected(584,20,21));
    std::puts("AI planned-position registry checks passed");
}
