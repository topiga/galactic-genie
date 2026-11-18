// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "power_field_checks.h"
static void powerGameplayChecks(swgb::Assets &assets) {
    using namespace swgb;
    powerFieldChecks();
    Scenario scenario;scenario.map.width=scenario.map.height=64;scenario.map.tiles.resize(64*64);
    scenario.playerSlotCount=2;scenario.nextUnitId=1;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i) {
        scenario.players[i].active=true;scenario.players[i].human=true;scenario.players[i].civilization=1;
        scenario.players[i].diplomacy.fill(0);
    }
    std::string error;
    const auto spawn=[](Game &g,int id,int owner,float ox,float oy) {
        return g.spawnObjectForTesting(1,id,owner,oy,64-ox);
    };
    Game game(assets);check(game.initScenario(scenario,&error),error.c_str());
    const int radius=std::min(20,int(game.playerAttributeForTesting(1,18)));
    check(radius>=1 && radius<20,"Original core radius permits boundary fixture");
    const auto core=spawn(game,12,1,24.5f,24.5f);
    const auto edge=spawn(game,84,1,24.5f+radius,24.5f);
    const auto outside=spawn(game,84,1,25.5f+radius,24.5f);
    const auto ally=spawn(game,84,2,24.5f+radius,24.5f);
    check(core&&edge&&outside&&ally,"Power boundary objects created");
    check(!game.objectPoweredForTesting(edge),"New source does not mutate previous world power snapshot");
    game.updatePaused(1,{});
    check(!game.objectPoweredForTesting(edge),"Pause does not rebuild power field");
    game.update(.1f,{});
    check(game.objectPoweredForTesting(edge)&&!game.objectPoweredForTesting(outside),
          "Power reaches integer stencil edge but does not add collision footprint");
    check(!game.objectPoweredForTesting(ally),"Own-only power query does not borrow allied field");
    check(game.damageObjectForTesting(core,100000),"Destroy power source");
    check(game.objectPoweredForTesting(edge),"Destroyed source remains in current world snapshot until next step");
    game.updatePaused(1,{});
    check(game.objectPoweredForTesting(edge),"Pause preserves pre-destruction power snapshot");
    game.update(.1f,{});
    check(!game.objectPoweredForTesting(edge),"Next admitted world step clears destroyed source field");

    Game construction(assets);check(construction.initScenario(scenario,&error),error.c_str());
    const auto foundation=construction.spawnFoundationForTesting(1,12,1,24.5f,39.5f,{});
    const auto receiver=spawn(construction,84,1,26.5f,24.5f);
    check(foundation&&receiver,"Power construction fixture");
    construction.update(.1f,{});
    check(!construction.objectPoweredForTesting(receiver),"Incomplete core emits no power");
    check(construction.completeFoundationForTesting(foundation),"Finish core through building completion");
    check(!construction.objectPoweredForTesting(receiver),"Completion waits for next power snapshot");
    // The helper finishes work; the scheduled owner callback performs actual
    // building completion after this world's power snapshot has been captured.
    for(int n=0;n<20&&construction.underConstructionObjectCount();++n)construction.update(.1f,{});
    check(construction.underConstructionObjectCount()==0,"Scheduled owner callback completes core");
    check(!construction.objectPoweredForTesting(receiver),"Completion after snapshot cannot emit in the same world step");
    construction.update(.1f,{});
    check(construction.objectPoweredForTesting(receiver),"Completed core emits on following world step");

    const auto &dat=assets.dat();
    // CC has no radius research. This authored DAT-resource fixture verifies
    // loading a different radius, without inventing an original technology.
    {
        auto &loadedRadius=const_cast<swgb::dat::DatFile&>(assets.dat()).civs[1].resources[18];
        struct RestoreRadius {float &value,saved;~RestoreRadius(){value=saved;}} restore{loadedRadius,loadedRadius};
        loadedRadius=float(radius+2)+.75f;
        Game altered(assets);check(altered.initScenario(scenario,&error),error.c_str());
        const auto alteredCore=spawn(altered,12,1,24.5f,24.5f);
        const auto alteredReceiver=spawn(altered,84,1,25.5f+radius,24.5f);
        check(alteredCore&&alteredReceiver,"Authored loaded-radius fixture");
        check(altered.playerAttributeForTesting(1,18)==loadedRadius,"Loaded DAT radius reaches actual owner provider");
        altered.update(.1f,{});
        check(altered.objectPoweredForTesting(alteredReceiver),"Loaded radius expands original stencil beyond default boundary");
    }

    int droidId=-1,transportId=-1;
    for(const auto &unit:dat.civs[1].units)if(unit.cls==61&&unit.speed>0&&unit.id>=0&&size_t(unit.id)<dat.unitHeaders.size()) {
        for(const auto &task:dat.unitHeaders[size_t(unit.id)].tasks)if(task.actionType==3)
            for(const auto &container:dat.civs[1].units)
                if(container.speed>0&&container.garrisonCapacity>0&&container.flyMode>0&&
                   ((task.classId>=0&&task.classId==container.cls)||(task.unitId>=0&&task.unitId==container.id))) {
                    droidId=unit.id;transportId=container.id;break;
                }
        if(droidId>=0)break;
    }
    check(droidId>=0&&transportId>=0,"Original DAT provides power droid and compatible transport");
    Game droid(assets);check(droid.initScenario(scenario,&error),error.c_str());
    const auto portable=spawn(droid,droidId,1,24.5f,24.5f);
    const auto nearby=spawn(droid,84,1,24.5f,26.5f);
    const auto transport=spawn(droid,transportId,1,24.5f,24.5f);
    check(portable&&nearby&&transport,"Portable power fixture");droid.update(.1f,{});
    check(droid.objectPoweredForTesting(nearby),"Class61 droid emits owner resource30 field");
    check(droid.garrisonForTesting(portable,transport),"Board power droid into compatible transport");
    for(int n=0;n<300&&droid.garrisonContainerForTesting(portable)<0;++n)droid.update(.1f,{});
    check(droid.garrisonContainerForTesting(portable)==int32_t(transport),"Power droid reaches transport");
    droid.update(.1f,{});
    check(!droid.objectPoweredForTesting(nearby),"Contained class61 droid no longer emits power");
    for(int count:{2,3}) {
        Game facts(assets);check(facts.initScenario(scenario,&error),error.c_str());
        check(spawn(facts,12,1,24.5f,24.5f)!=0,"Allied core for unpowered PER fact");
        for(int i=0;i<count;++i)check(spawn(facts,84,2,24.5f+2*i,24.5f)!=0,"Owned unpowered production building");
        check(spawn(facts,68,2,24.5f,27.5f)!=0,"Excluded unpowered building does not inflate count");
        check(facts.loadAiSourceForTesting(2,"original-unpowered",R"(
(defrule (strategic-number sn-max-unpowered-buildings == 2) => (set-goal 932 1))
(defrule (too-many-unpowered-buildings) => (set-goal 930 1))
(defrule (not (too-many-unpowered-buildings)) => (set-goal 931 1))
)",{},&error),error.c_str());
        for(int n=0;n<12;++n)facts.update(.1f,{});
        check(facts.aiGoalForTesting(2,932)==1,"Original SN162 default is2");
        check(facts.aiGoalForTesting(2,930)==(count>2) && facts.aiGoalForTesting(2,931)==(count<=2),
              "PER strict2/3 unpowered threshold ignores allied power and excluded prototypes");
    }

}
