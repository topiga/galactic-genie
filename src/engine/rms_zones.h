// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace swgb {
struct RmsZones {
    std::vector<uint8_t> tiles;
    uint32_t count=0; // Original caps at255; component IDs wrap through0..254.
};
// Original terrain-only zone generation (624ee0), for square maps.
// Both positive and nonpositive passability components receive IDs, row-major.
// Diagonal contact does not connect zones. Adjacency tables/cache lifetime are separate.
// Failure clears output; no RNG used.
bool generateRmsZones(uint32_t size,const std::vector<uint8_t> &terrain,
    const std::vector<float> &passability,RmsZones &out,std::string *err=nullptr);
struct RmsZoneAdjacency {
    // Row-major count*count tables. Contact includes diagonals and self.
    std::vector<uint8_t> contact;
    // For noncontacting pairs: first common neighbor, or255. Contacting pairs stay255.
    std::vector<uint8_t> commonNeighbor;
};
// Original625080/6254c0: a two-hop lookup, not transitive reachability.
// Failure clears output; no RNG used.
bool generateRmsZoneAdjacency(uint32_t size,const RmsZones &zones,
    RmsZoneAdjacency &out,std::string *err=nullptr);
class RmsZoneCache {
public:
    struct Entry {
        std::vector<bool> positive;
        RmsZones zones;
        RmsZoneAdjacency adjacency;
    };
    // One map per cache. Raw terrain changes intentionally do not invalidate entries.
    // Clear at the map/object-stage lifecycle boundary before using a different map.
    bool lookup(uint32_t size,const std::vector<uint8_t> &terrain,
        const std::vector<float> &passability,uint32_t &index,std::string *err=nullptr);
    // Borrowed until the next lookup or clear. Failed lookups leave the cache intact.
    const Entry *entry(uint32_t index) const;
    void clear();
private:
    uint32_t size_=0;
    std::vector<Entry> entries_;
};
}
