// SPDX-License-Identifier: GPL-3.0-or-later
// Standalone: c++ -std=c++17 -Isrc tools/rms_test.cpp src/engine/rms_script.cpp src/core/drs.cpp -o /tmp/rms_test
#include "engine/rms_script.h"
#include "core/drs.h"
#include "core/genie_dat.h"
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <unordered_map>

static void require(bool ok, const std::string &message) {
    if (!ok) { std::cerr << message << '\n'; std::exit(1); }
}
int main(int argc, char **argv) {
    using namespace swgb;
    std::unordered_map<int32_t, std::string> files;
    auto resolve = [&](int32_t id, std::vector<uint8_t> &out) {
        auto it = files.find(id);
        if (it == files.end()) return false;
        out.assign(it->second.begin(), it->second.end());
        return true;
    };
    RmsScript script;
    RmsScript::Limits limits;
    std::string err;
    files[1] = "#include_drs constants.def 2\n<LAND_GENERATION> /* comment */\ncreate_land { if HOME terrain_type GRASS else terrain_type WATER endif }";
    files[2] = "#const GRASS 0\nstart_random percent_chance 100 #define HOME end_random";
    require(script.load(1, resolve, limits, &err), err);
    require(script.tokens().front().text == "#const" && script.tokens().front().resourceId == 2,
            "include expansion loses source or constants");
    bool retained = false;
    for (const auto &t : script.tokens()) {
        if (t.text == "<LAND_GENERATION>") require(t.line == 2 && t.resourceId == 1, "stage source");
        if (t.text == "if") retained = true;
    }
    require(retained, "conditions must remain unresolved");
    auto reject = [&](const std::string &source, const std::string &reason) {
        files[1] = source;
        require(!script.load(1, resolve, limits, &err), "accepted malformed source: " + source);
        require(err.find(reason) != std::string::npos, "unexpected diagnostic: " + err);
        require(script.tokens().empty(), "failed load retained partial program");
    };
    reject("#include_drs ../escape.def 2", "filename");
    reject("#include_drs ok.def -2", "resource ID");
    reject("#include_drs ok.def 999", "unavailable");
    reject("#include_drs self.def 1", "recursive");
    reject("#include_drs missing.def", "requires");
    reject("#include local.rms", "unsupported");
    reject("/* never closed", "unterminated");
    reject("create_land {", "unterminated");
    reject("if HOME else elseif AWAY endif", "unbalanced");
    reject("start_random endif", "unbalanced");
    reject("<FAKE_GENERATION>", "stage");
    reject(std::string("abc\0def", 7), "NUL");
    files[1] = "#include_drs constants.def 2";
    limits.maxDepth = 1;
    require(!script.load(1, resolve, limits, &err) && err.find("depth") != std::string::npos, "depth bound");
    limits = {};
    limits.maxBytes = 4;
    require(!script.load(1, resolve, limits, &err) && err.find("byte") != std::string::npos, "byte bound");
    limits = {};
    files[1] = "#include_drs constants.def 2\n#include_drs constants.def 2";
    limits.maxTokens = 15;
    require(!script.load(1, resolve, limits, &err) && err.find("token") != std::string::npos, "expanded token bound");
    limits = {};
    std::vector<RmsToken> processed;
    auto preprocess = [&](const std::string &source, const std::unordered_set<std::string> &defines) {
        files[1] = source;
        require(script.load(1, resolve, limits, &err), err);
        return script.preprocess(defines, processed, &err);
    };
    auto text = [&] {
        std::string joined;
        for (const auto &t : processed) joined += t.text + " ";
        return joined;
    };
    require(preprocess("#const GRASS 0\n#define HOME\n"
        "<LAND_GENERATION> create_land { if HOME terrain_type GRASS "
        "elseif OTHER terrain_type 2 else terrain_type 3 endif }", {}), err);
    require(text() == "<LAND_GENERATION> create_land { terrain_type 0 } ", "first matching branch and constant");
    require(processed[4].resourceId == 1 && processed[4].line == 3, "constant use source location");
    require(preprocess("if MISSING #define LEAK if PRESENT wrong else wrong endif "
        "elseif PRESENT #define PICK else wrong endif "
        "if LEAK wrong elseif PICK selected else wrong endif", {"PRESENT"}), err);
    require(text() == "selected ", "nested inactive condition or elseif side effects");
    require(preprocess("#define HOME #undefine HOME if HOME wrong else fallback endif", {}), err);
    require(text() == "wrong ", "original undefine is a no-op");
    require(preprocess("if MISSING start_random percent_chance 100 #define LEAK end_random endif "
        "if LEAK wrong else safe endif", {}), err);
    require(text() == "safe ", "inactive random leaked definitions");
    require(preprocess("#const NEG -7 amount NEG", {}), err);
    require(text() == "amount -7 ", "negative integer constant");
    auto rejectPreprocess = [&](const std::string &source, const std::string &reason) {
        require(!preprocess(source, {}), "accepted unsupported preprocessing: " + source);
        require(err.find(reason) != std::string::npos, "unexpected preprocess error: " + err);
        require(processed.empty(), "failed preprocessing retained partial output");
    };
    rejectPreprocess("prefix start_random percent_chance 100 selected end_random", "random");
    rejectPreprocess("#const BAD 2147483648", "32-bit");
    rejectPreprocess("#const BAD OTHER", "integer");
    rejectPreprocess("#const BAD", "missing");
    rejectPreprocess("#define 12", "symbol");
    rejectPreprocess("if\nendif", "missing");
    require(preprocess("#const X 0 if X selected endif", {}), err);
    require(text() == "selected ", "zero constant condition tests existence");
    rejectPreprocess("#const X 1 #const X 2", "conflicting");
    files[2] = "#const GRASS 0\n#define HOME";
    require(preprocess("#include_drs constants.def 2\nif HOME terrain_type GRASS endif", {}), err);
    require(text() == "terrain_type 0 " && processed[1].resourceId == 1, "included definitions");
    require(script.preprocess({}, processed, &err) && text() == "terrain_type 0 ", "repeat preprocessing changed source");
    files[1] = "start_random percent_chance 10 first percent_chance 90 second end_random";
    require(script.load(1, resolve, limits, &err), err);
    uint32_t rng = 992;
    require(script.preprocess({}, processed, &err, &rng), err);
    require(text() == "first " && rng == 0x0cce1323u, "inclusive equality branch and CRT state");
    rng = 1093;
    require(script.preprocess({}, processed, &err, &rng), err);
    require(text() == "second " && rng == 0x0e17e5f4u, "subtract chance before next branch");
    rng = 30091;
    require(script.preprocess({}, processed, &err, &rng), err);
    require(text() == "second " && rng == 0x7fff2a22u, "inclusive100 roll");
    files[1] = "start_random percent_chance 0 zero percent_chance 100 other end_random";
    require(script.load(1, resolve, limits, &err), err);
    rng = 0;
    require(script.preprocess({}, processed, &err, &rng), err);
    require(text() == "zero " && rng == 2531011u, "original zero-weight branch can win at roll zero");
    files[1] = "start_random percent_chance 10 first percent_chance 10 second end_random after";
    require(script.load(1, resolve, limits, &err), err);
    rng = 30091;
    require(script.preprocess({}, processed, &err, &rng), err);
    require(text() == "after ", "underweight random can select nothing");
    files[1] = "if MISSING start_random percent_chance 100 bad end_random endif "
        "start_random percent_chance 100 #define HOME end_random if HOME good endif";
    require(script.load(1, resolve, limits, &err), err);
    rng = 0;
    require(script.preprocess({}, processed, &err, &rng), err);
    require(text() == "good " && rng == 2531011u, "inactive condition consumes no RNG");
    files[1] = "start_random percent_chance 0 if MISSING bad else bad endif "
        "percent_chance 100 if HOME selected else fallback endif end_random";
    require(script.load(1, resolve, limits, &err), err);
    rng = 1093;
    require(script.preprocess({"HOME"}, processed, &err, &rng), err);
    require(text() == "selected ", "unselected random conditions cannot affect selected branch");
    files[1] = "start_random percent_chance 101 bad end_random";
    require(script.load(1, resolve, limits, &err), err);
    rng = 123;
    require(!script.preprocess({}, processed, &err, &rng) && rng == 123 && processed.empty(),
            "failed random preprocessing must preserve caller RNG and clear output");
    rejectPreprocess("start_random percent_chance 100 start_random percent_chance 100 x end_random end_random",
                     "nested random");
    dat::TerrainBlock terrainData{};
    terrainData.terrains.resize(4);
    terrainData.terrainsUsed = 4;
    terrainData.terrains[0].enabled = 1;
    terrainData.terrains[1].enabled = 1;
    terrainData.terrains[2].terrainToDraw = 1;
    std::vector<uint8_t> terrain;
    auto baseMap = [&](const std::string &source, uint32_t width, uint32_t height) {
        require(preprocess(source, {}), err);
        return generateRmsBaseTerrain(processed, width, height, terrainData, terrain, &err);
    };
    require(baseMap("#const WATER 1 <LAND_GENERATION> base_terrain WATER", 3, 5), err);
    require(terrain.size() == 15 && std::all_of(terrain.begin(), terrain.end(), [](uint8_t t) { return t == 1; }),
            "rectangular base fill misses edges");
    require(baseMap("<LAND_GENERATION> base_terrain 0 base_terrain 258", 1, 1), err);
    require(terrain == std::vector<uint8_t>{2}, "last low-byte assignment and draw-alias source ID");
    require(baseMap("<LAND_GENERATION> base_terrain 0", 1024, 1024), err);
    require(terrain.size() == 1024u * 1024u && terrain.back() == 0, "maximum bounded fill");
    require(!baseMap("<LAND_GENERATION> base_terrain 3", 3, 5) && terrain.empty(), "disabled terrain rejected");
    require(!baseMap("<LAND_GENERATION> base_terrain 4", 3, 5) && terrain.empty(), "out of table terrain rejected");
    require(!baseMap("<LAND_GENERATION> base_terrain -1", 3, 5), "wrapped invalid terrain rejected");
    require(!baseMap("<LAND_GENERATION> base_terrain 0", 0, 5), "zero size rejected");
    require(!baseMap("<LAND_GENERATION> base_terrain 0", 1025, 1), "oversized map rejected");
    require(!baseMap("<LAND_GENERATION> base_terrain 0 create_land { terrain_type 1 }", 3, 5) && terrain.empty(),
            "unsupported land placement cannot silently become flat terrain");
    require(!baseMap("<LAND_GENERATION> base_terrain 0 <OBJECTS_GENERATION> create_object 83", 3, 5),
            "unsupported object stage rejected");
    require(!baseMap("<LAND_GENERATION>", 3, 5), "unknown default terrain rejected");
    require(!baseMap("base_terrain 0", 3, 5), "base terrain outside stage rejected");
    auto landSeeds = [&](const std::string &source, uint32_t width, uint32_t height) {
        require(preprocess(source, {}), err);
        return generateRmsLandSeedTerrain(processed, width, height, terrainData, terrain, &err);
    };
    const std::string landPrefix = "<LAND_GENERATION> base_terrain 0 ";
    require(landSeeds(landPrefix + "create_land { terrain_type 1 land_position 50 50 base_size 1 number_of_tiles 0 }", 7, 5), err);
    require(terrain.size() == 35, "land-seed raster dimensions");
    for (int y = 0; y < 5; ++y) for (int x = 0; x < 7; ++x)
        require(terrain[y * 7 + x] == (x >= 2 && x <= 4 && y >= 1 && y <= 3 ? 1 : 0),
                "percent center truncation or inclusive seed radius");
    require(landSeeds(landPrefix + "create_land { terrain_type 1 land_position 0 0 base_size 1 number_of_tiles 0 }", 7, 5), err);
    require(std::count(terrain.begin(), terrain.end(), 1) == 4 && terrain[0] == 1 && terrain[8] == 1,
            "corner seed clipping");
    require(landSeeds(landPrefix +
        "create_land { terrain_type 1 land_position 50 50 base_size 1 number_of_tiles 0 } "
        "create_land { terrain_type 258 land_position 50 50 base_size 0 number_of_tiles 0 }", 7, 5), err);
    require(terrain[17] == 2 && std::count(terrain.begin(), terrain.end(), 1) == 8,
            "later seed overwrites overlap with narrowed source terrain ID");
    require(landSeeds(landPrefix + "create_land { terrain_type 1 land_position 99 99 base_size 0 number_of_tiles 0 }", 7, 5), err);
    require(terrain.back() == 1 && std::count(terrain.begin(), terrain.end(), 1) == 1,
            "zero growth budget still paints initial seed");
    require(!landSeeds(landPrefix + "create_land { terrain_type 1 land_position 50 50 base_size 1 number_of_tiles 1 }", 7, 5)
                && terrain.empty(), "unsupported growth must not return a seed as finished terrain");
    require(!landSeeds(landPrefix + "create_player_lands { terrain_type 1 }", 7, 5), "automatic player placement rejected");
    require(!landSeeds(landPrefix + "create_land { terrain_type 1 base_size 1 number_of_tiles 0 }", 7, 5), "implicit random center rejected");
    require(!landSeeds(landPrefix + "create_land { terrain_type 1 land_position 100 50 base_size 1 number_of_tiles 0 }", 7, 5), "off-map percent boundary rejected");
    require(!landSeeds(landPrefix + "create_land { terrain_type 1 land_position 50 50 base_size -1 number_of_tiles 0 }", 7, 5), "negative radius rejected");
    require(!landSeeds(landPrefix + "create_land { terrain_type 3 land_position 50 50 base_size 1 number_of_tiles 0 }", 7, 5)
                && terrain.empty(), "disabled seed terrain cannot leave partial output");
    // Golden corpus from the original Windows 1.1 function 0x492ab0, executed
    // with its tile setter reduced to the terrain-byte write. All 256 possible
    // neighborhoods exercise boundaries, in-place ordering and convergence.
    uint32_t smoothHash = 2166136261u;
    constexpr int neighborPositions[] = {1, 2, 5, 8, 7, 6, 3, 0};
    for (unsigned mask = 0; mask < 256; ++mask) {
        std::vector<uint8_t> before(9, 1), after, repeated;
        for (unsigned bit = 0; bit < 8; ++bit)
            if (mask & (1u << bit)) before[neighborPositions[bit]] = 0;
        require(smoothRmsTerrain(3, 3, 0, before, after, &err), err);
        for (uint8_t tile : after) smoothHash = (smoothHash ^ tile) * 16777619u;
        require(smoothRmsTerrain(3, 3, 0, after, repeated, &err) && repeated == after,
                "smoothing must converge to an idempotent raster");
    }
    require(smoothHash == 2477551712u, "original executable smoothing corpus mismatch");
    std::vector<uint8_t> rectangular;
    require(smoothRmsTerrain(7, 5, 0,
        {1,1,0,1,1,1,1, 1,0,1,1,1,1,0, 1,1,1,0,1,0,1, 0,1,1,1,1,1,1, 1,1,0,1,1,1,1},
        rectangular, &err), err);
    require(rectangular == std::vector<uint8_t>(
        {1,1,0,1,1,1,1, 1,0,0,0,0,0,0, 0,0,0,0,0,0,1, 0,0,0,0,1,1,1, 1,1,0,1,1,1,1}),
        "original executable rectangular smoothing result");
    std::vector<uint8_t> smoothing = {9, 7, 9, 8, 6, 8, 9, 7, 9};
    require(smoothRmsTerrain(3, 3, 7, smoothing, smoothing, &err), err);
    require(smoothing == std::vector<uint8_t>({9, 7, 9, 8, 7, 8, 9, 7, 9}),
            "smoothing opposite gap must preserve unrelated terrain IDs and support aliasing");
    require(smoothRmsTerrain(1, 1, 0, {1}, smoothing, &err) && smoothing == std::vector<uint8_t>({1}),
            "outside-map neighbors cannot count as base terrain");
    require(!smoothRmsTerrain(0, 1, 0, {}, smoothing, &err) && smoothing.empty(), "invalid smoothing dimensions");
    require(!smoothRmsTerrain(2, 2, 0, {1}, smoothing, &err) && smoothing.empty(), "invalid smoothing raster size");
    require(!smoothRmsTerrain(3, 3, 0, {1,0,1,1,1,1,1,0,1}, smoothing, &err, 18)
                && smoothing.empty() && err.find("limit") != std::string::npos,
            "work limit must reject partial smoothing rather than publish it");
    std::vector<uint32_t> paints={99};
    require(smoothRmsTerrain(3,3,10,{10,10,10,10,9,10,10,10,10},smoothing,&err,
                             64*1024*1024,&paints) && paints==std::vector<uint32_t>{4},
            "original active-base setter event order");
    require(smoothRmsTerrain(3,3,10,smoothing,smoothing,&err,64*1024*1024,&paints) && paints.empty(),
            "unchanged terrain must not emit scenery callbacks");
    paints={99};
    require(!smoothRmsTerrain(3,3,10,{10,10,10,10,9,10,10,10,10},smoothing,&err,9,&paints)
                && paints.empty() && smoothing.empty(),
            "failed smoothing must discard early setter events");
    require(smoothRmsTerrain(8,9,10,{44,44,41,41,44,10,9,6,9,10,9,6,10,41,6,44,10,41,9,9,41,10,6,10,41,10,10,10,10,9,10,6,10,10,41,9,10,6,41,44,44,41,44,44,41,41,6,9,10,9,9,6,9,10,10,10,6,6,41,41,10,10,10,44,6,6,44,6,10,6,10,9},smoothing,&err,64*1024*1024,&paints)
                && paints==std::vector<uint32_t>{13,17,20,22,24,29,40,69,44,45,51,52,37,42,43,49,50,34,35,41},
            "original multi-pass setter order must not be sorted into raster order");
    // Complete original 0x4da6f0, with only the rectangle painter intercepted.
    // Golden terrain hashes and final RNG include frontier gap fill, before smoothing.
    struct GrowthCase { uint32_t w, h, x, y, radius, target, coefficient, seed, finalRng, hash; };
    const GrowthCase growthCases[] = {
        {7u, 7u, 3u, 3u, 0u, 0u, 10u, 1u, 1u, 1720441516u},
        {7u, 7u, 3u, 3u, 0u, 2u, 10u, 1u, 3884216597u, 648626517u},
        {7u, 7u, 3u, 3u, 0u, 5u, 10u, 1u, 2179684817u, 111229868u},
        {7u, 7u, 3u, 3u, 0u, 12u, 10u, 1u, 1116436030u, 3540787871u},
        {7u, 7u, 3u, 3u, 0u, 49u, 10u, 1u, 3168190081u, 1155915612u},
        {7u, 7u, 3u, 3u, 0u, 0u, 10u, 12345u, 12345u, 1720441516u},
        {7u, 7u, 3u, 3u, 0u, 2u, 10u, 12345u, 1449997005u, 648626517u},
        {7u, 7u, 3u, 3u, 0u, 5u, 10u, 12345u, 922525705u, 111229868u},
        {7u, 7u, 3u, 3u, 0u, 12u, 10u, 12345u, 2581596540u, 1330568607u},
        {7u, 7u, 3u, 3u, 0u, 49u, 10u, 12345u, 2907807417u, 1155915612u},
        {9u, 5u, 0u, 0u, 1u, 0u, 0u, 12345u, 12345u, 906432951u},
        {9u, 5u, 0u, 0u, 1u, 22u, 0u, 12345u, 2458847943u, 957096697u},
        {9u, 5u, 0u, 0u, 1u, 45u, 0u, 12345u, 538347524u, 3816926912u},
        {9u, 5u, 0u, 0u, 1u, 0u, 10u, 12345u, 12345u, 906432951u},
        {9u, 5u, 0u, 0u, 1u, 22u, 10u, 12345u, 242781834u, 4086995393u},
        {9u, 5u, 0u, 0u, 1u, 45u, 10u, 12345u, 538347524u, 3816926912u},
        {9u, 5u, 0u, 0u, 1u, 0u, 100u, 12345u, 12345u, 906432951u},
        {9u, 5u, 0u, 0u, 1u, 22u, 100u, 12345u, 3372601100u, 4237100985u},
        {9u, 5u, 0u, 0u, 1u, 45u, 100u, 12345u, 538347524u, 3816926912u},
        {9u, 5u, 8u, 4u, 1u, 0u, 0u, 12345u, 12345u, 3390298743u},
        {9u, 5u, 8u, 4u, 1u, 22u, 0u, 12345u, 235103120u, 3909591316u},
        {9u, 5u, 8u, 4u, 1u, 45u, 0u, 12345u, 538347524u, 3816926912u},
        {9u, 5u, 8u, 4u, 1u, 0u, 10u, 12345u, 12345u, 3390298743u},
        {9u, 5u, 8u, 4u, 1u, 22u, 10u, 12345u, 3999531325u, 3426947543u},
        {9u, 5u, 8u, 4u, 1u, 45u, 10u, 12345u, 538347524u, 3816926912u},
        {9u, 5u, 8u, 4u, 1u, 0u, 100u, 12345u, 12345u, 3390298743u},
        {9u, 5u, 8u, 4u, 1u, 22u, 100u, 12345u, 2622461201u, 297960161u},
        {9u, 5u, 8u, 4u, 1u, 45u, 100u, 12345u, 538347524u, 3816926912u},
        {5u, 9u, 2u, 4u, 1u, 0u, 0u, 12345u, 12345u, 1293157564u},
        {5u, 9u, 2u, 4u, 1u, 22u, 0u, 12345u, 4007045953u, 3645860967u},
        {5u, 9u, 2u, 4u, 1u, 45u, 0u, 12345u, 580572337u, 3816926912u},
        {5u, 9u, 2u, 4u, 1u, 0u, 10u, 12345u, 12345u, 1293157564u},
        {5u, 9u, 2u, 4u, 1u, 22u, 10u, 12345u, 910293955u, 79078806u},
        {5u, 9u, 2u, 4u, 1u, 45u, 10u, 12345u, 580572337u, 3816926912u},
        {5u, 9u, 2u, 4u, 1u, 0u, 100u, 12345u, 12345u, 1293157564u},
        {5u, 9u, 2u, 4u, 1u, 22u, 100u, 12345u, 3629701229u, 464830707u},
        {5u, 9u, 2u, 4u, 1u, 45u, 100u, 12345u, 580572337u, 3816926912u},
        {1u, 9u, 0u, 4u, 0u, 0u, 0u, 12345u, 12345u, 2185116220u},
        {1u, 9u, 0u, 4u, 0u, 4u, 0u, 12345u, 3681394607u, 32589315u},
        {1u, 9u, 0u, 4u, 0u, 9u, 0u, 12345u, 1424270039u, 3164567556u},
        {1u, 9u, 0u, 4u, 0u, 0u, 10u, 12345u, 12345u, 2185116220u},
        {1u, 9u, 0u, 4u, 0u, 4u, 10u, 12345u, 3681394607u, 2590708811u},
        {1u, 9u, 0u, 4u, 0u, 9u, 10u, 12345u, 1424270039u, 3164567556u},
        {1u, 9u, 0u, 4u, 0u, 0u, 100u, 12345u, 12345u, 2185116220u},
        {1u, 9u, 0u, 4u, 0u, 4u, 100u, 12345u, 3681394607u, 2590708811u},
        {1u, 9u, 0u, 4u, 0u, 9u, 100u, 12345u, 1424270039u, 3164567556u},
        {9u, 1u, 4u, 0u, 1u, 0u, 0u, 12345u, 12345u, 3272250040u},
        {9u, 1u, 4u, 0u, 1u, 4u, 0u, 12345u, 1255958651u, 32589315u},
        {9u, 1u, 4u, 0u, 1u, 9u, 0u, 12345u, 3681310499u, 3164567556u},
        {9u, 1u, 4u, 0u, 1u, 0u, 10u, 12345u, 12345u, 3272250040u},
        {9u, 1u, 4u, 0u, 1u, 4u, 10u, 12345u, 1255958651u, 32589315u},
        {9u, 1u, 4u, 0u, 1u, 9u, 10u, 12345u, 3681310499u, 3164567556u},
        {9u, 1u, 4u, 0u, 1u, 0u, 100u, 12345u, 12345u, 3272250040u},
        {9u, 1u, 4u, 0u, 1u, 4u, 100u, 12345u, 1255958651u, 32589315u},
        {9u, 1u, 4u, 0u, 1u, 9u, 100u, 12345u, 3681310499u, 3164567556u},
        {1u, 1u, 0u, 0u, 0u, 0u, 0u, 12345u, 12345u, 67918732u},
        {1u, 1u, 0u, 0u, 0u, 1u, 0u, 12345u, 12345u, 67918732u},
        {1u, 1u, 0u, 0u, 0u, 0u, 10u, 12345u, 12345u, 67918732u},
        {1u, 1u, 0u, 0u, 0u, 1u, 10u, 12345u, 12345u, 67918732u},
        {1u, 1u, 0u, 0u, 0u, 0u, 100u, 12345u, 12345u, 67918732u},
        {1u, 1u, 0u, 0u, 0u, 1u, 100u, 12345u, 12345u, 67918732u},
    };
    for (const auto &c : growthCases) {
        uint32_t state = c.seed;
        RmsLandGrowth land{1, c.x, c.y, c.radius, c.target, c.coefficient};
        require(growRmsSingleLand(c.w, c.h, 0, land, terrainData, state, terrain, &err), err);
        uint32_t hash = 2166136261u;
        for (uint8_t tile : terrain) hash = (hash ^ tile) * 16777619u;
        require(hash == c.hash && state == c.finalRng,
                "original single-land growth terrain/RNG mismatch: " + std::to_string(c.w) + "x" +
                std::to_string(c.h) + " target=" + std::to_string(c.target) + " seed=" + std::to_string(c.seed));
    }
    require(RmsLandGrowth{}.clumpingFactor == 8, "original default clumping factor");
    RmsLandGrowth land{1, 3, 3, 0, 12, 10};
    land.terrain = 0;
    uint32_t identicalTerrainState = 1;
    require(growRmsSingleLand(7, 7, 0, land, terrainData, identicalTerrainState, terrain, &err)
                && identicalTerrainState == 1116436030u && terrain == std::vector<uint8_t>(49, 0),
            "original growth tracks zones and consumes RNG even when land and base terrain match");
    land.terrain = 1;
    uint32_t growthState = 12345;
    require(!growRmsSingleLand(7, 7, 0, land, terrainData, growthState, terrain, &err, 40)
                && terrain.empty() && growthState == 12345 && err.find("limit") != std::string::npos,
            "growth work exhaustion must not publish partial terrain or RNG");
    land.terrain = 3;
    require(!growRmsSingleLand(7, 7, 0, land, terrainData, growthState, terrain, &err)
                && terrain.empty() && growthState == 12345, "invalid growth terrain rejected");
    land.terrain = 1;
    land.centerX = 7;
    require(!growRmsSingleLand(7, 7, 0, land, terrainData, growthState, terrain, &err), "off-map growth center rejected");
    land.centerX = 3;
    land.targetTiles = 50;
    require(!growRmsSingleLand(7, 7, 0, land, terrainData, growthState, terrain, &err), "oversized growth target rejected");
    land.targetTiles = 12;
    land.clumpingFactor = 101;
    require(!growRmsSingleLand(7, 7, 0, land, terrainData, growthState, terrain, &err), "unsupported coefficient rejected");
    // Original 0x4da6f0 multi-land golden corpus: source-order competition,
    // overlapping seeds, shared queue transfers, same-terrain zones and tiny maps.
    struct MultiGrowthCase {
        uint32_t w, h, seed, finalRng, hash;
        std::vector<RmsLandGrowth> lands;
    };
    const MultiGrowthCase multiCases[] = {
        {9, 7, 1u, 2529530053u, 4038973349u, {{1, 2, 3, 0, 20, 10}, {2, 6, 3, 0, 20, 10}}},
        {9, 7, 1u, 410959018u, 452018575u, {{2, 6, 3, 0, 20, 10}, {1, 2, 3, 0, 20, 10}}},
        {9, 7, 1u, 3871632827u, 131056421u, {{1, 4, 3, 0, 30, 0}, {2, 4, 3, 0, 30, 100}}},
        {9, 7, 1u, 3770629007u, 2585261414u, {{1, 3, 3, 1, 40, 10}, {2, 5, 3, 1, 40, 10}}},
        {9, 7, 1u, 1587391380u, 2090768720u, {{1, 0, 0, 1, 63, 10}, {2, 8, 6, 1, 63, 10}}},
        {9, 7, 1u, 1587391380u, 1622318865u, {{1, 0, 0, 1, 0, 10}, {2, 8, 6, 1, 63, 10}}},
        {9, 7, 1u, 2529530053u, 1834508171u, {{1, 2, 3, 0, 20, 10}, {1, 6, 3, 0, 20, 10}}},
        {9, 7, 1u, 805255903u, 3272050113u, {{1, 2, 3, 0, 10, 0}, {2, 6, 3, 0, 20, 100}, {0, 4, 3, 0, 30, 10}}},
        {9, 7, 1u, 1u, 3738720020u, {{1, 2, 3, 1, 0, 10}, {2, 6, 3, 1, 0, 10}}},
        {9, 7, 12345u, 2982061137u, 1759734513u, {{1, 2, 3, 0, 20, 10}, {2, 6, 3, 0, 20, 10}}},
        {9, 7, 12345u, 1521716349u, 3446282018u, {{2, 6, 3, 0, 20, 10}, {1, 2, 3, 0, 20, 10}}},
        {9, 7, 12345u, 580572337u, 2312775591u, {{1, 4, 3, 0, 30, 0}, {2, 4, 3, 0, 30, 100}}},
        {9, 7, 12345u, 1313306631u, 3604979893u, {{1, 3, 3, 1, 40, 10}, {2, 5, 3, 1, 40, 10}}},
        {9, 7, 12345u, 2140623980u, 4241315213u, {{1, 0, 0, 1, 63, 10}, {2, 8, 6, 1, 63, 10}}},
        {9, 7, 12345u, 2140623980u, 1622318865u, {{1, 0, 0, 1, 0, 10}, {2, 8, 6, 1, 63, 10}}},
        {9, 7, 12345u, 2982061137u, 558808833u, {{1, 2, 3, 0, 20, 10}, {1, 6, 3, 0, 20, 10}}},
        {9, 7, 12345u, 157067805u, 215059669u, {{1, 2, 3, 0, 10, 0}, {2, 6, 3, 0, 20, 100}, {0, 4, 3, 0, 30, 10}}},
        {9, 7, 12345u, 12345u, 3738720020u, {{1, 2, 3, 1, 0, 10}, {2, 6, 3, 1, 0, 10}}},
        {9, 7, 4294967295u, 1659630298u, 1076087807u, {{1, 2, 3, 0, 20, 10}, {2, 6, 3, 0, 20, 10}}},
        {9, 7, 4294967295u, 3992224559u, 3870843024u, {{2, 6, 3, 0, 20, 10}, {1, 2, 3, 0, 20, 10}}},
        {9, 7, 4294967295u, 678144427u, 1285638983u, {{1, 4, 3, 0, 30, 0}, {2, 4, 3, 0, 30, 100}}},
        {9, 7, 4294967295u, 66165533u, 3604979893u, {{1, 3, 3, 1, 40, 10}, {2, 5, 3, 1, 40, 10}}},
        {9, 7, 4294967295u, 3893445882u, 2105001007u, {{1, 0, 0, 1, 63, 10}, {2, 8, 6, 1, 63, 10}}},
        {9, 7, 4294967295u, 3893445882u, 1622318865u, {{1, 0, 0, 1, 0, 10}, {2, 8, 6, 1, 63, 10}}},
        {9, 7, 4294967295u, 1659630298u, 11951401u, {{1, 2, 3, 0, 20, 10}, {1, 6, 3, 0, 20, 10}}},
        {9, 7, 4294967295u, 987800192u, 4226924559u, {{1, 2, 3, 0, 10, 0}, {2, 6, 3, 0, 20, 100}, {0, 4, 3, 0, 30, 10}}},
        {9, 7, 4294967295u, 4294967295u, 3738720020u, {{1, 2, 3, 1, 0, 10}, {2, 6, 3, 1, 0, 10}}},
        {1, 9, 1u, 3673273548u, 3329763169u, {{1, 0, 0, 0, 9, 10}, {2, 0, 8, 0, 9, 10}}},
        {9, 1, 1u, 3673273548u, 3329763169u, {{1, 0, 0, 0, 9, 10}, {2, 8, 0, 0, 9, 10}}},
        {1, 1, 1u, 1u, 118251589u, {{1, 0, 0, 0, 1, 10}, {2, 0, 0, 0, 1, 10}}},
    };
    for (const auto &c : multiCases) {
        uint32_t state = c.seed;
        require(growRmsLands(c.w, c.h, 0, c.lands, terrainData, state, terrain, &err), err);
        uint32_t hash = 2166136261u;
        for (uint8_t tile : terrain) hash = (hash ^ tile) * 16777619u;
        require(hash == c.hash && state == c.finalRng, "original competing land terrain/RNG mismatch");
    }
    std::vector<RmsLandGrowth> crowdedLands;
    for (uint32_t i = 0; i < 64; ++i)
        crowdedLands.push_back({static_cast<uint8_t>(1 + i % 2), i % 9, (i / 9) % 7, 0, 63, 10});
    uint32_t multiState = 1;
    require(growRmsLands(9, 7, 0, crowdedLands, terrainData, multiState, terrain, &err), err);
    uint32_t crowdedHash = 2166136261u;
    for (uint8_t tile : terrain) crowdedHash = (crowdedHash ^ tile) * 16777619u;
    require(crowdedHash == 1309143968u && multiState == 3601754070u, "original 64-land boundary mismatch");

    multiState = 12345;
    require(!growRmsLands(9, 7, 0, multiCases[0].lands, terrainData, multiState, terrain, &err, 100)
                && terrain.empty() && multiState == 12345, "multi-land work exhaustion must be transactional");
    require(!growRmsLands(9, 7, 0, {}, terrainData, multiState, terrain, &err), "empty land plan rejected");
    crowdedLands.push_back(crowdedLands.front());
    require(!growRmsLands(9, 7, 0, crowdedLands, terrainData, multiState, terrain, &err), "more than64 lands rejected");
    auto invalidLands = multiCases[0].lands;
    invalidLands.back().terrain = 3;
    require(!growRmsLands(9, 7, 0, invalidLands, terrainData, multiState, terrain, &err)
                && terrain.empty() && multiState == 12345, "invalid later land cannot publish partial output");
    // Original seeds store area by zone byte, while growth budgets use ordinal.
    const MultiGrowthCase zoneCases[] = {
        {9, 7, 1u, 1571997048u, 392505435u, {{1, 2, 3, 0, 20, 10, 10}, {2, 6, 3, 0, 20, 10, 11}}},
        {9, 7, 1u, 1978610859u, 3726235164u, {{1, 3, 3, 1, 30, 10, 10}, {2, 5, 3, 0, 30, 10, 11}}},
        {9, 7, 1u, 1764518064u, 381960564u, {{1, 2, 3, 0, 20, 10, 0}, {2, 6, 3, 0, 20, 10, 0}}},
        {9, 7, 1u, 1978610859u, 2659844066u, {{1, 3, 3, 1, 30, 10, 0}, {2, 5, 3, 0, 30, 10, 0}}},
        {9, 7, 1u, 1020636519u, 4038973349u, {{1, 2, 3, 0, 20, 10, 1}, {2, 6, 3, 0, 20, 10, 0}}},
        {9, 7, 1u, 1978610859u, 3726235164u, {{1, 3, 3, 1, 30, 10, 1}, {2, 5, 3, 0, 30, 10, 0}}},
        {9, 7, 1u, 410959018u, 381960564u, {{1, 2, 3, 0, 20, 10, 63}, {2, 6, 3, 0, 20, 10, 63}}},
        {9, 7, 1u, 1978610859u, 2659844066u, {{1, 3, 3, 1, 30, 10, 63}, {2, 5, 3, 0, 30, 10, 63}}},
        {9, 7, 1u, 1883859121u, 2890937318u, {{1, 2, 3, 0, 20, 10, 1}, {2, 6, 3, 0, 20, 10, 1}}},
        {9, 7, 1u, 1978610859u, 2659844066u, {{1, 3, 3, 1, 30, 10, 1}, {2, 5, 3, 0, 30, 10, 1}}},
        {9, 7, 12345u, 1944022629u, 824862340u, {{1, 2, 3, 0, 20, 10, 10}, {2, 6, 3, 0, 20, 10, 11}}},
        {9, 7, 12345u, 3530115491u, 4078798167u, {{1, 3, 3, 1, 30, 10, 10}, {2, 5, 3, 0, 30, 10, 11}}},
        {9, 7, 12345u, 2429814310u, 1534715012u, {{1, 2, 3, 0, 20, 10, 0}, {2, 6, 3, 0, 20, 10, 0}}},
        {9, 7, 12345u, 3530115491u, 84440426u, {{1, 3, 3, 1, 30, 10, 0}, {2, 5, 3, 0, 30, 10, 0}}},
        {9, 7, 12345u, 156777811u, 1277845443u, {{1, 2, 3, 0, 20, 10, 1}, {2, 6, 3, 0, 20, 10, 0}}},
        {9, 7, 12345u, 3530115491u, 4078798167u, {{1, 3, 3, 1, 30, 10, 1}, {2, 5, 3, 0, 30, 10, 0}}},
        {9, 7, 12345u, 2982061137u, 1534715012u, {{1, 2, 3, 0, 20, 10, 63}, {2, 6, 3, 0, 20, 10, 63}}},
        {9, 7, 12345u, 3530115491u, 84440426u, {{1, 3, 3, 1, 30, 10, 63}, {2, 5, 3, 0, 30, 10, 63}}},
        {9, 7, 12345u, 261506783u, 1534715012u, {{1, 2, 3, 0, 20, 10, 1}, {2, 6, 3, 0, 20, 10, 1}}},
        {9, 7, 12345u, 3530115491u, 84440426u, {{1, 3, 3, 1, 30, 10, 1}, {2, 5, 3, 0, 30, 10, 1}}},
        {9, 7, 4294967295u, 1745473417u, 4192392648u, {{1, 2, 3, 0, 20, 10, 10}, {2, 6, 3, 0, 20, 10, 11}}},
        {9, 7, 4294967295u, 786318169u, 2659844066u, {{1, 3, 3, 1, 30, 10, 10}, {2, 5, 3, 0, 30, 10, 11}}},
        {9, 7, 4294967295u, 1551019573u, 2217776227u, {{1, 2, 3, 0, 20, 10, 0}, {2, 6, 3, 0, 20, 10, 0}}},
        {9, 7, 4294967295u, 786318169u, 655926969u, {{1, 3, 3, 1, 30, 10, 0}, {2, 5, 3, 0, 30, 10, 0}}},
        {9, 7, 4294967295u, 1806936100u, 4062106631u, {{1, 2, 3, 0, 20, 10, 1}, {2, 6, 3, 0, 20, 10, 0}}},
        {9, 7, 4294967295u, 786318169u, 2659844066u, {{1, 3, 3, 1, 30, 10, 1}, {2, 5, 3, 0, 30, 10, 0}}},
        {9, 7, 4294967295u, 725030846u, 4093196422u, {{1, 2, 3, 0, 20, 10, 63}, {2, 6, 3, 0, 20, 10, 63}}},
        {9, 7, 4294967295u, 786318169u, 655926969u, {{1, 3, 3, 1, 30, 10, 63}, {2, 5, 3, 0, 30, 10, 63}}},
        {9, 7, 4294967295u, 1806936100u, 125159956u, {{1, 2, 3, 0, 20, 10, 1}, {2, 6, 3, 0, 20, 10, 1}}},
        {9, 7, 4294967295u, 786318169u, 655926969u, {{1, 3, 3, 1, 30, 10, 1}, {2, 5, 3, 0, 30, 10, 1}}},
        {9, 7, 1u, 3257732100u, 2208029693u, {{1, 2, 3, 0, 20, 10, 2}, {2, 6, 3, 0, 20, 10, 0}, {0, 4, 3, 1, 20, 10, 1}}},
    };
    for (const auto &c : zoneCases) {
        uint32_t state = c.seed;
        require(growRmsLands(c.w, c.h, 0, c.lands, terrainData, state, terrain, &err), err);
        uint32_t hash = 2166136261u;
        for (uint8_t tile : terrain) hash = (hash ^ tile) * 16777619u;
        require(hash == c.hash && state == c.finalRng, "original explicit/shared zone accounting mismatch");
    }
    auto badZone = zoneCases[0].lands;
    badZone[0].zone = 64;
    multiState = 12345;
    require(!growRmsLands(9, 7, 0, badZone, terrainData, multiState, terrain, &err)
                && terrain.empty() && multiState == 12345, "out-of-bound zone must fail transactionally");
    badZone[0].zone = -2;
    require(!growRmsLands(9, 7, 0, badZone, terrainData, multiState, terrain, &err), "negative explicit zone rejected");
    // Exact original two-land growth with inset bounds: rejected candidates
    // become zone255, affecting other lands' future queue expansion and RNG.
    struct EdgeCase { uint32_t seed, fuzziness, finalRng, hash; };
    const EdgeCase edgeCases[] = {
        {1u, 0, 3957879258u, 2819919929u},
        {1u, 1, 108516897u, 3090479649u},
        {1u, 15, 3950160536u, 4180151673u},
        {1u, 20, 3365727239u, 2258020281u},
        {1u, 100, 1883859121u, 105295285u},
        {1u, 255, 1883859121u, 105295285u},
        {12345u, 0, 1280788466u, 1618060882u},
        {12345u, 1, 3748080539u, 264007472u},
        {12345u, 15, 1628176098u, 1916756001u},
        {12345u, 20, 3530115491u, 1622472266u},
        {12345u, 100, 2803282665u, 3558578369u},
        {12345u, 255, 2803282665u, 3558578369u},
    };
    std::vector<RmsLandGrowth> edgeLands = {{1,2,3,0,63,10}, {2,6,3,0,63,10}};
    for (auto &l : edgeLands) { l.leftBorder=1; l.topBorder=1; l.rightBorder=7; l.bottomBorder=5; }
    for (const auto &c : edgeCases) {
        for (auto &l : edgeLands) l.borderFuzziness=c.fuzziness;
        uint32_t state=c.seed;
        require(growRmsLands(9,7,0,edgeLands,terrainData,state,terrain,&err),err);
        uint32_t hash=2166136261u;
        for (uint8_t tile:terrain) hash=(hash^tile)*16777619u;
        require(hash==c.hash && state==c.finalRng,"original edge-rejection growth/RNG mismatch");
    }
    auto defaultEdges = multiCases[0].lands;
    // Seed30091 first draws32767 (roll100). Original score>=100 becomes101,
    // so even that maximum roll rejects rather than accepting an edge tile.
    RmsLandGrowth hardEdge{1,0,0,0,63,10,-1,100,3,3,5,5};
    multiState=30091;
    require(growRmsSingleLand(9,7,0,hardEdge,terrainData,multiState,terrain,&err),err);
    require(multiState==3317704797u && terrain[0]==1 && std::count(terrain.begin(),terrain.end(),1)==1,
            "original101 rejection threshold must reject maximum random roll100");
    for (auto &l : defaultEdges) l.borderFuzziness=20;
    multiState=multiCases[0].seed;
    require(growRmsLands(9,7,0,defaultEdges,terrainData,multiState,terrain,&err),err);
    uint32_t defaultEdgeHash=2166136261u;
    for (uint8_t tile:terrain) defaultEdgeHash=(defaultEdgeHash^tile)*16777619u;
    require(defaultEdgeHash==multiCases[0].hash && multiState==multiCases[0].finalRng,
            "full-map default bounds have zero edge score even with original fuzziness20");
    multiState=12345;
    require(!growRmsLands(9,7,0,edgeLands,terrainData,multiState,terrain,&err,100)
                && terrain.empty() && multiState==12345,"edge work exhaustion must preserve state");
    edgeLands[0].borderFuzziness=256;
    require(!growRmsLands(9,7,0,edgeLands,terrainData,multiState,terrain,&err),"invalid edge byte rejected");
    edgeLands[0].borderFuzziness=20;
    edgeLands[0].rightBorder=0;
    require(!growRmsLands(9,7,0,edgeLands,terrainData,multiState,terrain,&err),"inverted edge bounds rejected");
    edgeLands[0].rightBorder=10;
    require(!growRmsLands(9,7,0,edgeLands,terrainData,multiState,terrain,&err),"off-map edge bounds rejected");
    // Full original 0x4da6f0 fixtures, including its 0x4da400 separation predicate.
    struct SeparationCase { uint32_t seed, a, b, zoneA, zoneB, overlapping, finalRng, hash; };
    const SeparationCase separationCases[] = {
        {1u, 0u, 0u, 0u, 1u, 0u, 3957879258u, 2819919929u},
        {1u, 0u, 0u, 1u, 1u, 0u, 3957879258u, 1292919084u},
        {1u, 1u, 1u, 0u, 1u, 0u, 1229611062u, 3312653651u},
        {1u, 1u, 1u, 1u, 1u, 0u, 3957879258u, 1292919084u},
        {1u, 2u, 2u, 0u, 1u, 0u, 3327618242u, 1521259424u},
        {1u, 2u, 2u, 1u, 1u, 0u, 3957879258u, 1292919084u},
        {1u, 3u, 3u, 0u, 1u, 0u, 1221339053u, 3212817590u},
        {1u, 3u, 3u, 1u, 1u, 0u, 3957879258u, 1292919084u},
        {1u, 4u, 4u, 0u, 1u, 0u, 1084083701u, 3270113133u},
        {1u, 4u, 4u, 1u, 1u, 0u, 3957879258u, 1292919084u},
        {1u, 5u, 5u, 0u, 1u, 0u, 1924036713u, 3447318956u},
        {1u, 5u, 5u, 1u, 1u, 0u, 3957879258u, 1292919084u},
        {1u, 6u, 6u, 0u, 1u, 0u, 1924036713u, 3447318956u},
        {1u, 6u, 6u, 1u, 1u, 0u, 3957879258u, 1292919084u},
        {1u, 9u, 9u, 0u, 1u, 0u, 1924036713u, 3447318956u},
        {1u, 9u, 9u, 1u, 1u, 0u, 3957879258u, 1292919084u},
        {1u, 20u, 20u, 0u, 1u, 0u, 1924036713u, 3447318956u},
        {1u, 20u, 20u, 1u, 1u, 0u, 3957879258u, 1292919084u},
        {1u, 0u, 1u, 10u, 11u, 1u, 583889235u, 1902008752u},
        {1u, 3u, 0u, 10u, 11u, 1u, 1587391380u, 3095973256u},
        {1u, 1u, 5u, 10u, 11u, 1u, 1978610859u, 226037942u},
        {1u, 6u, 2u, 10u, 11u, 1u, 3250303071u, 1068506822u},
        {12345u, 0u, 0u, 0u, 1u, 0u, 1280788466u, 1618060882u},
        {12345u, 0u, 0u, 1u, 1u, 0u, 1280788466u, 3450314790u},
        {12345u, 1u, 1u, 0u, 1u, 0u, 2896408581u, 2119129294u},
        {12345u, 1u, 1u, 1u, 1u, 0u, 1280788466u, 3450314790u},
        {12345u, 2u, 2u, 0u, 1u, 0u, 1631149365u, 3273639964u},
        {12345u, 2u, 2u, 1u, 1u, 0u, 1280788466u, 3450314790u},
        {12345u, 3u, 3u, 0u, 1u, 0u, 1944022629u, 3212817590u},
        {12345u, 3u, 3u, 1u, 1u, 0u, 1280788466u, 3450314790u},
        {12345u, 4u, 4u, 0u, 1u, 0u, 1367366578u, 214379920u},
        {12345u, 4u, 4u, 1u, 1u, 0u, 1280788466u, 3450314790u},
        {12345u, 5u, 5u, 0u, 1u, 0u, 354763681u, 3447318956u},
        {12345u, 5u, 5u, 1u, 1u, 0u, 1280788466u, 3450314790u},
        {12345u, 6u, 6u, 0u, 1u, 0u, 354763681u, 3447318956u},
        {12345u, 6u, 6u, 1u, 1u, 0u, 1280788466u, 3450314790u},
        {12345u, 9u, 9u, 0u, 1u, 0u, 354763681u, 3447318956u},
        {12345u, 9u, 9u, 1u, 1u, 0u, 1280788466u, 3450314790u},
        {12345u, 20u, 20u, 0u, 1u, 0u, 354763681u, 3447318956u},
        {12345u, 20u, 20u, 1u, 1u, 0u, 1280788466u, 3450314790u},
        {12345u, 0u, 1u, 10u, 11u, 1u, 3159914280u, 2906783701u},
        {12345u, 3u, 0u, 10u, 11u, 1u, 157067805u, 632918946u},
        {12345u, 1u, 5u, 10u, 11u, 1u, 3530115491u, 226037942u},
        {12345u, 6u, 2u, 10u, 11u, 1u, 1424270039u, 1068506822u},
    };
    for (const auto &c:separationCases) {
        std::vector<RmsLandGrowth> lands = c.overlapping
            ? std::vector<RmsLandGrowth>{{1,3,3,1,63,10}, {2,5,3,0,63,10}}
            : std::vector<RmsLandGrowth>{{1,2,3,0,63,10}, {2,6,3,0,63,10}};
        lands[0].separation=c.a; lands[1].separation=c.b;
        lands[0].zone=c.zoneA; lands[1].zone=c.zoneB;
        uint32_t state=c.seed;
        require(growRmsLands(9,7,0,lands,terrainData,state,terrain,&err),err);
        uint32_t hash=2166136261u;
        for (uint8_t tile:terrain) hash=(hash^tile)*16777619u;
        require(hash==c.hash && state==c.finalRng,"original separated-land terrain/RNG mismatch");
    }
    RmsLandGrowth distantLand{1,3,3,0,63,10};
    distantLand.separation=1024;
    multiState=12345;
    require(!growRmsSingleLand(9,7,0,distantLand,terrainData,multiState,terrain,&err,100)
                && terrain.empty() && multiState==12345 && err.find("limit")!=std::string::npos,
            "separation scans must respect work limits and roll back");
    distantLand.separation=1025;
    require(!growRmsSingleLand(9,7,0,distantLand,terrainData,multiState,terrain,&err)
                && terrain.empty() && multiState==12345,"oversized separation must fail atomically");
    if (argc == 2) {
        ResourceSet resources;
        require(resources.add(argv[1], &err), err);
        require(script.load(54205, resources, &err), "Shoreline: " + err);
        size_t stages = 0, includes = 0;
        bool worker = false, constant = false, resourceInclude = false;
        for (const auto &t : script.tokens()) {
            stages += t.text.front() == '<';
            includes += t.text == "#include_drs";
            worker |= t.text == "UNIT-WORKER";
            constant |= t.resourceId == 54000;
            resourceInclude |= t.resourceId == 54103;
        }
        require(stages == 7 && !includes && worker && constant && resourceInclude,
                "Shoreline stages or packaged include content missing");
        require(!script.preprocess({}, processed, &err) && err.find("random") != std::string::npos,
                "Shoreline must not fabricate random choices");
        rng = 1;
        require(script.preprocess({"TINY_MAP"}, processed, &err, &rng), "seeded Shoreline: " + err);
        size_t generatedStages = 0;
        for (const auto &t : processed) {
            generatedStages += t.text.front() == '<';
            require(t.text != "start_random" && t.text != "percent_chance" && t.text != "if" &&
                    t.text != "#const" && t.text != "#define", "unresolved Shoreline control");
        }
        require(generatedStages == 7, "seeded preprocessing lost stages");
        require(!generateRmsBaseTerrain(processed, 64, 64, terrainData, terrain, &err) && terrain.empty(),
                "complete Shoreline must reject unsupported generation");
        const auto firstRun = text();
        const uint32_t finalRng = rng;
        rng = 1;
        require(script.preprocess({"TINY_MAP"}, processed, &err, &rng), err);
        require(text() == firstRun && rng == finalRng, "seeded preprocessing repeatability");
        std::cout << "Seeded Shoreline: " << processed.size() << " directive tokens; final RNG=" << rng << '\n';
        std::cout << "Packaged Shoreline: " << script.tokens().size() << " tokens, seven stages, includes resolved\n";
        // The original constants are data, not a reconstructed fixture.
        auto packaged = [&](int32_t id, std::vector<uint8_t> &out) {
            if (id != 1) return resources.read(id, out);
            const std::string source = "#include_drs random_map.def 54000\n"
                "<OBJECTS_GENERATION> create_object BLDG-MAIN1 { "
                "if MIGRATION group_placement_radius 14 else group_placement_radius 18 endif } "
                "create_object UNIT-WORKER { terrain_to_place_on GRASS1 }";
            out.assign(source.begin(), source.end());
            return true;
        };
        require(script.load(1, packaged, limits, &err), err);
        require(script.preprocess({}, processed, &err), err);
        require(text() == "<OBJECTS_GENERATION> create_object 109 { group_placement_radius 18 } "
                          "create_object 83 { terrain_to_place_on 0 } ", "packaged constants/default branch");
        require(script.preprocess({"MIGRATION"}, processed, &err), err);
        require(text().find("group_placement_radius 14") != std::string::npos, "packaged migration branch");
        auto packagedBase = [&](int32_t id, std::vector<uint8_t> &out) {
            if (id != 1) return resources.read(id, out);
            const std::string source = "#include_drs random_map.def 54000\n"
                "<LAND_GENERATION> base_terrain WATER_SHALLOW";
            out.assign(source.begin(), source.end());
            return true;
        };
        require(script.load(1, packagedBase, limits, &err), err);
        require(script.preprocess({}, processed, &err), err);
        require(generateRmsBaseTerrain(processed, 7, 3, terrainData, terrain, &err), err);
        require(terrain.size() == 21 && std::all_of(terrain.begin(), terrain.end(), [](uint8_t t) { return t == 1; }),
                "original packaged base terrain constant fill");
    }
    std::cout << "RMS loading boundary checks passed\n";
}
