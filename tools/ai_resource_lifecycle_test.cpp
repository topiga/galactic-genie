// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/ai_resource_lifecycle.h"
#include <cstdlib>
#include <iostream>
#include <map>
using namespace swgb;
static void check(bool value,const char *message) {if(!value){std::cerr<<message<<'\n';std::exit(1);}}
int main() {
 check(aiDepotRefreshBuckets(109,6,false)==std::vector<int>{0,1,3,2},"Original depot event bucket order");
 check(aiDepotRefreshBuckets(584,6,false)==std::vector<int>{3,2},"Original depot event bucket order");
 check(aiDepotRefreshBuckets(323,6,false)==std::vector<int>{},"Original depot event bucket order");
 check(aiDepotRefreshBuckets(68,6,false)==std::vector<int>{0},"Original depot event bucket order");
 check(aiDepotRefreshBuckets(45,6,false)==std::vector<int>{0},"Original depot event bucket order");
 check(aiDepotRefreshBuckets(562,6,false)==std::vector<int>{1},"Original depot event bucket order");
 check(aiDepotRefreshBuckets(50,7,false)==std::vector<int>{},"Original depot event bucket order");
 check(aiDepotRefreshBuckets(109,58,false)==std::vector<int>{},"Original depot event bucket order");
 check(aiDepotRefreshBuckets(109,6,true)==std::vector<int>{0,1,3,2},"Original depot event bucket order");
 check(aiDepotRefreshBuckets(584,6,true)==std::vector<int>{2},"Original depot event bucket order");
 check(aiDepotRefreshBuckets(323,6,true)==std::vector<int>{3},"Original depot event bucket order");
 check(aiDepotRefreshBuckets(68,6,true)==std::vector<int>{0},"Original depot event bucket order");
 check(aiDepotRefreshBuckets(45,6,true)==std::vector<int>{},"Original depot event bucket order");
 check(aiDepotRefreshBuckets(562,6,true)==std::vector<int>{1},"Original depot event bucket order");
 check(aiDepotRefreshBuckets(50,7,true)==std::vector<int>{},"Original depot event bucket order");
 check(aiDepotRefreshBuckets(109,58,true)==std::vector<int>{0,1,3,2},"Original depot event bucket order");
 {AiResourceBucket bucket;std::map<int,AiResourceDepotMetric> metrics;
 refreshAiResourceBucket(bucket,[&](int id)->std::optional<AiResourceDepotMetric>{auto i=metrics.find(id);return i==metrics.end()?std::nullopt:std::optional<AiResourceDepotMetric>(i->second);});
 check(bucket.cachedBestId==-1 && bucket.cachedBestDistance==-1,"Original refreshed cache result");
 }
 {AiResourceBucket bucket;std::map<int,AiResourceDepotMetric> metrics;
 bucket.records.push_back(AiResourceRecord{11,41,42,200,888,0,0,88,0,999,123456});
 metrics.emplace(11,AiResourceDepotMetric{9,5});
 refreshAiResourceBucket(bucket,[&](int id)->std::optional<AiResourceDepotMetric>{auto i=metrics.find(id);return i==metrics.end()?std::nullopt:std::optional<AiResourceDepotMetric>(i->second);});
 check(bucket.cachedBestId==11 && bucket.cachedBestDistance==5,"Original refreshed cache result");
 check(bucket.records[0].baseScore==10 && bucket.records[0].depotDistance==5 && bucket.records[0].depotId==9 && bucket.records[0].x==41 && bucket.records[0].y==42 && bucket.records[0].selections==200 && bucket.records[0].cooldown==0 && bucket.records[0].invalid==0 && bucket.records[0].blockedAt==123456,"Original metric refresh preserves independent record state");
 }
 {AiResourceBucket bucket;std::map<int,AiResourceDepotMetric> metrics;
 bucket.records.push_back(AiResourceRecord{11,41,42,200,888,0,0,88,0,999,123456});
 refreshAiResourceBucket(bucket,[&](int id)->std::optional<AiResourceDepotMetric>{auto i=metrics.find(id);return i==metrics.end()?std::nullopt:std::optional<AiResourceDepotMetric>(i->second);});
 check(bucket.cachedBestId==-1 && bucket.cachedBestDistance==-1,"Original refreshed cache result");
 check(bucket.records[0].baseScore==888 && bucket.records[0].depotDistance==88 && bucket.records[0].depotId==999 && bucket.records[0].x==41 && bucket.records[0].y==42 && bucket.records[0].selections==200 && bucket.records[0].cooldown==0 && bucket.records[0].invalid==0 && bucket.records[0].blockedAt==123456,"Original metric refresh preserves independent record state");
 }
 {AiResourceBucket bucket;std::map<int,AiResourceDepotMetric> metrics;
 bucket.records.push_back(AiResourceRecord{11,41,42,200,888,0,1,88,0,999,123456});
 metrics.emplace(11,AiResourceDepotMetric{9,5});
 refreshAiResourceBucket(bucket,[&](int id)->std::optional<AiResourceDepotMetric>{auto i=metrics.find(id);return i==metrics.end()?std::nullopt:std::optional<AiResourceDepotMetric>(i->second);});
 check(bucket.cachedBestId==-1 && bucket.cachedBestDistance==-1,"Original refreshed cache result");
 check(bucket.records[0].baseScore==888 && bucket.records[0].depotDistance==88 && bucket.records[0].depotId==999 && bucket.records[0].x==41 && bucket.records[0].y==42 && bucket.records[0].selections==200 && bucket.records[0].cooldown==0 && bucket.records[0].invalid==1 && bucket.records[0].blockedAt==123456,"Original metric refresh preserves independent record state");
 }
 {AiResourceBucket bucket;std::map<int,AiResourceDepotMetric> metrics;
 bucket.records.push_back(AiResourceRecord{11,41,42,200,888,0,0,88,0,999,123456});
 metrics.emplace(11,AiResourceDepotMetric{9,500});
 refreshAiResourceBucket(bucket,[&](int id)->std::optional<AiResourceDepotMetric>{auto i=metrics.find(id);return i==metrics.end()?std::nullopt:std::optional<AiResourceDepotMetric>(i->second);});
 check(bucket.cachedBestId==11 && bucket.cachedBestDistance==255,"Original refreshed cache result");
 check(bucket.records[0].baseScore==510 && bucket.records[0].depotDistance==255 && bucket.records[0].depotId==9 && bucket.records[0].x==41 && bucket.records[0].y==42 && bucket.records[0].selections==200 && bucket.records[0].cooldown==0 && bucket.records[0].invalid==0 && bucket.records[0].blockedAt==123456,"Original metric refresh preserves independent record state");
 }
 {AiResourceBucket bucket;std::map<int,AiResourceDepotMetric> metrics;
 bucket.records.push_back(AiResourceRecord{11,41,42,200,888,0,0,88,0,999,123456});
 metrics.emplace(11,AiResourceDepotMetric{9,9.9});
 bucket.records.push_back(AiResourceRecord{22,41,42,200,888,0,0,88,0,999,123456});
 metrics.emplace(22,AiResourceDepotMetric{9,9.1});
 refreshAiResourceBucket(bucket,[&](int id)->std::optional<AiResourceDepotMetric>{auto i=metrics.find(id);return i==metrics.end()?std::nullopt:std::optional<AiResourceDepotMetric>(i->second);});
 check(bucket.cachedBestId==11 && bucket.cachedBestDistance==9,"Original refreshed cache result");
 check(bucket.records[0].baseScore==18 && bucket.records[0].depotDistance==9 && bucket.records[0].depotId==9 && bucket.records[0].x==41 && bucket.records[0].y==42 && bucket.records[0].selections==200 && bucket.records[0].cooldown==0 && bucket.records[0].invalid==0 && bucket.records[0].blockedAt==123456,"Original metric refresh preserves independent record state");
 check(bucket.records[1].baseScore==18 && bucket.records[1].depotDistance==9 && bucket.records[1].depotId==9 && bucket.records[1].x==41 && bucket.records[1].y==42 && bucket.records[1].selections==200 && bucket.records[1].cooldown==0 && bucket.records[1].invalid==0 && bucket.records[1].blockedAt==123456,"Original metric refresh preserves independent record state");
 }
 {AiResourceBucket bucket;std::map<int,AiResourceDepotMetric> metrics;
 bucket.records.push_back(AiResourceRecord{11,41,42,200,888,3,2,88,0,999,123456});
 metrics.emplace(11,AiResourceDepotMetric{9,5});
 refreshAiResourceBucket(bucket,[&](int id)->std::optional<AiResourceDepotMetric>{auto i=metrics.find(id);return i==metrics.end()?std::nullopt:std::optional<AiResourceDepotMetric>(i->second);});
 check(bucket.cachedBestId==11 && bucket.cachedBestDistance==5,"Original refreshed cache result");
 check(bucket.records[0].baseScore==10 && bucket.records[0].depotDistance==5 && bucket.records[0].depotId==9 && bucket.records[0].x==41 && bucket.records[0].y==42 && bucket.records[0].selections==200 && bucket.records[0].cooldown==3 && bucket.records[0].invalid==2 && bucket.records[0].blockedAt==123456,"Original metric refresh preserves independent record state");
 }
 {AiResourceBucket bucket;std::map<int,AiResourceDepotMetric> metrics;
 bucket.records.push_back(AiResourceRecord{11,41,42,200,888,0,0,88,0,999,123456});
 bucket.records.push_back(AiResourceRecord{22,41,42,200,888,0,0,88,0,999,123456});
 metrics.emplace(22,AiResourceDepotMetric{9,2});
 bucket.records.push_back(AiResourceRecord{33,41,42,200,888,0,1,88,0,999,123456});
 metrics.emplace(33,AiResourceDepotMetric{9,1});
 refreshAiResourceBucket(bucket,[&](int id)->std::optional<AiResourceDepotMetric>{auto i=metrics.find(id);return i==metrics.end()?std::nullopt:std::optional<AiResourceDepotMetric>(i->second);});
 check(bucket.cachedBestId==22 && bucket.cachedBestDistance==2,"Original refreshed cache result");
 check(bucket.records[0].baseScore==888 && bucket.records[0].depotDistance==88 && bucket.records[0].depotId==999 && bucket.records[0].x==41 && bucket.records[0].y==42 && bucket.records[0].selections==200 && bucket.records[0].cooldown==0 && bucket.records[0].invalid==0 && bucket.records[0].blockedAt==123456,"Original metric refresh preserves independent record state");
 check(bucket.records[1].baseScore==4 && bucket.records[1].depotDistance==2 && bucket.records[1].depotId==9 && bucket.records[1].x==41 && bucket.records[1].y==42 && bucket.records[1].selections==200 && bucket.records[1].cooldown==0 && bucket.records[1].invalid==0 && bucket.records[1].blockedAt==123456,"Original metric refresh preserves independent record state");
 check(bucket.records[2].baseScore==888 && bucket.records[2].depotDistance==88 && bucket.records[2].depotId==999 && bucket.records[2].x==41 && bucket.records[2].y==42 && bucket.records[2].selections==200 && bucket.records[2].cooldown==0 && bucket.records[2].invalid==1 && bucket.records[2].blockedAt==123456,"Original metric refresh preserves independent record state");
 }
 {AiResourceBucket bucket;bucket.cachedBestId=-1;bucket.cachedBestDistance=-1;AiResourceRecord record{11,41,42,200,888,3,0,88,1,999,123456};
 observeAiResourceMetric(bucket,record,{9,5});
 check(bucket.cachedBestId==11 && bucket.cachedBestDistance==5 && record.baseScore==10 && record.depotDistance==5 && record.x==41 && record.y==42 && record.invalid==0 && record.cooldown==3 && record.selections==200 && record.blockedAt==123456,"Original duplicate admission preserves coordinates and does not revive records");}
 {AiResourceBucket bucket;bucket.cachedBestId=22;bucket.cachedBestDistance=4;AiResourceRecord record{11,41,42,200,888,3,0,88,1,999,123456};
 observeAiResourceMetric(bucket,record,{9,5});
 check(bucket.cachedBestId==22 && bucket.cachedBestDistance==4 && record.baseScore==10 && record.depotDistance==5 && record.x==41 && record.y==42 && record.invalid==0 && record.cooldown==3 && record.selections==200 && record.blockedAt==123456,"Original duplicate admission preserves coordinates and does not revive records");}
 {AiResourceBucket bucket;bucket.cachedBestId=22;bucket.cachedBestDistance=5;AiResourceRecord record{11,41,42,200,888,3,0,88,1,999,123456};
 observeAiResourceMetric(bucket,record,{9,5});
 check(bucket.cachedBestId==22 && bucket.cachedBestDistance==5 && record.baseScore==10 && record.depotDistance==5 && record.x==41 && record.y==42 && record.invalid==0 && record.cooldown==3 && record.selections==200 && record.blockedAt==123456,"Original duplicate admission preserves coordinates and does not revive records");}
 {AiResourceBucket bucket;bucket.cachedBestId=22;bucket.cachedBestDistance=9;AiResourceRecord record{11,41,42,200,888,3,0,88,1,999,123456};
 observeAiResourceMetric(bucket,record,{9,5});
 check(bucket.cachedBestId==11 && bucket.cachedBestDistance==5 && record.baseScore==10 && record.depotDistance==5 && record.x==41 && record.y==42 && record.invalid==0 && record.cooldown==3 && record.selections==200 && record.blockedAt==123456,"Original duplicate admission preserves coordinates and does not revive records");}
 {AiResourceBucket bucket;bucket.cachedBestId=11;bucket.cachedBestDistance=5;AiResourceRecord record{11,41,42,200,888,3,0,88,1,999,123456};
 observeAiResourceMetric(bucket,record,{9,9});
 check(bucket.cachedBestId==11 && bucket.cachedBestDistance==5 && record.baseScore==18 && record.depotDistance==9 && record.x==41 && record.y==42 && record.invalid==0 && record.cooldown==3 && record.selections==200 && record.blockedAt==123456,"Original duplicate admission preserves coordinates and does not revive records");}
 {AiResourceBucket bucket;bucket.cachedBestId=-1;bucket.cachedBestDistance=-1;AiResourceRecord record{11,41,42,200,888,3,1,88,1,999,123456};
 observeAiResourceMetric(bucket,record,{9,5});
 check(bucket.cachedBestId==11 && bucket.cachedBestDistance==5 && record.baseScore==10 && record.depotDistance==5 && record.x==41 && record.y==42 && record.invalid==1 && record.cooldown==3 && record.selections==200 && record.blockedAt==123456,"Original duplicate admission preserves coordinates and does not revive records");}
 {AiResourceBucket bucket;bucket.cachedBestId=-1;bucket.cachedBestDistance=-1;AiResourceRecord record{11,41,42,200,888,3,0,88,1,999,123456};
 observeAiResourceMetric(bucket,record,{9,500});
 check(bucket.cachedBestId==11 && bucket.cachedBestDistance==255 && record.baseScore==510 && record.depotDistance==255 && record.x==41 && record.y==42 && record.invalid==0 && record.cooldown==3 && record.selections==200 && record.blockedAt==123456,"Original duplicate admission preserves coordinates and does not revive records");}
 check(!aiFoodMetricRefreshDue(15000,0) && aiFoodMetricRefreshDue(15001,0) && !aiFoodMetricRefreshDue(30001,15001),"Strict periodic food metric refresh boundary");
 std::cout<<"PASS: original resource metric lifecycle\n";
}
