// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/ai_gather_controller.h"
#include <cstdlib>
#include <iostream>
#include <vector>
using namespace swgb;
struct Record {int id=42,resource=0,batch=20,completed=0;uint32_t lastUpdate=0;};
struct Call {int bucket;bool force,mask;std::array<int32_t,4> tried{};};
static void check(bool value,const char *message) {if(!value){std::cerr<<message<<'\n';std::exit(1);}}
int main() {
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=false;in.task=609;in.command=711;in.priority=0;in.carriedAmount=0;in.hostTime=12345;
 in.assigned={4,0,0,0};in.desired={1,3,0,0};record.resource=0;record.batch=20;record.completed=0;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return true;});
 check(result==true && calls.size()==0,"Original worker controller result/call count: active-incomplete");
 check(record.resource==0 && record.batch==20 && record.completed==0 && record.lastUpdate==12345,"Original record reset/preservation: active-incomplete");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=false;in.task=609;in.command=711;in.priority=0;in.carriedAmount=0;in.hostTime=12345;
 in.assigned={4,0,0,0};in.desired={1,3,0,0};record.resource=0;record.batch=20;record.completed=20;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return true;});
 check(result==true && calls.size()==1,"Original worker controller result/call count: active-atbatch");
 check(calls[0].bucket==1 && calls[0].force==false && calls[0].mask==false,"Original assignment order/force");
 check(record.resource==0 && record.batch==20 && record.completed==20 && record.lastUpdate==12345,"Original record reset/preservation: active-atbatch");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=false;in.task=609;in.command=711;in.priority=0;in.carriedAmount=0;in.hostTime=12345;
 in.assigned={4,0,0,0};in.desired={1,3,0,0};record.resource=0;record.batch=20;record.completed=21;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return true;});
 check(result==true && calls.size()==1,"Original worker controller result/call count: active-overbatch");
 check(calls[0].bucket==1 && calls[0].force==false && calls[0].mask==false,"Original assignment order/force");
 check(record.resource==0 && record.batch==20 && record.completed==21 && record.lastUpdate==12345,"Original record reset/preservation: active-overbatch");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=false;in.task=-1;in.command=711;in.priority=0;in.carriedAmount=0;in.hostTime=12345;
 in.assigned={4,0,0,0};in.desired={1,3,0,0};record.resource=0;record.batch=20;record.completed=0;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return true;});
 check(result==true && calls.size()==1,"Original worker controller result/call count: idle-incomplete");
 check(calls[0].bucket==0 && calls[0].force==false && calls[0].mask==false,"Original assignment order/force");
 check(record.resource==0 && record.batch==20 && record.completed==0 && record.lastUpdate==12345,"Original record reset/preservation: idle-incomplete");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=false;in.task=-1;in.command=711;in.priority=0;in.carriedAmount=0;in.hostTime=12345;
 in.assigned={4,0,0,0};in.desired={1,3,0,0};record.resource=0;record.batch=20;record.completed=20;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return true;});
 check(result==true && calls.size()==1,"Original worker controller result/call count: idle-atbatch");
 check(calls[0].bucket==1 && calls[0].force==false && calls[0].mask==false,"Original assignment order/force");
 check(record.resource==0 && record.batch==20 && record.completed==20 && record.lastUpdate==12345,"Original record reset/preservation: idle-atbatch");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=false;in.task=-1;in.command=711;in.priority=0;in.carriedAmount=0;in.hostTime=12345;
 in.assigned={4,0,0,0};in.desired={1,3,0,0};record.resource=0;record.batch=20;record.completed=0;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return false;});
 check(result==true && calls.size()==3,"Original worker controller result/call count: idle-retry-fails");
 check(calls[0].bucket==0 && calls[0].force==false && calls[0].mask==false,"Original assignment order/force");
 check(calls[1].bucket==1 && calls[1].force==false && calls[1].mask==false,"Original assignment order/force");
 check(calls[2].bucket==-1 && calls[2].force==true && calls[2].mask==true,"Original assignment order/force");
 check(calls[2].tried==std::array<int32_t,4>{1,1,0,0},"Original tried-bucket fallback mask");
 check(record.resource==-1 && record.batch==-1 && record.completed==-1 && record.lastUpdate==12345,"Original record reset/preservation: idle-retry-fails");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=false;in.task=609;in.command=711;in.priority=0;in.carriedAmount=0;in.hostTime=12345;
 in.assigned={1,0,0,0};in.desired={1,3,0,0};record.resource=0;record.batch=20;record.completed=20;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return true;});
 check(result==true && calls.size()==0,"Original worker controller result/call count: satisfied-active");
 check(record.resource==0 && record.batch==20 && record.completed==20 && record.lastUpdate==12345,"Original record reset/preservation: satisfied-active");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=false;in.task=-1;in.command=711;in.priority=0;in.carriedAmount=0;in.hostTime=12345;
 in.assigned={1,0,0,0};in.desired={1,3,0,0};record.resource=0;record.batch=20;record.completed=20;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return true;});
 check(result==true && calls.size()==1,"Original worker controller result/call count: satisfied-idle");
 check(calls[0].bucket==0 && calls[0].force==false && calls[0].mask==false,"Original assignment order/force");
 check(record.resource==0 && record.batch==20 && record.completed==20 && record.lastUpdate==12345,"Original record reset/preservation: satisfied-idle");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=false;in.task=609;in.command=711;in.priority=0;in.carriedAmount=2;in.hostTime=12345;
 in.assigned={4,0,0,0};in.desired={1,3,0,0};record.resource=0;record.batch=20;record.completed=20;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return true;});
 check(result==true && calls.size()==0,"Original worker controller result/call count: carrying-active");
 check(record.resource==0 && record.batch==20 && record.completed==20 && record.lastUpdate==12345,"Original record reset/preservation: carrying-active");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=false;in.task=-1;in.command=711;in.priority=0;in.carriedAmount=2;in.hostTime=12345;
 in.assigned={4,0,0,0};in.desired={1,3,0,0};record.resource=0;record.batch=20;record.completed=20;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return true;});
 check(result==true && calls.size()==1,"Original worker controller result/call count: carrying-idle");
 check(calls[0].bucket==0 && calls[0].force==false && calls[0].mask==false,"Original assignment order/force");
 check(record.resource==0 && record.batch==20 && record.completed==20 && record.lastUpdate==12345,"Original record reset/preservation: carrying-idle");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=false;in.task=-1;in.command=711;in.priority=0;in.carriedAmount=2;in.hostTime=12345;
 in.assigned={4,0,0,0};in.desired={1,3,0,0};record.resource=0;record.batch=20;record.completed=0;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return false;});
 check(result==true && calls.size()==1,"Original worker controller result/call count: carrying-idle-fail");
 check(calls[0].bucket==0 && calls[0].force==false && calls[0].mask==false,"Original assignment order/force");
 check(record.resource==-1 && record.batch==-1 && record.completed==-1 && record.lastUpdate==12345,"Original record reset/preservation: carrying-idle-fail");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=false;in.task=600;in.command=711;in.priority=0;in.carriedAmount=0;in.hostTime=12345;
 in.assigned={4,0,0,0};in.desired={1,3,0,0};record.resource=0;record.batch=20;record.completed=20;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return true;});
 check(result==true && calls.size()==0,"Original worker controller result/call count: attack");
 check(record.resource==0 && record.batch==20 && record.completed==20 && record.lastUpdate==12345,"Original record reset/preservation: attack");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=false;in.task=602;in.command=711;in.priority=0;in.carriedAmount=0;in.hostTime=12345;
 in.assigned={4,0,0,0};in.desired={1,3,0,0};record.resource=0;record.batch=20;record.completed=20;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return true;});
 check(result==true && calls.size()==0,"Original worker controller result/call count: build");
 check(record.resource==0 && record.batch==20 && record.completed==20 && record.lastUpdate==12345,"Original record reset/preservation: build");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=false;in.task=618;in.command=711;in.priority=0;in.carriedAmount=0;in.hostTime=12345;
 in.assigned={4,0,0,0};in.desired={1,3,0,0};record.resource=0;record.batch=20;record.completed=20;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return true;});
 check(result==true && calls.size()==0,"Original worker controller result/call count: repair");
 check(record.resource==0 && record.batch==20 && record.completed==20 && record.lastUpdate==12345,"Original record reset/preservation: repair");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=false;in.task=609;in.command=710;in.priority=100;in.carriedAmount=0;in.hostTime=12345;
 in.assigned={4,0,0,0};in.desired={1,3,0,0};record.resource=0;record.batch=20;record.completed=20;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return true;});
 check(result==true && calls.size()==0,"Original worker controller result/call count: forced-move");
 check(record.resource==0 && record.batch==20 && record.completed==20 && record.lastUpdate==12345,"Original record reset/preservation: forced-move");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=false;in.task=609;in.command=710;in.priority=99;in.carriedAmount=0;in.hostTime=12345;
 in.assigned={4,0,0,0};in.desired={1,3,0,0};record.resource=0;record.batch=20;record.completed=20;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return true;});
 check(result==true && calls.size()==1,"Original worker controller result/call count: normal-move");
 check(calls[0].bucket==1 && calls[0].force==false && calls[0].mask==false,"Original assignment order/force");
 check(record.resource==0 && record.batch==20 && record.completed==20 && record.lastUpdate==12345,"Original record reset/preservation: normal-move");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=false;in.task=609;in.command=702;in.priority=0;in.carriedAmount=0;in.hostTime=12345;
 in.assigned={4,0,0,0};in.desired={1,3,0,0};record.resource=0;record.batch=20;record.completed=20;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return true;});
 check(result==true && calls.size()==0,"Original worker controller result/call count: command702");
 check(record.resource==0 && record.batch==20 && record.completed==20 && record.lastUpdate==12345,"Original record reset/preservation: command702");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=false;in.task=-1;in.command=711;in.priority=0;in.carriedAmount=0;in.hostTime=12345;
 in.assigned={4,0,0,0};in.desired={1,3,0,0};record.resource=-1;record.batch=-1;record.completed=-1;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return true;});
 check(result==true && calls.size()==1,"Original worker controller result/call count: unassigned-idle");
 check(calls[0].bucket==1 && calls[0].force==false && calls[0].mask==false,"Original assignment order/force");
 check(record.resource==-1 && record.batch==-1 && record.completed==-1 && record.lastUpdate==12345,"Original record reset/preservation: unassigned-idle");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=false;in.task=-1;in.command=711;in.priority=0;in.carriedAmount=0;in.hostTime=12345;
 in.assigned={4,0,0,0};in.desired={0,0,0,0};record.resource=0;record.batch=20;record.completed=20;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return true;});
 check(result==true && calls.size()==1,"Original worker controller result/call count: no-needed-resource");
 check(calls[0].bucket==-1 && calls[0].force==true && calls[0].mask==true,"Original assignment order/force");
 check(calls[0].tried==std::array<int32_t,4>{0,0,0,0},"Original tried-bucket fallback mask");
 check(record.resource==0 && record.batch==20 && record.completed==20 && record.lastUpdate==12345,"Original record reset/preservation: no-needed-resource");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=true;in.contained=true;in.task=609;in.command=711;in.priority=0;in.carriedAmount=0;in.hostTime=12345;
 in.assigned={4,0,0,0};in.desired={1,3,0,0};record.resource=0;record.batch=20;record.completed=0;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return true;});
 check(result==false && calls.size()==0,"Original worker controller result/call count: contained");
 check(record.resource==0 && record.batch==20 && record.completed==0 && record.lastUpdate==0,"Original record reset/preservation: contained");
 }
 {AiGatherControllerInput in;Record record;std::vector<Call> calls;
 in.registered=false;in.contained=false;in.task=609;in.command=711;in.priority=0;in.carriedAmount=0;in.hostTime=12345;
 in.assigned={4,0,0,0};in.desired={1,3,0,0};record.resource=0;record.batch=20;record.completed=0;
 const bool result=updateAiGatherController(record,in,[&](int bucket,bool force,const std::array<int32_t,4> *mask) {calls.push_back({bucket,force,mask!=nullptr,mask?*mask:std::array<int32_t,4>{}});return true;});
 check(result==false && calls.size()==0,"Original worker controller result/call count: unregistered");
 check(record.resource==0 && record.batch==20 && record.completed==0 && record.lastUpdate==0,"Original record reset/preservation: unregistered");
 }
 std::cout<<"PASS: original gatherer preservation and reassignment\n";
}
