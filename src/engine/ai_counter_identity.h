// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <optional>
#include <cstdint>
namespace swgb {
struct AiCounterIdentity { int prototype=-1, unitClass=-1; bool classCount=true; };
// Original4c4240/4c4350 normalize at admission/removal, not at fact lookup.
// Master byte+9f is DAT interfaceKind (serialized offset85), not interactionMode (+9d).
// Lookup returns the owner's effective master (including prototype upgrades).
template<class Lookup>
std::optional<AiCounterIdentity> aiCounterIdentity(int physicalId,Lookup lookup) {
    if(physicalId<0 || physicalId>=1600)return std::nullopt;
    const auto *unit=lookup(physicalId);
    if(!unit)return std::nullopt;
    int id=physicalId;
    if(unit->interfaceKind!=2 && unit->interfaceKind!=10 && unit->interfaceKind!=11)
        id=unit->copyId;
    unit=lookup(id);
    if(!unit)return std::nullopt;
    // ESI remains the pre-stack master at4c43d6/4c43df; only the
    // prototype and class lookup switch to the stack target.
    const bool classCount=!(unit->type==80 && unit->headUnit!=-1 && unit->annexes[0].unitId==-1);
    if(unit->type==80 && unit->stackUnitId!=-1) {
        id=unit->stackUnitId;unit=lookup(id);
        if(!unit)return std::nullopt;
    }
    return AiCounterIdentity{id,unit->cls,classCount};
}
inline int aiSignedPrototypeCount(uint32_t count) {
    const uint32_t low=count&65535u;
    return low<32768u?int(low):int(low)-65536;
}
}
