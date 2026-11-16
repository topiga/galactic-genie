// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_work_attack.h"
#include <cstdint>
#include <cstring>
#include <limits>

static void runWorkAttackChecks() {
    // Actual PE c01d78a039514211a845b41338c6f342e0d9c9b3396c8ff558f05d00ae67420d.
    // tree-work-attack-dt-matrix-pc64-oracle-2026-11-16.json: explicit FPCW037f.
    // Original 30-phase 0.1s golden plus four additional dt controls. These
    // start at attack state7; target validity/range/facing remain satisfied.
    struct Row {
        int frame, previous, actionState, pending, advanced, cycle, hits;
        uint32_t reloadBits, remainderBits;
    };
    struct Case { uint32_t dtBits; Row rows[30]; };
    const Case cases[] = {
        {0x3d4ccccdu, {
            {0, 0, 7, 1, 0, 0, 0, 0x3ff9999au, 0x3d4ccccdu},
            {1, 0, 7, 1, 1, 0, 0, 0x3ff33334u, 0x00000000u},
            {1, 0, 7, 1, 0, 0, 0, 0x3fecccceu, 0x3d4ccccdu},
            {2, 1, 7, 1, 1, 0, 0, 0x3fe66668u, 0x00000000u},
            {2, 1, 7, 1, 0, 0, 0, 0x3fe00002u, 0x3d4ccccdu},
            {3, 2, 7, 1, 1, 0, 0, 0x3fd9999cu, 0x00000000u},
            {3, 2, 7, 1, 0, 0, 0, 0x3fd33336u, 0x3d4ccccdu},
            {4, 3, 7, 0, 1, 0, 1, 0x3fccccd0u, 0x00000000u},
            {4, 3, 7, 0, 0, 0, 1, 0x3fc6666au, 0x3d4ccccdu},
            {5, 4, 7, 0, 1, 0, 1, 0x3fc00004u, 0x00000000u},
            {5, 4, 7, 0, 0, 0, 1, 0x3fb9999eu, 0x3d4ccccdu},
            {6, 5, 7, 0, 1, 0, 1, 0x3fb33338u, 0x00000000u},
            {6, 5, 7, 0, 0, 0, 1, 0x3facccd2u, 0x3d4ccccdu},
            {7, 6, 7, 0, 1, 0, 1, 0x3fa6666cu, 0x00000000u},
            {7, 6, 7, 0, 0, 0, 1, 0x3fa00006u, 0x3d4ccccdu},
            {0, 7, 6, 0, 1, 1, 1, 0x3f9999a0u, 0x00000000u},
            {0, 7, 6, 0, 0, 0, 1, 0x3f93333au, 0x3d4ccccdu},
            {1, 0, 6, 0, 1, 0, 1, 0x3f8cccd4u, 0x00000000u},
            {1, 0, 6, 0, 0, 0, 1, 0x3f86666eu, 0x3d4ccccdu},
            {2, 1, 6, 0, 1, 0, 1, 0x3f800008u, 0x00000000u},
            {2, 1, 6, 0, 0, 0, 1, 0x3f733343u, 0x3d4ccccdu},
            {3, 2, 6, 0, 1, 0, 1, 0x3f666676u, 0x00000000u},
            {3, 2, 6, 0, 0, 0, 1, 0x3f5999a9u, 0x3d4ccccdu},
            {4, 3, 6, 0, 1, 0, 1, 0x3f4cccdcu, 0x00000000u},
            {4, 3, 6, 0, 0, 0, 1, 0x3f40000fu, 0x3d4ccccdu},
            {5, 4, 6, 0, 1, 0, 1, 0x3f333342u, 0x00000000u},
            {5, 4, 6, 0, 0, 0, 1, 0x3f266675u, 0x3d4ccccdu},
            {6, 5, 6, 0, 1, 0, 1, 0x3f1999a8u, 0x00000000u},
            {6, 5, 6, 0, 0, 0, 1, 0x3f0cccdbu, 0x3d4ccccdu},
            {7, 6, 6, 0, 1, 0, 1, 0x3f00000eu, 0x00000000u},
        }},
        {0x3dcccccdu, {
            {1, 0, 7, 1, 1, 0, 0, 0x3ff33333u, 0x00000000u},
            {2, 1, 7, 1, 1, 0, 0, 0x3fe66666u, 0x00000000u},
            {3, 2, 7, 1, 1, 0, 0, 0x3fd99999u, 0x00000000u},
            {4, 3, 7, 0, 1, 0, 1, 0x3fccccccu, 0x00000000u},
            {5, 4, 7, 0, 1, 0, 1, 0x3fbfffffu, 0x00000000u},
            {6, 5, 7, 0, 1, 0, 1, 0x3fb33332u, 0x00000000u},
            {7, 6, 7, 0, 1, 0, 1, 0x3fa66665u, 0x00000000u},
            {0, 7, 6, 0, 1, 1, 1, 0x3f999998u, 0x00000000u},
            {1, 0, 6, 0, 1, 0, 1, 0x3f8ccccbu, 0x00000000u},
            {2, 1, 6, 0, 1, 0, 1, 0x3f7ffffcu, 0x00000000u},
            {3, 2, 6, 0, 1, 0, 1, 0x3f666662u, 0x00000000u},
            {4, 3, 6, 0, 1, 0, 1, 0x3f4cccc8u, 0x00000000u},
            {5, 4, 6, 0, 1, 0, 1, 0x3f33332eu, 0x00000000u},
            {6, 5, 6, 0, 1, 0, 1, 0x3f199994u, 0x00000000u},
            {7, 6, 6, 0, 1, 0, 1, 0x3efffff5u, 0x00000000u},
            {0, 7, 6, 0, 1, 1, 1, 0x3eccccc2u, 0x00000000u},
            {1, 0, 6, 0, 1, 0, 1, 0x3e99998fu, 0x00000000u},
            {2, 1, 6, 0, 1, 0, 1, 0x3e4cccb8u, 0x00000000u},
            {3, 2, 6, 0, 1, 0, 1, 0x3dcccca3u, 0x00000000u},
            {4, 3, 7, 1, 1, 0, 1, 0x40000000u, 0x00000000u},
            {5, 4, 7, 1, 1, 0, 1, 0x3ff33333u, 0x00000000u},
            {6, 5, 7, 1, 1, 0, 1, 0x3fe66666u, 0x00000000u},
            {7, 6, 7, 1, 1, 0, 1, 0x3fd99999u, 0x00000000u},
            {0, 7, 7, 0, 1, 1, 2, 0x3fccccccu, 0x00000000u},
            {1, 0, 7, 0, 1, 0, 2, 0x3fbfffffu, 0x00000000u},
            {2, 1, 7, 0, 1, 0, 2, 0x3fb33332u, 0x00000000u},
            {3, 2, 7, 0, 1, 0, 2, 0x3fa66665u, 0x00000000u},
            {4, 3, 7, 0, 1, 0, 2, 0x3f999998u, 0x00000000u},
            {5, 4, 7, 0, 1, 0, 2, 0x3f8ccccbu, 0x00000000u},
            {6, 5, 7, 0, 1, 0, 2, 0x3f7ffffcu, 0x00000000u},
        }},
        {0x3e23d70au, {
            {1, 0, 7, 1, 1, 0, 0, 0x3feb851fu, 0x3d75c28eu},
            {3, 1, 7, 1, 1, 0, 0, 0x3fd70a3eu, 0x3ca3d704u},
            {4, 3, 7, 0, 1, 0, 1, 0x3fc28f5du, 0x3da3d708u},
            {6, 4, 7, 0, 1, 0, 1, 0x3fae147cu, 0x3d23d704u},
            {7, 6, 7, 0, 1, 0, 1, 0x3f99999bu, 0x3dccccc9u},
            {1, 7, 6, 0, 1, 1, 1, 0x3f851ebau, 0x3d75c286u},
            {3, 1, 6, 0, 1, 0, 1, 0x3f6147b2u, 0x3ca3d6f4u},
            {4, 3, 6, 0, 1, 0, 1, 0x3f3851f0u, 0x3da3d704u},
            {6, 4, 6, 0, 1, 0, 1, 0x3f0f5c2eu, 0x3d23d6fcu},
            {7, 6, 6, 0, 1, 0, 1, 0x3eccccd7u, 0x3dccccc5u},
            {1, 7, 6, 0, 1, 1, 1, 0x3e75c2a4u, 0x3d75c27eu},
            {3, 1, 6, 0, 1, 0, 1, 0x3da3d734u, 0x3ca3d6e4u},
            {4, 3, 7, 1, 1, 0, 1, 0x40000000u, 0x3da3d700u},
            {6, 4, 7, 1, 1, 0, 1, 0x3feb851fu, 0x3d23d6f4u},
            {7, 6, 7, 1, 1, 0, 1, 0x3fd70a3eu, 0x3dccccc1u},
            {1, 7, 7, 0, 1, 1, 2, 0x3fc28f5du, 0x3d75c276u},
            {3, 1, 7, 0, 1, 0, 2, 0x3fae147cu, 0x3ca3d6d4u},
            {4, 3, 7, 0, 1, 0, 2, 0x3f99999bu, 0x3da3d6fcu},
            {6, 4, 7, 0, 1, 0, 2, 0x3f851ebau, 0x3d23d6ecu},
            {7, 6, 7, 0, 1, 0, 2, 0x3f6147b2u, 0x3dccccbdu},
            {1, 7, 6, 0, 1, 1, 2, 0x3f3851f0u, 0x3d75c26eu},
            {3, 1, 6, 0, 1, 0, 2, 0x3f0f5c2eu, 0x3ca3d6c4u},
            {4, 3, 6, 0, 1, 0, 2, 0x3eccccd7u, 0x3da3d6f8u},
            {6, 4, 6, 0, 1, 0, 2, 0x3e75c2a4u, 0x3d23d6e4u},
            {7, 6, 6, 0, 1, 0, 2, 0x3da3d734u, 0x3dccccb9u},
            {1, 7, 7, 1, 1, 1, 2, 0x40000000u, 0x3d75c266u},
            {3, 1, 7, 1, 1, 0, 2, 0x3feb851fu, 0x3ca3d6b4u},
            {4, 3, 7, 0, 1, 0, 3, 0x3fd70a3eu, 0x3da3d6f4u},
            {6, 4, 7, 0, 1, 0, 3, 0x3fc28f5du, 0x3d23d6dcu},
            {7, 6, 7, 0, 1, 0, 3, 0x3fae147cu, 0x3dccccb5u},
        }},
        {0x3e800000u, {
            {2, 0, 7, 1, 1, 0, 0, 0x3fe00000u, 0x3d4cccccu},
            {4, 2, 7, 0, 1, 0, 1, 0x3fc00000u, 0x3dccccccu},
            {7, 4, 7, 0, 1, 0, 1, 0x3fa00000u, 0x3d4ccccau},
            {1, 7, 6, 0, 1, 1, 1, 0x3f800000u, 0x3dcccccbu},
            {4, 1, 6, 0, 1, 0, 1, 0x3f400000u, 0x3d4cccc8u},
            {6, 4, 6, 0, 1, 0, 1, 0x3f000000u, 0x3dcccccau},
            {1, 6, 6, 0, 1, 1, 1, 0x3e800000u, 0x3d4cccc6u},
            {3, 1, 7, 1, 1, 0, 1, 0x40000000u, 0x3dccccc9u},
            {6, 3, 7, 0, 1, 0, 2, 0x3fe00000u, 0x3d4cccc4u},
            {0, 6, 6, 0, 1, 1, 2, 0x3fc00000u, 0x3dccccc8u},
            {3, 0, 6, 0, 1, 0, 2, 0x3fa00000u, 0x3d4cccc2u},
            {5, 3, 6, 0, 1, 0, 2, 0x3f800000u, 0x3dccccc7u},
            {0, 5, 6, 0, 1, 1, 2, 0x3f400000u, 0x3d4cccc0u},
            {2, 0, 6, 0, 1, 0, 2, 0x3f000000u, 0x3dccccc6u},
            {5, 2, 6, 0, 1, 0, 2, 0x3e800000u, 0x3d4cccbeu},
            {7, 5, 7, 1, 1, 0, 2, 0x40000000u, 0x3dccccc5u},
            {2, 7, 7, 0, 1, 1, 3, 0x3fe00000u, 0x3d4cccbcu},
            {4, 2, 7, 0, 1, 0, 3, 0x3fc00000u, 0x3dccccc4u},
            {7, 4, 7, 0, 1, 0, 3, 0x3fa00000u, 0x3d4cccbau},
            {1, 7, 6, 0, 1, 1, 3, 0x3f800000u, 0x3dccccc3u},
            {4, 1, 6, 0, 1, 0, 3, 0x3f400000u, 0x3d4cccb8u},
            {6, 4, 6, 0, 1, 0, 3, 0x3f000000u, 0x3dccccc2u},
            {1, 6, 6, 0, 1, 1, 3, 0x3e800000u, 0x3d4cccb6u},
            {3, 1, 7, 1, 1, 0, 3, 0x40000000u, 0x3dccccc1u},
            {6, 3, 7, 0, 1, 0, 4, 0x3fe00000u, 0x3d4cccb4u},
            {0, 6, 6, 0, 1, 1, 4, 0x3fc00000u, 0x3dccccc0u},
            {3, 0, 6, 0, 1, 0, 4, 0x3fa00000u, 0x3d4cccb2u},
            {5, 3, 6, 0, 1, 0, 4, 0x3f800000u, 0x3dccccbfu},
            {0, 5, 6, 0, 1, 1, 4, 0x3f400000u, 0x3d4cccb0u},
            {2, 0, 6, 0, 1, 0, 4, 0x3f000000u, 0x3dccccbeu},
        }},
        {0x3f800000u, {
            {1, 0, 7, 0, 1, 1, 1, 0x3f800000u, 0x3dcccccbu},
            {3, 1, 7, 0, 1, 1, 2, 0x40000000u, 0x3dccccc9u},
            {5, 3, 6, 0, 1, 1, 2, 0x3f800000u, 0x3dccccc7u},
            {7, 5, 7, 1, 1, 1, 2, 0x40000000u, 0x3dccccc5u},
            {1, 7, 7, 0, 1, 1, 3, 0x3f800000u, 0x3dccccc3u},
            {3, 1, 7, 0, 1, 1, 4, 0x40000000u, 0x3dccccc1u},
            {5, 3, 6, 0, 1, 1, 4, 0x3f800000u, 0x3dccccbfu},
            {7, 5, 7, 1, 1, 1, 4, 0x40000000u, 0x3dccccbdu},
            {1, 7, 7, 0, 1, 1, 5, 0x3f800000u, 0x3dccccbbu},
            {3, 1, 7, 0, 1, 1, 6, 0x40000000u, 0x3dccccb9u},
            {5, 3, 6, 0, 1, 1, 6, 0x3f800000u, 0x3dccccb7u},
            {7, 5, 7, 1, 1, 1, 6, 0x40000000u, 0x3dccccb5u},
            {1, 7, 7, 0, 1, 1, 7, 0x3f800000u, 0x3dccccb3u},
            {3, 1, 7, 0, 1, 1, 8, 0x40000000u, 0x3dccccb1u},
            {5, 3, 6, 0, 1, 1, 8, 0x3f800000u, 0x3dccccafu},
            {7, 5, 7, 1, 1, 1, 8, 0x40000000u, 0x3dccccadu},
            {1, 7, 7, 0, 1, 1, 9, 0x3f800000u, 0x3dccccabu},
            {3, 1, 7, 0, 1, 1, 10, 0x40000000u, 0x3dcccca9u},
            {5, 3, 6, 0, 1, 1, 10, 0x3f800000u, 0x3dcccca7u},
            {7, 5, 7, 1, 1, 1, 10, 0x40000000u, 0x3dcccca5u},
            {1, 7, 7, 0, 1, 1, 11, 0x3f800000u, 0x3dcccca3u},
            {3, 1, 7, 0, 1, 1, 12, 0x40000000u, 0x3dcccca1u},
            {5, 3, 6, 0, 1, 1, 12, 0x3f800000u, 0x3dcccc9fu},
            {7, 5, 7, 1, 1, 1, 12, 0x40000000u, 0x3dcccc9du},
            {1, 7, 7, 0, 1, 1, 13, 0x3f800000u, 0x3dcccc9bu},
            {3, 1, 7, 0, 1, 1, 14, 0x40000000u, 0x3dcccc99u},
            {5, 3, 6, 0, 1, 1, 14, 0x3f800000u, 0x3dcccc97u},
            {7, 5, 7, 1, 1, 1, 14, 0x40000000u, 0x3dcccc95u},
            {1, 7, 7, 0, 1, 1, 15, 0x3f800000u, 0x3dcccc93u},
            {3, 1, 7, 0, 1, 1, 16, 0x40000000u, 0x3dcccc91u},
        }},
    };
    auto value=[](uint32_t v) { float f; std::memcpy(&f,&v,4); return f; };
    auto bits=[](float f) { uint32_t v; std::memcpy(&v,&f,4); return v; };
    const swgb::GenieWorkAttackConfig config{8,0.1f,2.f,3,0.f,0.f};
    for (const auto& c : cases) {
        swgb::GenieWorkAttackState state;
        check(swgb::initializeGenieWorkAttackAtState7(config,state),
              "Stationary melee oracle initializes at explicit state7 boundary");
        int hits=0;
        for (const auto& row : c.rows) {
            const auto result=swgb::stepGenieWorkAttack(config,state,value(c.dtBits));
            hits += result.hit;
            check(result.supported && state.frame==row.frame &&
                  state.previousFrame==row.previous && state.actionState==row.actionState &&
                  state.pendingHit==bool(row.pending) &&
                  state.frameAdvanced==bool(row.advanced) &&
                  state.cycleCompleted==bool(row.cycle) && hits==row.hits &&
                  bits(state.reloadRemaining)==row.reloadBits &&
                  bits(state.frameRemainder)==row.remainderBits,
                  "Work attack phase matches original frames, state, hit and float bits");
        }
    }
    for (int variant=0; variant<7; ++variant) {
        auto unsupported=config;
        if (variant==0) unsupported.flags=1;
        if (variant==1) unsupported.flags=7;
        if (variant==2) unsupported.animationSpeed=1.f;
        if (variant==3) unsupported.holdTime=1.f;
        if (variant==4) unsupported.frameCount=0;
        if (variant==5) unsupported.frameDuration=0.f;
        if (variant==6) unsupported.reloadTime=-1.f;
        swgb::GenieWorkAttackState state;
        state.frame=2;
        check(!swgb::initializeGenieWorkAttackAtState7(unsupported,state) && state.frame==2,
              "Unsupported work animation does not initialize a fabricated controller");
        const auto result=swgb::stepGenieWorkAttack(unsupported,state,0.1f);
        check(!result.supported && !result.hit && state.frame==2,
              "Unsupported work animation preserves controller state");
    }
    for (float dt : {-1.f, std::numeric_limits<float>::infinity(),
                     std::numeric_limits<float>::quiet_NaN(), 10000.f}) {
        swgb::GenieWorkAttackState state;
        swgb::initializeGenieWorkAttackAtState7(config,state);
        const auto result=swgb::stepGenieWorkAttack(config,state,dt);
        check(!result.supported && !result.hit && state.frame==0 &&
              state.reloadRemaining==2.f && state.pendingHit,
              "Invalid or overflowing work timestep preserves state");
    }
    swgb::GenieWorkAttackState state;
    swgb::initializeGenieWorkAttackAtState7(config,state);
    state.actionState=4;
    check(!swgb::stepGenieWorkAttack(config,state,0.1f).supported && state.actionState==4,
          "Movement state requires the owning action controller");
}
