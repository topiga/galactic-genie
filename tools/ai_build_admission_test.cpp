// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/engine/ai_build_admission.h"
#include <cassert>
#include <iostream>
int main(){
    swgb::AiBuildAttemptLatch a,b;
    assert(a.canCheck(70,false) && a.begin(70,false) && a.attempted);
    assert(!a.canCheck(109,false) && !a.begin(109,false));
    assert(b.canCheck(70,false)); // Distinct owner/context has no inherited attempt.
    a.reset();assert(!a.begin(70,true) && !a.attempted);
    assert(a.begin(109,true) && a.attempted); // Original command-center exception.
    a.reset();assert(!a.canCheck(72,false) && !a.attempted);
    assert(!a.begin(72,false) && a.attempted); // Invalid action still consumes latch.
    a.reset();assert(a.begin(70,false)); // Later unavailable/cost/pending failure must not roll this back.
    assert(!a.canCheck(70,false));
    for(int id:{72,117,155,195,487,-2}) {a.reset();assert(!a.begin(id,false) && a.attempted);}
    a.reset();assert(a.canCheck(-3,false) && a.canCheck(-4,false)); // Aliases resolved by downstream original provider.
    std::cout<<"AI build admission checks passed\n";
}
