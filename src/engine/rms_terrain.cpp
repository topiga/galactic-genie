// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_terrain.h"
#include "../core/genie_random.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace swgb {
bool generateRmsTerrain(uint32_t width, uint32_t height,
    const std::vector<RmsTerrainRow> &rows, uint32_t &randomState,
    std::vector<uint8_t> &terrain, std::string *err,
    const std::vector<RmsTerrainProtection> &protection) {
    if (err) err->clear();
    auto fail = [&](const char *message) {
        if (err) *err = std::string("RMS terrain: ") + message;
        return false;
    };
    if (!width || !height || width > 1024 || height > 1024 ||
        terrain.size() != static_cast<size_t>(width) * height || rows.size() > 99)
        return fail("unsupported dimensions, terrain size or row count");
    for (const auto &row : rows)
        if (row.terrain == row.baseTerrain || row.clumps > 999 ||
            (terrain.size() == 1 && row.clumps) ||
            row.spacing > 1024 || row.clumping > 10000 ||
            row.targetTiles > static_cast<uint32_t>(std::numeric_limits<int32_t>::max()))
            return fail("unsupported terrain row");
    if (protection.size() > 100) return fail("unsupported protection count");
    for (const auto &area : protection)
        if (area.x < 0 || area.y < 0 || static_cast<uint32_t>(area.x) >= width ||
            static_cast<uint32_t>(area.y) >= height || area.radius > 2048 || area.weight > 100)
            return fail("unsupported player-start protection");
    auto result = terrain;
    uint32_t rng = randomState;
    uint64_t work = 0;
    constexpr uint64_t workLimit = 64000000;
    const int w = static_cast<int>(width), h = static_cast<int>(height);
    std::vector<uint8_t> mask(result.size(),0);
    if (std::any_of(rows.begin(), rows.end(), [](const auto &row) { return row.avoidPlayerStarts; }))
        for (int y=0;y<h;++y)
            for (int x=0;x<w;++x) {
                uint32_t score=0;
                for (const auto &area : protection) {
                    if (++work > workLimit) return fail("work limit exceeded");
                    const int dx=x-area.x, dy=y-area.y;
                    const auto distance=static_cast<uint32_t>(std::sqrt(static_cast<double>(dx*dx+dy*dy)));
                    if (distance<area.radius) score+=(area.radius-distance)*area.weight;
                }
                mask[y*w+x]=static_cast<uint8_t>(score>100 ? 101 : score);
            }
    struct Node { int next = -1, previous = -1, owner = -1, priority = 0; };
    std::vector<Node> nodes(result.size());
    for (const auto &row : rows) {
        std::fill(nodes.begin(), nodes.end(), Node{});
        std::vector<int> heads(row.clumps + 1, -1);
        heads[0] = 0;
        for (size_t i = 0; i < nodes.size(); ++i)
            nodes[i] = {i + 1 < nodes.size() ? static_cast<int>(i + 1) : -1,
                        i ? static_cast<int>(i - 1) : -1, 0, 0};
        auto remove = [&](int i) {
            auto &n = nodes[i];
            if (n.owner < 0) return;
            if (n.previous >= 0) nodes[n.previous].next = n.next;
            else heads[n.owner] = n.next;
            if (n.next >= 0) nodes[n.next].previous = n.previous;
            n.next = n.previous = n.owner = -1;
        };
        auto enqueue = [&](int owner, int i, int priority) {
            remove(i);
            int previous = -1, next = heads[owner];
            while (next >= 0 && nodes[next].priority < priority) {
                if (++work > workLimit) return false;
                previous = next; next = nodes[next].next;
            }
            nodes[i] = {next, previous, owner, priority};
            if (previous >= 0) nodes[previous].next = i;
            else heads[owner] = i;
            if (next >= 0) nodes[next].previous = i;
            return ++work <= workLimit;
        };
        auto gate = [&](int x, int y) {
            const int spacing = static_cast<int>(row.spacing);
            if (spacing) {
                for (int yy = std::max(0, y-spacing); yy <= std::min(h-1, y+spacing); ++yy)
                    for (int xx = std::max(0, x-spacing); xx <= std::min(w-1, x+spacing); ++xx) {
                        if (++work > workLimit) return 0;
                        const auto t = result[yy*w+xx];
                        if (t != row.baseTerrain && t != row.terrain) return 0;
                    }
            }
            int score = 1;
            for (int yy = std::max(0, y-2); yy <= std::min(h-1, y+2); ++yy)
                for (int xx = std::max(0, x-2); xx <= std::min(w-1, x+2); ++xx) {
                    if (++work > workLimit) return 0;
                    score += result[yy*w+xx] == row.terrain;
                }
            return score;
        };
        // Original partial shuffle moves sampled cells to the front of the raster list.
        for (size_t i = 0; i < result.size()/8; ++i) {
            const int x = static_cast<int>(genieRandomDraw(rng)*(width-1)/32767);
            const int y = static_cast<int>(genieRandomDraw(rng)*(height-1)/32767);
            const int index = y*w+x;
            remove(index);
            const int next = heads[0];
            nodes[index] = {next, -1, 0, 0}; heads[0] = index;
            if (next >= 0) nodes[next].previous = index;
        }
        if (!row.clumps) continue;
        const int radius = std::max(2, 2*static_cast<int>(
            std::sqrt(static_cast<double>(row.targetTiles)/row.clumps)));
        uint32_t created = 0, count = 0;
        while (created < row.clumps && heads[0] >= 0) {
            if (++work > workLimit) return fail("work limit exceeded");
            const int i = heads[0], x = i%w, y = i/w;
            remove(i);
            if (result[i] != row.baseTerrain || !gate(x,y) ||
                (row.avoidPlayerStarts && mask[i])) continue;
            for (int yy = std::max(0,y-radius); yy <= std::min(h-1,y+radius); ++yy)
                for (int xx = std::max(0,x-radius); xx <= std::min(w-1,x+radius); ++xx) {
                    if (++work > workLimit) return fail("work limit exceeded");
                    remove(yy*w+xx);
                }
            result[i] = row.terrain; ++created; ++count;
            const int xs[] = {x-1,x,x+1,x}, ys[] = {y,y-1,y,y+1};
            for (int k=0;k<4;++k)
                if (xs[k]>=0 && xs[k]<w && ys[k]>=0 && ys[k]<h &&
                    !enqueue(static_cast<int>(created),ys[k]*w+xs[k],0))
                    return fail("work limit exceeded");
        }
        bool popped;
        do {
            popped = false;
            for (uint32_t owner=1;owner<=created;++owner) {
                if (count >= row.targetTiles || heads[owner] < 0) continue;
                if (++work > workLimit) return fail("work limit exceeded");
                const int i = heads[owner], x = i%w, y = i/w;
                remove(i); popped = true;
                if (row.avoidPlayerStarts && mask[i] > genieRandomDraw(rng)*100/32767) continue;
                const int score = gate(x,y);
                if (result[i] != row.baseTerrain || !score) continue;
                const int priority = 250-static_cast<int>(row.clumping)*score+
                    (row.avoidPlayerStarts ? mask[i] : 0);
                result[i] = row.terrain; ++count;
                const int xs[] = {x-1,x+1,x,x}, ys[] = {y,y,y-1,y+1};
                for (int k=0;k<4;++k)
                    if (xs[k]>=0 && xs[k]<w && ys[k]>=0 && ys[k]<h &&
                        result[ys[k]*w+xs[k]] == row.baseTerrain &&
                        !enqueue(static_cast<int>(owner),ys[k]*w+xs[k],
                            priority+static_cast<int>(genieRandomDraw(rng)*100/32767)))
                        return fail("work limit exceeded");
            }
        } while (popped);
        for (uint32_t owner=1;owner<=row.clumps;++owner)
            while (heads[owner]>=0) {
                if (++work > workLimit) return fail("work limit exceeded");
                const int i=heads[owner], x=i%w;
                remove(i);
                // The original vertical fill branch has an unreachable bottom-bound test.
                if (x>0 && x<w-1 && result[i-1]==row.terrain && result[i+1]==row.terrain)
                    result[i]=row.terrain;
            }
        if (work > workLimit) return fail("work limit exceeded");
    }
    terrain.swap(result); randomState = rng;
    return true;
}
} // namespace swgb
