// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/engine/ai_power_field.h"
static void powerFieldChecks() {
    // Actual61f140 raster controls, power-stencil-controls-2026-11-18.json.
    const int radii[]={1,9,20},centerCounts[]={9,285,1313},cornerCounts[]={4,81,349};
    for(int i=0;i<3;++i)for(float center:{.5f,24.5f,47.9f}) {
        int count=0;
        swgb::forEachOriginalPowerTile(48,48,center,center,radii[i],[&](int x,int y) {
            check(x>=0&&x<48&&y>=0&&y<48,"Power tiles clipped to map");++count;
        });
        check(count==(center==24.5f?centerCounts[i]:cornerCounts[i]),"Original power stencil golden tile count");
    }
    check(swgb::originalPowerRadius(12,18,false,2,9.9f,3)==9,"Core radius truncates owner resource18");
    check(swgb::originalPowerRadius(12,18,true,2,9,3)==9,"Contained core can emit at container position");
    check(swgb::originalPowerRadius(99,61,false,2,9,3.9f)==3,"Power droid uses resource30");
    check(swgb::originalPowerRadius(99,61,true,2,9,3)==0,"Contained power droid does not emit");
    check(swgb::originalPowerRadius(12,18,false,0,9,3)==0,"Foundation does not emit power");
    check(swgb::originalPowerRadius(12,18,false,2,99,3)==20,"Original power radius caps20");
    check(swgb::originalUnpoweredBuildingCandidate(84)&&!swgb::originalUnpoweredBuildingCandidate(68)&&
          !swgb::originalUnpoweredBuildingCandidate(109),"Original informationAI unpowered prototype gates");
}
