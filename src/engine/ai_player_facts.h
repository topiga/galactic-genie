// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

namespace swgb {
enum class AiPlayerFact { Human, InGame, PlayersStance, StanceToward };
struct AiPlayerFactPlayer {
    bool exists=true;
    bool computer=false; // Original player virtual1c returns nonzero for computer.
    uint8_t gameState=0; // Original player+a6: only zero satisfies player-in-game.
    std::vector<uint8_t> diplomacy; // Directed stance indexed by actual player ID, including Gaia.
};
struct AiPlayerFactResult {
    bool value=false;
    std::optional<int> matchedPlayer; // Successful any-selector cache; direct/every leave it unchanged.
};
inline std::optional<int> aiPlayerSelector(std::string_view name) {
    constexpr std::string_view any[]={"any-ally","any-computer","any-computer-ally",
        "any-computer-enemy","any-computer-neutral","any-enemy","any-human",
        "any-human-ally","any-human-enemy","any-human-neutral","any-neutral"};
    constexpr std::string_view every[]={"every-ally","every-computer","every-enemy","every-human","every-neutral"};
    for(int i=0;i<11;++i)if(name==any[i])return -101-i;
    for(int i=0;i<5;++i)if(name==every[i])return -201-i;
    return std::nullopt;
}
// Original57cbb0/57cbf0/57d580/57e0b0, with shared578e20/5794d0 selectors.
// Slots are in world order, including Gaia at0; selectors start at1 and do not
// implicitly exclude defeated players. Native callers supply their state adapter.
inline AiPlayerFactResult evaluateAiPlayerFact(AiPlayerFact fact,int selector,int stance,
                                               int self,const std::vector<AiPlayerFactPlayer> &players) {
    const auto valid=[&](int id){return id>=0 && size_t(id)<players.size() && players[size_t(id)].exists;};
    if(!valid(self))return {};
    const auto relation=[&](int from,int to)->int {
        const auto &values=players[size_t(from)].diplomacy;
        return to>=0 && size_t(to)<values.size()?int(values[size_t(to)]):-1;
    };
    const auto test=[&](int id) {
        if(!valid(id))return false;
        switch(fact) {
        case AiPlayerFact::Human:return !players[size_t(id)].computer;
        case AiPlayerFact::InGame:return players[size_t(id)].gameState==0;
        case AiPlayerFact::PlayersStance:return id!=self && relation(id,self)==stance;
        case AiPlayerFact::StanceToward:return relation(self,id)==stance;
        }
        return false;
    };
    const bool any=selector<=-101 && selector>=-111;
    const bool every=selector<=-201 && selector>=-205;
    if(!any && !every)return {test(selector),std::nullopt};
    for(size_t index=1;index<players.size();++index) {
        const int id=int(index);if(!valid(id))continue;
        const auto &player=players[index];const int directed=relation(self,id);
        bool matches=false;
        switch(selector) {
        case -101:case -201:matches=id!=self && directed==0;break;
        case -102:case -202:matches=player.computer;break;
        case -103:matches=id!=self && player.computer && directed==0;break;
        case -104:matches=player.computer && directed==3;break;
        case -105:matches=player.computer && directed==1;break;
        case -106:case -203:matches=directed==3;break;
        case -107:case -204:matches=!player.computer;break;
        case -108:matches=id!=self && !player.computer && directed==0;break;
        case -109:matches=!player.computer && directed==3;break;
        case -110:matches=!player.computer && directed==1;break;
        case -111:case -205:matches=directed==1;break;
        }
        if(!matches)continue;
        const bool result=test(id);
        if(any && result)return {true,id};
        if(every && !result)return {};
    }
    return {every,std::nullopt};
}
} // namespace swgb
