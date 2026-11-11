// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/engine/game.h"
#include "../src/render/soft_renderer.h"
#include <algorithm>
#include <cstdio>
#include <stdexcept>

static void check(bool value, const char *message) {
    if (!value) throw std::runtime_error(message);
}
#include "memory_inspection_checks.h"
#include "rubble_checks.h"
#include "healing_checks.h"
#include "combat_modifier_checks.h"
#include "ai_fog_checks.h"
#include "selection_checks.h"
#include "civilization_checks.h"
#include "conquest_checks.h"
#include "fog_edge_checks.h"

static void fogRendering(swgb::Assets &assets,swgb::SoftRenderer &renderer) {
    swgb::Game game(assets);
    std::string error;
    check(game.initSkirmish(0x5A17,&error),error.c_str());
    // Restart must not carry explored terrain from a previous setup choice.
    for(bool explored:{true,false,true,false}) {
        check(game.initSkirmish(0x5A17,&error,explored),error.c_str());
        check(game.tileVisibilityForTesting(60,2)==(explored?1:0),"Reveal-map choice survives initialization and resets prior exploration");
        check(!game.observedForTesting(1,60,2),"Explored terrain does not grant actual observation");
        check(!game.objectVisibleForTesting(game.commandCenterId(2)),"Neither map mode reveals unseen enemy buildings");
    }
    check(game.initSkirmish(0x5A17,&error),error.c_str());
    game.setLocalPlayerForTesting(3);
    game.setClassicHud(true);
    game.lookAt(30.5f,30.5f);
    const auto mobile=game.spawnObjectForTesting(1,83,2,30.5f,30.5f);
    check(mobile!=0,"Hidden mobile render fixture");
    game.render(renderer,960,544);
    const auto hidden=renderer.pixels();
    check(game.moveObjectForTesting(mobile,32.5f,30.5f),"Move unseen mobile");
    game.render(renderer,960,544);
    check(renderer.pixels()==hidden,"Unseen mobile motion leaks no world or minimap pixels");

    const auto building=game.spawnObjectForTesting(1,70,2,30.5f,30.5f);
    const auto scout=game.spawnObjectForTesting(1,83,3,29.5f,30.5f);
    check(building && scout,"Remembered building render fixture");
    game.render(renderer,960,544);
    check(renderer.pixels()!=hidden && game.rememberedForTesting(building),"Sight reveals building and records last known state");
    check(renderer.savePng("fog-visible.png"),"Save visible fog comparison");
    check(game.moveObjectForTesting(scout,4.5f,4.5f),"Leave building sight");
    game.render(renderer,960,544);
    const auto remembered=renderer.pixels();
    check(renderer.savePng("fog-remembered.png"),"Save remembered fog comparison");
    check(game.damageObjectForTesting(building,30),"Damage unseen building");
    game.render(renderer,960,544);
    check(renderer.pixels()==remembered,"Unseen building damage does not update remembered art");
    check(game.damageObjectForTesting(building,100000),"Destroy unseen building");
    game.render(renderer,960,544);
    check(renderer.pixels()==remembered,"Unseen building destruction and remains do not leak pixels");
    const int terrain=game.terrainAtForTesting(30,30);
    check(game.setTerrainForTesting(30,30,terrain==0?1:0),"Change unseen terrain");
    game.render(renderer,960,544);
    check(renderer.pixels()==remembered,"Unseen terrain changes preserve remembered terrain and minimap");
    check(game.moveObjectForTesting(scout,29.5f,30.5f),"Revisit remembered building");
    game.render(renderer,960,544);
    check(!game.rememberedForTesting(building) && renderer.pixels()!=remembered,"Revisit removes destroyed building memory");
    game.setInitiallyExploredForTesting(false);
    check(game.moveObjectForTesting(scout,4.5f,4.5f),"Unexplored fog fixture");
    game.render(renderer,960,544);
    check(renderer.savePng("fog-unexplored.png"),"Save unexplored fog comparison");
    check(game.moveObjectForTesting(scout,29.5f,30.5f),"Reveal normal-map edge fixture");
    game.render(renderer,960,544);
    check(renderer.savePng("fog-unexplored-edge.png"),"Save original black-edge comparison");
    swgb::Game sharing(assets);
    check(sharing.initSkirmish(0x5A17,&error),error.c_str());
    sharing.setLocalPlayerForTesting(3);sharing.setClassicHud(true);sharing.setInitiallyExploredForTesting(false);
    sharing.setDiplomacyForTesting(3,1,3);sharing.setDiplomacyForTesting(3,2,3);
    check(sharing.researchTechnology(3,61),"Shared memory render research");
    const auto allyBuilding=sharing.spawnObjectForTesting(1,70,2,30.5f,30.5f);
    check(allyBuilding!=0,"Shared memory render building");
    sharing.lookAt(30.5f,30.5f);sharing.render(renderer,960,544);
    const auto unknown=renderer.pixels();
    sharing.setDiplomacyForTesting(3,2,0);sharing.setDiplomacyForTesting(2,3,0);
    sharing.render(renderer,960,544);
    check(sharing.rememberedForTesting(allyBuilding) && renderer.pixels()!=unknown,"Shared building sight creates a memory");
    sharing.setDiplomacyForTesting(3,2,3);sharing.render(renderer,960,544);
    check(renderer.pixels()==unknown,"Alliance break hides shared-only memories from world and minimap");
    std::printf("PASS: fog render hides mobile/minimap activity and preserves remembered building/terrain state\n");
}

