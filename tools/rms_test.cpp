// SPDX-License-Identifier: GPL-3.0-or-later
// Standalone: c++ -std=c++17 -Isrc tools/rms_test.cpp src/engine/rms_script.cpp src/core/drs.cpp -o /tmp/rms_test
#include "engine/rms_script.h"
#include "core/drs.h"
#include <cstdlib>
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
    require(text() == "fallback ", "undefine and else");
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
    rejectPreprocess("#const X 0 if X selected endif", "condition semantics");
    rejectPreprocess("#const X 1 #const X 2", "conflicting");
    files[2] = "#const GRASS 0\n#define HOME";
    require(preprocess("#include_drs constants.def 2\nif HOME terrain_type GRASS endif", {}), err);
    require(text() == "terrain_type 0 " && processed[1].resourceId == 1, "included definitions");
    require(script.preprocess({}, processed, &err) && text() == "terrain_type 0 ", "repeat preprocessing changed source");
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
    }
    std::cout << "RMS loading boundary checks passed\n";
}
