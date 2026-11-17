// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void frozenTargetVisibilityChecks(swgb::Assets &assets) {
    auto &units=const_cast<swgb::dat::DatFile &>(assets.dat()).civs.at(1).units;
    struct Restore {swgb::dat::Unit &unit, original;~Restore(){unit=std::move(original);}};
    Restore restoreObserver{units.at(83),units.at(83)},restoreBuilding{units.at(70),units.at(70)};
    units[83].lineOfSight=0;
    units[70].collisionSize[0]=units[70].collisionSize[1]=1.f;
    swgb::Scenario scenario;scenario.map.width=scenario.map.height=32;scenario.map.tiles.resize(32*32);
    scenario.nextUnitId=1;scenario.playerSlotCount=2;scenario.civilizations.fill(1);
    for(int i=0;i<2;++i){scenario.players[i].active=true;scenario.players[i].human=i==0;scenario.players[i].civilization=1;}
    swgb::Game game(assets);std::string error;check(game.initScenario(scenario,&error),error.c_str());
    const auto observer=game.spawnObjectForTesting(1,83,1,15.5f,16.5f);
    const auto large=game.spawnObjectForTesting(1,70,2,16.f,16.f);
    const auto small=game.spawnObjectForTesting(1,72,2,16.f,16.f);
    check(observer && large && small && game.setBuildingFrozenForTesting(large,true) &&
          game.setBuildingFrozenForTesting(small,true),"Frozen target footprint fixtures");
    check(!game.worldSoundVisibleForTesting(16.f,16.f),"Target center is outside observer's zero-radius sight");
    check(game.targetVisibleForTesting(1,large) && !game.targetVisibleForTesting(1,small),
          "Original radius>=1 four-cell mask sees a noncenter cell; radius<1 remains center-only");
    for(const auto &point:std::array<std::array<float,2>,4>{{{15.5f,16.5f},{16.5f,16.5f},{15.5f,15.5f},{16.5f,15.5f}}}) {
        check(game.moveObjectForTesting(observer,point[0],point[1]),"Place observer on original sampled cell");
        check(game.targetVisibleForTesting(1,large),"Each original frozen-footprint cell independently admits the target");
    }
    game.moveObjectForTesting(observer,14.5f,16.5f);
    check(!game.targetVisibleForTesting(1,large),"Adjacent cells outside original four-cell mask do not admit target");
    game.moveObjectForTesting(observer,15.5f,16.5f);
    game.setBuildingFrozenForTesting(large,false);
    check(game.targetVisibleForTesting(1,large),"Integer-center radius1 visibility survives thaw through normal stencil");
    game.setBuildingFrozenForTesting(small,false);
    check(game.targetVisibleForTesting(1,small),"Normal small-radius integer-center stencil differs from frozen center-only sampling");
    game.moveObjectForTesting(large,16.5f,15.5f);
    check(game.targetVisibleForTesting(1,large),"Fractional-center normal footprint includes its outer corner");
    game.moveObjectForTesting(observer,16.5f,16.5f);
    check(!game.targetVisibleForTesting(1,large),"Normal stencil does not fill unsampled side/interior cells");
    game.moveObjectForTesting(observer,16.5f,15.5f);
    check(game.targetVisibleForTesting(1,large),"Center remains visible independently of perimeter stencil");
    std::puts("PASS: original frozen-target four-cell sampling and small-footprint discriminator");
}
