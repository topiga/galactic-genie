// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_rms_startup.h"

static void rmsFinalizationChecks() {
    // Original controller instructions with controlled object/AI callback bodies.
    struct PassCase {uint32_t players,seed,objectDraws,aiDraws,result;};
    const PassCase cases[]={
        {1u,1u,0u,0u,467153234u},
        {1u,1u,0u,4u,467153234u},
        {1u,1u,1u,4u,942262094u},
        {1u,12345u,0u,0u,1798587708u},
        {1u,12345u,0u,4u,1798587708u},
        {1u,12345u,1u,4u,2933867072u},
        {1u,4294967295u,0u,0u,724610873u},
        {1u,4294967295u,0u,4u,724610873u},
        {1u,4294967295u,1u,4u,1021078479u},
        {2u,1u,0u,0u,2279415318u},
        {2u,1u,0u,4u,2279415318u},
        {2u,1u,1u,4u,934985652u},
        {2u,12345u,0u,0u,2844195625u},
        {2u,12345u,0u,4u,2844195625u},
        {2u,12345u,1u,4u,850236504u},
        {2u,4294967295u,0u,0u,2368658739u},
        {2u,4294967295u,0u,4u,2368658739u},
        {2u,4294967295u,1u,4u,1525447519u},
        {8u,1u,0u,0u,902943303u},
        {8u,1u,0u,4u,902943303u},
        {8u,1u,1u,4u,3552364642u},
        {8u,12345u,0u,0u,4218373098u},
        {8u,12345u,0u,4u,4218373098u},
        {8u,12345u,1u,4u,869771288u},
        {8u,4294967295u,0u,0u,2368658739u},
        {8u,4294967295u,0u,4u,2368658739u},
        {8u,4294967295u,1u,4u,1016156180u},
    };
    for(const auto &c:cases) {
        uint32_t state=c.seed,calls=0;
        swgb::genieRmsPrepasses(state,c.players,[&](uint32_t owner,uint32_t pass) {
            check(owner==calls%(c.players+1) && pass==calls/(c.players+1),"Startup traverses three ascending owner passes including Gaia");
            ++calls;
            for(uint32_t i=0;i<c.objectDraws;++i)swgb::genieRandomDraw(state);
            if(owner) {
                const auto saved=swgb::genieRandomDraw(state);
                for(uint32_t i=0;i<c.aiDraws;++i)swgb::genieRandomDraw(state);
                state=saved;
            }
        });
        check(calls==3*(c.players+1) && state==c.result,"Original RMS prepasses preserve object draws and discard wrapped AI draws");
    }
    struct ChainCase {uint32_t players,seed,passes,checkpointCRT,checkpoint,activated;};
    const ChainCase chain[]={
        {1u,1u,467153234u,2913850061u,11693u,7042u},
        {1u,12345u,1798587708u,1889648399u,28833u,25134u},
        {1u,4294967295u,724610873u,2060104984u,31434u,1832u},
        {2u,1u,2279415318u,2127502465u,32463u,12086u},
        {2u,12345u,2844195625u,3485700424u,20419u,13494u},
        {2u,4294967295u,2368658739u,1660195626u,25332u,6615u},
        {8u,1u,902943303u,2439054318u,4449u,5204u},
        {8u,12345u,4218373098u,1738603269u,26528u,19312u},
        {8u,4294967295u,2368658739u,1660195626u,25332u,26271u},
    };
    for(const auto &c:chain) {
        uint32_t state=c.seed;
        swgb::genieRmsPrepasses(state,c.players,[&](uint32_t owner,uint32_t) {if(owner)state=swgb::genieRandomDraw(state);});
        check(state==c.passes,"Original chained post-map passes");
        const auto checkpoint=swgb::genieRandomDraw(state);
        check(state==c.checkpointCRT && checkpoint==c.checkpoint,"Original nonempty-owner common tail saves the world seed");
        for(uint32_t owner=1;owner<=c.players;++owner)state=swgb::genieRandomDraw(state);
        check(state==c.activated,"Original RMS activates owners after saving the world checkpoint");
    }
    std::puts("PASS: 27 original RMS prepass controls and nine chained finalization controls");
}
