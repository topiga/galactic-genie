// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/engine/ai_counter_identity.h"
#include "../src/core/genie_dat.h"
#include <array>
[[gnu::noinline]] static void aiCounterIdentityChecks() {
    std::array<swgb::dat::Unit,3> masters;
    for(int i=0;i<3;++i) {masters[i].id=int16_t(10+i);masters[i].copyId=int16_t(10+i);masters[i].type=70;masters[i].cls=18;}
    const auto lookup=[&](int id)->const swgb::dat::Unit* {return id>=10 && id<=12?&masters[size_t(id-10)]:nullptr;};
    masters[0].copyId=11;
    // Four executed original4c2a70/4c4240/4c4350 controls: count2 moves to11
    // only for mode0; later removing physical10 addresses that same counter.
    for(int mode:{0,2,10,11}) {
        masters[0].interfaceKind=uint8_t(mode);
        masters[0].interactionMode=mode==0?2:0; // Deliberately different: +9d does not gate counters.
        const auto id=swgb::aiCounterIdentity(10,lookup);
        check(id && id->prototype==(mode==0?11:10),"Original counter identity follows copyId with interface-kind exceptions");
    }
    masters[0].interfaceKind=0;masters[0].copyId=-1;
    check(!swgb::aiCounterIdentity(10,lookup) && !swgb::aiCounterIdentity(-1,lookup) &&
          !swgb::aiCounterIdentity(1600,lookup),"Missing/out-of-range master has no counter admission");
    // Six original4c4240/4c4350 building controls. Head/annex suppression
    // reads the pre-stack master; prototype/class read the stack target.
    for(const auto &c:std::array<std::array<int,5>,6>{{
            {{-1,-1,-1,10,1}},{{-1,12,-1,10,0}},{{-1,12,10,10,1}},
            {{11,-1,-1,11,1}},{{11,12,-1,11,1}},{{12,-1,-1,-1,0}}}}) {
        for(int i=0;i<2;++i) {
            masters[i].type=80;masters[i].interfaceKind=2;
            masters[i].copyId=int16_t(10+i);masters[i].stackUnitId=-1;
            masters[i].headUnit=-1;masters[i].annexes[0].unitId=-1;
        }
        masters[0].stackUnitId=int16_t(c[0]);
        auto &decorated=masters[c[0]==11?1:0];
        decorated.headUnit=int16_t(c[1]);decorated.annexes[0].unitId=int16_t(c[2]);
        const auto boundedLookup=[&](int id)->const swgb::dat::Unit* {
            return id==12?nullptr:lookup(id);
        };
        const auto identity=swgb::aiCounterIdentity(10,boundedLookup);
        check(c[3]<0?!identity:identity && identity->prototype==c[3] &&
              identity->classCount==bool(c[4]),"Original building stack identity and pre-stack class suppression");
    }
    check(swgb::aiSignedPrototypeCount(32767)==32767 && swgb::aiSignedPrototypeCount(32768)==-32768 &&
          swgb::aiSignedPrototypeCount(65535)==-1 && swgb::aiSignedPrototypeCount(65536)==0,
          "Per-prototype counters retain signedword wrapping before aggregation");
}
[[gnu::noinline]] static void aiCounterIdentityGameplayChecks(swgb::Assets &assets) {
    auto &data=const_cast<swgb::dat::DatFile &>(assets.dat());
    const swgb::dat::UnitLine *line=nullptr;
    for(const auto &entry:data.unitLines)
        if(swgb::normalizeAiSymbol(entry.name)=="unit-laser-line")line=&entry;
    check(line && line->unitIds.size()>=2,"Asset-backed mobile upgrade line");
    const int source=line->unitIds[0],target=line->unitIds[1];
    auto &commands=data.effects[size_t(data.techs[501].effectId)].commands;
    struct Restore {std::vector<swgb::dat::EffectCommand> &commands,old;~Restore(){commands=std::move(old);}} restore{commands,commands};
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=64;scenario.map.tiles.resize(64*64);
    scenario.nextUnitId=1;scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i) {scenario.players[i].active=true;scenario.players[i].human=i==0;
        scenario.players[i].civilization=1;scenario.players[i].populationLimit=75;
        scenario.players[i].diplomacy.fill(3);scenario.players[i].diplomacy[size_t(i+1)]=0;}
    swgb::Game game(assets);std::string error;
    check(game.initScenario(scenario,&error),error.c_str());
    const auto actor=game.spawnObjectForTesting(1,source,1,20,20);
    check(actor && game.loadAiSourceForTesting(1,"counter-identities","",{},&error),"Initialize native counter fixture");
    check(game.aiObjectCountForTesting(1,std::to_string(source),false)==1 &&
          game.aiObjectCountForTesting(1,std::to_string(target),false)==0,"Before upgrade only source tier is counted");
    commands={{3,int16_t(source),int16_t(target),-1,0}};
    game.invalidateUnitPrototypesForTesting();
    check(game.researchTechnologyForTesting(1,501),"Actual native research applies master upgrade");
    check(game.objectUnitId(actor)==source && game.unitPrototypeForTesting(1,source)->copyId==target,
          "Upgrade preserves physical actor identity while changing counted master identity");
    check(game.aiObjectCountForTesting(1,std::to_string(source),false)==0 &&
          game.aiObjectCountForTesting(1,std::to_string(target),false)==1 &&
          game.aiObjectCountForTesting(1,"UNIT-LASER-LINE",false)==1 &&
          game.aiObjectCountForTesting(1,"UNIT-LASER-LINE",true)==1,
          "Current/total tiers and line sums use normalized identity without duplication");
    const std::string rules="(defrule (cc-players-unit-type-count 1 "+std::to_string(source)+
        " == 0) (cc-players-unit-type-count 1 "+std::to_string(target)+
        " == 1) (cc-players-unit-type-count 1 UNIT-LASER-LINE == 1) => (set-goal 990 1))";
    check(game.loadAiSourceForTesting(2,"tier-counter-observer",rules,{},&error),error.c_str());
    for(int i=0;i<25;++i)game.update(0,{});
    check(game.aiGoalForTesting(2,990)==1,"Production CC fact sees upgraded tier despite preserved physical ID");
    // Use the unmodified real age-up effect, including its building upgrades.
    commands=restore.old;
    check(game.initAridSkirmish(1,&error),error.c_str());
    for(int id:{68,562,87})check(game.spawnObjectForTesting(1,id,2,50+id%5*3,50+id%7*3),
                              "Create ordinary buildings before the real Tech Level2 upgrade");
    check(game.loadAiSourceForTesting(2,"age-up-building-counters","",{},&error),error.c_str());
    for(int id:{109,68,562,87})check(game.aiObjectCountForTesting(2,std::to_string(id),false)==1,
                                  "Original building identities are counted before age-up");
    check(game.researchTechnologyForTesting(2,1),"Apply the actual installed Tech Level2 research");
    for(int id:{109,68,562,87}) {
        const auto *master=game.unitPrototypeForTesting(2,id);
        check(master && master->id==id && master->copyId!=id && master->interfaceKind==(id==87?10:2) &&
              master->interactionMode==3,"Real upgraded buildings distinguish interface kind from interaction mode");
        check(game.aiObjectCountForTesting(2,std::to_string(id),false)==1 &&
              game.aiObjectCountForTesting(2,std::to_string(id),true)==1,
              "Real age-up retains current and total building counters under original identities");
    }
    check(game.loadAiSourceForTesting(2,"age-up-building-facts",
        "(defrule (building-type-count BLDG-MAIN == 1) (building-type-count BLDG-TRAINTROOPER == 1) "
        "(building-type-count-total BLDG-DROPCHOW == 1) => (set-goal 991 1))",{},&error),error.c_str());
    for(int i=0;i<25;++i)game.update(0,{});
    check(game.aiGoalForTesting(2,991)==1,"Original PER building facts survive real age-up without repeated depot requests");
}
