// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/rms_player_starts.h"
#include <cstdlib>
#include <iostream>
static void check(bool ok, const std::string &message) {
    if (!ok) { std::cerr << message << '\n'; std::exit(1); }
}
#include "rms_player_start_regions_checks.h"
#include "start_placement_checks.h"
int main() {
    checkRmsPlayerStartRegions();
    checkStartPlacement();
    using namespace swgb;
    // Full original handler/initializer/0x4e0740 executed in Unicorn, exact CC EXE
    // c01d78a0...; rms-player-starts-probe-2026-11-13.json. Includes draw32767 offset.
    struct Case { uint32_t w, h; int32_t r; uint32_t seed, finalState; int32_t x1,y1,x2,y2; };
    const Case cases[] = {
        {7, 7, 0, 0u, 2234209426u, 1, 0, 5, 4},
        {7, 7, 0, 1u, 373929026u, 3, 0, 3, 4},
        {7, 7, 0, 12345u, 669685740u, 0, 5, 4, 1},
        {7, 7, 0, 4294967295u, 2774064250u, 4, 1, 0, 5},
        {7, 7, 0, 1967u, 2610467196u, 2, 4, 4, 0},
        {7, 7, 0, 31490u, 2147458677u, 5, 4, 0, 1},
        {9, 7, 0, 0u, 2234209426u, 1, 0, 6, 4},
        {9, 7, 0, 1u, 373929026u, 4, 0, 3, 4},
        {9, 7, 0, 12345u, 669685740u, 0, 5, 5, 1},
        {9, 7, 0, 4294967295u, 2774064250u, 5, 1, 0, 5},
        {9, 7, 0, 1967u, 2610467196u, 3, 4, 4, 0},
        {9, 7, 0, 31490u, 2147458677u, 6, 4, 0, 1},
        {32, 32, 3, 0u, 2234209426u, 10, 5, 21, 20},
        {32, 32, 3, 1u, 373929026u, 18, 5, 13, 20},
        {32, 32, 3, 12345u, 669685740u, 5, 20, 20, 11},
        {32, 32, 3, 4294967295u, 2774064250u, 20, 10, 5, 21},
        {32, 32, 3, 1967u, 2610467196u, 11, 20, 20, 5},
        {32, 32, 3, 31490u, 2147458677u, 23, 20, 5, 8},
        {32, 48, 3, 0u, 2234209426u, 11, 5, 20, 30},
        {32, 48, 3, 1u, 373929026u, 21, 5, 10, 30},
        {32, 48, 3, 12345u, 669685740u, 5, 24, 20, 17},
        {32, 48, 3, 4294967295u, 2774064250u, 20, 16, 5, 25},
        {32, 48, 3, 1967u, 2610467196u, 5, 31, 20, 10},
        {32, 48, 3, 31490u, 2147458677u, 23, 30, 5, 8},
        {120, 120, 3, 0u, 2234209426u, 35, 14, 83, 82},
        {120, 120, 3, 1u, 373929026u, 72, 14, 46, 82},
        {120, 120, 3, 12345u, 669685740u, 14, 77, 82, 41},
        {120, 120, 3, 4294967295u, 2774064250u, 82, 36, 14, 82},
        {120, 120, 3, 1967u, 2610467196u, 35, 82, 83, 14},
        {120, 120, 3, 31490u, 2147458677u, 93, 82, 14, 25},
        {120, 120, 10, 0u, 2234209426u, 39, 20, 81, 80},
        {120, 120, 10, 1u, 373929026u, 71, 20, 49, 80},
        {120, 120, 10, 12345u, 669685740u, 20, 76, 80, 44},
        {120, 120, 10, 4294967295u, 2774064250u, 80, 40, 20, 80},
        {120, 120, 10, 1967u, 2610467196u, 39, 80, 81, 20},
        {120, 120, 10, 31490u, 2147458677u, 90, 80, 20, 30},
    };
    std::vector<RmsPlayerStart> out;
    std::string error;
    for (const auto &c : cases) {
        uint32_t state = c.seed;
        check(generateRmsPlayerStarts(c.w,c.h,c.r,2,state,out,&error),error);
        check(state == c.finalState && out[0].x == c.x1 && out[0].y == c.y1 &&
              out[1].x == c.x2 && out[1].y == c.y2, "original player-start fixture mismatch");
    }
    // Actual 0x4e76a0 + 0x4c0450 enemy diplomacy, no hooked callees:
    // rms-fixed-player-starts-oracle-2026-11-14.json (same CC EXE).
    // First fixture also matches both CC centers in the live Windows saved game.
    struct FixedCase { uint32_t size; int32_t radius; uint32_t players, seed, finalState;
        std::vector<RmsPlayerStart> centers; };
    const FixedCase fixedCases[] = {
        {120, 13, 2, 1993207714u, 3249354450u, {{19,36},{42,96}}},
        {120, 13, 2, 16160u, 3249354450u, {{19,36},{42,96}}},
        {120, 13, 2, 1u, 2800339366u, {{96,34},{37,92}}},
        {120, 13, 2, 12345u, 172814960u, {{21,83},{86,14}}},
        {120, 13, 2, 4294967295u, 252642142u, {{97,40},{39,89}}},
        {32, 3, 1, 1u, 3673273548u, {{22,26}}},
        {32, 3, 1, 12345u, 728679878u, {{11,4}}},
        {32, 3, 2, 1u, 2800339366u, {{25,8},{10,24}}},
        {32, 3, 2, 12345u, 172814960u, {{5,22},{22,3}}},
        {32, 3, 3, 1u, 196285776u, {{20,4},{22,23},{3,18}}},
        {32, 3, 3, 12345u, 1402573930u, {{3,11},{24,9},{16,26}}},
        {32, 3, 8, 1u, 3289249106u, {{18,3},{27,17},{5,23},{15,23},{23,10},{3,9},{22,24},{5,16}}},
        {32, 3, 8, 12345u, 2581596540u, {{15,7},{26,11},{8,3},{4,15},{27,18},{14,23},{21,26},{4,22}}},
        {12, 2, 1, 1u, 3673273548u, {{7,6}}},
        {12, 2, 1, 12345u, 728679878u, {{3,2}}},
        {12, 2, 2, 1u, 2800339366u, {{6,2},{5,7}}},
        {12, 2, 2, 12345u, 172814960u, {{4,7},{6,2}}},
        {12, 2, 3, 1u, 196285776u, {{5,2},{7,6},{2,7}}},
        {12, 2, 3, 12345u, 1402573930u, {{2,5},{7,3},{6,7}}},
        {12, 2, 8, 1u, 3289249106u, {{5,2},{7,5},{2,7},{5,7},{7,3},{3,2},{7,7},{2,5}}},
        {12, 2, 8, 12345u, 2581596540u, {{5,2},{7,3},{3,2},{2,5},{7,5},{5,7},{7,7},{2,7}}},
        {8, 2, 1, 1u, 3673273548u, {{4,3}}},
        {8, 2, 1, 12345u, 728679878u, {{2,2}}},
        {8, 2, 2, 1u, 2800339366u, {{3,2},{3,4}}},
        {8, 2, 2, 12345u, 172814960u, {{3,4},{3,2}}},
        {8, 2, 3, 1u, 196285776u, {{3,2},{4,3},{3,4}}},
        {8, 2, 3, 12345u, 1402573930u, {{2,4},{4,2},{4,4}}},
        {8, 2, 8, 1u, 3289249106u, {{3,2},{4,3},{2,4},{3,4},{4,2},{2,2},{4,4},{2,3}}},
        {8, 2, 8, 12345u, 2581596540u, {{3,2},{4,2},{2,2},{2,3},{4,3},{3,4},{4,4},{2,4}}},
    };
    for (const auto &c : fixedCases) {
        uint32_t state = c.seed;
        check(generateRmsFixedPlayerStarts(c.size,c.size,c.radius,c.players,state,out,&error),error);
        check(state == c.finalState && out.size() == c.centers.size(),"fixed start RNG/count mismatch");
        for (size_t i=0;i<out.size();++i)
            check(out[i].x == c.centers[i].x && out[i].y == c.centers[i].y,
                "original fixed player-start coordinates mismatch");
    }
    // Actual 4e76a0/4c0450: two mutual allies preserve the enemy permutation and draws.
    // rms-two-player-allied-fixed-starts-oracle-2026-11-16.json; translated border rectangles.
    struct BorderedCase { uint32_t w,h,seed,finalState; int32_t left,top,x1,y1,x2,y2; };
    const BorderedCase borderedCases[] = {
        {102, 102, 1u, 2800339366u, 10, 8, 90, 37, 42, 85},
        {100, 104, 1u, 2800339366u, 10, 8, 89, 38, 41, 87},
        {102, 102, 5000u, 125209764u, 10, 8, 61, 85, 74, 20},
        {100, 104, 5000u, 125209764u, 10, 8, 60, 87, 74, 20},
        {102, 102, 16160u, 3249354450u, 10, 8, 27, 40, 47, 88},
        {100, 104, 16160u, 3249354450u, 10, 8, 27, 40, 46, 90},
    };
    for(const auto &c:borderedCases) {
        uint32_t state=c.seed;
        check(generateRmsFixedPlayerStarts(c.w,c.h,12,2,state,out,&error),error);
        check(state==c.finalState && out.size()==2 &&
              out[0].x+c.left==c.x1 && out[0].y+c.top==c.y1 &&
              out[1].x+c.left==c.x2 && out[1].y+c.top==c.y2,
              "two-player allied bordered fixed starts mismatch");
    }
    const Case invalid[] = {{0,7,0,0,0,0,0,0,0}, {1025,7,0,0,0,0,0,0,0},
        {7,7,-1,0,0,0,0,0,0}, {7,7,4,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0},
        {7,7,2147483647,0,0,0,0,0,0}, {32,2,0,0,0,0,0,0,0}};
    for (const auto &c : invalid) {
        uint32_t state = 123;
        out = {{9,9},{9,9}};
        check(!generateRmsPlayerStarts(c.w,c.h,c.r,2,state,out,&error),"unsupported input accepted");
        check(state == 123 && out.empty() && !error.empty(),"failure was not atomic");
        out = {{9,9}};
        check(!generateRmsFixedPlayerStarts(c.w,c.h,c.r,2,state,out,&error) &&
            state == 123 && out.empty() && !error.empty(),"fixed failure was not atomic");
    }
    // Original full-handler fixtures for all counts, in the same dimension/seed order above.
    const uint32_t fingerprints[]={1649044140u,1270203993u,2592644030u,3482872803u,
                                   2598939072u,1662183351u,1395131406u,3659279705u};
    for (uint32_t players=1;players<=8;++players) {
        uint32_t hash=2166136261u;
        for (const auto &c:cases) {
            uint32_t state=c.seed;
            check(generateRmsPlayerStarts(c.w,c.h,c.r,players,state,out,&error),error);
            check(out.size()==players,"wrong number of player starts");
            for (const auto &point:out) {
                hash=(hash^static_cast<uint32_t>(point.x))*16777619u;
                hash=(hash^static_cast<uint32_t>(point.y))*16777619u;
            }
            hash=(hash^state)*16777619u;
        }
        check(hash==fingerprints[players-1],"original player-count fixture mismatch: "+std::to_string(players));
    }
    for (uint32_t players:{0u,9u,UINT32_MAX}) {
        uint32_t state=123; out={{9,9}};
        check(!generateRmsPlayerStarts(32,32,3,players,state,out,&error) &&
              state==123 && out.empty() && !error.empty(),"invalid player count must fail atomically");
    }
    std::cout << "288 randomized, 35 fixed and 492 multi-land original player-start fixtures and invalid-input checks passed\n";
}
