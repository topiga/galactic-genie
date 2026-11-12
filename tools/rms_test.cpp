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