// Script the player's normal build/train actions; leave the opponent AI running.
static void playerMatch(swgb::Game &game, swgb::Assets &assets, swgb::SoftRenderer &renderer,bool conquest=false) {
    std::string error;
    check(game.initSkirmish(0x5A17, &error,true,1,3,200,conquest), error.c_str());
    game.setKeyboardMouseControls(true);
    const auto enemyCenter=game.objectPosition(game.commandCenterId(2));
    auto pulse = [&](swgb::InputState input) { game.update(0.001f, input); };
    auto menu = [&](uint32_t id, size_t index) {
        swgb::InputState input; input.menuBack = true; pulse(input);
        check(game.selectObjectForTesting(id), "Player can select producer/worker");
        input = {}; input.cycleAttackMode = true; pulse(input);
        check(game.actionMenuOpenForTesting(), "Player action menu opens");
        input = {}; input.pointerX = 214 + (index % 5)*52;
        input.pointerY = 190 + (index / 5)*52; input.selectPressed = true; pulse(input);
    };
    const auto buildings = game.buildingOptionIds(6, 2);
    const auto house = std::find(buildings.begin(), buildings.end(), 70);
    check(house != buildings.end(), "Worker can build habitation");
    const float carbon = game.resource(1, 1);
    menu(6, size_t(house - buildings.begin()));
    game.lookAt(12, 24);
    swgb::InputState place; place.pointerX=480; place.pointerY=272; place.cursorVisible=true;
    pulse(place); place.selectPressed=true; pulse(place);
    bool assigned=false;
    for(auto id:game.underConstructionObjectIds()) assigned |= game.constructionBuilderId(id)==6;
    check(game.resource(1,1) < carbon && assigned, "House placement charges carbon and assigns worker");
    bool gathered = false, recruited = false, houseFinished = false;
    std::vector<uint32_t> defenders{11,12,13,14};
    bool attackLaunched=false;
    for(auto id:defenders)
        check(game.garrisonForTesting(id,game.commandCenterId(1)),"Starting troopers can defend from Command Center");
    const auto options = game.productionOptionIds(4);
    check(!options.empty(), "Starting Troop Center has recruits");
    size_t troopIndex = options.size();
    for (size_t i=0;i<options.size();++i)
        if (assets.dat().civs[1].units[options[i]].cls == 52) { troopIndex=i; break; }
    check(troopIndex < options.size(), "Trooper is trainable at start");
    // IDs 7..10 are starting workers. Select the nearest starting food/carbon node.
    for (uint32_t worker=7;worker<=10;++worker) {
        uint32_t node=0; float nearest=1e9f;
        const auto position=game.objectPosition(worker);
        for (uint32_t id=1;id<256;++id) {
            if (!game.objectActive(id) || game.objectPlayer(id)!=0) continue;
            const int unitId=game.objectUnitId(id);
            const auto &unit=assets.dat().civs[0].units[unitId];
            if (unit.name.find(worker<9 ? "OBJ-VEGETABLE" : "OBJ-TIMBERA") != 0) continue;
            const auto p=game.objectPosition(id);
            float distance=(p[0]-position[0])*(p[0]-position[0])+(p[1]-position[1])*(p[1]-position[1]);
            if(distance<nearest) {nearest=distance;node=id;}
        }
        check(node && game.issueGatherForTesting(worker,node), "Player worker receives gather order");
    }
    for(int frame=0;frame<30*(conquest?2700:1200) && game.matchResult()<0;++frame) {
        if(frame%90==0) {
            if(game.objectActive(4) && game.productionQueueSizeForTesting(4)<3 &&
               game.populationUsedForTesting(1)<game.populationCapacityForTesting(1)) {
                const auto before=game.productionQueueSizeForTesting(4);
                menu(4,troopIndex);
                recruited |= game.productionQueueSizeForTesting(4)>before;
                swgb::InputState back;back.menuBack=true;pulse(back);
            }
            if(!attackLaunched && game.garrisonedCount(game.commandCenterId(1))>=12) {
                for(auto id:defenders) game.ejectGarrisonedUnitForTesting(game.commandCenterId(1),id);
                attackLaunched=true;
            }
            std::vector<swgb::MovingObjectInfo> mobile;
            // This bounded fixture creates fewer than 2048 objects, including expired ones.
            for(uint32_t id=1;id<2048;++id) {
                if(!game.objectActive(id) || game.objectPlayer(id)<1) continue;
                const int unitId=game.objectUnitId(id);
                if(assets.dat().civs[1].units[unitId].speed<=0) continue;
                const auto p=game.objectPosition(id);
                swgb::MovingObjectInfo info;
                info.spawnId=id;info.unitId=unitId;info.player=game.objectPlayer(id);info.x=p[0];info.y=p[1];
                mobile.push_back(info);
            }
            for(const auto &unit:mobile) {
                if(unit.player!=1) continue;
                const auto &data=assets.dat().civs[1].units[unit.unitId];
                if(data.cls==58) {gathered |= game.objectCarriedAmount(unit.spawnId)>0;continue;}
                if(!attackLaunched) {
                    if(std::find(defenders.begin(),defenders.end(),unit.spawnId)==defenders.end() &&
                       game.garrisonForTesting(unit.spawnId,game.commandCenterId(1))) defenders.push_back(unit.spawnId);
                    continue;
                }
                uint32_t target=game.commandCenterId(2);
                float nearest=100;
                for(const auto &enemy:mobile) {
                    if(enemy.player!=2 || !game.canAttackTargetForTesting(unit.spawnId,enemy.spawnId)) continue;
                    float distance=(unit.x-enemy.x)*(unit.x-enemy.x)+(unit.y-enemy.y)*(unit.y-enemy.y);
                    if(distance<nearest) {nearest=distance;target=enemy.spawnId;}
                }
                if(conquest && game.objectHitPoints(game.commandCenterId(2))<=0) {
                    target=0;nearest=1e9f;
                    for(uint32_t id=1;id<2048;++id) {
                        if(!game.objectActive(id) || game.objectHitPoints(id)<=0 || game.objectPlayer(id)!=2 || !game.objectVisibleForTesting(id) ||
                           !game.canAttackTargetForTesting(unit.spawnId,id)) continue;
                        const auto p=game.objectPosition(id);
                        const float distance=(unit.x-p[0])*(unit.x-p[0])+(unit.y-p[1])*(unit.y-p[1]);
                        if(distance<nearest) {nearest=distance;target=id;}
                    }
                }
                if(target && game.attackTargetForTesting(unit.spawnId)!=target)
                    game.issueAttackForTesting(unit.spawnId,target);
                else if(conquest && !target && frame%(30*30)==0) {
                    // Search the known enemy base perimeter without reading hidden survivors.
                    const float offsets[4][2]={{-10,-10},{10,-10},{10,10},{-10,10}};
                    const auto &offset=offsets[(frame/(30*30))%4];
                    game.groupMoveForTesting({unit.spawnId},enemyCenter[0]+offset[0],enemyCenter[1]+offset[1]);
                }
            }
        }
        game.update(1.f/30,{});
        houseFinished |= game.populationCapacityForTesting(1)>15;
        if(frame%1800==0) {
            std::printf("player test time=%.1f CC=%.0f/%.0f population=%.0f/%.0f food=%.0f carbon=%.0f\n",
                game.matchTime(),game.objectHitPoints(game.commandCenterId(1)),game.objectHitPoints(game.commandCenterId(2)),
                game.populationUsedForTesting(1),game.populationCapacityForTesting(1),game.resource(1,0),game.resource(1,1));
            std::fflush(stdout);
        }
    }
    game.clearSelectionForTesting();game.lookAt(enemyCenter[0],enemyCenter[1]);game.render(renderer,960,544);
    check(renderer.savePng(conquest?"player-conquest-result.png":"player-match-result.png"),"Save match result evidence");
    check(gathered && recruited,"Player gathers and recruits through normal costs and queues");
    check(houseFinished,"Player house finishes");
    if(game.matchResult()!=1) for(uint32_t id=1;id<2048;++id) {
        if(!game.objectActive(id) || game.objectPlayer(id)<1 || game.objectHitPoints(id)<=0) continue;
        const auto p=game.objectPosition(id);
        std::printf("SURVIVOR id=%u player=%d unit=%d hp=%.0f pos=%.1f,%.1f visible=%d attack=%u\n",
            id,game.objectPlayer(id),game.objectUnitId(id),game.objectHitPoints(id),p[0],p[1],
            game.objectVisibleForTesting(id),game.attackTargetForTesting(id));
    }
    check(game.matchResult()==1,"Player wins through combat against active AI");
    std::printf("PASS: scripted player victory at %.1fs, no resource/unit/damage cheats\n",game.matchTime());
}

