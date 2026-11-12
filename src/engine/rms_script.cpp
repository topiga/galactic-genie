// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_script.h"
#include "../core/drs.h"

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
                rng = rng * 214013u + 2531011u;
                remaining = static_cast<int>(((rng >> 16) & 32767u) * 100u / 32767u);
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
} // namespace swgb
