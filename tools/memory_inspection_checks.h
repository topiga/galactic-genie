// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void memoryInspectionChecks(swgb::Assets &assets,swgb::SoftRenderer &renderer) {
    swgb::Game game(assets);
    std::string error;
    check(game.initSkirmish(0x5A17,&error),error.c_str());
    game.setLocalPlayerForTesting(3);game.setClassicHud(true);
    const auto building=game.spawnObjectForTesting(1,109,2,30.5f,30.5f);
    const auto scout=game.spawnObjectForTesting(1,83,3,29.5f,30.5f);
    check(building && scout,"Remembered inspection fixture");
    game.lookAt(30.5f,30.5f);game.render(renderer,960,544);
    float x=0,y=0;
    check(game.objectScreenPosition(building,960,544,x,y),"Inspect building screen position");
    auto click=[&]() {
        swgb::InputState in;in.selectPressed=true;in.pointerX=x;in.pointerY=y-12;
        game.updatePaused(0,in);
    };
    click();check(game.selectedObjectCount()==1,"Visible building selects normally");
    check(game.moveObjectForTesting(scout,4.5f,4.5f),"Leave inspected building sight");
    game.updatePaused(0,{});
    check(game.rememberedForTesting(building) && game.selectedObjectCount()==0,
          "Lost sight clears selection but retains the building image");
    game.render(renderer,960,544);
    const auto inspected=renderer.pixels();
    const float hp=game.rememberedHitPointsForTesting(building);
    for(size_t slot=0;slot<16;++slot)
        check(game.classicHudIconForTesting(slot)==-1,"Remembered building has no executable command buttons");
    check(game.damageObjectForTesting(building,30),"Damage unseen inspected building");
    game.render(renderer,960,544);
    check(game.rememberedHitPointsForTesting(building)==hp && renderer.pixels()==inspected,
          "Building memory never exposes unseen health or damage updates");
    check(game.damageObjectForTesting(building,100000),"Destroy unseen inspected building");
    game.render(renderer,960,544);
    check(game.rememberedForTesting(building) && game.selectedObjectCount()==0 && renderer.pixels()==inspected,
          "Building memory remains unchanged after unseen destruction");
    check(renderer.savePng("fog-remembered-inspection.png"),"Save remembered inspection comparison");
    game.clearSelectionForTesting();game.render(renderer,960,544);
    const auto blank=renderer.pixels();
    check(blank==inspected,"Losing sight leaves the same empty HUD as clearing selection");
    click();game.render(renderer,960,544);
    check(game.selectedObjectCount()==0 && renderer.pixels()==blank,
          "Clicking a fogged building does not open a remembered HUD");
    check(game.moveObjectForTesting(scout,29.5f,30.5f),"Revisit destroyed inspected building");
    game.updatePaused(0,{});game.render(renderer,960,544);
    check(game.selectedObjectCount()==0 && !game.rememberedForTesting(building),
          "Revisiting the destroyed site removes the stale building memory");
    check(game.initSkirmish(0x5A17,&error),error.c_str());game.render(renderer,960,544);
    check(game.selectedObjectCount()==0,"New match clears memory inspection");
    const auto target=game.spawnObjectForTesting(1,109,2,30.5f,30.5f);
    const auto observer=game.spawnObjectForTesting(1,83,3,29.5f,30.5f);
    const auto soldier=game.spawnObjectForTesting(1,game.objectUnitId(11),3,20.5f,30.5f);
    game.setLocalPlayerForTesting(3);game.setClassicHud(true);
    game.setDiplomacyForTesting(3,2,3);game.setAttackModeForTesting(soldier,3);
    game.lookAt(30.5f,30.5f);game.render(renderer,960,544);
    check(game.objectScreenPosition(target,960,544,x,y),"Remembered order screen position");
    click();check(game.selectedObjectCount()==1,"Select living building before loss of sight");
    check(game.moveObjectForTesting(observer,4.5f,4.5f),"Hide selected living target");
    game.updatePaused(0,{});
    check(game.selectedObjectCount()==0,"Living hidden building loses selection");
    check(game.moveObjectForTesting(observer,29.5f,30.5f),"Reveal previously selected living target");
    game.updatePaused(0,{});
    check(game.selectedObjectCount()==0,"Revealing a building does not restore its old selection");
    check(game.moveObjectForTesting(observer,4.5f,4.5f),"Hide order target");
    game.updatePaused(0,{});click();
    check(game.selectedObjectCount()==0,
          "A still-living building under fog also cannot be newly inspected");
    game.selectObjectForTesting(soldier);
    swgb::InputState order;order.commandPressed=true;order.pointerX=x;order.pointerY=y-12;
    order.screenW=960;order.screenH=544;
    game.updatePaused(0,order);
    check(game.rememberedAttackForTesting(soldier)==0,"Pause forbids remembered orders");
    game.update(.001f,order);
    check(game.rememberedAttackForTesting(soldier)==target && game.attackTargetForTesting(soldier)==0,
          "Remembered order defers attack until sight returns");
    const auto rememberedDestination=game.moveAnchorForTesting(soldier);
    game.groupMoveForTesting({soldier},30.5f,30.5f);
    check(game.moveAnchorForTesting(soldier)==rememberedDestination,
          "Remembered order uses building location with normal collision avoidance");
    check(game.rememberedAttackForTesting(soldier)==0,"Replacement move clears remembered intent");
    game.update(.001f,order);
    check(game.stopUnitForTesting(soldier),"Stop remembered order");
    check(game.rememberedAttackForTesting(soldier)==0,"Stop clears remembered attack intention");
    game.update(.001f,order);
    check(game.moveObjectForTesting(soldier,28.5f,30.5f),"Reveal ordered building");
    game.update(.001f,{});
    check(game.rememberedAttackForTesting(soldier)==0 && game.attackTargetForTesting(soldier)==target,
          "Explicit remembered attack resolves on sight even in passive stance");
    check(game.stopUnitForTesting(soldier) && game.moveObjectForTesting(soldier,20.5f,30.5f),
          "Leave ordered building sight");
    game.updatePaused(0,{});game.update(.001f,order);
    check(game.damageObjectForTesting(target,100000),"Destroy pending target out of sight");
    game.update(.001f,{});
    check(game.rememberedAttackForTesting(soldier)==target,"Hidden death does not cancel remembered intent");
    check(game.moveObjectForTesting(soldier,28.5f,30.5f),"Revisit empty ordered site");
    game.update(.001f,{});
    check(game.rememberedAttackForTesting(soldier)==0 && game.attackTargetForTesting(soldier)==0,
          "Empty-site revisit clears remembered intent without attacking dead target");
    std::printf("PASS: remembered building mouse inspection, cleared HUD, no orders and revisit cleanup\n");
}
