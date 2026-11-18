// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
[[gnu::noinline]] static void conversionOwnershipGameplayChecks(swgb::Assets &assets) {
    using namespace swgb;
    auto &data=const_cast<dat::DatFile&>(assets.dat());
    auto &first=data.effects[size_t(data.techs[501].effectId)].commands;
    auto &later=data.effects[size_t(data.techs[174].effectId)].commands;
    struct Restore { std::vector<dat::EffectCommand>& commands;std::vector<dat::EffectCommand> before;
        ~Restore(){commands=std::move(before);} } restoreFirst{first,first},restoreLater{later,later};
    const auto &unit=data.civs[6].units[307];
    check(!unit.attacks.empty() && !unit.armours.empty(),"Conversion fixture has actual troop weapons and armour");
    const int attackClass=unit.attacks.front().cls,armourClass=unit.armours.front().cls;
    first={{4,307,-1,0,20},{4,307,-1,5,.3f},{4,307,-1,23,3},
           {4,307,-1,9,float(attackClass*256+7)},{4,307,-1,8,float(armourClass*256+5)}};
    later={{4,307,-1,0,80},{4,307,-1,5,.9f},{4,307,-1,23,9},
           {4,307,-1,9,float(attackClass*256+19)},{4,307,-1,8,float(armourClass*256+17)}};
    Game game(assets);std::string error;
    check(game.init(0x5A17,64,&error,6,6),error.c_str());
    const auto foreign=game.spawnObjectForTesting(6,307,1,15,15);
    const auto native=game.spawnObjectForTesting(6,307,2,25,15);
    check(foreign && native && game.researchTechnologyForTesting(1,501) &&
          game.researchTechnologyForTesting(2,174),"Owners research distinct troop statistics before conversion");
    check(game.damageObjectForTesting(foreign,10),"Converted troop starts wounded");
    const float hp=game.objectHitPoints(foreign),maximum=game.objectMaxHitPoints(foreign);
    const int attack=game.unitAttackForTesting(foreign,attackClass),armour=game.unitArmourForTesting(foreign,armourClass);
    const float speed=game.unitAttributeForTesting(foreign,5,unit.speed);
    const float search=game.unitAttributeForTesting(foreign,23,unit.searchRadius);
    const int ownBefore=game.aiObjectCountForTesting(1,"307",false),otherBefore=game.aiObjectCountForTesting(2,"307",false);
    check(game.convertObjectForTesting(foreign,2),"Ordinary mobile conversion changes ownership");
    const auto preserved=[&] {
        return game.objectHitPoints(foreign)==hp && game.objectMaxHitPoints(foreign)==maximum &&
            game.unitAttackForTesting(foreign,attackClass)==attack && game.unitArmourForTesting(foreign,armourClass)==armour &&
            game.unitAttributeForTesting(foreign,5,unit.speed)==speed && game.unitAttributeForTesting(foreign,23,unit.searchRadius)==search;
    };
    check(game.objectPlayer(foreign)==2 && preserved(),"Conversion retains wounded HP and source attack armour speed and search statistics");
    check(game.aiObjectCountForTesting(1,"307",false)==ownBefore-1 && game.aiObjectCountForTesting(2,"307",false)==otherBefore+1,
          "Converted actor moves between owner prototype counters exactly once");
    check(game.researchTechnologyForTesting(2,501) && game.researchTechnologyForTesting(1,174) && preserved(),
          "Later source and recipient attribute research leaves the captured private master unchanged");
    check(game.unitAttackForTesting(native,attackClass)!=attack,"Recipient shared prototype still receives ordinary research");
    check(game.convertObjectForTesting(foreign,1) && preserved(),"Repeated conversion preserves the same captured stat history");
    check(game.aiObjectCountForTesting(1,"307",false)==ownBefore && game.aiObjectCountForTesting(2,"307",false)==otherBefore,
          "Repeated conversion does not double-register ownership");
    first={{3,307,329,-1,0},{4,307,-1,0,45}};
    check(game.init(0x5A17,64,&error,6,6),error.c_str());
    const auto oldTroop=game.spawnObjectForTesting(6,307,1,15,15);
    const auto recipientTroop=game.spawnObjectForTesting(6,307,2,25,15);
    check(oldTroop && recipientTroop && game.researchTechnologyForTesting(1,174),"Private-master upgrade fixture");
    const float oldMaximum=game.objectMaxHitPoints(oldTroop);
    const int oldAttack=game.unitAttackForTesting(oldTroop,attackClass);
    check(game.convertObjectForTesting(oldTroop,2) && game.researchTechnologyForTesting(2,501),
          "Recipient researches a real prototype-copy upgrade after conversion");
    check(game.objectMaxHitPoints(oldTroop)==oldMaximum && game.unitAttackForTesting(oldTroop,attackClass)==oldAttack &&
          game.objectMaxHitPoints(recipientTroop)!=oldMaximum,
          "Ordinary recipient upgrade skips the captured private master while updating its own troop");
    first={{4,-1,58,0,100},{4,-1,58,5,.4f}};
    later={{4,-1,58,0,20}};
    check(game.init(0x5A17,64,&error,6,6),error.c_str());
    const auto worker=game.spawnObjectForTesting(6,83,1,15,15);
    const auto farm=game.spawnObjectForTesting(6,50,1,18,15);
    const auto carbon=game.spawnObjectForTesting(0,66,0,20,20);
    check(worker && farm && carbon && game.researchTechnologyForTesting(1,501) &&
          game.researchTechnologyForTesting(2,174),"Worker owners have distinct maximum HP before conversion");
    check(game.issueGatherForTesting(worker,farm) && game.setWorkerCargoForTesting(worker,0,3.5f),
          "Worker conversion starts with a real farm order and carried food");
    game.update(0,{});
    const float workerHP=game.objectHitPoints(worker),workerMaximum=game.objectMaxHitPoints(worker);
    check(game.convertObjectForTesting(worker,2) && game.privateMasterForTesting(worker) &&
          game.objectHitPoints(worker)==workerHP && game.objectMaxHitPoints(worker)==workerMaximum &&
          game.workerCargoForTesting(worker)==3.5f && !game.objectGatheringTarget(worker,farm),
          "523 clears converted worker task references while retaining private statistics and cargo");
    const auto &recipientBase=*game.unitPrototypeForTesting(2,83);
    const float recipientMaximum=game.unitAttributeForTesting(2,recipientBase,0,recipientBase.hitPoints);
    check(game.issueGatherForTesting(worker,carbon) && !game.privateMasterForTesting(worker),
          "A new worker role retires the converted master and adopts recipient research");
    check(std::abs(game.objectHitPoints(worker)-float((double(workerHP)/recipientMaximum+double(1.0e-7f))*recipientMaximum))<.001f &&
          game.objectMaxHitPoints(worker)==recipientMaximum && game.workerCargoForTesting(worker)==3.5f,
          "Role change uses recipient old-role HP denominator without clamping carried HP or discarding cargo");
    int ramSource=-1,ramDestination=-1;
    for(const auto &candidate:data.civs[6].units)
        if(candidate.exists && candidate.type==dat::UT_Creatable && candidate.cls==35) {
            if(ramSource<0)ramSource=candidate.id;
            else if(candidate.hitPoints!=data.civs[6].units[size_t(ramSource)].hitPoints) {ramDestination=candidate.id;break;}
        }
    check(ramSource>=0 && ramDestination>=0,"Actual Clone Campaigns ram upgrade has distinct source and destination HP");
    first={{4,int16_t(ramSource),-1,0,100}};
    later={{3,int16_t(ramSource),int16_t(ramDestination),-1,0},{4,int16_t(ramSource),-1,0,50}};
    check(game.init(0x5A17,64,&error,6,6),error.c_str());
    const auto ram=game.spawnObjectForTesting(6,ramSource,1,15,15);
    check(ram && game.researchTechnologyForTesting(1,501) && game.damageObjectForTesting(ram,10) &&
          game.convertObjectForTesting(ram,2),"Converted ram initially retains source upgrades and damaged HP");
    const float ramHP=game.objectHitPoints(ram);
    const auto &recipientRam=*game.unitPrototypeForTesting(2,ramSource);
    const float oldRamMaximum=game.unitAttributeForTesting(2,recipientRam,0,recipientRam.hitPoints);
    const auto &recipientUpgrade=*game.unitPrototypeForTesting(2,ramDestination);
    const float newRamMaximum=game.unitAttributeForTesting(2,recipientUpgrade,0,recipientUpgrade.hitPoints);
    check(game.researchTechnologyForTesting(2,174),"Recipient researches the original private-ram upgrade exception");
    check(game.privateMasterForTesting(ram) && game.objectUnitId(ram)==ramSource &&
          game.objectMaxHitPoints(ram)==newRamMaximum &&
          game.objectHitPoints(ram)==float(double(float(double(ramHP)/oldRamMaximum))*newRamMaximum),
          "Primary-list private ram adopts recipient upgrade with source ID and recipient HP denominator; later attribute stays private");
    first=restoreFirst.before;later=restoreLater.before;
    check(game.initSkirmish(0x5A17,&error,false,6,6),error.c_str());
    const auto training=game.spawnObjectForTesting(6,87,1,35,35);
    for(int resource=0;resource<4;++resource)game.setResourceForTesting(1,resource,1000);
    std::array<float,4> oldStock{},newStock{};
    for(int resource=0;resource<4;++resource) {
        oldStock[resource]=game.resource(1,resource);newStock[resource]=game.resource(2,resource);
    }
    check(training && game.queueUnitForTesting(training,307) && game.queueTechnologyForTesting(training,174) &&
          game.productionQueueSizeForTesting(training)==2,
          "Conversion cancellation fixture installs unit and research entries at the old owner's building");
    // The queue test hooks install entries without charging stock; assert the exact refund delta.
    for(const auto &cost:game.unitCostsForTesting(1,data.civs[6].units[307]))
        if(cost.flag && cost.type>=0 && cost.type<4 && cost.amount>0)oldStock[size_t(cost.type)]+=cost.amount;
    for(const auto &cost:game.researchCostsForTesting(1,data.techs[174]))
        if(cost.flag && cost.type>=0 && cost.type<4 && cost.amount>0)oldStock[size_t(cost.type)]+=cost.amount;
    check(game.convertObjectForTesting(training,2) && game.productionQueueSizeForTesting(training)==0,
          "Converted building clears its former owner's training queue");
    for(int resource=0;resource<4;++resource)
        check(game.resource(1,resource)==oldStock[resource] && game.resource(2,resource)==newStock[resource],
              "Conversion refunds the full queued cost to the old owner only");

    // Scenario interiors preserve admission order without movement timing obscuring transfer semantics.
    const auto occupantsFixture=[&](int container,bool blocked) {
        Scenario scenario;scenario.map.width=scenario.map.height=64;scenario.map.tiles.resize(64*64);
        scenario.playerSlotCount=2;scenario.enabledPlayerCount=2;scenario.nextUnitId=4;
        scenario.civilizations.fill(6);
        for(int player=0;player<2;++player) {
            scenario.players[size_t(player)].active=true;scenario.players[size_t(player)].human=true;
            scenario.players[size_t(player)].civilization=6;
        }
        for(int id=1;id<=3;++id) {
            ScenarioUnit u;u.spawnId=uint32_t(id);u.unitId=uint16_t(id==1?container:83);
            u.player=1;u.state=2;u.x=u.y=30;u.garrisonedInId=id==1?-1:1;scenario.units.push_back(u);
        }
        check(game.initScenario(scenario,&error),error.c_str());
        check(game.garrisonedCount(1)==2,"Conversion fixture retains two ordered passengers");
        if(blocked)for(int y=20;y<=40;++y)for(int x=20;x<=40;++x)game.setTerrainForTesting(x,y,1);
        check(game.convertObjectForTesting(1,2),"Occupied container transfers through the original ownership policy");
        check(game.objectPlayer(1)==2 && (!game.objectActive(2) || game.objectPlayer(2)==1) &&
              (!game.objectActive(3) || game.objectPlayer(3)==1),
              "Container conversion never converts its passengers");
    };
    occupantsFixture(109,false);
    check(game.garrisonedCount(1)==0 && game.objectActive(2) && game.objectActive(3),
          "Building conversion ejects passengers alive when exits are available");
    occupantsFixture(109,true);
    check(game.garrisonedCount(1)==0 && !game.objectActive(2) && !game.objectActive(3),
          "Building conversion destroys remaining passengers when every exit is blocked");
    int assault=-1,transport=-1;
    for(const auto &candidate:data.civs[6].units)if(candidate.exists && candidate.type==dat::UT_Creatable && candidate.garrisonCapacity>0) {
        if(candidate.cls==53 && assault<0)assault=candidate.id;
        if(candidate.cls!=53 && candidate.cls!=35 && transport<0)transport=candidate.id;
    }
    check(assault>=0 && transport>=0,"Installed Clone Campaigns data has assault and ordinary mobile containers");
    occupantsFixture(assault,true);
    check(game.garrisonedCount(1)==0 && !game.objectActive(2) && !game.objectActive(3),
          "Class53 conversion destroys passengers left after failed ejection");
    occupantsFixture(transport,false);
    check(game.garrisonedCount(1)==2 && game.objectActive(2) && game.objectActive(3) &&
          game.garrisonContainerForTesting(2)==1 && game.garrisonContainerForTesting(3)==1,
          "Other mobile containers retain their old-owner passengers inside");
    std::puts("PASS private converted troop statistics and recipient research isolation");
}
