// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace swgb {
class ResourceSet;

struct RmsToken {
    std::string text;
    int32_t resourceId;
    size_t line;
};

// Loading boundary only: includes are expanded, but constants, conditions and
// random choices remain tokens. This is NOT an executable map-generation plan.
class RmsScript {
public:
    using Resolver = std::function<bool(int32_t, std::vector<uint8_t> &)>;
    struct Limits {
        size_t maxDepth = 16;
        size_t maxBytes = 4 * 1024 * 1024;
        size_t maxTokens = 200000;
    };
    bool load(int32_t resourceId, ResourceSet &resources, std::string *err = nullptr);
    bool load(int32_t resourceId, const Resolver &resolver,
              const Limits &limits, std::string *err = nullptr);
    const std::vector<RmsToken> &tokens() const { return tokens_; }
private:
    std::vector<RmsToken> tokens_;
};
} // namespace swgb
