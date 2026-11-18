// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/ai_player_facts.h"
#include "engine/ai_military_scheduler.h"
#include <cstdlib>
#include <iostream>
using namespace swgb;
static void check(bool value,const char *message) {if(!value){std::cerr<<message<<'\n';std::exit(1);}}
int main() {
    const std::vector<AiPlayerFactPlayer> players={
        {true,true,0,{0,0,0,0,0}}, {true,true,0,{1,0,3,0,0}},
        {true,false,0,{1,0,0,1,3}}, {true,true,0,{1,3,1,0,3}}, {true,false,2,{1,0,3,3,0}}};
    struct Golden {AiPlayerFact fact;int selector,stance;bool expected;};
    // Actual original fact+selector execution; only rule-generation40f680 modeled.
    const Golden golden[]={
        {AiPlayerFact::Human,-111,0,false},
        {AiPlayerFact::Human,-110,0,false},
        {AiPlayerFact::Human,-109,0,true},
        {AiPlayerFact::Human,-108,0,true},
        {AiPlayerFact::Human,-107,0,true},
        {AiPlayerFact::Human,-106,0,true},
        {AiPlayerFact::Human,-105,0,false},
        {AiPlayerFact::Human,-104,0,false},
        {AiPlayerFact::Human,-103,0,false},
        {AiPlayerFact::Human,-102,0,false},
        {AiPlayerFact::Human,-101,0,true},
        {AiPlayerFact::Human,-205,0,true},
        {AiPlayerFact::Human,-204,0,true},
        {AiPlayerFact::Human,-203,0,true},
        {AiPlayerFact::Human,-202,0,false},
        {AiPlayerFact::Human,-201,0,false},
        {AiPlayerFact::Human,0,0,false},
        {AiPlayerFact::Human,1,0,false},
        {AiPlayerFact::Human,2,0,true},
        {AiPlayerFact::Human,3,0,false},
        {AiPlayerFact::Human,4,0,true},
        {AiPlayerFact::Human,5,0,false},
        {AiPlayerFact::Human,-99,0,false},
        {AiPlayerFact::Human,-112,0,false},
        {AiPlayerFact::InGame,-111,0,false},
        {AiPlayerFact::InGame,-110,0,false},
        {AiPlayerFact::InGame,-109,0,true},
        {AiPlayerFact::InGame,-108,0,false},
        {AiPlayerFact::InGame,-107,0,true},
        {AiPlayerFact::InGame,-106,0,true},
        {AiPlayerFact::InGame,-105,0,false},
        {AiPlayerFact::InGame,-104,0,false},
        {AiPlayerFact::InGame,-103,0,true},
        {AiPlayerFact::InGame,-102,0,true},
        {AiPlayerFact::InGame,-101,0,true},
        {AiPlayerFact::InGame,-205,0,true},
        {AiPlayerFact::InGame,-204,0,false},
        {AiPlayerFact::InGame,-203,0,true},
        {AiPlayerFact::InGame,-202,0,true},
        {AiPlayerFact::InGame,-201,0,false},
        {AiPlayerFact::InGame,0,0,true},
        {AiPlayerFact::InGame,1,0,true},
        {AiPlayerFact::InGame,2,0,true},
        {AiPlayerFact::InGame,3,0,true},
        {AiPlayerFact::InGame,4,0,false},
        {AiPlayerFact::InGame,5,0,false},
        {AiPlayerFact::InGame,-99,0,false},
        {AiPlayerFact::InGame,-112,0,false},
        {AiPlayerFact::PlayersStance,-111,0,false},
        {AiPlayerFact::PlayersStance,-111,1,false},
        {AiPlayerFact::PlayersStance,-111,3,false},
        {AiPlayerFact::PlayersStance,-110,0,false},
        {AiPlayerFact::PlayersStance,-110,1,false},
        {AiPlayerFact::PlayersStance,-110,3,false},
        {AiPlayerFact::PlayersStance,-109,0,true},
        {AiPlayerFact::PlayersStance,-109,1,false},
        {AiPlayerFact::PlayersStance,-109,3,false},
        {AiPlayerFact::PlayersStance,-108,0,true},
        {AiPlayerFact::PlayersStance,-108,1,false},
        {AiPlayerFact::PlayersStance,-108,3,false},
        {AiPlayerFact::PlayersStance,-107,0,true},
        {AiPlayerFact::PlayersStance,-107,1,false},
        {AiPlayerFact::PlayersStance,-107,3,false},
        {AiPlayerFact::PlayersStance,-106,0,true},
        {AiPlayerFact::PlayersStance,-106,1,false},
        {AiPlayerFact::PlayersStance,-106,3,false},
        {AiPlayerFact::PlayersStance,-105,0,false},
        {AiPlayerFact::PlayersStance,-105,1,false},
        {AiPlayerFact::PlayersStance,-105,3,false},
        {AiPlayerFact::PlayersStance,-104,0,false},
        {AiPlayerFact::PlayersStance,-104,1,false},
        {AiPlayerFact::PlayersStance,-104,3,false},
        {AiPlayerFact::PlayersStance,-103,0,false},
        {AiPlayerFact::PlayersStance,-103,1,false},
        {AiPlayerFact::PlayersStance,-103,3,true},
        {AiPlayerFact::PlayersStance,-102,0,false},
        {AiPlayerFact::PlayersStance,-102,1,false},
        {AiPlayerFact::PlayersStance,-102,3,true},
        {AiPlayerFact::PlayersStance,-101,0,true},
        {AiPlayerFact::PlayersStance,-101,1,false},
        {AiPlayerFact::PlayersStance,-101,3,true},
        {AiPlayerFact::PlayersStance,-205,0,true},
        {AiPlayerFact::PlayersStance,-205,1,true},
        {AiPlayerFact::PlayersStance,-205,3,true},
        {AiPlayerFact::PlayersStance,-204,0,true},
        {AiPlayerFact::PlayersStance,-204,1,false},
        {AiPlayerFact::PlayersStance,-204,3,false},
        {AiPlayerFact::PlayersStance,-203,0,true},
        {AiPlayerFact::PlayersStance,-203,1,false},
        {AiPlayerFact::PlayersStance,-203,3,false},
        {AiPlayerFact::PlayersStance,-202,0,false},
        {AiPlayerFact::PlayersStance,-202,1,false},
        {AiPlayerFact::PlayersStance,-202,3,false},
        {AiPlayerFact::PlayersStance,-201,0,false},
        {AiPlayerFact::PlayersStance,-201,1,false},
        {AiPlayerFact::PlayersStance,-201,3,false},
        {AiPlayerFact::PlayersStance,0,0,true},
        {AiPlayerFact::PlayersStance,0,1,false},
        {AiPlayerFact::PlayersStance,0,3,false},
        {AiPlayerFact::PlayersStance,1,0,false},
        {AiPlayerFact::PlayersStance,1,1,false},
        {AiPlayerFact::PlayersStance,1,3,false},
        {AiPlayerFact::PlayersStance,2,0,true},
        {AiPlayerFact::PlayersStance,2,1,false},
        {AiPlayerFact::PlayersStance,2,3,false},
        {AiPlayerFact::PlayersStance,3,0,false},
        {AiPlayerFact::PlayersStance,3,1,false},
        {AiPlayerFact::PlayersStance,3,3,true},
        {AiPlayerFact::PlayersStance,4,0,true},
        {AiPlayerFact::PlayersStance,4,1,false},
        {AiPlayerFact::PlayersStance,4,3,false},
        {AiPlayerFact::PlayersStance,5,0,false},
        {AiPlayerFact::PlayersStance,5,1,false},
        {AiPlayerFact::PlayersStance,5,3,false},
        {AiPlayerFact::PlayersStance,-99,0,false},
        {AiPlayerFact::PlayersStance,-99,1,false},
        {AiPlayerFact::PlayersStance,-99,3,false},
        {AiPlayerFact::PlayersStance,-112,0,false},
        {AiPlayerFact::PlayersStance,-112,1,false},
        {AiPlayerFact::PlayersStance,-112,3,false},
        {AiPlayerFact::StanceToward,-111,0,false},
        {AiPlayerFact::StanceToward,-111,1,false},
        {AiPlayerFact::StanceToward,-111,3,false},
        {AiPlayerFact::StanceToward,-110,0,false},
        {AiPlayerFact::StanceToward,-110,1,false},
        {AiPlayerFact::StanceToward,-110,3,false},
        {AiPlayerFact::StanceToward,-109,0,false},
        {AiPlayerFact::StanceToward,-109,1,false},
        {AiPlayerFact::StanceToward,-109,3,true},
        {AiPlayerFact::StanceToward,-108,0,true},
        {AiPlayerFact::StanceToward,-108,1,false},
        {AiPlayerFact::StanceToward,-108,3,false},
        {AiPlayerFact::StanceToward,-107,0,true},
        {AiPlayerFact::StanceToward,-107,1,false},
        {AiPlayerFact::StanceToward,-107,3,true},
        {AiPlayerFact::StanceToward,-106,0,false},
        {AiPlayerFact::StanceToward,-106,1,false},
        {AiPlayerFact::StanceToward,-106,3,true},
        {AiPlayerFact::StanceToward,-105,0,false},
        {AiPlayerFact::StanceToward,-105,1,false},
        {AiPlayerFact::StanceToward,-105,3,false},
        {AiPlayerFact::StanceToward,-104,0,false},
        {AiPlayerFact::StanceToward,-104,1,false},
        {AiPlayerFact::StanceToward,-104,3,false},
        {AiPlayerFact::StanceToward,-103,0,true},
        {AiPlayerFact::StanceToward,-103,1,false},
        {AiPlayerFact::StanceToward,-103,3,false},
        {AiPlayerFact::StanceToward,-102,0,true},
        {AiPlayerFact::StanceToward,-102,1,false},
        {AiPlayerFact::StanceToward,-102,3,false},
        {AiPlayerFact::StanceToward,-101,0,true},
        {AiPlayerFact::StanceToward,-101,1,false},
        {AiPlayerFact::StanceToward,-101,3,false},
        {AiPlayerFact::StanceToward,-205,0,true},
        {AiPlayerFact::StanceToward,-205,1,true},
        {AiPlayerFact::StanceToward,-205,3,true},
        {AiPlayerFact::StanceToward,-204,0,false},
        {AiPlayerFact::StanceToward,-204,1,false},
        {AiPlayerFact::StanceToward,-204,3,false},
        {AiPlayerFact::StanceToward,-203,0,false},
        {AiPlayerFact::StanceToward,-203,1,false},
        {AiPlayerFact::StanceToward,-203,3,true},
        {AiPlayerFact::StanceToward,-202,0,true},
        {AiPlayerFact::StanceToward,-202,1,false},
        {AiPlayerFact::StanceToward,-202,3,false},
        {AiPlayerFact::StanceToward,-201,0,true},
        {AiPlayerFact::StanceToward,-201,1,false},
        {AiPlayerFact::StanceToward,-201,3,false},
        {AiPlayerFact::StanceToward,0,0,false},
        {AiPlayerFact::StanceToward,0,1,true},
        {AiPlayerFact::StanceToward,0,3,false},
        {AiPlayerFact::StanceToward,1,0,true},
        {AiPlayerFact::StanceToward,1,1,false},
        {AiPlayerFact::StanceToward,1,3,false},
        {AiPlayerFact::StanceToward,2,0,false},
        {AiPlayerFact::StanceToward,2,1,false},
        {AiPlayerFact::StanceToward,2,3,true},
        {AiPlayerFact::StanceToward,3,0,true},
        {AiPlayerFact::StanceToward,3,1,false},
        {AiPlayerFact::StanceToward,3,3,false},
        {AiPlayerFact::StanceToward,4,0,true},
        {AiPlayerFact::StanceToward,4,1,false},
        {AiPlayerFact::StanceToward,4,3,false},
        {AiPlayerFact::StanceToward,5,0,false},
        {AiPlayerFact::StanceToward,5,1,false},
        {AiPlayerFact::StanceToward,5,3,false},
    };
    for(const auto &g:golden)check(evaluateAiPlayerFact(g.fact,g.selector,g.stance,1,players).value==g.expected,
        "Original player fact golden mismatch");
    const float population[]{0.f,2.9f,5.8f,-1.9f,12.f};
    struct PopulationGolden {int selector,op;bool result;};
    const PopulationGolden populationGoldens[]{
        {-111,0,false},
        {-111,1,false},
        {-111,2,false},
        {-111,3,false},
        {-111,4,false},
        {-111,5,false},
        {-110,0,false},
        {-110,1,false},
        {-110,2,false},
        {-110,3,false},
        {-110,4,false},
        {-110,5,false},
        {-109,0,false},
        {-109,1,false},
        {-109,2,true},
        {-109,3,true},
        {-109,4,false},
        {-109,5,true},
        {-108,0,false},
        {-108,1,false},
        {-108,2,true},
        {-108,3,true},
        {-108,4,false},
        {-108,5,true},
        {-107,0,false},
        {-107,1,false},
        {-107,2,true},
        {-107,3,true},
        {-107,4,false},
        {-107,5,true},
        {-106,0,false},
        {-106,1,false},
        {-106,2,true},
        {-106,3,true},
        {-106,4,false},
        {-106,5,true},
        {-105,0,false},
        {-105,1,false},
        {-105,2,false},
        {-105,3,false},
        {-105,4,false},
        {-105,5,false},
        {-104,0,false},
        {-104,1,false},
        {-104,2,false},
        {-104,3,false},
        {-104,4,false},
        {-104,5,false},
        {-103,0,true},
        {-103,1,true},
        {-103,2,false},
        {-103,3,false},
        {-103,4,false},
        {-103,5,true},
        {-102,0,true},
        {-102,1,true},
        {-102,2,false},
        {-102,3,false},
        {-102,4,false},
        {-102,5,true},
        {-101,0,true},
        {-101,1,true},
        {-101,2,true},
        {-101,3,true},
        {-101,4,false},
        {-101,5,true},
        {-205,0,true},
        {-205,1,true},
        {-205,2,true},
        {-205,3,true},
        {-205,4,true},
        {-205,5,true},
        {-204,0,false},
        {-204,1,false},
        {-204,2,true},
        {-204,3,true},
        {-204,4,false},
        {-204,5,true},
        {-203,0,false},
        {-203,1,false},
        {-203,2,true},
        {-203,3,true},
        {-203,4,false},
        {-203,5,true},
        {-202,0,true},
        {-202,1,true},
        {-202,2,false},
        {-202,3,false},
        {-202,4,false},
        {-202,5,true},
        {-201,0,false},
        {-201,1,false},
        {-201,2,false},
        {-201,3,false},
        {-201,4,false},
        {-201,5,true},
        {0,0,true},
        {0,1,true},
        {0,2,false},
        {0,3,false},
        {0,4,false},
        {0,5,true},
        {1,0,true},
        {1,1,true},
        {1,2,false},
        {1,3,false},
        {1,4,false},
        {1,5,true},
        {2,0,false},
        {2,1,false},
        {2,2,true},
        {2,3,true},
        {2,4,false},
        {2,5,true},
        {3,0,true},
        {3,1,true},
        {3,2,false},
        {3,3,false},
        {3,4,false},
        {3,5,true},
        {4,0,false},
        {4,1,false},
        {4,2,true},
        {4,3,true},
        {4,4,false},
        {4,5,true},
        {5,0,false},
        {5,1,false},
        {5,2,false},
        {5,3,false},
        {5,4,false},
        {5,5,false},
        {-99,0,false},
        {-99,1,false},
        {-99,2,false},
        {-99,3,false},
        {-99,4,false},
        {-99,5,false},
        {-112,0,false},
        {-112,1,false},
        {-112,2,false},
        {-112,3,false},
        {-112,4,false},
        {-112,5,false},
    };
    for(const auto &g:populationGoldens) {
        const auto result=evaluateAiPlayerSelection(g.selector,1,players,[&](int id) {
            const int value=int(population[id]);
            return g.op==0?value<3:g.op==1?value<=3:g.op==2?value>3:g.op==3?value>=3:g.op==4?value==3:value!=3;
        });
        check(result.value==g.result,"Original military-population selector golden");
    }
    // Full57c280 oracle: no live object lookup, state filter, or clamp.
    { std::vector<AiMilitaryGatherGroup> groups;
      const int v=aiDefendSoldierCount(0,groups);check(v==0 && ((v<3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      const int v=aiDefendSoldierCount(0,groups);check(v==0 && ((v<=3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      const int v=aiDefendSoldierCount(0,groups);check(v==0 && ((v>3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      const int v=aiDefendSoldierCount(0,groups);check(v==0 && ((v>=3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      const int v=aiDefendSoldierCount(0,groups);check(v==0 && ((v==3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      const int v=aiDefendSoldierCount(0,groups);check(v==0 && ((v!=3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      const int v=aiDefendSoldierCount(8,groups);check(v==8 && ((v<3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      const int v=aiDefendSoldierCount(8,groups);check(v==8 && ((v<=3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      const int v=aiDefendSoldierCount(8,groups);check(v==8 && ((v>3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      const int v=aiDefendSoldierCount(8,groups);check(v==8 && ((v>=3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      const int v=aiDefendSoldierCount(8,groups);check(v==8 && ((v==3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      const int v=aiDefendSoldierCount(8,groups);check(v==8 && ((v!=3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(3);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==5 && ((v<3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(3);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==5 && ((v<=3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(3);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==5 && ((v>3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(3);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==5 && ((v>=3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(3);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==5 && ((v==3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(3);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==5 && ((v!=3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(3);groups.push_back(g); }
      { AiMilitaryGatherGroup g{};g.type=101;g.members.resize(2);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==5 && ((v<3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(3);groups.push_back(g); }
      { AiMilitaryGatherGroup g{};g.type=101;g.members.resize(2);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==5 && ((v<=3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(3);groups.push_back(g); }
      { AiMilitaryGatherGroup g{};g.type=101;g.members.resize(2);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==5 && ((v>3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(3);groups.push_back(g); }
      { AiMilitaryGatherGroup g{};g.type=101;g.members.resize(2);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==5 && ((v>=3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(3);groups.push_back(g); }
      { AiMilitaryGatherGroup g{};g.type=101;g.members.resize(2);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==5 && ((v==3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(3);groups.push_back(g); }
      { AiMilitaryGatherGroup g{};g.type=101;g.members.resize(2);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==5 && ((v!=3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(3);groups.push_back(g); }
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(4);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==1 && ((v<3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(3);groups.push_back(g); }
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(4);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==1 && ((v<=3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(3);groups.push_back(g); }
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(4);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==1 && ((v>3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(3);groups.push_back(g); }
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(4);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==1 && ((v>=3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(3);groups.push_back(g); }
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(4);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==1 && ((v==3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(3);groups.push_back(g); }
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(4);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==1 && ((v!=3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(4);groups.push_back(g); }
      const int v=aiDefendSoldierCount(2,groups);check(v==-2 && ((v<3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(4);groups.push_back(g); }
      const int v=aiDefendSoldierCount(2,groups);check(v==-2 && ((v<=3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(4);groups.push_back(g); }
      const int v=aiDefendSoldierCount(2,groups);check(v==-2 && ((v>3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(4);groups.push_back(g); }
      const int v=aiDefendSoldierCount(2,groups);check(v==-2 && ((v>=3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(4);groups.push_back(g); }
      const int v=aiDefendSoldierCount(2,groups);check(v==-2 && ((v==3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=100;g.members.resize(4);groups.push_back(g); }
      const int v=aiDefendSoldierCount(2,groups);check(v==-2 && ((v!=3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=108;g.members.resize(5);groups.push_back(g); }
      { AiMilitaryGatherGroup g{};g.type=102;g.members.resize(3);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==8 && ((v<3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=108;g.members.resize(5);groups.push_back(g); }
      { AiMilitaryGatherGroup g{};g.type=102;g.members.resize(3);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==8 && ((v<=3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=108;g.members.resize(5);groups.push_back(g); }
      { AiMilitaryGatherGroup g{};g.type=102;g.members.resize(3);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==8 && ((v>3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=108;g.members.resize(5);groups.push_back(g); }
      { AiMilitaryGatherGroup g{};g.type=102;g.members.resize(3);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==8 && ((v>=3)==true),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=108;g.members.resize(5);groups.push_back(g); }
      { AiMilitaryGatherGroup g{};g.type=102;g.members.resize(3);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==8 && ((v==3)==false),"Original defend-count golden"); }
    { std::vector<AiMilitaryGatherGroup> groups;
      { AiMilitaryGatherGroup g{};g.type=108;g.members.resize(5);groups.push_back(g); }
      { AiMilitaryGatherGroup g{};g.type=102;g.members.resize(3);groups.push_back(g); }
      const int v=aiDefendSoldierCount(8,groups);check(v==8 && ((v!=3)==true),"Original defend-count golden"); }
    const auto match=evaluateAiPlayerFact(AiPlayerFact::Human,-107,0,1,players);
    check(match.value && match.matchedPlayer==2,"Any selectors retain first successful player in original order");
    check(!evaluateAiPlayerFact(AiPlayerFact::Human,-204,0,1,players).matchedPlayer,
          "Every selectors do not replace the any-selector cache");
    check(aiPlayerSelector("any-human-ally")==-108 && aiPlayerSelector("every-neutral")==-205 &&
          !aiPlayerSelector("any-player"),"Original registered selector constants");
    std::cout<<"AI player facts: 186 player +144 population +42 defender original goldens PASS\n";
}
