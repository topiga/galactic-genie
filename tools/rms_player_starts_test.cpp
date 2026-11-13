// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/rms_player_starts.h"
#include <cstdlib>
#include <iostream>
static void check(bool ok, const std::string &message) {
    if (!ok) { std::cerr << message << '\n'; std::exit(1); }
}
int main() {
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
    std::array<RmsPlayerStart, 2> out;
    std::string error;
    for (const auto &c : cases) {
        uint32_t state = c.seed;
        check(generateRmsPlayerStarts(c.w,c.h,c.r,state,out,&error),error);
        check(state == c.finalState && out[0].x == c.x1 && out[0].y == c.y1 &&
              out[1].x == c.x2 && out[1].y == c.y2, "original player-start fixture mismatch");
    }
    const Case invalid[] = {{0,7,0,0,0,0,0,0,0}, {1025,7,0,0,0,0,0,0,0},
        {7,7,-1,0,0,0,0,0,0}, {7,7,4,0,0,0,0,0,0}, {1,1,0,0,0,0,0,0,0},
        {7,7,2147483647,0,0,0,0,0,0}, {32,2,0,0,0,0,0,0,0}};
    for (const auto &c : invalid) {
        uint32_t state = 123;
        out = {{{9,9},{9,9}}};
        check(!generateRmsPlayerStarts(c.w,c.h,c.r,state,out,&error),"unsupported input accepted");
        check(state == 123 && out[0].x == 0 && out[0].y == 0 &&
              out[1].x == 0 && out[1].y == 0 && !error.empty(),"failure was not atomic");
    }
    std::cout << "36 original player-start fixtures and invalid-input checks passed\n";
}
