// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/genie_orientation.h"
static void conversionOrientationChecks() {
 const struct {float angle,last,x,y,period;uint32_t clock;int count;float result,lastResult;bool ready,changed;} cases[]={
 {0.0f,0.0f,0.0f,1.0f,0.0f,0,8,1.570796251296997f,0.0f,true,true}, //snap_north
 {0.0f,0.0f,0.0f,-1.0f,0.0f,0,8,4.71238899230957f,0.0f,true,true}, //snap_wrap_negative
 {0.0f,0.0f,0.0f,1.0f,0.25f,249,8,0.0f,0.0f,false,false}, //wait
 {0.0f,0.0f,0.0f,1.0f,0.25f,250,8,0.7853981852531433f,0.25f,false,true}, //turn_threshold
 {0.0f,0.0f,0.0f,1.0f,0.25f,2000,8,0.7853981852531433f,2.0f,false,true}, //large_dt_one_facet
 {0.0f,0.0f,0.0f,-1.0f,0.25f,250,8,5.4977874755859375f,0.25f,false,true}, //negative_turn
 {1.0f,0.0f,0.0f,1.0f,0.25f,250,0,1.0f,0.0f,true,false}, //no_graphic
 {1.0f,0.0f,0.0f,1.0f,0.25f,250,0,1.0f,0.0f,true,false}, //zero_angles
 {0.0f,0.0f,0.0f,1.0f,0.25f,250,5,0.7853981852531433f,0.25f,false,true}, //angles5_uses8
 {0.0f,0.0f,0.0f,1.0f,0.25f,250,16,0.39269909262657166f,0.25f,false,true}, //angles16
 {1.4f,0.0f,0.0f,1.0f,0.25f,250,8,1.399999976158142f,0.0f,true,false}, //already_near
 {0.7853981852531433f,0.25f,0.0f,1.0f,0.25f,499,8,0.7853981852531433f,0.25f,false,false}, //last_turn_wait
 {0.7853981852531433f,0.25f,0.0f,1.0f,0.25f,500,8,1.5707963705062866f,0.5f,true,true}, //second_turn
 {0.0f,0.0f,0.0f,1.0f,0.25f,123456789,8,0.7853981852531433f,123456.796875f,false,true}, //floatclock_large
 {6.0f,0.0f,1.0f,0.1f,0.25f,250,8,6.0f,0.0f,true,false}, //wrap_positive
 };
 for(const auto &c:cases) {
 swgb::GenieOrientation state{c.angle,c.last};
 const auto result=swgb::genieOrientToward(state,0,0,c.x,c.y,c.count,c.period,c.clock);
 check(state.angle==c.result && state.lastTurnSeconds==c.lastResult && result.ready==c.ready && result.facetChanged==c.changed,"Original 4a11b0 orientation clock/facet/arrival");
 }
}
