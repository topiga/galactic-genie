// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_ai_constructor_metadata.h"
#include "../src/core/genie_ai_timing.h"
static void aiConstructorMetadataChecks() {
    // Actual594f00 prefix controls stop at583a83 after the second draw.
    check(swgb::genieInfoConstructorRevisitTime(1)==33815,"Original AI constructor timer seed1");
    check(swgb::genieInfoConstructorRevisitTime(12345)==35091,"Original AI constructor timer seed12345");
    check(swgb::genieInfoConstructorRevisitTime(UINT32_MAX)==54455,"Original AI constructor timer unsigned seed");
    for(uint32_t seed:{0u,1u,12345u,UINT32_MAX}) {
        uint32_t existing=seed,withMetadata=seed,nested=seed;
        swgb::genieRandomDraw(nested);
        swgb::geniePlayerConstructorRandom(existing);
        const auto metadata=swgb::geniePlayerConstructorWithMetadata(withMetadata);
        check(existing==withMetadata,"Retaining nested AI timer does not perturb ambient constructor RNG");
        check(metadata.infoRevisitTime==swgb::genieInfoConstructorRevisitTime(nested),
              "AI prefix uses full post-draw state, not truncated saved draw");
    }
}
