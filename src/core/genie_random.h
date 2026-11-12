// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>

namespace swgb {
// Original Windows CRT draw (0x632bdd); callers own the shared stream position.
inline uint32_t genieRandomDraw(uint32_t &state) {
    state=state*214013u+2531011u;
    return (state>>16)&32767u;
}
} // namespace swgb
