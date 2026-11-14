// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "engine/rms_connections.h"
#include <cassert>
// Original prefix4dfe90..4dff09 and real virtual isAlly4c0450, no stubs.
// c01d78a039514211a845b41338c6f342e0d9c9b3396c8ff558f05d00ae67420d.
inline void checkOriginalConnectionDiplomacy() {
    using namespace swgb;
    uint32_t hash=2166136261u;
    for(unsigned mask=0;mask<4096;++mask) {
        std::vector<std::vector<uint8_t>> matrix(5,std::vector<uint8_t>(5,3));
        unsigned bit=0;
        for(unsigned player=1;player<5;++player) {
            matrix[player][player]=0;
            for(unsigned other=1;other<5;++other)if(player!=other)
                matrix[player][other]=(mask&(1u<<bit++))?0:3;
        }
        std::vector<int> teams;
        assert(resolveRmsConnectionTeams(matrix,teams));
        for(int team:teams)hash=(hash^uint32_t(team))*16777619u;
    }
    assert(hash==4204562885u);
    {
        std::vector<int> teams;
        assert(resolveRmsConnectionTeams({{0,3,3,2,0,3,3},{1,2,1,1,2,2,0},{3,0,3,2,0,2,0},{1,3,1,2,3,1,3},{0,1,1,0,0,0,2},{1,1,3,1,1,1,1},{2,2,1,2,0,3,3}},teams));
        assert(teams==std::vector<int>({0,1,1,2,2,3,2}));
    }
    {
        std::vector<int> teams;
        assert(resolveRmsConnectionTeams({{3,0,1},{2,0,1},{0,1,1}},teams));
        assert(teams==std::vector<int>({0,1,2}));
    }
    {
        std::vector<int> teams;
        assert(resolveRmsConnectionTeams({{3,0,2,3,0,0,0,1,1},{1,1,0,0,2,0,0,1,1},{1,1,0,0,0,3,1,0,3},{3,3,0,0,2,0,0,1,0},{0,3,2,2,0,0,2,1,1},{3,2,1,0,1,3,0,0,0},{0,1,0,0,2,0,0,1,1},{0,0,2,1,1,1,0,0,3},{1,2,0,3,0,3,1,3,1}},teams));
        assert(teams==std::vector<int>({0,1,2,2,3,2,2,1,2}));
    }
    {
        std::vector<int> teams;
        assert(resolveRmsConnectionTeams({{2,1,0,0,0,2},{1,1,0,1,0,0},{2,1,3,0,2,0},{2,0,2,0,1,0},{1,0,1,1,1,1},{1,2,3,3,1,2}},teams));
        assert(teams==std::vector<int>({0,1,2,1,1,3}));
    }
    {
        std::vector<int> teams;
        assert(resolveRmsConnectionTeams({{3,3,3,0,3,1,3},{1,1,3,1,1,2,0},{2,0,1,0,0,1,2},{3,3,2,2,1,0,0},{2,3,0,3,3,2,1},{2,3,2,1,0,1,2},{1,2,2,2,0,3,2}},teams));
        assert(teams==std::vector<int>({0,1,1,2,1,1,1}));
    }
    {
        std::vector<int> teams;
        assert(resolveRmsConnectionTeams({{0,3,0},{3,0,3},{0,3,1}},teams));
        assert(teams==std::vector<int>({0,1,2}));
    }
    {
        std::vector<int> teams;
        assert(resolveRmsConnectionTeams({{0,3,2,3,3,0,3},{2,3,3,3,0,3,2},{3,0,1,0,1,2,3},{0,2,3,3,3,3,1},{3,3,1,1,1,3,2},{2,0,1,0,0,3,2},{0,3,3,3,2,2,0}},teams));
        assert(teams==std::vector<int>({0,1,1,2,3,1,4}));
    }
    {
        std::vector<int> teams;
        assert(resolveRmsConnectionTeams({{1,2,2,2,3,1,2,1},{2,0,2,3,1,0,3,2},{3,1,2,0,3,2,0,1},{1,1,3,3,2,0,0,0},{2,3,2,1,1,1,1,3},{2,1,0,0,0,3,3,0},{2,3,1,3,1,3,1,0},{3,3,0,3,2,2,0,0}},teams));
        assert(teams==std::vector<int>({0,1,2,3,4,2,5,2}));
    }
    {
        std::vector<int> teams;
        assert(resolveRmsConnectionTeams({{0,2,2,1,2,2,2,1,2},{3,1,2,2,3,2,0,0,0},{1,2,0,1,2,1,0,0,2},{0,2,2,0,1,2,3,2,0},{1,3,3,2,3,3,3,0,0},{1,1,3,1,3,0,3,2,2},{2,2,1,3,3,2,3,3,3},{1,3,1,3,3,0,2,2,0},{3,1,0,3,0,2,1,0,2}},teams));
        assert(teams==std::vector<int>({0,1,2,3,4,5,6,5,2}));
    }
    {
        std::vector<int> teams;
        assert(resolveRmsConnectionTeams({{0,3,0,2},{3,0,1,2},{0,2,2,0},{0,1,2,0}},teams));
        assert(teams==std::vector<int>({0,1,2,3}));
    }
    {
        std::vector<int> teams;
        assert(resolveRmsConnectionTeams({{1,1},{3,2}},teams));
        assert(teams==std::vector<int>({0,1}));
    }
    {
        std::vector<int> teams;
        assert(resolveRmsConnectionTeams({{0,3,0,3,0},{2,2,0,1,0},{0,1,2,0,2},{3,2,2,2,0},{1,1,3,0,2}},teams));
        assert(teams==std::vector<int>({0,1,2,3,3}));
    }
    // Ally arrows from earlier players do not influence later players' query.
    std::vector<int> teams;
    assert(resolveRmsConnectionTeams({{3,3,3},{3,0,0},{3,3,0}},teams));
    assert(teams==std::vector<int>({0,1,2}));
    const auto before=teams;std::string error;
    assert(!resolveRmsConnectionTeams({{0,0},{0}},teams,&error));
    assert(teams==before && !error.empty());
}
