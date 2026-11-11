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
        std::cout << "Packaged Shoreline: " << script.tokens().size() << " tokens, seven stages, includes resolved\n";
    }
    std::cout << "RMS loading boundary checks passed\n";
}
