// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void additiveSelectionChecks(swgb::Assets &assets,swgb::SoftRenderer &renderer) {
    swgb::Game game(assets);std::string error;
    check(game.initSkirmish(0x5A17,&error),error.c_str());
    game.setLocalPlayerForTesting(3);game.setClassicHud(true);game.setKeyboardMouseControls(true);
    const auto first=game.spawnObjectForTesting(1,83,3,20.5f,20.5f);
    const auto second=game.spawnObjectForTesting(1,83,3,24.5f,20.5f);
    const auto third=game.spawnObjectForTesting(1,83,3,20.5f,24.5f);
    const auto enemy=game.spawnObjectForTesting(1,83,2,22.5f,20.5f);
    game.lookAt(22.5f,22.5f);game.render(renderer,960,544);
    auto click=[&](uint32_t id,bool additive,bool paused) {
        float x,y;check(game.objectScreenPosition(id,960,544,x,y),"Selection fixture screen position");
        swgb::InputState in;in.pointerX=x;in.pointerY=y-12;in.selectPressed=true;in.addToSelection=additive;
        if(paused) game.updatePaused(0,in);else game.update(0,in);
    };
    click(first,false,false);check(game.selectedObjectIds()==std::vector<uint32_t>{first},"Ordinary click replaces selection");
    click(second,true,false);check(game.selectedObjectIds()==std::vector<uint32_t>({first,second}),"Shift click adds a unit in selection order");
    click(first,true,false);check(game.selectedObjectIds()==std::vector<uint32_t>{second},"Shift click removes an existing member");
    click(second,true,true);check(game.selectedObjectIds().empty(),"Paused Shift click removes the final member");
    click(first,true,true);click(second,true,false);
    check(game.selectedObjectIds()==std::vector<uint32_t>({first,second}),"Shift click re-adds removed members in click order");
    click(enemy,true,true);check(game.selectedObjectIds()==std::vector<uint32_t>({first,second}),"Shift click cannot merge enemy inspection into an army");
    const auto time=game.matchTime();
    click(third,true,true);check(game.selectedObjectIds()==std::vector<uint32_t>({first,second,third}) && game.matchTime()==time,"Paused Shift selection preserves simulation time");
    swgb::InputState empty;empty.pointerX=0;empty.pointerY=0;empty.selectPressed=true;empty.addToSelection=true;
    game.updatePaused(0,empty);check(game.selectedObjectCount()==3,"Shift click on empty ground preserves selection");
    game.selectObjectsForTesting({first});
    float x,y;check(game.objectScreenPosition(second,960,544,x,y),"Box selection screen position");
    swgb::InputState box;box.boxSelectCommit=true;box.addToSelection=true;
    box.boxStartX=x-5;box.boxEndX=x+5;box.boxStartY=y-5;box.boxEndY=y+5;
    game.updatePaused(0,box);check(game.selectedObjectIds()==std::vector<uint32_t>({first,second}),"Paused Shift box adds without discarding prior selection");
    game.update(0,box);check(game.selectedObjectIds()==std::vector<uint32_t>({first,second}),"Live Shift box preserves existing member order");
    box.addToSelection=false;game.updatePaused(0,box);
    check(game.selectedObjectIds()==std::vector<uint32_t>{second},"Unmodified box still replaces the group");
    click(enemy,false,true);check(game.selectedObjectIds()==std::vector<uint32_t>{enemy},"Visible enemy inspection fixture");
    click(first,true,true);check(game.selectedObjectIds()==std::vector<uint32_t>{first},"Adding an owned unit clears foreign inspection");
    std::puts("PASS: live/paused additive selection, ownership, stable order and replacement");
}
