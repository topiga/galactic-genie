// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Actual original933 live/master callbacks, including fractional and negative controls.
static void hitPointEffectChecks(swgb::Assets &assets) {
    auto &data=const_cast<swgb::dat::DatFile &>(assets.dat());
    auto &commands=data.effects[size_t(data.techs[501].effectId)].commands;
    struct Restore {
        std::vector<swgb::dat::EffectCommand> &commands;
        std::vector<swgb::dat::EffectCommand> original;
        ~Restore(){commands=std::move(original);}
    } restore{commands,commands};
    auto &later=data.effects[size_t(data.techs[174].effectId)].commands;
    Restore restoreLater{later,later};later.clear();
    const struct {std::vector<swgb::dat::EffectCommand> commands;float hp,maximum;} cases[]={
        {{{0,933,-1,0,-1000.0f}},-400.0f,-1000.0f},
        {{{4,933,-1,0,-6000.0f}},-400.0f,-1000.0f},
        {{{5,933,-1,0,-0.1f}},-199.0f,-499.0f},
        {{{4,933,-1,0,-1.9f}},1999.0f,4999.0f},
        {{{4,933,-1,0,1.9f}},2000.0f,5001.0f},
        {{{0,933,-1,0,5500.0f}},2200.0f,5500.0f},
        {{{0,933,-1,0,1000.0f}},400.0f,1000.0f},
        {{{0,933,-1,0,0.0f}},0.0f,0.0f},
        {{{0,933,-1,0,0.5f}},0.0f,0.0f},
        {{{0,933,-1,0,5000.75f}},2000.0f,5000.0f},
        {{{4,933,-1,0,500.0f}},2200.0f,5500.0f},
        {{{4,933,-1,0,-3000.0f}},800.0f,2000.0f},
        {{{4,933,-1,0,-5000.0f}},0.0f,0.0f},
        {{{4,933,-1,0,0.5f}},2000.0f,5000.0f},
        {{{4,933,-1,0,-0.5f}},1999.0f,5000.0f},
        {{{5,933,-1,0,1.1f}},2200.0f,5500.0f},
        {{{5,933,-1,0,0.1f}},200.0f,500.0f},
        {{{5,933,-1,0,0.0f}},0.0f,0.0f},
        {{{5,933,-1,0,0.0001f}},0.0f,0.0f},
        {{{4,933,-1,0,500.0f},{5,933,-1,0,1.1f}},2420.0f,6050.0f},
        {{{5,933,-1,0,1.1f},{4,933,-1,0,500.0f}},2400.0f,6000.0f},
    };
    swgb::Game game(assets);std::string error;
    for(const auto &c:cases) {
        commands=restore.original;
        check(game.init(0x5A17,64,&error,1,1),error.c_str());
        const auto id=game.spawnObjectForTesting(1,933,1,8.5f,8.5f);
        check(id && game.objectMaxHitPoints(id)==5000 && game.damageObjectForTesting(id,3000),"Original injured fortress setup");
        commands=c.commands;
        check(game.researchTechnologyForTesting(1,501),"Apply HP command sequence through research");
        check(game.objectHitPoints(id)==c.hp && game.objectMaxHitPoints(id)==c.maximum,
              "Original per-command live HP and signed-short master arithmetic");
        check(game.researchTechnologyForTesting(1,501) && game.objectHitPoints(id)==c.hp,
              "Repeated research must not replay HP commands");
        check(game.researchTechnologyForTesting(1,174) && game.objectHitPoints(id)==c.hp &&
              game.objectMaxHitPoints(id)==c.maximum,"Unrelated refresh must not replay earlier HP commands");
    }
    std::puts("PASS: original live HP scaling, fractional rounding, reductions and command ordering");
}
