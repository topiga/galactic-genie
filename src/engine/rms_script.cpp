// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_script.h"
#include "../core/drs.h"
#include "../core/genie_dat.h"
#include "../core/genie_random.h"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <stdexcept>
#include <unordered_map>

namespace swgb {
namespace {
struct Reader {
    const RmsScript::Resolver &resolve;
    const RmsScript::Limits &limits;
    size_t bytes = 0;
    std::vector<int32_t> active;
    std::vector<RmsToken> output;

    [[noreturn]] void fail(int32_t id, size_t line, const std::string &reason) {
        throw std::runtime_error("RMS " + std::to_string(id) + ":" +
                                 std::to_string(line) + ": " + reason);
    }
    std::vector<RmsToken> lex(int32_t id, const std::vector<uint8_t> &data) {
        std::vector<RmsToken> result;
        size_t line = 1;
        for (size_t i = 0; i < data.size();) {
            unsigned char c = data[i];
            if (std::isspace(c)) { if (c == '\n') ++line; ++i; continue; }
            if (!c) {
                // Archive text may have a trailing C string terminator.
                if (std::all_of(data.begin() + i, data.end(), [](uint8_t b) { return !b; })) break;
                fail(id, line, "embedded NUL");
            }
            if (c == '/' && i + 1 < data.size() && data[i + 1] == '*') {
                size_t start = line;
                i += 2;
                while (i + 1 < data.size() && !(data[i] == '*' && data[i + 1] == '/')) {
                    if (data[i++] == '\n') ++line;
                }
                if (i + 1 >= data.size()) fail(id, start, "unterminated comment");
                i += 2;
                continue;
            }
            size_t start = i;
            if (c == '{' || c == '}') ++i;
            else {
                while (i < data.size() && data[i] && !std::isspace(data[i]) &&
                       data[i] != '{' && data[i] != '}' &&
                       !(data[i] == '/' && i + 1 < data.size() && data[i + 1] == '*')) ++i;
            }
            if (result.size() >= limits.maxTokens) fail(id, line, "token limit exceeded");
            result.push_back({std::string(data.begin() + start, data.begin() + i), id, line});
        }
        return result;
    }
    void read(int32_t id) {
        if (id < 0) fail(id, 1, "negative resource ID");
        if (active.size() >= limits.maxDepth) fail(id, 1, "include depth exceeded");
        if (std::find(active.begin(), active.end(), id) != active.end()) fail(id, 1, "recursive include");
        std::vector<uint8_t> data;
        if (!resolve(id, data)) fail(id, 1, "resource unavailable");
        if (bytes > limits.maxBytes || data.size() > limits.maxBytes - bytes) fail(id, 1, "byte limit exceeded");
        bytes += data.size();
        active.push_back(id);
        auto ts = lex(id, data);
        for (size_t i = 0; i < ts.size(); ++i) {
            const auto &t = ts[i];
            if (t.text == "#include_drs") {
                if (i + 2 >= ts.size() || ts[i + 1].line != t.line || ts[i + 2].line != t.line)
                    fail(id, t.line, "include requires filename and resource ID");
                const auto &name = ts[i + 1].text;
                if (name.empty() || name == "." || name.find("..") != std::string::npos ||
                    !std::all_of(name.begin(), name.end(), [](unsigned char c) {
                        return std::isalnum(c) || c == '_' || c == '-' || c == '.';
                    })) fail(id, t.line, "invalid include filename");
                int32_t child = -1;
                const auto &number = ts[i + 2].text;
                auto parsed = std::from_chars(number.data(), number.data() + number.size(), child);
                if (parsed.ec != std::errc{} || parsed.ptr != number.data() + number.size() || child < 0)
                    fail(id, t.line, "invalid include resource ID");
                read(child);
                i += 2;
            } else {
                if (t.text.front() == '#' && t.text != "#const" && t.text != "#define" && t.text != "#undefine")
                    fail(id, t.line, "unsupported preprocessor directive " + t.text);
                if (output.size() >= limits.maxTokens) fail(id, t.line, "expanded token limit exceeded");
                output.push_back(t);
            }
        }
        active.pop_back();
    }
    void validate() {
        struct Block { std::string kind; bool sawElse = false; };
        std::vector<Block> blocks;
        const std::vector<std::string> stages = {"<PLAYER_SETUP>", "<LAND_GENERATION>",
            "<ELEVATION_GENERATION>", "<CLIFF_GENERATION>", "<TERRAIN_GENERATION>",
            "<CONNECTION_GENERATION>", "<OBJECTS_GENERATION>"};
        for (const auto &t : output) {
            const auto &s = t.text;
            auto bad = [&] { fail(t.resourceId, t.line, "unbalanced control or brace: " + s); };
            if (s == "{" || s == "if" || s == "start_random") blocks.push_back({s});
            else if (s == "}" || s == "endif" || s == "end_random") {
                std::string expected = s == "}" ? "{" : s == "endif" ? "if" : "start_random";
                if (blocks.empty() || blocks.back().kind != expected) bad();
                blocks.pop_back();
            } else if (s == "else" || s == "elseif") {
                if (blocks.empty() || blocks.back().kind != "if" || blocks.back().sawElse) bad();
                if (s == "else") blocks.back().sawElse = true;
            } else if (s == "percent_chance") {
                if (blocks.empty() || blocks.back().kind != "start_random") bad();
            } else if (s.front() == '<') {
                if (!blocks.empty() || std::find(stages.begin(), stages.end(), s) == stages.end())
                    fail(t.resourceId, t.line, "invalid stage " + s);
            }
        }
        if (!blocks.empty()) throw std::runtime_error("RMS: unterminated control or brace");
    }
};
} // namespace

bool RmsScript::load(int32_t id, ResourceSet &resources, std::string *err) {
    return load(id, [&](int32_t resourceId, std::vector<uint8_t> &out) {
        const auto *entry = resources.find(resourceId);
        return entry && entry->type == DrsType::Bina && entry->size <= Limits{}.maxBytes &&
               resources.read(resourceId, out);
    }, Limits{}, err);
}
bool RmsScript::load(int32_t id, const Resolver &resolver, const Limits &limits, std::string *err) {
    tokens_.clear();
    if (err) err->clear();
    try {
        Reader reader{resolver, limits, 0, {}, {}};
        reader.read(id);
        reader.validate();
        tokens_ = std::move(reader.output);
        return true;
    } catch (const std::runtime_error &e) {
        if (err) *err = e.what();
        return false;
    }
}
bool RmsScript::preprocess(const std::unordered_set<std::string> &initialDefines,
                           std::vector<RmsToken> &out, std::string *err, uint32_t *randomState) const {
    out.clear();
    if (err) err->clear();
    std::vector<RmsToken> result;
    auto defines = initialDefines;
    std::unordered_map<std::string, int32_t> constants;
    struct Branch { bool parentActive; bool taken; bool active; };
    std::vector<Branch> branches;
    auto conditionActive = [&] { return branches.empty() || branches.back().active; };
    uint32_t rng = randomState ? *randomState : 0;
    int randomPhase = 0; // 0 outside, 1 searching, 2 selected, 3 finished
    int remaining = 0;
    auto randomActive = [&] { return randomPhase == 0 || randomPhase == 2; };
    auto active = [&] { return conditionActive() && randomActive(); };
    auto fail = [](const RmsToken &token, const std::string &reason) {
        throw std::runtime_error("RMS " + std::to_string(token.resourceId) + ":" +
                                 std::to_string(token.line) + ": " + reason);
    };
    auto symbol = [](const std::string &name) {
        return !name.empty() && (std::isalpha(static_cast<unsigned char>(name[0])) || name[0] == '_') &&
            std::all_of(name.begin(), name.end(), [](unsigned char c) {
                return std::isalnum(c) || c == '_' || c == '-';
            });
    };
    try {
        // shortcut: reject nested random blocks until their original interactions
        // with condition state have dedicated comparison fixtures.
        bool inRandom = false;
        for (const auto &t : tokens_) {
            if (t.text == "start_random") {
                if (inRandom) fail(t, "nested random evaluation unsupported");
                inRandom = true;
            } else if (t.text == "end_random") inRandom = false;
        }
        for (size_t i = 0; i < tokens_.size(); ++i) {
            const auto &t = tokens_[i];
            const auto &s = t.text;
            auto argument = [&]() -> const RmsToken & {
                if (i + 1 >= tokens_.size() || tokens_[i + 1].resourceId != t.resourceId ||
                    tokens_[i + 1].line != t.line)
                    fail(t, "missing argument for " + s);
                return tokens_[++i];
            };
            if (s == "if" || s == "elseif") {
                const auto &name = argument().text;
                if (!symbol(name)) fail(t, "invalid condition symbol");
                if (!randomActive()) continue;
                // Original condition argument lookup tests label existence, not value.
                bool matches = defines.count(name) != 0 || constants.count(name) != 0;
                if (s == "if") {
                    bool parent = active();
                    branches.push_back({parent, matches, parent && matches});
                } else {
                    if (branches.empty()) fail(t, "elseif without if");
                    auto &b = branches.back();
                    b.active = b.parentActive && !b.taken && matches;
                    b.taken = b.taken || matches;
                }
            } else if (s == "else") {
                if (!randomActive()) continue;
                if (branches.empty()) fail(t, "else without if");
                auto &b = branches.back();
                b.active = b.parentActive && !b.taken;
                b.taken = true;
            } else if (s == "endif") {
                if (!randomActive()) continue;
                if (branches.empty()) fail(t, "endif without if");
                branches.pop_back();
            } else if (s == "start_random") {
                if (!conditionActive()) {
                    while (++i < tokens_.size() && tokens_[i].text != "end_random") {}
                    continue;
                }
                if (!randomState) fail(t, "random choice evaluation requires RNG state");
                // Original 0x632bdd and 0x4e24cb: inclusive roll 0..100.
                remaining = static_cast<int>(genieRandomDraw(rng) * 100u / 32767u);
                randomPhase = 1;
            } else if (s == "percent_chance") {
                const auto &number = argument().text;
                int weight = 0;
                auto parsed = std::from_chars(number.data(), number.data() + number.size(), weight);
                if (parsed.ec != std::errc{} || parsed.ptr != number.data() + number.size() ||
                    weight < 0 || weight > 100) fail(t, "percent_chance requires integer 0..100");
                if (!conditionActive() || randomPhase == 0) fail(t, "invalid random branch state");
                if (randomPhase == 1) {
                    if (remaining > weight) remaining -= weight;
                    else randomPhase = 2; // original selects at equality, including zero
                } else randomPhase = 3;
            } else if (s == "end_random") {
                if (!conditionActive()) fail(t, "invalid random end state");
                randomPhase = 0;
            } else if (s == "#const" || s == "#define" || s == "#undefine") {
                const auto &name = argument().text;
                if (!symbol(name)) fail(t, "invalid definition symbol");
                if (s == "#const") {
                    const auto &number = argument().text;
                    int32_t value = 0;
                    auto parsed = std::from_chars(number.data(), number.data() + number.size(), value);
                    if (parsed.ec != std::errc{} || parsed.ptr != number.data() + number.size())
                        fail(t, "only signed 32-bit integer constants supported");
                    if (active()) {
                        if (defines.count(name) || (constants.count(name) && constants.at(name) != value))
                            fail(t, "conflicting definition unsupported: " + name);
                        constants[name] = value;
                    }
                } else if (active()) {
                    if (s == "#define" && constants.count(name)) fail(t, "constant redefinition unsupported: " + name);
                    if (s == "#define") defines.insert(name);
                    // Original opcode1 (#undefine) returns without deleting a label.
                    // Preserve that observed behavior instead of assuming C macros.
                }
            } else if (active()) {
                auto token = t;
                auto constant = constants.find(s);
                if (constant != constants.end()) token.text = std::to_string(constant->second);
                result.push_back(std::move(token));
            }
        }
        if (!branches.empty()) throw std::runtime_error("RMS: unterminated condition");
        out = std::move(result);
        if (randomState) *randomState = rng;
        return true;
    } catch (const std::runtime_error &e) {
        if (err) *err = e.what();
        return false;
    }
}
bool generateRmsBaseTerrain(const std::vector<RmsToken> &directives,
                            uint32_t width, uint32_t height,
                            const dat::TerrainBlock &terrainData,
                            std::vector<uint8_t> &out, std::string *err) {
    out.clear();
    if (err) err->clear();
    auto fail = [&](const std::string &reason) {
        if (err) *err = "RMS base terrain: " + reason;
        return false;
    };
    // Explicit implementation bounds; not a claim about original map-size limits.
    if (!width || !height || width > 1024 || height > 1024)
        return fail("dimensions must be 1..1024");
    bool landStage = false, hasBase = false;
    uint8_t terrain = 0;
    for (size_t i = 0; i < directives.size(); ++i) {
        const auto &token = directives[i];
        if (token.text == "<LAND_GENERATION>") landStage = true;
        else if (token.text == "base_terrain" && landStage) {
            if (++i == directives.size()) return fail("missing base_terrain argument");
            const auto &value = directives[i].text;
            int32_t parsed = 0;
            auto result = std::from_chars(value.data(), value.data() + value.size(), parsed);
            if (result.ec != std::errc{} || result.ptr != value.data() + value.size())
                return fail("base_terrain requires a resolved integer constant");
            // Original 0x4e2d26 retains only the low byte; last assignment wins.
            terrain = static_cast<uint8_t>(parsed);
            hasBase = true;
        } else {
            return fail("unsupported directive " + token.text + " at " +
                        std::to_string(token.resourceId) + ":" + std::to_string(token.line));
        }
    }
    if (!landStage || !hasBase) return fail("explicit LAND_GENERATION/base_terrain required");
    const auto &terrains = terrainData.terrains;
    if (terrainData.terrainsUsed <= 0 || terrain >= terrainData.terrainsUsed || terrain >= terrains.size())
        return fail("terrain ID outside the loaded terrain table");
    if (!terrains[terrain].enabled) {
        const int draw = terrains[terrain].terrainToDraw;
        if (draw < 0 || static_cast<size_t>(draw) >= terrains.size() || terrains[draw].enabled != 1)
            return fail("terrain and its draw alias are disabled");
    }
    // 0x4da380 paints the inclusive full-map rectangle through 0x495f80.
    // A disabled source with an enabled draw alias still stores the source ID.
    out.assign(static_cast<size_t>(width) * height, terrain);
    return true;
}
bool generateRmsLandSeedTerrain(const std::vector<RmsToken> &directives,
                                uint32_t width, uint32_t height,
                                const dat::TerrainBlock &terrainData,
                                std::vector<uint8_t> &out, std::string *err) {
    out.clear();
    if (err) err->clear();
    auto fail = [&](const std::string &reason) {
        if (err) *err = "RMS land seeds: " + reason;
        return false;
    };
    if (!width || !height || width > 1024 || height > 1024)
        return fail("dimensions must be 1..1024");
    struct Seed { int32_t terrain = 0, x = 0, y = 0, radius = 0; };
    std::vector<Seed> seeds;
    std::vector<RmsToken> base;
    bool landStage = false;
    auto number = [&](size_t &i, int32_t &value) {
        if (++i >= directives.size()) return false;
        const auto &text = directives[i].text;
        auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
        return parsed.ec == std::errc{} && parsed.ptr == text.data() + text.size();
    };
    for (size_t i = 0; i < directives.size(); ++i) {
        const auto &token = directives[i];
        if (token.text == "<LAND_GENERATION>") {
            landStage = true;
            base.push_back(token);
        } else if (landStage && token.text == "base_terrain") {
            base.push_back(token);
            if (++i == directives.size()) return fail("missing base_terrain value");
            base.push_back(directives[i]);
        } else if (landStage && token.text == "create_land") {
            if (seeds.size() >= 64) return fail("at most64 seed lands supported");
            if (++i == directives.size() || directives[i].text != "{") return fail("create_land requires a block");
            Seed seed;
            bool hasTerrain = false, hasPosition = false, hasRadius = false, hasBudget = false;
            for (++i; i < directives.size() && directives[i].text != "}"; ++i) {
                const auto &name = directives[i].text;
                if (name == "terrain_type") {
                    if (!number(i, seed.terrain)) return fail("terrain_type requires a resolved integer");
                    hasTerrain = true;
                } else if (name == "land_position") {
                    if (!number(i, seed.x) || !number(i, seed.y) || seed.x < 0 || seed.y < 0 || seed.x > 99 || seed.y > 99)
                        return fail("land_position must be two percentages0..99");
                    hasPosition = true;
                } else if (name == "base_size") {
                    if (!number(i, seed.radius) || seed.radius < 0 || seed.radius > 1024)
                        return fail("base_size must be0..1024");
                    hasRadius = true;
                } else if (name == "number_of_tiles") {
                    int32_t budget = -1;
                    if (!number(i, budget) || budget != 0)
                        return fail("land growth unsupported: require number_of_tiles0");
                    hasBudget = true;
                } else return fail("unsupported land parameter " + name);
            }
            if (i == directives.size()) return fail("unterminated create_land block");
            if (!hasTerrain || !hasPosition || !hasRadius || !hasBudget)
                return fail("seed land requires explicit terrain, position, radius and zero growth budget");
            seeds.push_back(seed);
        } else return fail("unsupported directive " + token.text);
    }
    std::vector<uint8_t> raster;
    if (!generateRmsBaseTerrain(base, width, height, terrainData, raster, err)) return false;
    for (const auto &seed : seeds) {
        // Reuse the exact terrain-byte/metadata validation of the base painter.
        std::vector<RmsToken> color = {{"<LAND_GENERATION>", 0, 0}, {"base_terrain", 0, 0},
                                      {std::to_string(seed.terrain), 0, 0}};
        std::vector<uint8_t> pixel;
        if (!generateRmsBaseTerrain(color, 1, 1, terrainData, pixel, err)) return false;
        // Original handler0x4e2c40 converts percent positions by integer truncation.
        const int cx = static_cast<int>(width) * seed.x / 100;
        const int cy = static_cast<int>(height) * seed.y / 100;
        const int left = std::max(0, cx - seed.radius), top = std::max(0, cy - seed.radius);
        const int right = std::min(static_cast<int>(width) - 1, cx + seed.radius);
        const int bottom = std::min(static_cast<int>(height) - 1, cy + seed.radius);
        // 0x4da742..0x4da7b1 paints all seed rectangles before testing growth budgets.
        for (int y = top; y <= bottom; ++y)
            std::fill(raster.begin() + static_cast<size_t>(y) * width + left,
                      raster.begin() + static_cast<size_t>(y) * width + right + 1, pixel[0]);
    }
    out = std::move(raster);
    return true;
}
bool smoothRmsTerrain(uint32_t width, uint32_t height, uint8_t baseTerrain,
                      const std::vector<uint8_t> &input, std::vector<uint8_t> &out,
                      std::string *err, size_t maxTileVisits) {
    if (err) err->clear();
    auto fail = [&](const char *reason) {
        out.clear();
        if (err) *err = std::string("RMS terrain smoothing: ") + reason;
        return false;
    };
    if (!width || !height || width > 1024 || height > 1024)
        return fail("dimensions must be 1..1024");
    if (input.size() != static_cast<size_t>(width) * height)
        return fail("terrain raster size does not match dimensions");
    auto raster = input;
    // Original 0x492ab0: two row-major, in-place passes, repeated until neither
    // changes a tile. Outside-map neighbors never match the base terrain.
    constexpr int dx[] = {0, 1, 1, 1, 0, -1, -1, -1};
    constexpr int dy[] = {-1, -1, 0, 1, 1, 1, 0, -1};
    bool changed;
    do {
        changed = false;
        for (int phase = 0; phase < 2; ++phase) {
            if (maxTileVisits < raster.size()) return fail("tile-visit limit exceeded");
            maxTileVisits -= raster.size();
            for (int y = 0; y < static_cast<int>(height); ++y) {
                for (int x = 0; x < static_cast<int>(width); ++x) {
                    auto &tile = raster[static_cast<size_t>(y) * width + x];
                    if (tile == baseTerrain) continue;
                    bool neighbors[8];
                    for (int i = 0; i < 8; ++i) {
                        const int nx = x + dx[i], ny = y + dy[i];
                        neighbors[i] = nx >= 0 && ny >= 0 && nx < static_cast<int>(width) &&
                            ny < static_cast<int>(height) &&
                            raster[static_cast<size_t>(ny) * width + nx] == baseTerrain;
                    }
                    bool replace = (neighbors[0] && neighbors[4]) || (neighbors[2] && neighbors[6]);
                    if (phase == 1) {
                        replace = false;
                        // The second instruction block tests each diagonal and
                        // the same five surrounding gap patterns, rotated 90°.
                        for (int corner = 1; corner < 8 && !replace; corner += 2) {
                            auto n = [&](int offset) { return neighbors[(corner + offset) % 8]; };
                            replace = n(0) && ((n(2) && !n(1)) || (n(3) && !n(2)) ||
                                (n(6) && !n(7)) || (n(5) && !n(6)) || (n(4) && !n(5) && !n(3)));
                        }
                    }
                    if (replace) { tile = baseTerrain; changed = true; }
                }
            }
        }
    } while (changed);
    out = std::move(raster);
    return true;
}
bool growRmsSingleLand(uint32_t width, uint32_t height, uint8_t baseTerrain,
                       const RmsLandGrowth &land, const dat::TerrainBlock &terrainData,
                       uint32_t &randomState, std::vector<uint8_t> &out,
                       std::string *err, size_t maxWork) {
    return growRmsLands(width, height, baseTerrain, {land}, terrainData, randomState, out, err, maxWork);
}
bool growRmsLands(uint32_t width, uint32_t height, uint8_t baseTerrain,
                       const std::vector<RmsLandGrowth> &lands, const dat::TerrainBlock &terrainData,
                       uint32_t &randomState, std::vector<uint8_t> &out,
                       std::string *err, size_t maxWork) {
    out.clear();
    if (err) err->clear();
    auto fail = [&](const char *reason) {
        if (err) *err = std::string("RMS land growth: ") + reason;
        return false;
    };
    if (!width || !height || width > 1024 || height > 1024)
        return fail("dimensions must be 1..1024");
    if (lands.empty() || lands.size() > 64) return fail("require 1..64 lands");
    // This coefficient bound keeps every computed priority exactly representable
    // in the original single-precision node field (adjacency is at most 25).
    for (const auto &land : lands) if (land.centerX >= width || land.centerY >= height || land.radius > 1024 ||
        land.targetTiles > static_cast<size_t>(width) * height || land.clumpingFactor > 100 || land.zone < -1 || land.zone > 63)
        return fail("invalid center, radius, target count, clumpingFactor (0..100) or zone (-1..63)");
    auto paint = [&](uint8_t id, uint32_t w, uint32_t h, std::vector<uint8_t> &raster) {
        return generateRmsBaseTerrain({{"<LAND_GENERATION>", 0, 0}, {"base_terrain", 0, 0},
                                      {std::to_string(id), 0, 0}}, w, h, terrainData, raster, err);
    };
    std::vector<uint8_t> raster, pixel;
    if (!paint(baseTerrain, width, height, raster)) return false;
    for (const auto &land : lands) if (!paint(land.terrain, 1, 1, pixel)) return false;
    std::vector<uint8_t> zones(raster.size(), 254);
    struct Node { int next = -1, previous = -1, priority = 0; int owner = -1; };
    std::vector<Node> nodes(raster.size());
    std::vector<int> heads(lands.size(), -1);
    std::vector<uint32_t> areas(64, 0);
    auto zoneFor = [&](int owner) { return lands[owner].zone < 0 ? owner : lands[owner].zone; };
    // Original 0x4d69a0 uses one intrusive node per tile across ALL lands.
    // Reinsertion transfers the existing node out of any prior land queue; ascending priorities insert BEFORE equal priorities.
    auto remove = [&](int id) {
        auto &n = nodes[id];
        if (n.owner < 0) return;
        if (n.previous < 0) heads[n.owner] = n.next; else nodes[n.previous].next = n.next;
        if (n.next >= 0) nodes[n.next].previous = n.previous;
        n.next = n.previous = -1;
        n.owner = -1;
    };
    auto enqueue = [&](int owner, int x, int y, int priority) {
        if (!maxWork) return false;
        --maxWork;
        const int id = y * static_cast<int>(width) + x;
        remove(id);
        int previous = -1, next = heads[owner];
        while (next >= 0 && nodes[next].priority < priority) {
            if (!maxWork) return false;
            --maxWork;
            previous = next;
            next = nodes[next].next;
        }
        nodes[id] = {next, previous, priority, owner};
        if (previous < 0) heads[owner] = id; else nodes[previous].next = id;
        if (next >= 0) nodes[next].previous = id;
        return true;
    };
    const int w = static_cast<int>(width), h = static_cast<int>(height);
    for (int owner = 0; owner < static_cast<int>(lands.size()); ++owner) {
        const auto &land = lands[owner];
        const int radius = static_cast<int>(land.radius);
        const int left = std::max(0, static_cast<int>(land.centerX) - radius);
        const int right = std::min(w - 1, static_cast<int>(land.centerX) + radius);
        const int top = std::max(0, static_cast<int>(land.centerY) - radius);
        const int bottom = std::min(h - 1, static_cast<int>(land.centerY) + radius);
        const auto seedArea = static_cast<uint32_t>((right - left + 1) * (bottom - top + 1));
        if (maxWork < seedArea) return fail("work limit exceeded");
        maxWork -= seedArea;
        // Original 0x4da75a resets by land ordinal, but 0x4da7e2 stores the
        // seed area by zone byte. Growth reads by ordinal again. Do not normalize
        // zones: shared/nonordinal IDs change budgets, including later overwrites.
        areas[owner] = 0;
        areas[zoneFor(owner)] = seedArea;
        for (int y = top; y <= bottom; ++y) for (int x = left; x <= right; ++x) {
            raster[y * w + x] = land.terrain;
            zones[y * w + x] = static_cast<uint8_t>(zoneFor(owner));
        }
        if (left > 0) for (int y = top; y <= bottom; ++y)
            if (!enqueue(owner, left - 1, y, 0)) return fail("work limit exceeded");
        if (top > 0) for (int x = left; x <= right; ++x)
            if (!enqueue(owner, x, top - 1, 0)) return fail("work limit exceeded");
        if (right + 1 < w) for (int y = top; y <= bottom; ++y)
            if (!enqueue(owner, right + 1, y, 0)) return fail("work limit exceeded");
        if (bottom + 1 < h) for (int x = left; x <= right; ++x)
            if (!enqueue(owner, x, bottom + 1, 0)) return fail("work limit exceeded");
    }
    uint32_t rng = randomState;
    auto roll = [&]() { return static_cast<int>(genieRandomDraw(rng) * 100u / 32767u); };
    bool active;
    do {
        active = false;
        for (int owner = 0; owner < static_cast<int>(lands.size()); ++owner) {
            const auto &land = lands[owner];
            if (areas[owner] >= land.targetTiles || heads[owner] < 0) continue;
            active = true;
            if (maxWork < 26) return fail("work limit exceeded");
            maxWork -= 26;
            const int id = heads[owner], x = id % w, y = id / w;
            remove(id);
            // Even a zero edge-rejection gate consumes its comparison draw.
            roll();
            if (zones[id] != 254) continue;
            int adjacency = 0;
            // 0x4da400 with separation zero: count owned cells in clipped 5x5.
            // Out-of-map neighbors contribute nothing for this supported subset.
            for (int ny = std::max(0, y - 2); ny <= std::min(h - 1, y + 2); ++ny)
                for (int nx = std::max(0, x - 2); nx <= std::min(w - 1, x + 2); ++nx)
                    adjacency += zones[ny * w + nx] == zoneFor(owner);
            if (!adjacency) continue;
            zones[id] = static_cast<uint8_t>(zoneFor(owner));
            raster[id] = land.terrain;
            ++areas[owner];
            constexpr int dx[] = {-1, 1, 0, 0}, dy[] = {0, 0, -1, 1};
            for (int i = 0; i < 4; ++i) {
                const int nx = x + dx[i], ny = y + dy[i];
                if (nx < 0 || ny < 0 || nx >= w || ny >= h || zones[ny * w + nx] != 254) continue;
                const int priority = 250 + roll() - static_cast<int>(land.clumpingFactor) * adjacency;
                if (!enqueue(owner, nx, ny, priority)) return fail("work limit exceeded");
            }
        }
    } while (active);
    // Original 0x4dac62 drains remaining frontier and paints opposite-zone gaps
    // without changing zone membership or incrementing the land's tile budget.
    for (int owner = 0; owner < static_cast<int>(lands.size()); ++owner) {
        while (heads[owner] >= 0) {
            if (!maxWork) return fail("work limit exceeded");
            --maxWork;
            const int id = heads[owner], x = id % w, y = id / w;
            remove(id);
            if ((x > 0 && x + 1 < w && zones[id - 1] == zoneFor(owner) && zones[id + 1] == zoneFor(owner)) ||
                (y > 0 && y + 1 < h && zones[id - w] == zoneFor(owner) && zones[id + w] == zoneFor(owner)))
                raster[id] = lands[owner].terrain;
        }
    }
    randomState = rng;
    out = std::move(raster);
    return true;
}
} // namespace swgb
