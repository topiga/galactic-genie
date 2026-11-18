// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Original 481560: no member default task ->481860 movement; task7 ->481ed0.
[[gnu::noinline]] static void groupDefaultTaskGameplayChecks(swgb::Assets &assets) {
    const auto makeGame=[&] {
        swgb::Scenario s;s.map.width=s.map.height=64;s.map.tiles.resize(64*64);
        s.nextUnitId=1;s.playerSlotCount=2;s.civilizations.fill(1);
        for(size_t p=0;p<s.players.size();++p) {
            s.players[p].active=p<2;s.players[p].human=true;s.players[p].civilization=1;
            s.players[p].diplomacy.fill(3);if(p<2)s.players[p].diplomacy[p+1]=0;
        }
        auto g=std::make_unique<swgb::Game>(assets);std::string error;
        check(g->initScenario(s,&error),error.c_str());return g;
    };
    {
        auto game=makeGame();auto &g=*game;
        const auto first=g.spawnObjectForTesting(1,702,1,12,12);
        const auto second=g.spawnObjectForTesting(1,702,1,12,14);
        const auto target=g.spawnObjectForTesting(1,87,2,36,12);
        check(first && second && target,"Create actual anti-air and Troop Center assets");
        for(auto id:{first,second,target})g.setAttackModeForTesting(id,3);
        g.update(0,{});
        check(!g.canAttackTargetForTesting(first,target),"Anti-air still cannot damage ground buildings");
        const float hp=g.objectHitPoints(target);
        check(g.issueGroupAttackForTesting({first,second},target),"No default task uses original group movement fallback");
        g.updatePaused(5,{});
        check(g.objectPosition(first)[0]==12,"Paused fallback movement does not advance");
        for(int i=0;i<180;++i)g.update(1.f/30,{});
        check(g.objectPosition(first)[0]>14 && g.objectPosition(second)[0]>14,
              "Both rejected attackers move toward the target instead of remaining idle");
        check(g.attackTargetForTesting(first)==0 && g.attackTargetForTesting(second)==0 &&
              g.objectHitPoints(target)==hp,"Movement fallback never grants a ground attack");
    }
    {
        auto game=makeGame();auto &g=*game;
        const auto aa=g.spawnObjectForTesting(1,702,1,12,12);
        const auto troop=g.spawnObjectForTesting(1,329,1,13,15);
        const auto nearby=g.spawnObjectForTesting(1,702,1,32,12);
        const auto target=g.spawnObjectForTesting(1,87,2,36,12);
        for(auto id:{aa,troop,nearby,target})g.setAttackModeForTesting(id,3);
        g.update(0,{});
        check(g.issueGroupAttackForTesting({aa,troop,nearby},target),"Mixed group finds a ground-attack task");
        check(g.attackTargetForTesting(troop)==target,"Valid attack dispatch is retained");
        for(int i=0;i<60;++i)g.update(1.f/30,{});
        check(g.objectPosition(aa)[0]>12 && !g.attackTargetForTesting(aa),
              "Distant anti-air accompanies the attack without acquiring a ground target");
        check(g.objectPosition(nearby)[0]==32 && g.objectPosition(nearby)[1]==12 &&
              !g.attackTargetForTesting(nearby),"An anti-air member in range receives no movement or ground attack");
    }
    {
        auto game=makeGame();auto &g=*game;
        int aircraft=-1;
        for(const auto &unit:assets.dat().civs[1].units)
            if(unit.type==70 && unit.cls==48 && unit.speed>0 && unit.hitPoints>0) {aircraft=unit.id;break;}
        check(aircraft>=0,"Actual DAT provides an aircraft target");
        const auto aa=g.spawnObjectForTesting(1,702,1,20,20);
        const auto target=g.spawnObjectForTesting(1,aircraft,2,24,20);
        g.setAttackModeForTesting(aa,3);g.setAttackModeForTesting(target,3);g.update(0,{});
        check(g.canAttackTargetForTesting(aa,target) && g.issueGroupAttackForTesting({aa},target),
              "Anti-air still selects its attack task against aircraft");
        check(g.attackTargetForTesting(aa)==target,"Aircraft order remains an attack rather than fallback movement");
    }
}
