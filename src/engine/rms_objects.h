// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace swgb {
// Original initial, full-map, all-eligible candidate list (0x4dded0).
// Entries are y*width+x. This does not test placement or create objects.
// Input is RNG state at candidate construction, not a map seed; only success advances it.
bool generateRmsObjectCandidates(uint32_t width,uint32_t height,uint32_t &randomState,
                                 std::vector<uint32_t> &out,std::string *err=nullptr);
} // namespace swgb
