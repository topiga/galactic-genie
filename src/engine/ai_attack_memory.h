// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace swgb {

// Original5a5bd0 event513, after object eligibility and victim-ID dedup,
// through5a6029. This is only the military-group gate, NOT complete admission:
// caller must retain notification timing, dedup, diplomacy and owner checks.
// groupType=-1 represents no group. SN168 is tactical+594.
inline bool aiAttackMemoryGroupAllows(int groupType,int groupState,int sn168) {
    if(groupType==101 || groupType==109 || groupType==103)return false;
    return !(sn168!=0 && groupState==2 &&
             (groupType==100 || groupType==102 || groupType==108));
}

struct AiAttackMemoryNotification {
    uint32_t victimId=0;
    bool victimExists=false,attackerExists=false;
    int owner=0,attackerOwner=0,attackerMasterType=0;
    int groupType=-1,groupState=0,sn168=0;
};

// Counter-event admission for original event513 through5a637b. This does NOT
// implement the intervening diplomacy/strategy mutations or emit actor events.
// The caller supplies an actual queued owner notification, not an arbitrary hit.
// Reset processedVictims at admitted59dcd0, before its tactical work, not on
// every render/frame or at the end of a complete military scheduling cycle.
inline bool admitAiAttackMemoryNotification(
        std::vector<uint32_t> &processedVictims,const AiAttackMemoryNotification &event) {
    if(!event.attackerExists || event.attackerOwner==event.owner ||
       event.attackerMasterType==60 || !event.victimExists)return false;
    if(std::find(processedVictims.begin(),processedVictims.end(),event.victimId)!=processedVictims.end())
        return false;
    // Original consumes dedup before military-group and Gaia rejection.
    processedVictims.push_back(event.victimId);
    return aiAttackMemoryGroupAllows(event.groupType,event.groupState,event.sn168) &&
           event.attackerOwner!=0;
}

struct AiAttackMemoryQuarter {
    uint32_t explored=0,received=0,issued=0;
};
enum class AiAttackMemoryResult { Ignored,Recorded,UnsupportedQuarter };

// Original58f260 entry through58f30e; event-buffer insertion is separate.
// Arguments2/4 are rectangle X endpoints in original coordinates. The original
// reuses their byte sum for BOTH quarter axes; Y is intentionally absent here.
// ownerA/ownerB are event args6/7. Receiver takes precedence when both are self.
inline AiAttackMemoryResult recordAiAttackMemoryQuarter(
        std::array<AiAttackMemoryQuarter,16> &quarters,int width,int height,
        int32_t x1,int32_t x2,int owner,int ownerA,int ownerB) {
    if(width<=0 || height<=0)return AiAttackMemoryResult::Ignored;
    const int sum=4*(int(uint8_t(x1))+int(uint8_t(x2)));
    int qx=(sum/width)/2,qy=(sum/height)/2;
    if(qx>4)qx=3;
    if(qy>4)qy=3;
    const int index=4*qx+qy;
    const bool received=int(uint8_t(ownerB))==owner;
    const bool issued=int(uint8_t(ownerA))==owner;
    if(!received && !issued)return AiAttackMemoryResult::Ignored;
    // Original permits coordinate4 and can write beyond the16-record table.
    // Reject that unsupported input explicitly instead of emulating corruption.
    if(index>=int(quarters.size()))return AiAttackMemoryResult::UnsupportedQuarter;
    auto &quarter=quarters[std::size_t(index)];
    if(received)++quarter.received;
    else ++quarter.issued;
    return AiAttackMemoryResult::Recorded;
}
} // namespace swgb
