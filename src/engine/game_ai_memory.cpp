// SPDX-License-Identifier: GPL-3.0-or-later
#include "game.h"
#include "ai_attack_memory.h"
#include "../core/genie_random.h"
#include <algorithm>
#include <cmath>

namespace swgb {
void Game::notifyAiAttackMemory(Object &victim,const Object *attacker) {
    // Original4448ab emits actor500 only with a source; sourceless damage
    // follows owner530 instead. Gaia's owner/controller path is separate.
    if(!attacker || !victim.unit || !attacker->unit || victim.player<=0 ||
       size_t(victim.player)>=aiPlayers_.size())return;
    // Original5d2fda/5d3034 wraps normal-owner notifications even without AI:
    // draw once, run callbacks, then seed with that15-bit draw (not old CRT).
    struct Restore {
        uint32_t &stream;uint32_t seed;
        ~Restore(){stream=seed;}
    } restore{legacyRandomState_,genieRandomDraw(legacyRandomState_)};
    auto &state=aiPlayers_[size_t(victim.player)];
    // shortcut: Native loaded AI is the MainAI-presence adapter. Original
    // queued actor500/527 dispatch and specialized actor emission remain
    // separate from this native post-damage callback; no full timing claim.
    if(!state.loaded)return;
    updateAiExplorationInformation(victim.player);
    AiAttackMemoryNotification event;
    event.victimId=victim.spawnId;
    event.victimExists=true;event.attackerExists=true;
    event.owner=victim.player;event.attackerOwner=attacker->player;
    event.attackerMasterType=int(attacker->unit->type);
    for(const auto &group:state.militaryGroups)
        if(std::find(group.members.begin(),group.members.end(),int32_t(victim.spawnId))!=group.members.end()) {
            event.groupType=group.type;event.groupState=group.state;break;
        }
    // Verified596d10 profile+2a4 ->597300 SN168 setter is50, notzero.
    const auto sn=state.strategicNumbers.find(168);
    event.sn168=sn==state.strategicNumbers.end()?50:sn->second;
    if(!admitAiAttackMemoryNotification(state.processedAttackMemoryVictims,event))return;
    const float originalX=float(mapSize_)-victim.y;
    // Counter reads only original X endpoints; point wrapper repeats X.
    // Actual map objects fit int32. Reject malformed imported coordinates.
    if(!std::isfinite(originalX) || originalX<double(INT32_MIN) || originalX>double(INT32_MAX))return;
    recordAiAttackMemoryQuarter(state.explorationQuarters,int(mapSize_),int(mapSize_),
                               int32_t(originalX),int32_t(originalX),victim.player,
                               attacker->player,victim.player);
}
} // namespace swgb
