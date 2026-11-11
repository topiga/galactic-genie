// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

static void combatModifierChecks(swgb::Assets &assets) {
    swgb::Game game(assets); std::string error;
    check(game.init(0x5A17,64,&error,1,1),error.c_str());
    const auto turret=game.spawnObjectForTesting(1,79,1,10.5f,10.5f);
    const auto infantry=game.spawnObjectForTesting(1,307,1,12.5f,10.5f);
    const auto troop=game.spawnObjectForTesting(1,307,2,16.5f,10.5f);
    const auto house=game.spawnObjectForTesting(1,70,2,20.5f,10.5f);
    const auto ship=game.spawnObjectForTesting(1,815,2,24.5f,10.5f);
    check(turret && infantry && troop && house && ship,"Combat modifier fixtures");
    check(game.objectAttackDamage(turret,troop)==10 &&
          game.objectAttackDamage(turret,house)==1 &&
          game.objectAttackDamage(turret,ship)==1,"Original unmodified Light Turret damage");
    const int infantryDamage=game.objectAttackDamage(infantry,troop);
    check(game.researchTechnologyForTesting(1,174),"Research Ion Accu-Accelerator");
    // Original FUN_00489d20: class-specific unsigned percentage, rounded after each command.
    check(game.objectAttackDamage(turret,troop)==22,
          "Ion Accu-Accelerator applies packed 225 percent only to matching attack classes");
    check(game.objectAttackDamage(turret,house)==5,
          "Packed multiplier preserves zero attacks and subtracts building armour");
    const int shipDamage=game.objectAttackDamage(turret,ship);
    check(shipDamage==8,
          "Original cruiser comparison: naval multiplier yields eight damage per hit");
    check(game.objectAttackDamage(infantry,troop)==infantryDamage,
          "Turret technology leaves infantry attacks unchanged");
    check(game.researchTechnologyForTesting(1,174) && game.objectAttackDamage(turret,troop)==22,
          "Repeated research does not apply the multiplier twice");

    // Replace only this test's effect temporarily; restore before assertions or other fixtures run.
    auto &data=const_cast<swgb::dat::DatFile &>(assets.dat());
    auto &commands=data.effects[size_t(data.techs[174].effectId)].commands;
    const auto original=commands;
    auto multiplier=original.front();
    multiplier.type=5; multiplier.a=79; multiplier.b=-1; multiplier.c=9;
    multiplier.d=float(14*256+110);
    commands={multiplier,multiplier};
    const int twiceRoundedDamage=game.objectAttackDamage(turret,troop);
    commands=original;
    check(twiceRoundedDamage==10,
          "Two 110 percent commands each round attack 4 back to 4, not 5 after combining");
    std::puts("PASS: packed combat multipliers, armour subtraction, naval class, research isolation and per-command rounding");
}
