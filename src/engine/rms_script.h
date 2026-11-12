// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_set>
#include <vector>

namespace swgb {
class ResourceSet;
namespace dat { struct TerrainBlock; }

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
    // Resolve integer constants and defined-symbol branches without modifying
    // the loaded source. Optional RNG state enables original nonnested random
    // blocks; it is advanced only on success. This is shared CRT state at script
    // entry, not a promise that a map seed reproduces a generated map.
    bool preprocess(const std::unordered_set<std::string> &defines,
                    std::vector<RmsToken> &out, std::string *err = nullptr,
                    uint32_t *randomState = nullptr) const;
private:
    std::vector<RmsToken> tokens_;
};
// Execute only an explicit LAND_GENERATION/base_terrain script. Every other
// directive is rejected: this cannot silently turn a full RMS into a flat map.
// Produces terrain IDs only, without elevation, objects, borders or player starts.
bool generateRmsBaseTerrain(const std::vector<RmsToken> &directives,
                            uint32_t width, uint32_t height,
                            const dat::TerrainBlock &terrainData,
                            std::vector<uint8_t> &out, std::string *err = nullptr);
} // namespace swgb
