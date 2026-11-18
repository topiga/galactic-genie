// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "ai_resource_selection.h"
#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>
namespace swgb {
struct AiEconomicWorker {
    int32_t id=-1,resource=-1,batch=-1,completed=-1;
    uint32_t lastUpdate=0;
};
struct AiEconomyState {
    // Ordered native admission adapters for original civilian/owned-building registries.
    std::vector<int32_t> civilianIds,depotIds,civilianExplorers,pendingCivilianAssignments;
    std::array<AiEconomicWorker,125> economicWorkers{};
    std::array<AiResourceBucket,4> resources;
    std::unordered_map<uint32_t,std::pair<int,size_t>> resourceSlots;
    int explorerQuota=0,gathererQuota=0;
    std::array<int32_t,4> desired{},assigned{};
};
} // namespace swgb
