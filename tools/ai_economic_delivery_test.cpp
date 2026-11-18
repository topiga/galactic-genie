// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/ai_economic_delivery.h"
#include <array>
#include <cstdlib>
#include <iostream>
struct Worker {int32_t id=-1,resource=-1,completed=-1;};
int main() {
    struct Golden {int slot,id,type,amount,before,after;};
    const Golden rows[]={{0,42,0,10,0,10},{124,42,0,10,7,17},{0,99,0,10,7,7},
        {0,42,1,10,7,7},{0,42,0,0,7,7},{0,42,0,-3,7,4},{0,42,0,1,INT32_MAX,INT32_MIN}};
    for(const auto &g:rows) {
        std::array<Worker,125> workers{};workers[size_t(g.slot)]={42,0,g.before};
        swgb::creditAiEconomicDelivery(workers,g.id,g.type,g.amount);
        if(workers[size_t(g.slot)].completed!=g.after) {std::cerr<<"Original economic delivery credit mismatch\n";return 1;}
    }
    std::cout<<"PASS: original economic delivery credit\n";
}
