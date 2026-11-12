// SPDX-License-Identifier: GPL-3.0-or-later
#include "engine/rms_objects.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <numeric>

static void check(bool value, const char *message) {
    if (!value) { std::cerr << message << '\n'; std::exit(1); }
}

int main() {
    using namespace swgb;
    std::vector<uint32_t> out;
    std::string err;
    // Independently derived from original 4dded0 / 4d6ab0 and CRT draws.
    const auto fixture = [&](uint32_t seed, uint32_t finalState,
                             std::vector<uint32_t> prefix) {
        auto expected = prefix;
        for (uint32_t i=0;i<30;++i)
            if (std::find(prefix.begin(),prefix.end(),i)==prefix.end()) expected.push_back(i);
        check(generateRmsObjectCandidates(5,6,seed,out,&err), "candidate generation failed");
        check(out==expected, "original move-to-front ordering mismatch");
        check(seed==finalState, "candidate RNG consumption mismatch");
    };
    fixture(1,1030492215,{6,15,10});
    fixture(2,1287834384,{12,15}); // Repeated node is moved, not duplicated.
    fixture(1967,159055541,{1,13,5}); // Draw32767 reaches the span endpoint.
    uint32_t rng=1;
    check(generateRmsObjectCandidates(3,7,rng,out,&err), "narrow map failed");
    std::vector<uint32_t> rowMajor(21);
    std::iota(rowMajor.begin(),rowMajor.end(),0);
    check(out==rowMajor && rng==1, "narrow map must not consume RNG");
    check(generateRmsObjectCandidates(1,1,rng,out,&err) && out==std::vector<uint32_t>{0},
          "singleton candidate lost");
    check(!generateRmsObjectCandidates(0,1,rng,out,&err) && out.empty() && rng==1 && !err.empty(),
          "invalid dimensions must clear output and preserve RNG");
    check(!generateRmsObjectCandidates(1,1025,rng,out,&err) && rng==1, "dimension limit");
    check(generateRmsObjectCandidates(1024,1024,rng,out,&err) && err.empty(), "maximum map failed");
    std::sort(out.begin(),out.end());
    check(out.size()==1024*1024, "maximum map size");
    for (uint32_t i=0;i<out.size();++i) check(out[i]==i,"candidate list is not a permutation");
    std::cout << "RMS object candidate tests passed\n";
}
