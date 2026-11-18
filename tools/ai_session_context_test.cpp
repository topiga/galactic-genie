// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/engine/ai_session_context.h"
#include <cassert>
#include <iostream>
int main(){
    swgb::AiSessionContextSettings s;s.player=2;s.startingAge=0;s.resources=0;s.mapSizeIndex=0;s.mapId=9;s.victory=0;
    const auto c=swgb::makeAiSessionContext(s,{"NATURE","EMPIRE","GUNGANS","REBEL","NABOO","FEDERATION","WOOKIEES","REPUBLIC","CONFEDERACY"});
    const std::unordered_set<std::string> expected{"EMPIRE","TINY-MAP","DESERT-MAP","VICTORY-STANDARD","TECH-LEVEL-1-START","DIFFICULTY-MODERATE","POPULATION-CAP-75","LOW-RESOURCES-START"};
    assert(c.defines==expected);assert(c.constants.at("my-player-number")==2 && c.constants.at("my-civ")==1);
    assert(c.constants.at("empire")==1 && c.constants.at("federation")==5 && c.constants.at("confederacy")==8);
    s.mapId=999;s.victory.reset();s.mapSizeIndex.reset();s.resources=2;s.startingAge=4;s.difficulty=0;s.population=100;
    const auto other=swgb::makeAiSessionContext(s,{});
    assert(!other.defines.count("DESERT-MAP") && !other.defines.count("SCENARIO-MAP"));
    assert(other.defines.count("MEDIUM-RESOURCES-START") && other.defines.count("TECH-LEVEL-3-START") && other.defines.count("DIFFICULTY-HARDEST") && other.defines.count("POPULATION-CAP-100"));
    const auto unknown=swgb::makeAiSessionContext({},{});
    assert(!unknown.defines.count("TECH-LEVEL-1-START") && !unknown.defines.count("LOW-RESOURCES-START"));
    std::cout<<"AI session context checks passed\n";
}
