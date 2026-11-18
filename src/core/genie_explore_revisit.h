// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
namespace swgb {
// Original585ba8..585c60. Caller supplies a genuinely initialized timestamp;
// absence of constructor/session context must not be represented by zero.
inline bool genieExploreRevisitDue(uint32_t now,uint32_t &previous,
                                  uint32_t candidateCells,uint32_t totalCells) {
    if(uint32_t(now-previous)<=120000u)return false;
    previous=now; // Timestamp advances even when the coverage gate rejects.
    // Original compares an extended-precision fraction with float0.05. Exact
    // five-percent coverage is below that float and therefore DOES qualify.
    return totalCells && double(candidateCells)/double(totalCells)<double(0.05f);
}
} // namespace swgb