int main(int argc, char **argv) {
    const bool allCivilizations=argc==3 && std::string(argv[2])=="--civilizations";
    const bool conquestMatch=argc==3 && std::string(argv[2])=="--conquest";
    const bool playerConquest=argc==3 && std::string(argv[2])=="--player-conquest";
    if (argc != 2 && !allCivilizations && !conquestMatch && !playerConquest) return 2;
    try {
        swgb::SoftRenderer renderer;
        swgb::Assets assets(&renderer); std::string error;
        check(assets.init(argv[1], &error), error.c_str());
        if(allCivilizations || conquestMatch) {civilizationMatches(assets,conquestMatch);return 0;}
        if(playerConquest) {swgb::Game game(assets);playerMatch(game,assets,renderer,true);return 0;}
        fogEdgeChecks(argv[1]);
    fogRendering(assets,renderer);
    memoryInspectionChecks(assets,renderer);
    healingChecks(assets,renderer);
    combatModifierChecks(assets);
    rubbleChecks(assets,renderer);
    aiFogChecks(assets);
    additiveSelectionChecks(assets,renderer);
    civilizationChecks(assets,renderer);
    conquestChecks(assets);
        swgb::Game game(assets);
        check(game.initSkirmish(0x5A17, &error), error.c_str());
        check(game.commandCenterId(1) && game.commandCenterId(2), "Both objectives must exist");
        game.setClassicHud(true);
        game.setKeyboardMouseControls(true);
        auto hudClick=[&](float x,float y) {
            swgb::InputState in; in.pointerX=in.boxStartX=x*1.2f; in.pointerY=in.boxStartY=340+y*1.2f;
            in.pointerPressed=true; in.pointerHeld=true; game.update(0,in);
            in.pointerPressed=false; in.pointerHeld=false; in.selectPressed=true; game.update(0,in);
        };
        const auto center=game.commandCenterId(1);
        check(game.selectObjectForTesting(center),"HUD command center selection");
        const auto recruits=game.productionOptionIds(center);
        check(!recruits.empty(),"HUD recruit exists");
        const int slot=assets.dat().civs[1].units[recruits.front()].buttonId-1;
        check(slot>=0 && slot<12,"HUD recruit has a visible DAT slot");
        const float foodBefore=game.resource(1,0);
        hudClick(20+(slot%4)*42,20+(slot/4)*42);
        check(game.productionQueueSizeForTesting(center)==1 && game.resource(1,0)<foodBefore,"HUD trains through the paid production handler");
        hudClick(200,125);
        check(game.productionQueueSizeForTesting(center)==0 && game.resource(1,0)==foodBefore,"HUD queue click cancels and refunds");
        hudClick(440,155);
        check(game.objectSelected(center),"Empty HUD does not select world behind it");
        hudClick(770,45); check(game.takeHudMenuRequest() && !game.takeHudMenuRequest(),"HUD menu request consumed once");
        check(game.selectObjectForTesting(6),"HUD worker selection");
        check(game.classicHudIconForTesting(0)==30 && game.classicHudIconForTesting(1)==31 && game.classicHudIconForTesting(2)==29 && game.classicHudIconForTesting(7)==28,"Original worker category and repair icons");
        hudClick(20,20); check(game.classicHudSubmenuOpen(),"HUD economic building submenu opens");
        check(game.classicHudIconForTesting(13)==32 && game.classicHudIconForTesting(14)==0 && game.classicHudIconForTesting(15)==-1,"Original next-category/cancel slots and unused final cell");
        swgb::InputState back; back.menuBack=true; game.update(0,back);
        check(!game.classicHudSubmenuOpen(),"Escape closes HUD submenu");
        check(game.selectObjectsForTesting({6,7}),"HUD group selection");
        hudClick(235,95);
        check(game.selectedObjectIds()==std::vector<uint32_t>({7}),"HUD portrait selects one member");
        check(game.selectObjectForTesting(6),"HUD building placement worker");
        hudClick(20,20);
        const int houseSlot=assets.dat().civs[1].units[70].buttonId-1;
        check(houseSlot>=0 && houseSlot<12,"House has an available HUD slot");
        const float carbonBefore=game.resource(1,1);
        const auto foundationsBefore=game.underConstructionObjectIds();
        hudClick(20+(houseSlot%4)*42,20+(houseSlot/4)*42);
        hudClick(440,155);
        check(game.underConstructionObjectIds()==foundationsBefore && game.resource(1,1)==carbonBefore,"HUD does not place buildings beneath itself");
        game.lookAt(12,24);
        swgb::InputState place; place.pointerX=480; place.pointerY=170; place.cursorVisible=true;
        game.update(0,place); place.selectPressed=true; game.update(0,place);
        bool assignedBuilder=false;
        for(auto id:game.underConstructionObjectIds()) assignedBuilder |= game.constructionBuilderId(id)==6;
        check(assignedBuilder && game.resource(1,1)<carbonBefore,"HUD house action reaches paid world placement");
        check(game.initSkirmish(0x5A17,&error),error.c_str());
        check(game.selectObjectsForTesting({11,12,13,14}),"HUD military formation group");
        check(game.classicHudIconForTesting(0)==6 && game.classicHudIconForTesting(1)==7 && game.classicHudIconForTesting(2)==8 && game.classicHudIconForTesting(3)==3,"Original order slots: patrol, guard, follow, stop");
        check(game.classicHudIconForTesting(12)==1 && game.classicHudIconForTesting(14)==59,"Original garrison and delete slots");
        const int activeFormation[]={65,66,68,67};
        for(int i=0;i<4;++i) {
            hudClick(26+i*41,105);
            check(game.formationForTesting()==i && game.classicHudIconForTesting(8+i)==activeFormation[i],"Formation click changes the shared formation and original active icon");
        }
        check(game.garrisonForTesting(11,center) && game.garrisonForTesting(12,center),"Garrison HUD fixture");
        for(int i=0;i<1800 && game.garrisonedCount(center)<2;++i) game.update(1.f/30,{});
        check(game.garrisonedCount(center)==2,"Troopers physically enter the garrison");
        check(game.selectObjectForTesting(center),"Inspect occupied command center");
        check(game.classicHudIconForTesting(12)==2,"Original eject-all slot and asset");
        check(game.selectObjectsForTesting({center,4}),"Mixed selection garrison input fixture");
        hudClick(333,62);
        check(game.garrisonedCount(center)==2,"Invisible garrison portraits cannot eject during multi-selection");
        game.selectObjectForTesting(center);

        swgb::InputState releaseOnly; releaseOnly.pointerX=333*1.2f; releaseOnly.pointerY=340+62*1.2f; releaseOnly.selectPressed=true;
        game.update(0,releaseOnly);
        check(game.garrisonedCount(center)==2,"Garrison release without a press cannot eject");
        swgb::InputState dragOut=releaseOnly; dragOut.selectPressed=false; dragOut.pointerPressed=true; dragOut.pointerHeld=true;
        dragOut.boxStartX=dragOut.pointerX; dragOut.boxStartY=dragOut.pointerY; game.update(0,dragOut);
        dragOut.pointerPressed=false; dragOut.pointerHeld=false; dragOut.selectPressed=true; dragOut.pointerX=440*1.2f; game.update(0,dragOut);
        check(game.garrisonedCount(center)==2,"Garrison drag-out cancels the press");
        swgb::InputState pausedEject; pausedEject.pointerX=333*1.2f; pausedEject.pointerY=340+62*1.2f; pausedEject.selectPressed=true;
        pausedEject.boxStartX=pausedEject.pointerX; pausedEject.boxStartY=pausedEject.pointerY;
        pausedEject.pointerPressed=true; pausedEject.pointerHeld=true; pausedEject.selectPressed=false; game.updatePaused(.1f,pausedEject);
        pausedEject.pointerPressed=false; pausedEject.pointerHeld=false; pausedEject.selectPressed=true; game.updatePaused(.1f,pausedEject);
        pausedEject.pointerX=26*1.2f; pausedEject.pointerY=340+146*1.2f;
        game.updatePaused(.1f,pausedEject);
        check(game.garrisonedCount(center)==2,"Pause blocks individual and all-passenger ejection");
        hudClick(333,62);
        check(game.garrisonedCount(center)==1 && game.objectSelected(center),"Garrison portrait ejects one unit and preserves container selection");
        hudClick(26,146);
        check(game.garrisonedCount(center)==0 && game.classicHudIconForTesting(12)==-1,"Eject-all empties garrison and disappears");
        check(game.initSkirmish(0x5A17,&error),error.c_str());
        game.selectObjectForTesting(center);
        check(game.setGatherPointForTesting(center,20,20),"Rally fixture");
        hudClick(67,146);
        check(game.gatherPointCursorActiveForTesting() && game.classicHudIconForTesting(0)==46 && game.classicHudIconForTesting(14)==0,"Original rally targeting remove/cancel grid");
        hudClick(108,146);
        check(!game.gatherPointCursorActiveForTesting() && game.gatherPointActiveForTesting(center),"Cancel targeting preserves existing rally");
        hudClick(67,146); hudClick(26,23);
        check(!game.gatherPointCursorActiveForTesting() && !game.gatherPointActiveForTesting(center),"Remove rally clears destination and targeting mode");
        hudClick(26,23);
        check(game.productionQueueSizeForTesting(center)==1,"Paused inspection production fixture");
        const float pausedTime=game.matchTime(),pausedQueue=game.productionRemainingForTesting(center);
        const auto pausedWorker=game.objectPosition(6);
        const auto pausedCount=game.activeObjectCount();
        const float pausedFood=game.resource(1,0);
        const auto cameraBefore=game.cameraForTesting();
        swgb::InputState inspect; inspect.scrollX=1;
        for(int i=0;i<100;++i) game.updatePaused(.05f,inspect);
        check(game.cameraForTesting()!=cameraBefore,"Paused arrow keys move the camera");
        auto pausedClick=[&](float x,float y,bool right=false) {
            swgb::InputState in; in.pointerX=x*1.2f; in.pointerY=340+y*1.2f;
            in.selectPressed=!right; in.commandPressed=right; game.updatePaused(.05f,in);
        };
        pausedClick(26,23); pausedClick(200,135);
        check(game.productionQueueSizeForTesting(center)==1 && game.productionRemainingForTesting(center)==pausedQueue && game.resource(1,0)==pausedFood,"Pause blocks training and queue refunds without changing production time");
        const auto beforeMap=game.cameraForTesting(); pausedClick(586,98);
        check(game.cameraForTesting()!=beforeMap,"Paused minimap moves the camera");
        game.lookAt(pausedWorker[0],pausedWorker[1]); game.clearSelectionForTesting();
        inspect={}; inspect.pointerX=480; inspect.pointerY=158; inspect.selectPressed=true;
        game.updatePaused(.05f,inspect);
        check(game.objectSelected(6),"World unit can be inspected while paused");
        const auto ordersBefore=game.movingObjects();
        pausedClick(610,105,true);
        inspect={}; inspect.pointerX=700; inspect.pointerY=250; inspect.commandPressed=true;
        game.updatePaused(.05f,inspect);
        const auto ordersAfter=game.movingObjects();
        check(ordersAfter.size()==ordersBefore.size(),"Paused right clicks create no movement orders");
        for(size_t i=0;i<ordersBefore.size();++i) check(ordersBefore[i].spawnId==ordersAfter[i].spawnId && ordersBefore[i].targetX==ordersAfter[i].targetX && ordersBefore[i].targetY==ordersAfter[i].targetY,"Paused map/world clicks preserve existing destinations");
        check(game.matchTime()==pausedTime && game.objectPosition(6)==pausedWorker && game.activeObjectCount()==pausedCount,"Pause freezes simulation, objects and activities");
        game.update(.1f,{}); check(game.matchTime()>pausedTime,"Simulation resumes normally");
        check(game.initSkirmish(0x5A17,&error),error.c_str());
        std::vector<int> alarmSounds;
        game.setInterfaceSoundPlayer([&](int id) {alarmSounds.push_back(id);});
        uint32_t resourceNode=0;
        for(uint32_t id=1;id<256;++id) if(game.objectActive(id) && game.objectPlayer(id)==0) {
            const auto &u=assets.dat().civs[0].units[game.objectUnitId(id)];
            if(u.name.find("OBJ-TIMBERA")==0) {resourceNode=id;break;}
        }
        check(resourceNode && game.issueGatherForTesting(6,resourceNode),"Alarm gatherer fixture");
        const auto foundation=game.spawnFoundationForTesting(1,70,1,30,16,{7});
        check(foundation && game.objectBuildingTarget(7,foundation),"Alarm builder fixture");
        check(game.damageObjectForTesting(4,300) && game.issueRepairForTesting(8,4),"Alarm repairer fixture");
        game.selectObjectForTesting(center);
        check(game.classicHudIconForTesting(14)==49,"Original inactive alarm icon and slot");
        hudClick(108,146);
        check(game.alarmActiveForTesting(center) && game.classicHudIconForTesting(14)==61,"Alarm becomes active with original icon");
        check(game.garrisonForTesting(6,center),"Redirect incoming sheltered worker without replacing saved job");
        hudClick(108,146);
        check(!game.alarmActiveForTesting(center) && game.objectGatheringTarget(6,resourceNode) && game.objectBuildingTarget(7,foundation) && game.repairTargetForTesting(8)==4,"Alarm cancellation before boarding restores gather/build/repair orders");
        hudClick(108,146);
        for(int i=0;i<1800 && game.garrisonedCount(center)<5;++i) game.update(1.f/30,{});
        check(game.garrisonedCount(center)==5,"Alarm shelters all five nearby workers");
        check(game.classicHudIconForTesting(11)==64,"Original return-to-work slot and icon");
        swgb::InputState pausedReturn; pausedReturn.pointerX=149*1.2f; pausedReturn.pointerY=340+105*1.2f; pausedReturn.selectPressed=true;
        game.updatePaused(.1f,pausedReturn);
        check(game.garrisonedCount(center)==5,"Pause rejects return-to-work");
        const auto shelterPosition=game.objectPosition(center);
        for(int y=int(shelterPosition[1])-9;y<=int(shelterPosition[1])+9;++y)
            for(int x=int(shelterPosition[0])-9;x<=int(shelterPosition[0])+9;++x) game.setTerrainForTesting(x,y,1);
        hudClick(149,105);
        check(game.garrisonedCount(center)==5,"Blocked shelter exits retain workers and saved jobs");
        for(int y=int(shelterPosition[1])-9;y<=int(shelterPosition[1])+9;++y)
            for(int x=int(shelterPosition[0])-9;x<=int(shelterPosition[0])+9;++x) game.setTerrainForTesting(x,y,0);
        hudClick(149,105);
        check(game.garrisonedCount(center)==0 && game.objectGatheringTarget(6,resourceNode) && game.objectBuildingTarget(7,foundation) && game.repairTargetForTesting(8)==4,"Return-to-work restores gather/build/repair after ejection");
        check(game.classicHudIconForTesting(11)!=64,"Empty building removes return-to-work control");
        check(game.alarmActiveForTesting(center),"Return-to-work leaves alarm on as in original");
        hudClick(108,146);
        check(game.garrisonForTesting(6,center),"Manual worker garrison remembers work too");
        for(int i=0;i<1200 && game.garrisonContainerForTesting(6)<0;++i) game.update(1.f/30,{});
        check(game.garrisonContainerForTesting(6)==int32_t(center),"Manual worker enters shelter");
        game.setObjectResourceForTesting(resourceNode,0);
        hudClick(149,105);
        check(game.garrisonContainerForTesting(6)<0 && !game.objectGatheringTarget(6,resourceNode),"Exhausted job target falls back safely on return");
        check(game.initSkirmish(0x5A17,&error),error.c_str());
        check(game.issueGatherForTesting(6,resourceNode),"Capacity fixture gatherer");
        const auto centerPosition=game.objectPosition(center);
        std::vector<uint32_t> shelteredTroops;
        for(int i=0;i<15;++i) {
            const auto troop=game.spawnObjectForTesting(1,game.objectUnitId(11),1,centerPosition[0]+3,centerPosition[1]+3);
            shelteredTroops.push_back(troop);
            check(game.garrisonForTesting(troop,center),"Reserve all shelter spaces");
        }
        game.selectObjectForTesting(center); hudClick(108,146);
        check(game.objectGatheringTarget(6,resourceNode),"Full shelter does not discard worker job");
        for(int i=0;i<2400 && game.garrisonedCount(center)<15;++i) game.update(1.f/30,{});
        check(game.garrisonedCount(center)==15,"Full original garrison layout fixture");
        hudClick(404,144);
        check(game.garrisonContainerForTesting(shelteredTroops.front())<0 && game.garrisonedCount(center)==14,"Compressed three-row grid maps oldest roster entry to bottom-right");
        hudClick(310,62);
        check(game.garrisonContainerForTesting(shelteredTroops.back())<0 && game.garrisonedCount(center)==13,"Overlapping portrait click selects last-painted top-left entry");
        hudClick(26,23);
        check(game.productionQueueSizeForTesting(center)==1,"Busy garrison layout fixture");
        hudClick(280,62);
        check(game.garrisonContainerForTesting(shelteredTroops[shelteredTroops.size()-2])<0 && game.garrisonedCount(center)==12 && game.productionQueueSizeForTesting(center)==1,"Busy single-row portraits preserve production queue while ejecting correct member");

        check(game.initSkirmish(0x5A17,&error),error.c_str());
        check(game.moveObjectForTesting(6,61,2) && game.issueGatherForTesting(6,resourceNode),"Distant worker fixture");
        game.selectObjectForTesting(center); hudClick(108,146);
        check(game.objectGatheringTarget(6,resourceNode),"Alarm excludes workers outside original search area");
        check(std::find(alarmSounds.begin(),alarmSounds.end(),50357)!=alarmSounds.end() && std::find(alarmSounds.begin(),alarmSounds.end(),50358)!=alarmSounds.end(),"Original bell and recall audio resources dispatched");
        game.setInterfaceSoundPlayer({});
        game.lookAt(20.5f,20.5f);
        check(game.worldSoundMixForTesting(20.5f,20.5f)==std::array<int,2>{0,0},"Centered original spatial mix");
        check(game.worldSoundMixForTesting(28.5f,12.5f)==std::array<int,2>{1000,-3600} && game.worldSoundMixForTesting(12.5f,28.5f)==std::array<int,2>{-1000,-3600},"Original pan clamp and outside-width attenuation");
        const auto verticalMix=game.worldSoundMixForTesting(28.5f,28.5f);
        check(verticalMix[0]==0 && std::abs(verticalMix[1]+1950)<=3,"Original vertical curve includes fractional-position offset");
        check(game.initSkirmish(0x5A17,&error),error.c_str());
        const auto nearCenter=game.objectPosition(center);
        const auto turret=game.spawnObjectForTesting(1,79,1,nearCenter[0]+5,nearCenter[1]-5);
        check(turret!=0,"Spatial alarm second-shelter fixture");
        std::vector<std::array<int,3>> positioned;
        game.setSpatialInterfaceSoundPlayer([&](int id,int pan,int volume) {positioned.push_back({id,pan,volume});});
        game.lookAt(nearCenter[0],nearCenter[1]); game.selectObjectForTesting(center); hudClick(108,146);
        check(positioned.size()==2 && positioned[0][0]==50357 && positioned[0][1]==0 && positioned[1][1]>0,"Alarm emits original spatial event for each audible shelter");
        positioned.clear(); game.lookAt(60,60); hudClick(108,146);
        check(positioned.empty(),"Distant alarm uses original minus30dB cutoff");
        game.setSpatialInterfaceSoundPlayer({});

        {
            swgb::Game sight(assets);
            check(sight.initSkirmish(0x5A17,&error),error.c_str());
            sight.setLocalPlayerForTesting(3);
            check(!sight.worldSoundVisibleForTesting(20.5f,20.5f),"Enemy ownership does not grant audio sight");
            const auto scout=sight.spawnObjectForTesting(1,83,3,20.5f,20.5f);
            check(scout!=0 && sight.worldSoundVisibleForTesting(20.5f,20.5f),"Own worker grants current audio sight");
            const int radius=int(assets.dat().civs[1].units[83].lineOfSight);
            check(sight.worldSoundVisibleForTesting(20.5f+radius,20.5f) &&
                  !sight.worldSoundVisibleForTesting(21.5f+radius,20.5f),"Original sight cardinal edge is inclusive");
            int heard=0;
            sight.setSpatialUnitSoundPlayer([&](int,int,int,int){++heard;});
            sight.lookAt(20.5f,20.5f);
            sight.worldSoundForTesting(1,2,20.5f,20.5f);
            check(heard==1,"Visible foreign activity is audible");
            check(sight.moveObjectForTesting(scout,50.5f,50.5f),"Move sight source");
            sight.worldSoundForTesting(1,2,20.5f,20.5f);
            check(heard==1 && !sight.worldSoundVisibleForTesting(20.5f,20.5f),"Known terrain without current sight is silent, even at camera center");
            check(!sight.worldSoundVisibleForTesting(-1,10) && !sight.worldSoundVisibleForTesting(1000,1000),"Out-of-map audio is rejected");
            const auto foundation=sight.spawnFoundationForTesting(1,109,3,30,30,{});
            check(foundation && sight.worldSoundVisibleForTesting(30.5f,30.5f) &&
                  !sight.worldSoundVisibleForTesting(34.5f,30.5f),"Foundation reveals footprint only");
            check(sight.completeFoundationForTesting(foundation),"Complete sight foundation");
            sight.update(0.01f,{});
            check(sight.worldSoundVisibleForTesting(34.5f,30.5f),"Completed command center grants full sight");
            const auto overlap=sight.spawnObjectForTesting(1,83,3,50.5f,50.5f);
            check(sight.damageObjectForTesting(scout,100000) && sight.worldSoundVisibleForTesting(50.5f,50.5f),"Overlapping source keeps sight after a death");
            check(sight.damageObjectForTesting(overlap,100000) && !sight.worldSoundVisibleForTesting(50.5f,50.5f),"Last source death removes current sight");
            // A single raised corner must form two planes, not a bilinear bump.
            for(int yy=10;yy<=11;++yy) for(int xx=10;xx<=11;++xx)
                check(sight.setCornerElevationForTesting(xx,yy,(xx==10 && yy==10)?1:0),"Slope fixture");
            check(std::abs(sight.elevationForTesting(10.25f,10.25f)-0.75f)<0.001f &&
                  std::abs(sight.elevationForTesting(10.75f,10.25f)-0.25f)<0.001f,"Original triangular corner slope height");
            sight.setClassicHud(false); sight.lookAt(20.5f,20.5f);
            const auto full=sight.cameraForTesting();
            sight.setClassicHud(true); sight.lookAt(20.5f,20.5f);
            const auto hud=sight.cameraForTesting();
            check(std::abs(hud[0]-full[0])<0.01f && std::abs(hud[1]-full[1]-102)<0.01f,"Classic camera centers above 204px HUD");
            swgb::InputState zoomInput; zoomInput.zoomStep=-1;
            sight.updatePaused(0.01f,zoomInput);
            check(std::abs(sight.cameraForTesting()[1]-full[1]-127.5f)<0.01f &&
                  sight.worldSoundMixForTesting(20.5f,20.5f)==std::array<int,2>{0,0},"Zoom preserves gameplay center and sound listener while paused");
        }
        {
            swgb::Game fog(assets);
            check(fog.initSkirmish(0x5A17,&error),error.c_str());
            fog.setLocalPlayerForTesting(3);fog.setInitiallyExploredForTesting(false);
            const auto scout=fog.spawnObjectForTesting(1,83,3,20.5f,20.5f);
            const auto building=fog.spawnObjectForTesting(1,109,2,21.5f,20.5f);
            check(fog.tileVisibilityForTesting(20,20)==2 && fog.tileVisibilityForTesting(60,2)==0,"Normal map distinguishes visible and unexplored tiles");
            check(fog.rememberedForTesting(building),"Observed static object is remembered");
            const float rememberedHP=fog.rememberedHitPointsForTesting(building);
            check(fog.moveObjectForTesting(scout,50.5f,50.5f),"Scout leaves observed building");
            check(fog.tileVisibilityForTesting(20,20)==1 && !fog.objectVisibleForTesting(building),"Exploration persists after current sight leaves");
            check(fog.damageObjectForTesting(building,10) && fog.rememberedHitPointsForTesting(building)==rememberedHP,"Unseen building damage does not update memory");
            check(fog.damageObjectForTesting(building,100000) && fog.rememberedForTesting(building),"Unseen destruction keeps last-seen building");
            check(fog.moveObjectForTesting(scout,20.5f,20.5f) && !fog.rememberedForTesting(building),"Revisit removes destroyed building memory");
            check(fog.initSkirmish(0x5A17,&error),error.c_str());
            check(!fog.rememberedForTesting(building),"New match clears remembered objects");
        }
        {
            swgb::Game allied(assets);
            check(allied.initSkirmish(0x5A17,&error),error.c_str());
            allied.setLocalPlayerForTesting(3);allied.setInitiallyExploredForTesting(false);
            const auto allyScout=allied.spawnObjectForTesting(1,83,2,30.5f,30.5f);
            check(allyScout!=0,"Allied sight fixture");
            allied.setDiplomacyForTesting(3,2,0);allied.setDiplomacyForTesting(2,3,0);
            check(!allied.worldSoundVisibleForTesting(30.5f,30.5f) && allied.tileVisibilityForTesting(30,30)==0,"Mutual alliance alone does not grant shared sight");
            check(allied.researchTechnology(3,61),"Research original REVEAL-ALLY effect");
            check(allied.worldSoundVisibleForTesting(30.5f,30.5f) && allied.tileVisibilityForTesting(30,30)==2,"Viewer research shares mutual ally sight and audio");
            int heard=0; allied.lookAt(30.5f,30.5f);
            allied.setSpatialUnitSoundPlayer([&](int,int,int,int){++heard;});
            allied.worldSoundForTesting(1,2,30.5f,30.5f);
            check(heard==1,"Shared sight admits positioned ally activity");
            allied.setDiplomacyForTesting(2,3,3);
            check(!allied.worldSoundVisibleForTesting(30.5f,30.5f) && allied.tileVisibilityForTesting(30,30)==0,"One-way alliance does not retain shared sight or exploration");
            allied.worldSoundForTesting(1,2,30.5f,30.5f);
            check(heard==1,"Alliance break silences activity outside own sight");
            allied.setDiplomacyForTesting(2,3,0);
            check(allied.moveObjectForTesting(allyScout,50.5f,50.5f),"Ally scout moves");
            check(allied.tileVisibilityForTesting(30,30)==1 && !allied.worldSoundVisibleForTesting(30.5f,30.5f),"Shared exploration survives ally movement but not current sight/audio");
        }
        std::printf("PASS: original researched mutual-ally sight gate\n");
        std::printf("PASS: visual fog exploration and stale-building lifecycle\n");
        std::printf("PASS: original sight audio, slope interpolation and viewport camera\n");
        std::printf("PASS: original alarm/return-to-work UI, saved jobs, cancellation, capacity and range\n");
        {
            swgb::Game transport(assets);
            check(transport.init(7,96,&error),error.c_str());
            transport.setLocalPlayerForTesting(1); transport.setClassicHud(true);
            transport.setKeyboardMouseControls(true);
            const auto mech=transport.spawnObjectForTesting(3,249,1,44,20);
            const auto passenger=transport.spawnObjectForTesting(3,460,1,44,23);
            transport.update(1.f/30,{});
            check(transport.garrisonForTesting(passenger,mech),"Transport boarding order");
            for(int i=0;i<900 && transport.garrisonedCount(mech)==0;++i) transport.update(1.f/30,{});
            check(transport.garrisonedCount(mech)==1,"Transport passenger boarded");
            transport.selectObjectForTesting(mech);
            check(transport.classicHudIconForTesting(13)==17,"Original targeted unload icon in slot 13");
            check(transport.issueUnloadForTesting(mech,52,20),"Transport accepts unload destination");
            const auto position=transport.objectPosition(mech);
            for(int i=0;i<30;++i) transport.updatePaused(.1f,{});
            check(transport.objectPosition(mech)==position && transport.garrisonedCount(mech)==1,"Paused transport retains passengers and destination");
            swgb::InputState stop; stop.pointerX=149*1.2f; stop.pointerY=340+23*1.2f; stop.selectPressed=true;
            transport.update(0,stop);
            check(!transport.unloadActiveForTesting(mech) && transport.garrisonedCount(mech)==1,"Stop cancels unload without losing passenger");
            check(transport.issueUnloadForTesting(mech,52,20),"Unload reissued");
            for(int i=0;i<1800 && transport.garrisonedCount(mech)>0;++i) transport.update(1.f/30,{});
            check(transport.garrisonedCount(mech)==0 && transport.objectPosition(mech)[0]>48 && transport.objectActive(passenger),"Transport travels before safely unloading");
            check(transport.classicHudIconForTesting(13)==-1,"Empty transport hides unload");
            check(!transport.issueUnloadForTesting(mech,52,20),"Empty transport rejects unload");
        }
        game.setClassicHud(false);
        check(game.initSkirmish(0x5A17,&error),error.c_str());
        std::printf("PASS: classic HUD slots, formations, individual/all garrison ejection, rally controls, paused inspection, training, refund and placement\n");
        auto groupKey=[&](int slot,bool store) { swgb::InputState in; in.controlGroup=slot; in.storeControlGroup=store; game.update(0,in); };
        check(game.selectObjectsForTesting({6,7}),"Control-group fixture selection");
        groupKey(1,true); game.clearSelectionForTesting(); groupKey(1,false);
        check(game.selectedObjectIds()==std::vector<uint32_t>({6,7}),"Control group restores selection order");
        groupKey(8,false);
        check(game.selectedObjectIds()==std::vector<uint32_t>({6,7}),"Empty group preserves selection");
        check(game.damageObjectForTesting(6,1000000),"Remove a grouped unit");
        game.clearSelectionForTesting(); groupKey(1,false);
        check(game.selectedObjectIds()==std::vector<uint32_t>({7}),"Control group excludes dead members");
        check(game.selectObjectForTesting(game.commandCenterId(1)),"Select a groupable building");
        groupKey(0,true); game.clearSelectionForTesting(); groupKey(0,false);
        check(game.objectSelected(game.commandCenterId(1)),"Building control group recalls");
        check(game.selectObjectForTesting(game.commandCenterId(2)),"Enemy inspection fixture");
        groupKey(2,true); game.clearSelectionForTesting(); groupKey(2,false);
        check(game.selectedObjectCount()==0,"Control group cannot acquire enemies");
        check(game.initSkirmish(0x5A17,&error),error.c_str()); groupKey(1,false); groupKey(0,false);
        check(game.selectedObjectCount()==0,"Restart clears groups before spawn IDs are reused");
        std::printf("PASS: control-group storage, recall, ownership, dead members and restart\n");
        for(bool explored:{true,false}) {
        check(game.initSkirmish(0x5A17,&error,explored),error.c_str());
        for (int frame = 0; frame < 30*900 && game.matchResult() < 0; ++frame) {
            game.update(1.f/30, {});
            if (frame % (30*60) == 0) {
                const auto combat = game.combatStats();
                std::printf("time=%.1f objects=%zu CC=%.0f/%.0f attacks=%zu hits=%zu kills=%zu workers=%d gatherers=%zu\n",
                    game.matchTime(), game.activeObjectCount(), game.objectHitPoints(game.commandCenterId(1)),
                    game.objectHitPoints(game.commandCenterId(2)), combat.ordersIssued, combat.attacksLanded,
                    combat.unitsKilled, game.aiObjectCountForTesting(2,"UNIT-WORKER"),
                    game.aiGathererCountForTesting(2,0)+game.aiGathererCountForTesting(2,1)+game.aiGathererCountForTesting(2,3));
                std::fflush(stdout);
            }
        }
        check(game.combatStats().attacksLanded > 0, "Combat must actually happen");
        check(game.matchResult() == 0, "AI must defeat an idle player within 15 simulated minutes");
        check(game.aiObjectCountForTesting(2,"UNIT-WORKER") <= 10,"Worker cap includes queued recruits");
        std::printf("AI won at %.1f simulated seconds (%s map)\n", game.matchTime(),explored?"explored":"standard");
        float finishedAt=game.matchTime(); size_t objects=game.activeObjectCount();
        for (int i=0;i<90;i++) game.update(1.f/30,{});
        check(game.matchTime()==finishedAt && game.activeObjectCount()==objects,"Finished match must freeze");
        }
        for (int outcome=0;outcome<3;++outcome) {
            check(game.initSkirmish(0x5A17,&error),error.c_str());
            check(game.matchResult()==-1 && game.matchTime()==0,"Restart resets result and clock");
            if(outcome!=1) check(game.damageObjectForTesting(game.commandCenterId(1),1000000),"Damage human objective");
            if(outcome!=0) check(game.damageObjectForTesting(game.commandCenterId(2),1000000),"Damage AI objective");
            game.update(1.f/30,{});
            check(game.matchResult()==outcome,"Victory/defeat/simultaneous loss result");
        }
        check(game.initSkirmish(0x5A17,&error),error.c_str());
        game.resignSkirmish();check(game.matchResult()==0,"Resign ends match");
        playerMatch(game,assets,renderer);
        check(game.initCompactTestMap(0x5A17,64,&error),error.c_str());
        check(!game.skirmishActive() && game.matchResult()==-1,"Sandbox must not inherit skirmish state");
        std::puts("PASS: AI completes combat match; outcomes, freeze, restart, resign and sandbox isolation");
        return 0;
    } catch(const std::exception &e) {std::fprintf(stderr,"FAIL: %s\n",e.what());return 1;}
}
