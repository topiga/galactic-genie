// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cctype>
namespace swgb {
struct AiSessionContext {
    std::unordered_set<std::string> defines;
    std::unordered_map<std::string,int> constants;
};
struct AiSessionContextSettings {
    int player=1,civilization=1,difficulty=2,population=75;
    // Missing values mean unavailable session metadata, not invented defaults.
    std::optional<int> mapSizeIndex, mapId, victory;
    std::optional<int> startingAge,resources;
    bool deathMatch=false,terminateCommander=false,allTech=false,teamsLocked=false,speedLocked=false;
};
inline AiSessionContext makeAiSessionContext(const AiSessionContextSettings &s,
                                           const std::vector<std::string> &civilizationNames) {
    AiSessionContext out;
    //57f797 registers nonempty DAT civilization+18 strings with their index;
    //57ec2c/46 register the per-player constants before parsing that player's PER.
    for(size_t i=0;i<civilizationNames.size();++i)if(!civilizationNames[i].empty()) {
        auto name=civilizationNames[i];for(char &c:name)c=char(std::tolower(static_cast<unsigned char>(c)));
        out.constants[name]=int(i);
    }
    //57ee3f/410e00 adds only this player's civilization as a type5 define.
    if(s.civilization>=0 && size_t(s.civilization)<civilizationNames.size() && !civilizationNames[size_t(s.civilization)].empty())
        out.defines.insert(civilizationNames[size_t(s.civilization)]);
    out.constants["my-player-number"]=s.player;out.constants["my-civ"]=s.civilization;
    constexpr std::array<const char*,6> sizes{"TINY-MAP","SMALL-MAP","MEDIUM-MAP","NORMAL-MAP","LARGE-MAP","GIANT-MAP"};
    if(s.mapSizeIndex && *s.mapSizeIndex>=0 && *s.mapSizeIndex<int(sizes.size()))out.defines.insert(sizes[size_t(*s.mapSizeIndex)]);
    // Only the production Arid mapping is closed here; do not label unknown maps as scenarios.
    if(s.mapId && *s.mapId==9)out.defines.insert("DESERT-MAP");
    constexpr std::array<const char*,5> victories{"VICTORY-STANDARD","VICTORY-CONQUEST","VICTORY-TIME-LIMIT","VICTORY-SCORE","VICTORY-CUSTOM"};
    if(s.victory && *s.victory>=0 && *s.victory<int(victories.size()))out.defines.insert(victories[size_t(*s.victory)]);
    constexpr std::array<const char*,5> ages{"TECH-LEVEL-1-START","TECH-LEVEL-2-START","TECH-LEVEL-3-START","TECH-LEVEL-4-START","POST-TECH-LEVEL-4-START"};
    const int age=s.startingAge?(*s.startingAge?*s.startingAge:2):-1; //57eb93 default normalization.
    if(age>=2 && age<=6)out.defines.insert(ages[size_t(age-2)]);
    constexpr std::array<const char*,5> difficulties{"DIFFICULTY-HARDEST","DIFFICULTY-HARD","DIFFICULTY-MODERATE","DIFFICULTY-EASY","DIFFICULTY-EASIEST"};
    if(s.difficulty>=0 && s.difficulty<=4)out.defines.insert(difficulties[size_t(s.difficulty)]);
    // Original population getter supplies a byte before the25..250 dispatch.
    const int population=s.population&255;
    if(population>=25 && population<=250 && population%25==0)out.defines.insert("POPULATION-CAP-"+std::to_string(population));
    constexpr std::array<const char*,3> resources{"LOW-RESOURCES-START","MEDIUM-RESOURCES-START","HIGH-RESOURCES-START"};
    const int resource=s.resources?(*s.resources?*s.resources:1):-1; //57ebab default normalization.
    if(resource>=1 && resource<=3)out.defines.insert(resources[size_t(resource-1)]);
    if(s.deathMatch)out.defines.insert("DEATH-MATCH");
    if(s.terminateCommander)out.defines.insert("TERMINATE-THE-COMMANDER");
    if(s.allTech)out.defines.insert("ALL-TECH");
    if(s.teamsLocked)out.defines.insert("TEAMS-LOCKED");
    if(s.speedLocked)out.defines.insert("GAME-SPEED-LOCKED");
    return out;
}
} // namespace swgb
