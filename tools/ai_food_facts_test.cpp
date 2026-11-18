// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/engine/ai_food_facts.h"
#include <cstdio>
#include <cstdlib>
#include <string>
int main() {
    struct Row {const char *name;int count,cls,contained,invalid,cooldown,distance;uint32_t blocked,now;bool expected;};
    const Row rows[]{
        {"empty",0,27,0,0,0,10,4294967295u,100000u,true},
        {"forage10",1,27,0,0,0,10,4294967295u,100000u,false},
        {"forage11",1,27,0,0,0,11,4294967295u,100000u,true},
        {"sheep10",1,1,0,0,0,10,4294967295u,100000u,false},
        {"sheep-contained",1,1,1,0,0,10,4294967295u,100000u,true},
        {"forage-contained",1,27,1,0,0,10,4294967295u,100000u,false},
        {"animal4",1,4,0,0,0,0,4294967295u,100000u,true},
        {"farm7",1,7,0,0,0,0,4294967295u,100000u,true},
        {"invalid1",1,27,0,1,0,0,4294967295u,100000u,true},
        {"invalid2",1,27,0,2,0,0,4294967295u,100000u,false},
        {"cooldown1",1,27,0,0,1,0,4294967295u,100000u,true},
        {"cooldown255",1,27,0,0,255,0,4294967295u,100000u,true},
        {"blocked59999",1,27,0,0,0,0,40001u,100000u,true},
        {"blocked60000",1,27,0,0,0,0,40000u,100000u,false},
        {"blocked60001",1,27,0,0,0,0,39999u,100000u,false},
        {"blockedwrap",1,27,0,0,0,0,4294967280u,59984u,false},
        {"futurestamp",1,27,0,0,0,0,100001u,100000u,false},
        {"missing",1,27,0,0,0,0,4294967295u,100000u,true},
        {"forage255",1,27,0,0,0,255,4294967295u,100000u,true},
    };
    for(const auto &row:rows) {
        swgb::AiResourceBucket food;swgb::AiResourceRecord record;
        record.id=std::string(row.name)=="missing"?2:1;
        record.invalid=uint8_t(row.invalid);record.cooldown=uint8_t(row.cooldown);
        record.depotDistance=uint8_t(row.distance);record.blockedAt=row.blocked;
        if(row.count)food.records.push_back(record);
        const bool result=swgb::aiSheepAndForageTooFar(food,row.now,[&](int id)->std::optional<swgb::AiForageTarget> {
            if(id!=1)return std::nullopt;return swgb::AiForageTarget{row.cls,row.contained!=0};
        });
        if(result!=row.expected || (!food.records.empty() && food.records.front().cooldown!=row.cooldown)) {
            std::fprintf(stderr,"FAIL %s\n",row.name);return 1;
        }
    }
    std::puts("PASS original sheep-and-forage fact (19 oracle controls)");
}
