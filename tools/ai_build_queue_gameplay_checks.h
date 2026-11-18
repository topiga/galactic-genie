// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
[[gnu::noinline]] static void aiBuildQueueGameplayChecks(swgb::Assets &assets) {
    swgb::Game game(assets);std::string error;
    const auto &prefab=assets.dat().civs[1].units[70];
    auto reset=[&] {
        check(game.initSkirmish(0x5A17,&error,false,1,1),error.c_str());
        for(int player:{1,2})check(game.loadAiSourceForTesting(player,"build-queue-passive",
            "(defrule (true) => (disable-self))",{},&error),error.c_str());
        game.update(0,{});
        for(int type=0;type<4;++type)game.setResourceForTesting(1,type,1000);
    };
    auto prefabFoundations=[&] {
        std::vector<uint32_t> ids;
        for(uint32_t id:game.underConstructionObjectIds())if(game.objectUnitId(id)==70)ids.push_back(id);
        return ids;
    };
    reset();
    const auto costs=game.unitCostsForTesting(1,prefab);
    check(game.admitAiBuildForTesting(1,prefab),"Production admission accepts first prefab request");
    check(!game.admitAiBuildForTesting(1,prefab),"Duplicate pending prototype is rejected before another request exists");
    check(game.aiPendingBuildsForTesting(1,70)==1 && prefabFoundations().empty(),
          "Build request is pending without creating a foundation");
    for(const auto &cost:costs)if(cost.flag && cost.type>=0 && cost.amount>0)
        check(game.resource(1,cost.type)==1000,"Request admission does not spend resources");
    game.updateAiBuildRequestsForTesting(1);
    check(prefabFoundations().empty(),"Scheduler dispatch remains deferred until world command drain");
    check(game.loadAiSourceForTesting(1,"build-queue-reloaded",
        "(defrule (true) => (disable-self))",{},&error),error.c_str());
    check(game.aiPendingBuildsForTesting(1,70)==1,
          "Successful script reload preserves a pending request and its queued command identity");
    game.update(0,{});
    auto foundations=prefabFoundations();check(foundations.size()==1,"World command creates exactly one foundation");
    for(const auto &cost:costs)if(cost.flag && cost.type>=0 && cost.amount>0)
        check(game.resource(1,cost.type)==1000-cost.amount,"World command charges construction once");
    check(game.aiPendingBuildsForTesting(1,70)==1 && !game.admitAiBuildForTesting(1,prefab),
          "Pending deduplication lasts throughout construction");
    check(game.completeFoundationForTesting(foundations.front()),"Complete queued foundation through shared completion path");
    game.update(0,{});
    check(game.aiPendingBuildsForTesting(1,70)==0,"Original completion acknowledgement releases pending prototype");
    check(game.admitAiBuildForTesting(1,prefab),"A later request is admitted after construction completes");

    reset();check(game.admitAiBuildForTesting(1,prefab),"Admit failure-and-retry fixture");
    game.updateAiBuildRequestsForTesting(1);
    for(const auto &cost:costs)if(cost.flag && cost.type>=0 && cost.amount>0)game.setResourceForTesting(1,cost.type,0);
    game.update(0,{});
    check(prefabFoundations().empty() && game.aiPendingBuildsForTesting(1,70)==1,
          "Execution-time cost failure creates nothing and retains pending request for retry");
    for(int type=0;type<4;++type)game.setResourceForTesting(1,type,1000);
    game.updateAiBuildRequestsForTesting(1);game.update(0,{});
    foundations=prefabFoundations();check(foundations.size()==1,"Retained failed request can choose another site and retry");
    check(game.aiPendingBuildsForTesting(1,70)==1,"Successful retry remains pending until completion");
    check(game.setConstructionProgressForTesting(foundations.front(),.25f),"Partially construct cancellation fixture");
    std::array<float,4> before{};for(int type=0;type<4;++type)before[size_t(type)]=game.resource(1,type);
    check(game.selectObjectForTesting(foundations.front()) && game.destroyLastSelectedForTesting(),
          "Actual selected-foundation cancellation routes the original failure notification");
    check(prefabFoundations().empty(),"Cancelled foundation is removed");
    for(const auto &cost:costs)if(cost.flag && cost.type>=0 && cost.type<4 && cost.amount>0)
        check(std::abs(game.resource(1,cost.type)-before[size_t(cost.type)]-.75f*cost.amount)<.001f,
              "Explicit cancellation refunds only the unfinished construction fraction");
    check(game.aiPendingBuildsForTesting(1,70)==1,"Cancellation preserves pending request rather than granting a duplicate");
    check(!game.aiBuildRequestInProgressForTesting(1),"Cancellation clears in-progress so original request may retry");
    game.updateAiBuildRequestsForTesting(1);game.update(0,{});
    check(prefabFoundations().size()==1,"Cancelled request retries through actual world command execution");
    reset();
    const int workers=game.aiObjectCountForTesting(1,"UNIT-WORKER",false);
    check(workers>0 && workers<20,"Repeated-build fixture has a bounded original worker roster");
    for(int build=0;build<=workers;++build) {
        check(game.admitAiBuildForTesting(1,prefab),"Admit successive completed-worker request");
        for(int tick=0;tick<45 && prefabFoundations().empty();++tick)game.update(0,{});
        foundations=prefabFoundations();
        check(foundations.size()==1,"Admitted tactical passes release reservations so existing workers can build again");
        check(game.completeFoundationForTesting(foundations.front()),"Complete each successive request");
        game.update(0,{});
        check(game.aiPendingBuildsForTesting(1,70)==0,"Successive completion releases pending count");
    }
    std::puts("PASS: deferred build cost/factory, pending-through-completion, reload, retries, cancellation refund and worker reuse");
}
