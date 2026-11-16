// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void baseObjectDamageChecks(swgb::Assets &assets) {
    auto &data=const_cast<swgb::dat::DatFile &>(assets.dat());
    auto &worker=data.civs.at(1).units.at(123);
    struct Restore {
        swgb::dat::Unit &unit, original;
        ~Restore() { unit=std::move(original); }
    } restore{worker,worker};
    for (const int amount : {3,0,-2}) {
        // Original base calculator ignores packet class and has no minimum-one clamp.
        worker.attacks={{999,int16_t(amount)}};
        swgb::Game game(assets);std::string error;
        check(game.init(7,64,&error,1,1),error.c_str());
        const auto source=game.spawnObjectForTesting(1,123,1,10.5f,10.5f);
        for(const int type : {10,15,20,25,30,40}) {
            const auto &units=data.civs.at(0).units;
            const auto found=std::find_if(units.begin(),units.end(),[&](const swgb::dat::Unit &unit) {
                return unit.exists && unit.type==type;
            });
            if(found==units.end())continue; // Some runtime classes have no shipped Gaia prototype.
            const auto target=game.spawnObjectForTesting(0,int(found-units.begin()),0,12.5f,10.5f);
            check(source && target,"Base damage target fixture");
            check(game.objectAttackDamage(source,target)==amount,
                  "Base runtime classes sum signed attack amounts without armour matching or minimum damage");
        }
    }
}
