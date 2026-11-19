// SPDX-License-Identifier: GPL-3.0-or-later
#include "game.h"
#include <cmath>
#include <limits>

namespace swgb {
namespace {
int contextDefineGroup(const std::string &symbol) {
    const auto name=normalizeAiSymbol(symbol);
    if(name.rfind("difficulty-",0)==0)return 1;
    if(name.rfind("population-cap-",0)==0)return 2;
    if(name.rfind("victory-",0)==0)return 3;
    if(name.rfind("tech-level-",0)==0 || name.rfind("post-tech-level-",0)==0)return 4;
    if(name=="low-resources-start" || name=="medium-resources-start" || name=="high-resources-start")return 5;
    for(const char *size:{"tiny-map","small-map","medium-map","normal-map","large-map","giant-map"})
        if(name==size)return 6;
    return 0;
}
}
AiSessionContext Game::aiScriptContext(AiSessionContextSettings settings,
        const std::unordered_set<std::string> &defines) const {
    std::vector<std::string> civilizations;
    for(const auto &civilization:assets_.dat().civs)civilizations.push_back(civilization.name2);
    auto context=makeAiSessionContext(settings,civilizations);
    const auto &data=assets_.dat();
    if(!data.civs.empty())for(const auto &unit:data.civs.front().units) {
        if(!unit.exists || (unit.type!=dat::UT_Creatable && unit.type!=dat::UT_Building))continue;
        const auto symbol=normalizeAiSymbol(unit.name2);
        if(symbol.rfind("unit-",0)==0 || symbol.rfind("boat-",0)==0 ||
           symbol.rfind("siege-",0)==0 || symbol.rfind("sub-",0)==0)
            context.constants.emplace(symbol,unit.id);
    }
    for(size_t id=0;id<data.techs.size();++id) {
        const auto &tech=data.techs[id];
        const auto symbol=normalizeAiSymbol(tech.name2);
        if(tech.locationId>=0 && symbol.rfind("ut-",0)==0)
            context.constants.emplace(symbol,int(id));
    }
    //57f886 registers the signed line identity before parsing any personality/includes.
    for(const auto &line:assets_.dat().unitLines)if(!line.name.empty())
        context.constants[normalizeAiSymbol(line.name)]=-int(line.id);
    // Explicit compiler defines override the corresponding setup category;
    // otherwise a custom personality could see two difficulty/population modes.
    for(const auto &symbol:defines) {
        const int group=contextDefineGroup(symbol);
        if(group)for(auto it=context.defines.begin();it!=context.defines.end();) {
            if(contextDefineGroup(*it)==group)it=context.defines.erase(it);else ++it;
        }
    }
    context.defines.insert(defines.begin(),defines.end());
    return context;
}
AiSessionContext Game::aiScriptContext(int player,const std::unordered_set<std::string> &defines) const {
    auto settings=aiSessionContext_;
    settings.player=player;settings.civilization=civilizationForPlayer(player);settings.difficulty=difficulty_;
    const float population=players_[size_t(player-1)].populationLimit;
    settings.population=std::isfinite(population) && population>=0 && double(population)<=INT32_MAX
        ?int(population):-1;
    return aiScriptContext(settings,defines);
}
} // namespace swgb
