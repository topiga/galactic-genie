// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void combinedUpgradeChecks(swgb::Assets &assets) {
    auto &data=const_cast<swgb::dat::DatFile &>(assets.dat());
    auto &commands=data.effects[size_t(data.techs[501].effectId)].commands;
    auto &later=data.effects[size_t(data.techs[174].effectId)].commands;
    struct Restore {
        std::vector<swgb::dat::EffectCommand> &value,original;
        ~Restore(){value=std::move(original);}
    } restore{commands,commands},restoreLater{later,later};
    later.clear();
    // Seven actual original109→71 sequences, fresh damagedCC1000/2500 each time.
    const struct {std::vector<swgb::dat::EffectCommand> commands;float hp,source,target;} cases[]={
        {{{4,109,-1,0,500},{3,109,71,-1,0}},1000,2500,2500},
        {{{5,109,-1,0,1.1f},{3,109,71,-1,0}},1000,2500,2500},
        {{{3,109,71,-1,0},{4,71,-1,0,500}},1000,2500,3000},
        {{{3,109,71,-1,0},{5,71,-1,0,1.1f}},1000,2500,2750},
        {{{3,109,71,-1,0},{4,109,-1,0,500}},1200,3000,2500},
        {{{4,71,-1,0,500},{3,109,71,-1,0}},1200,3000,3000},
        {{{3,109,71,-1,0},{3,109,71,-1,0}},1000,2500,2500},
    };
    swgb::Game game(assets);std::string error;
    for(const auto &c:cases) {
        commands=restore.original;
        check(game.init(0x5A17,64,&error,6,1),error.c_str());
        const auto source=game.spawnObjectForTesting(6,109,1,8.5f,8.5f);
        const auto target=game.spawnObjectForTesting(6,71,1,16.5f,8.5f);
        check(source && target && game.objectMaxHitPoints(source)==2500 &&
              game.damageObjectForTesting(source,1500),"Original damagedCC setup");
        const auto *sourcePointer=game.unitPrototypeForTesting(1,109);
        const auto *targetPointer=game.unitPrototypeForTesting(1,71);
        commands=c.commands;
        game.invalidateUnitPrototypesForTesting();
        check(game.researchTechnologyForTesting(1,501),"Apply combined upgrade/HP sequence");
        check(game.objectUnitId(source)==109 && game.objectUnitId(target)==71 &&
              game.unitPrototypeForTesting(1,109)==sourcePointer &&
              game.unitPrototypeForTesting(1,71)==targetPointer && sourcePointer!=targetPointer,
              "Original upgrade preserves separate source/target IDs and stable prototype addresses");
        check(game.objectHitPoints(source)==c.hp && game.objectMaxHitPoints(source)==c.source &&
              game.objectMaxHitPoints(target)==c.target,"Original ordered upgrade and HP mutations");
        check(sourcePointer->standingGraphic[0]==data.civs[6].units[71].standingGraphic[0],
              "Source identity retains upgraded destination artwork");
        const auto newSource=game.spawnObjectForTesting(6,109,1,24.5f,8.5f);
        const auto newTarget=game.spawnObjectForTesting(6,71,1,32.5f,8.5f);
        check(game.objectUnitId(newSource)==109 && game.objectHitPoints(newSource)==c.source &&
              game.objectUnitId(newTarget)==71 && game.objectHitPoints(newTarget)==c.target,
              "New objects use their independent current source/target prototype");
        check(game.researchTechnologyForTesting(1,174) && game.objectHitPoints(source)==c.hp,
              "Unrelated refresh does not replay upgrade/HP history");
    }
    // Copy and reset history across every shared stat consumer, not only HP.
    commands=restore.original;
    check(game.init(0x5A17,64,&error,6,1),error.c_str());
    const auto source=game.spawnObjectForTesting(6,109,1,8.5f,8.5f);
    const auto target=game.spawnObjectForTesting(6,71,1,16.5f,8.5f);
    const auto &destination=data.civs[6].units[71];
    check(!destination.attacks.empty() && !destination.armours.empty(),"CC combat metadata fixture");
    const int attackClass=destination.attacks.front().cls,armourClass=destination.armours.front().cls;
    commands={{4,109,-1,100,7},{4,71,-1,100,20},
              {4,109,-1,101,7},{4,71,-1,101,2},
              {4,109,-1,23,7},{4,71,-1,23,2},
              {4,109,-1,9,float(attackClass*256+7)},{4,71,-1,9,float(attackClass*256+2)},
              {4,109,-1,8,float(armourClass*256+7)},{4,71,-1,8,float(armourClass*256+2)},
              {3,109,71,-1,0},{4,109,-1,100,3},{4,109,-1,101,3},{4,109,-1,23,3},
              {4,109,-1,9,float(attackClass*256+3)},{4,109,-1,8,float(armourClass*256+3)}};
    game.invalidateUnitPrototypesForTesting();
    check(game.researchTechnologyForTesting(1,501),"Apply prototype-copy stat history");
    const auto &sourceUnit=*game.unitPrototypeForTesting(1,109);
    const auto costs=game.unitCostsForTesting(1,sourceUnit),targetCosts=game.unitCostsForTesting(1,destination);
    for(size_t i=0;i<costs.size();++i)if(destination.costs[i].flag)
        check(costs[i].amount==destination.costs[i].amount+23 &&
              targetCosts[i].amount==destination.costs[i].amount+20,"Costs copy target history without aliasing");
    check(game.unitTrainingTimeForTesting(1,sourceUnit)==destination.trainTime+5 &&
          game.unitTrainingTimeForTesting(1,destination)==destination.trainTime+2,
          "Training-time history follows source prototype overwrite");
    check(game.unitSearchRadiusForTesting(1,sourceUnit)==destination.searchRadius+5 &&
          game.unitSearchRadiusForTesting(1,destination)==destination.searchRadius+2,
          "Generic attribute history follows source prototype overwrite");
    check(game.unitAttackForTesting(source,attackClass)==destination.attacks.front().amount+5 &&
          game.unitAttackForTesting(target,attackClass)==destination.attacks.front().amount+2 &&
          game.unitArmourForTesting(source,armourClass)==destination.armours.front().amount+5 &&
          game.unitArmourForTesting(target,armourClass)==destination.armours.front().amount+2,
          "Attack and armour histories copy target then remain independent");
    // Static original48d390/loader mapping: source identity fields survive the copy.
    {
        auto &rawSource=data.civs[6].units[109];
        auto &rawTarget=data.civs[6].units[71];
        struct RestoreUnits {
            swgb::dat::Unit &source,&target,oldSource,oldTarget;
            ~RestoreUnits(){source=std::move(oldSource);target=std::move(oldTarget);}
        } restoreUnits{rawSource,rawTarget,rawSource,rawTarget};
        rawSource.cls=55;rawSource.baseId=123;rawSource.copyId=124;
        rawSource.hideInEditor=0;rawSource.enabled=1;rawSource.disabled=1;rawSource.hotKeyId=456;
        rawTarget.cls=3;rawTarget.baseId=789;rawTarget.copyId=790;
        rawTarget.hideInEditor=0;rawTarget.enabled=0;rawTarget.disabled=0;rawTarget.hotKeyId=987;
        commands=restore.original;
        check(game.init(0x5A17,64,&error,6,1),error.c_str());
        commands={{3,109,71,-1,0},{4,-1,55,101,3},{4,-1,3,101,2}};
        game.invalidateUnitPrototypesForTesting();
        check(game.researchTechnologyForTesting(1,501),"Upgrade preserved-field fixture");
        const auto &src=*game.unitPrototypeForTesting(1,109);
        const auto &dst=*game.unitPrototypeForTesting(1,71);
        check(src.id==109 && src.baseId==123 && src.copyId==790 && src.cls==55 &&
              src.hideInEditor==0 && src.enabled==1 && src.disabled==1 && src.hotKeyId==456,
              "Original upgrade preserves source identity, class, visibility and availability fields");
        check(dst.id==71 && dst.baseId==109 && dst.copyId==790 && dst.cls==3 &&
              dst.hideInEditor==1 && dst.enabled==0 && dst.disabled==0 && dst.hotKeyId==987,
              "Original upgrade hides the independent target and records source base identity");
        check(game.unitTrainingTimeForTesting(1,src)==rawTarget.trainTime+3 &&
              game.unitTrainingTimeForTesting(1,dst)==rawTarget.trainTime+2,
              "Later class selectors use the preserved source class");
    }
    for(bool upgradeRefresh:{false,true}) {
        commands=restore.original;later.clear();
        check(game.init(0x5A17,64,&error,1,1),error.c_str());
        const auto gate=game.spawnObjectForTesting(1,64,1,8.5f,45.5f);
        const auto worker=game.spawnObjectForTesting(1,293,1,8.5f,45.5f);
        check(gate && worker,"Gate prototype refresh fixture");
        const auto &closed=data.civs[1].units[64];
        const std::string openName=closed.name.substr(0,closed.name.size()-4)+"OPEN";
        int openId=-1;
        for(const auto &unit:data.civs[1].units)if(unit.exists && unit.name==openName)openId=unit.id;
        check(openId>=0,"Original gate OPEN variant");
        game.update(.1f,{});
        check(game.objectUnitId(gate)==openId,"Friendly worker opens gate");
        if(upgradeRefresh)commands={{3,109,71,-1,0}};
        game.invalidateUnitPrototypesForTesting();
        check(game.researchTechnologyForTesting(1,upgradeRefresh?501:174),"Refresh open gate prototypes");
        check(game.objectUnitId(gate)==openId,"Refresh retains open gate state");
        check(game.moveObjectForTesting(worker,30.5f,45.5f),"Clear gate corridor");
        for(int step=0;step<5;++step)game.update(.2f,{});
        check(game.objectUnitId(gate)==64,"Cached OPEN gate closes after ordinary and upgrade refresh");
    }
    std::puts("PASS: original in-place unit upgrade identity, ordered HP and independent stat histories");
}
