// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_ai_constructor_metadata.h"
#include "../src/core/genie_ai_timing.h"
static void aiConstructorMetadataChecks() {
    check(swgb::genieInfoConstructorMetadata(0).infoKnownRefreshTime==173 &&
          swgb::genieInfoConstructorMetadata(1).infoKnownRefreshTime==187 &&
          swgb::genieInfoConstructorMetadata(12345).infoKnownRefreshTime==34717 &&
          swgb::genieInfoConstructorMetadata(UINT32_MAX).infoKnownRefreshTime==160,
          "Original first constructor draw supplies known-category refresh timer");
    // Actual594f00 prefix controls stop at583a83 after the second draw.
    check(swgb::genieInfoConstructorRevisitTime(1)==33815,"Original AI constructor timer seed1");
    check(swgb::genieInfoConstructorRevisitTime(12345)==35091,"Original AI constructor timer seed12345");
    check(swgb::genieInfoConstructorRevisitTime(UINT32_MAX)==54455,"Original AI constructor timer unsigned seed");
    check(swgb::genieInfoConstructorMetadata(1).infoFoodRefreshTime==1449,"Original food constructor timer seed1");
    check(swgb::genieInfoConstructorMetadata(12345).infoFoodRefreshTime==5904,"Original food constructor timer seed12345");
    check(swgb::genieInfoConstructorMetadata(UINT32_MAX).infoFoodRefreshTime==772,"Original food constructor timer unsigned seed");
    for(uint32_t seed:{0u,1u,12345u,UINT32_MAX}) {
        uint32_t existing=seed,withMetadata=seed,nested=seed;
        swgb::genieRandomDraw(nested);
        swgb::geniePlayerConstructorRandom(existing);
        const auto metadata=swgb::geniePlayerConstructorWithMetadata(withMetadata);
        check(existing==withMetadata,"Retaining nested AI timer does not perturb ambient constructor RNG");
        check(metadata.infoRevisitTime==swgb::genieInfoConstructorRevisitTime(nested),
              "AI prefix uses full post-draw state, not truncated saved draw");
        swgb::genieRandomDraw(nested);swgb::genieRandomDraw(nested);
        const uint32_t foodDraw=swgb::genieRandomDraw(nested);
        check(metadata.infoFoodRefreshTime==uint32_t(uint64_t(foodDraw)*7500u/32767u),
              "Food refresh timestamp retains the third nested constructor draw");
    }
}
