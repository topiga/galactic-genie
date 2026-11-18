// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
namespace swgb {
struct AiBuilderCandidate {
    int32_t id=-1;
    int prototype=-1;
    int controllerType=0;
    float carriedAmount=0;
    bool resolved=true,hasController=true,reservedInCurrentPass=false;
};
// Original5a7c10 runs after placement, over registered class58 workers in their
// original list order. Geometry callback is original551c60 (unrounded result).
// Live callers must supply actor-AI order code (+7c -> +28), not an action task or idle/busy boolean.
template<class Candidates,class Distance>
int32_t aiSelectBuilder(const Candidates &candidates,Distance distance) {
    int32_t selected=-1;
    float best=0;
    for(const auto &worker:candidates) {
        if(worker.reservedInCurrentPass || !worker.resolved || !worker.hasController ||
           worker.prototype==259 || worker.prototype==214 || worker.carriedAmount>7.f ||
           worker.controllerType==702 || worker.controllerType==718 ||
           worker.controllerType==700 || worker.controllerType==717)continue;
        const double candidate=distance(worker);
        if(selected==-1 || double(best)>candidate) {
            selected=worker.id;
            best=float(candidate);
        }
    }
    return selected;
}
} // namespace swgb
