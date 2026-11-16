// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/engine/local_command_queue.h"
#include <memory>
#include <vector>

static void localCommandQueueChecks() {
    using Queue = swgb::LocalCommandQueue;
    Queue queue;
    std::vector<int> seen;
    auto add = [&](int tag) { queue.enqueue([&,tag] { seen.push_back(tag); }); };
    // Actual original constructor/enqueue/dequeue controls in
    // rms-local-command-ring-oracle-2026-11-16.json.
    for (int tag=1;tag<2000;++tag) add(tag);
    queue.drain();
    bool ordered=seen.size()==1999;
    for (std::size_t i=0;i<seen.size();++i) ordered &= seen[i]==int(i+1);
    check(ordered,"Local commands drain in physical slot order before wrap");
    seen.clear();
    add(2000); add(2001); add(2002);
    queue.drain();
    check(seen==std::vector<int>({2001,2002,2000}),
          "Original drain preserves cursor and wraps physical dispatch order");
    queue.drain();
    check(seen.size()==3,"Consumed commands are not dispatched twice");

    queue.reset(); seen.clear();
    for (int tag=1;tag<=2002;++tag) add(tag);
    queue.drain();
    ordered=seen.size()==2000 && seen[0]==2001 && seen[1]==2002;
    for (std::size_t i=2;i<seen.size();++i) ordered &= seen[i]==int(i+1);
    check(ordered,"Ring overwrite replaces pending oldest slots without growth");

    queue.reset(); seen.clear();
    for (int tag=1;tag<2000;++tag) add(tag);
    queue.clear();
    add(2000); add(2001); add(2002);
    queue.drain();
    check(seen==std::vector<int>({2001,2002,2000}),
          "Clear consumes old records without resetting the write cursor");

    queue.reset(); seen.clear();
    int target=7;
    queue.enqueue([&,target] { seen.push_back(target); });
    target=99;
    queue.drain();
    check(seen==std::vector<int>({7}),"Queued payload can own an immutable input snapshot");

    queue.reset(); seen.clear();
    queue.enqueue([&] {
        // Fill the ring so the final enqueue replaces this executing slot.
        for (int tag=1;tag<=2000;++tag) add(tag);
        seen.push_back(-1);
    });
    queue.drain();
    ordered=seen.size()==2001 && seen[0]==-1 && seen[1]==2000;
    for (std::size_t i=2;i<seen.size();++i) ordered &= seen[i]==int(i-1);
    check(ordered,"Dispatch consumes before invocation and safely permits self-slot overwrite");

    queue.reset(); seen.clear();
    auto payload=std::make_shared<int>(42);
    std::weak_ptr<int> lifetime=payload;
    queue.enqueue([payload] {});
    payload.reset();
    queue.drain(); queue.clear();
    check(!lifetime.expired(),"Consumed command payload remains owned until overwrite or reset");
    queue.reset();
    check(lifetime.expired(),"Session reset releases retained command closures");

    queue.enqueue([&] { queue.reset(); add(9); });
    add(10);
    queue.drain();
    check(seen==std::vector<int>({9}),"Callback reset removes prior-session commands safely");
    check(!queue.enqueue({}),"Empty command is rejected without invoking an empty function");
}
