// SPDX-License-Identifier: GPL-3.0-or-later
#include "ai_script.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <sstream>

namespace swgb {

namespace {

struct Token {
    std::string text;
    bool quoted = false;
    int line = 0;
};

std::string trim(const std::string &text) {
    size_t begin = 0;
    while (begin < text.size() &&
           std::isspace((unsigned char)text[begin]))
        begin++;
    size_t end = text.size();
    while (end > begin &&
           std::isspace((unsigned char)text[end - 1]))
        end--;
    return text.substr(begin, end - begin);
}

std::string directoryOf(const std::string &path) {
    const size_t separator =
        path.find_last_of("/\\");
    return separator == std::string::npos
               ? std::string()
               : path.substr(0, separator);
}

std::string joinPath(
    const std::string &directory,
    const std::string &name) {
    if (directory.empty()) return name;
    const char last = directory.back();
    return directory +
           (last == '/' || last == '\\' ? "" : "/") +
           name;
}

bool readTextFile(
    const std::string &path,
    std::string &text,
    std::string *err) {
    FILE *file = fopen(path.c_str(), "rb");
    if (!file) {
        if (err)
            *err = "could not open AI script '" +
                   path + "'";
        return false;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        if (err)
            *err = "could not seek AI script '" +
                   path + "'";
        return false;
    }
    const long size = ftell(file);
    if (size < 0 ||
        size > 16 * 1024 * 1024) {
        fclose(file);
        if (err)
            *err = "invalid AI script size for '" +
                   path + "'";
        return false;
    }
    rewind(file);
    text.resize((size_t)size);
    if (size > 0 &&
        fread(text.data(), 1, (size_t)size, file) !=
            (size_t)size) {
        fclose(file);
        if (err)
            *err = "could not read AI script '" +
                   path + "'";
        return false;
    }
    fclose(file);
    return true;
}

bool parseInteger(
    const std::string &text,
    int &value) {
    if (text.empty()) return false;
    errno = 0;
    char *end = nullptr;
    const long parsed =
        std::strtol(text.c_str(), &end, 10);
    if (errno != 0 || !end || *end ||
        parsed < std::numeric_limits<int>::min() ||
        parsed > std::numeric_limits<int>::max())
        return false;
    value = (int)parsed;
    return true;
}

bool preprocess(
    const std::string &name,
    const std::string &source,
    const std::unordered_set<std::string> &defines,
    std::string &result,
    std::string *err) {
    struct Conditional {
        bool parent = true;
        bool condition = true;
        bool sawElse = false;
    };
    std::vector<Conditional> stack;
    bool active = true;
    std::istringstream stream(source);
    std::string line;
    int lineNumber = 0;
    while (std::getline(stream, line)) {
        lineNumber++;
        bool quoted = false;
        for (size_t index = 0;
             index < line.size(); ++index) {
            if (line[index] == '"' &&
                (index == 0 ||
                 line[index - 1] != '\\'))
                quoted = !quoted;
            if (!quoted && line[index] == ';') {
                line.resize(index);
                break;
            }
        }
        const std::string clean = trim(line);
        if (clean.rfind(
                "#load-if-defined", 0) == 0 ||
            clean.rfind(
                "#load-if-not-defined", 0) == 0) {
            const bool negate =
                clean.rfind(
                    "#load-if-not-defined", 0) == 0;
            const size_t prefix =
                negate ? 20 : 16;
            const std::string symbol =
                normalizeAiSymbol(
                    trim(clean.substr(prefix)));
            if (symbol.empty()) {
                if (err)
                    *err = name + ":" +
                           std::to_string(
                               lineNumber) +
                           ": missing conditional symbol";
                return false;
            }
            const bool defined =
                defines.count(symbol) != 0;
            stack.push_back(
                {active,
                 negate ? !defined : defined,
                 false});
            active =
                stack.back().parent &&
                stack.back().condition;
            result.push_back('\n');
            continue;
        }
        if (clean == "#else") {
            if (stack.empty() ||
                stack.back().sawElse) {
                if (err)
                    *err = name + ":" +
                           std::to_string(
                               lineNumber) +
                           ": unmatched #else";
                return false;
            }
            stack.back().sawElse = true;
            stack.back().condition =
                !stack.back().condition;
            active =
                stack.back().parent &&
                stack.back().condition;
            result.push_back('\n');
            continue;
        }
        if (clean == "#end-if") {
            if (stack.empty()) {
                if (err)
                    *err = name + ":" +
                           std::to_string(
                               lineNumber) +
                           ": unmatched #end-if";
                return false;
            }
            active = stack.back().parent;
            stack.pop_back();
            result.push_back('\n');
            continue;
        }
        if (!clean.empty() && clean[0] == '#') {
            if (err)
                *err = name + ":" +
                       std::to_string(lineNumber) +
                       ": unsupported preprocessor directive '" +
                       clean + "'";
            return false;
        }
        if (active) result += line;
        result.push_back('\n');
    }
    if (!stack.empty()) {
        if (err)
            *err = name +
                   ": unterminated AI conditional";
        return false;
    }
    return true;
}

bool tokenize(
    const std::string &name,
    const std::string &source,
    std::vector<Token> &tokens,
    std::string *err) {
    int line = 1;
    for (size_t index = 0;
         index < source.size();) {
        const char character = source[index];
        if (character == '\n') {
            line++;
            index++;
            continue;
        }
        if (std::isspace(
                (unsigned char)character)) {
            index++;
            continue;
        }
        if (character == '(' ||
            character == ')') {
            tokens.push_back(
                {std::string(1, character),
                 false, line});
            index++;
            continue;
        }
        if (character == '"') {
            const int startLine = line;
            std::string value;
            index++;
            bool closed = false;
            while (index < source.size()) {
                const char next = source[index++];
                if (next == '\n') line++;
                if (next == '"') {
                    closed = true;
                    break;
                }
                if (next == '\\' &&
                    index < source.size()) {
                    const char escaped =
                        source[index++];
                    value.push_back(
                        escaped == 'n'
                            ? '\n'
                            : escaped);
                    continue;
                }
                value.push_back(next);
            }
            if (!closed) {
                if (err)
                    *err = name + ":" +
                           std::to_string(
                               startLine) +
                           ": unterminated string";
                return false;
            }
            tokens.push_back(
                {std::move(value), true,
                 startLine});
            continue;
        }
        const size_t begin = index;
        while (index < source.size() &&
               !std::isspace(
                   (unsigned char)source[index]) &&
               source[index] != '(' &&
               source[index] != ')')
            index++;
        tokens.push_back(
            {source.substr(begin, index - begin),
             false, line});
    }
    return true;
}

bool parseNode(
    const std::string &name,
    const std::vector<Token> &tokens,
    size_t &index,
    AiNode &node,
    std::string *err) {
    if (index >= tokens.size()) {
        if (err)
            *err = name +
                   ": unexpected end of AI script";
        return false;
    }
    const Token &token = tokens[index++];
    node.line = token.line;
    if (token.text != "(") {
        if (token.text == ")") {
            if (err)
                *err = name + ":" +
                       std::to_string(token.line) +
                       ": unexpected ')'";
            return false;
        }
        node.value = token.text;
        node.quoted = token.quoted;
        return true;
    }
    while (index < tokens.size() &&
           tokens[index].text != ")") {
        AiNode child;
        if (!parseNode(
                name, tokens, index,
                child, err))
            return false;
        node.children.push_back(
            std::move(child));
    }
    if (index >= tokens.size()) {
        if (err)
            *err = name + ":" +
                   std::to_string(token.line) +
                   ": unterminated list";
        return false;
    }
    index++;
    if (node.children.empty()) {
        if (err)
            *err = name + ":" +
                   std::to_string(token.line) +
                   ": empty expression";
        return false;
    }
    return true;
}

} // namespace

std::string normalizeAiSymbol(
    const std::string &symbol) {
    std::string normalized;
    normalized.reserve(symbol.size());
    for (char character : symbol)
        normalized.push_back(
            (char)std::tolower(
                (unsigned char)character));
    return normalized;
}

bool AiProgram::load(
    const std::string &entryPath,
    const std::unordered_set<std::string> &defines,
    std::string *err) {
    rules.clear();
    constants.clear();
    files.clear();
    loaded_.clear();
    defines_.clear();
    for (const std::string &define : defines)
        defines_.insert(
            normalizeAiSymbol(define));
    constants = {
        {"no", 0},
        {"yes", 1},
        {"true", 1},
        {"false", 0},
        {"hardest", 0},
        {"hard", 1},
        {"moderate", 2},
        {"easy", 3},
        {"easiest", 4},
        {"tech-level-1", 1},
        {"tech-level-2", 2},
        {"tech-level-3", 3},
        {"tech-level-4", 4},
        {"food", 0},
        {"carbon", 1},
        {"metal", 2},
        {"ore", 2},
        {"nova", 3},
    };
    return loadFile(entryPath, err);
}

bool AiProgram::loadSource(
    const std::string &name,
    const std::string &source,
    const std::unordered_set<std::string> &defines,
    std::string *err) {
    rules.clear();
    constants.clear();
    files.clear();
    loaded_.clear();
    defines_.clear();
    for (const std::string &define : defines)
        defines_.insert(
            normalizeAiSymbol(define));
    constants = {
        {"no", 0},
        {"yes", 1},
        {"true", 1},
        {"false", 0},
        {"hardest", 0},
        {"hard", 1},
        {"moderate", 2},
        {"easy", 3},
        {"easiest", 4},
        {"tech-level-1", 1},
        {"tech-level-2", 2},
        {"tech-level-3", 3},
        {"tech-level-4", 4},
        {"food", 0},
        {"carbon", 1},
        {"metal", 2},
        {"ore", 2},
        {"nova", 3},
    };
    loaded_.insert(name);
    files.push_back(name);
    return processSource(
        name, directoryOf(name),
        source, err);
}

bool AiProgram::loadFile(
    const std::string &path,
    std::string *err) {
    std::string key =
        normalizeAiSymbol(path);
    std::replace(
        key.begin(), key.end(), '\\', '/');
    if (!loaded_.insert(key).second)
        return true;
    std::string source;
    if (!readTextFile(path, source, err))
        return false;
    files.push_back(path);
    return processSource(
        path, directoryOf(path),
        source, err);
}

bool AiProgram::processSource(
    const std::string &name,
    const std::string &directory,
    const std::string &source,
    std::string *err) {
    std::string processed;
    if (!preprocess(
            name, source, defines_,
            processed, err))
        return false;
    std::vector<Token> tokens;
    if (!tokenize(
            name, processed, tokens, err))
        return false;
    size_t index = 0;
    while (index < tokens.size()) {
        AiNode form;
        if (!parseNode(
                name, tokens, index,
                form, err))
            return false;
        if (!form.isList()) {
            if (err)
                *err = name + ":" +
                       std::to_string(form.line) +
                       ": expected top-level list";
            return false;
        }
        const std::string command =
            normalizeAiSymbol(form.name());
        if (command == "load") {
            if (form.children.size() != 2 ||
                form.children[1].value.empty()) {
                if (err)
                    *err = name + ":" +
                           std::to_string(
                               form.line) +
                           ": malformed load";
                return false;
            }
            std::string fileName =
                form.children[1].value;
            const std::string normalized =
                normalizeAiSymbol(fileName);
            if (normalized.size() < 4 ||
                normalized.substr(
                    normalized.size() - 4) !=
                    ".per")
                fileName += ".per";
            if (!loadFile(
                    joinPath(
                        directory, fileName),
                    err))
                return false;
            continue;
        }
        if (command == "defconst") {
            if (form.children.size() != 3) {
                if (err)
                    *err = name + ":" +
                           std::to_string(
                               form.line) +
                           ": malformed defconst";
                return false;
            }
            int value = 0;
            if (!parseInteger(
                    form.children[2].value,
                    value)) {
                const std::string reference =
                    normalizeAiSymbol(
                        form.children[2].value);
                const auto found =
                    constants.find(reference);
                if (found == constants.end()) {
                    if (err)
                        *err = name + ":" +
                               std::to_string(
                                   form.line) +
                               ": unresolved defconst value '" +
                               form.children[2].value +
                               "'";
                    return false;
                }
                value = found->second;
            }
            constants[
                normalizeAiSymbol(
                    form.children[1].value)] =
                value;
            continue;
        }
        if (command != "defrule")
            continue;
        size_t arrow = 0;
        for (size_t child = 1;
             child < form.children.size();
             ++child)
            if (!form.children[child].isList() &&
                form.children[child].value == "=>") {
                arrow = child;
                break;
            }
        if (arrow <= 1 ||
            arrow + 1 >=
                form.children.size()) {
            if (err)
                *err = name + ":" +
                       std::to_string(form.line) +
                       ": malformed defrule";
            return false;
        }
        AiRule rule;
        rule.source = name;
        rule.line = form.line;
        rule.conditions.assign(
            form.children.begin() + 1,
            form.children.begin() +
                (std::ptrdiff_t)arrow);
        rule.actions.assign(
            form.children.begin() +
                (std::ptrdiff_t)arrow + 1,
            form.children.end());
        rules.push_back(std::move(rule));
    }
    return true;
}

int AiProgram::constant(
    const std::string &name,
    int fallback) const {
    int number = 0;
    if (parseInteger(name, number))
        return number;
    const auto found =
        constants.find(
            normalizeAiSymbol(name));
    return found == constants.end()
               ? fallback
               : found->second;
}

int AiProgram::strategicNumberId(const std::string &name) const {
    if (hasConstant(name)) {
        const int id=constant(name);
        return id>=0 && id<242 ? id : -1;
    }
    // Original executable's 0x57fc8c registration table; defaults are separate state.
    static const std::unordered_map<std::string,int> ids={
        {"sn-percent-civilian-explorers",0},
        {"sn-percent-civilian-builders",1},
        {"sn-percent-civilian-gatherers",2},
        {"sn-cap-civilian-explorers",3},
        {"sn-cap-civilian-builders",4},
        {"sn-cap-civilian-gatherers",5},
        {"sn-minimum-attack-group-size",16},
        {"sn-minimum-air-attack-group-size",17},
        {"sn-total-number-explorers",18},
        {"sn-percent-enemy-sighted-response",19},
        {"sn-enemy-sighted-response-distance",20},
        {"sn-maximum-jedi-support-group-size",21},
        {"sn-holocron-return-distance",23},
        {"sn-minimum-defend-group-size",25},
        {"sn-maximum-attack-group-size",26},
        {"sn-maximum-air-attack-group-size",27},
        {"sn-maximum-defend-group-size",28},
        {"sn-minimum-peace-like-level",29},
        {"sn-percent-exploration-required",32},
        {"sn-zero-priority-distance",34},
        {"sn-minimum-civilian-explorers",35},
        {"sn-number-attack-groups",36},
        {"sn-number-defend-groups",38},
        {"sn-attack-group-gather-spacing",41},
        {"sn-number-explore-groups",42},
        {"sn-minimum-explore-group-size",43},
        {"sn-maximum-explore-group-size",44},
        {"sn-number-air-attack-groups",40},
        {"sn-nova-defend-priority",50},
        {"sn-metal-defend-priority",51},
        {"sn-forage-defend-priority",52},
        {"sn-town-defend-priority",56},
        {"sn-defense-distance",57},
        {"sn-number-boat-attack-groups",58},
        {"sn-minimum-boat-attack-group-size",59},
        {"sn-maximum-boat-attack-group-size",60},
        {"sn-number-boat-explore-groups",61},
        {"sn-minimum-boat-explore-group-size",62},
        {"sn-maximum-boat-explore-group-size",63},
        {"sn-number-boat-defend-groups",67},
        {"sn-minimum-boat-defend-group-size",68},
        {"sn-maximum-boat-defend-group-size",69},
        {"sn-dock-defend-priority",70},
        {"sn-sentry-distance-variation",72},
        {"sn-minimum-town-size",73},
        {"sn-maximum-town-size",74},
        {"sn-group-commander-selection-method",75},
        {"sn-consecutive-idle-unit-limit",76},
        {"sn-target-evaluation-distance",77},
        {"sn-target-evaluation-hitpoints",78},
        {"sn-target-evaluation-kills",80},
        {"sn-target-evaluation-ally-proximity",81},
        {"sn-target-evaluation-rof",82},
        {"sn-target-evaluation-randomness",83},
        {"sn-camp-max-distance",86},
        {"sn-mill-max-distance",87},
        {"sn-target-evaluation-attack-attempts",89},
        {"sn-target-evaluation-range",90},
        {"sn-defend-overlap-distance",92},
        {"sn-scale-minimum-attack-group-size",93},
        {"sn-scale-maximum-attack-group-size",94},
        {"sn-attack-group-size-randomness",98},
        {"sn-scaling-frequency",99},
        {"sn-maximum-gaia-attack-response",100},
        {"sn-build-frequency",101},
        {"sn-attack-separation-time-randomness",102},
        {"sn-attack-intelligence",103},
        {"sn-initial-attack-delay",104},
        {"sn-save-scenario-information",105},
        {"sn-special-attack-type1",106},
        {"sn-special-attack-influence1",109},
        {"sn-minimum-water-body-size-for-dock",112},
        {"sn-number-build-attempts-before-skip",114},
        {"sn-max-skips-per-attempt",115},
        {"sn-food-gatherer-percentage",117},
        {"sn-nova-gatherer-percentage",118},
        {"sn-metal-gatherer-percentage",119},
        {"sn-carbon-gatherer-percentage",120},
        {"sn-target-evaluation-continent",122},
        {"sn-target-evaluation-siege-weapon",123},
        {"sn-group-leader-defense-distance",131},
        {"sn-initial-attack-delay-type",134},
        {"sn-blot-exploration-map",135},
        {"sn-blot-size",136},
        {"sn-intelligent-gathering",142},
        {"sn-task-ungrouped-soldiers",143},
        {"sn-target-evaluation-boat",144},
        {"sn-number-enemy-objects-required",145},
        {"sn-number-max-skip-cycles",146},
        {"sn-retask-gather-amount",148},
        {"sn-max-build-plan-gatherer-percentage",160},
        {"sn-max-unpowered-buildings",162},
        {"sn-food-dropsite-distance",163},
        {"sn-carbon-dropsite-distance",164},
        {"sn-metal-dropsite-distance",165},
        {"sn-nova-dropsite-distance",166},
        {"sn-initial-exploration-required",167},
        {"sn-random-placement-factor",168},
        {"sn-required-forest-tiles",169},
        {"sn-attack-diplomacy-impact",178},
        {"sn-percent-half-exploration",179},
        {"sn-target-evaluation-time-kill-ratio",184},
        {"sn-target-evaluation-in-progress",185},
        {"sn-attack-winning-player",188},
        {"sn-coop-share-information",194},
        {"sn-attack-winning-player-factor",195},
        {"sn-coop-share-attacking",196},
        {"sn-coop-share-attacking-interval",197},
        {"sn-percentage-explore-exterminators",200},
        {"sn-minimum-dropsite-buffer",202},
        {"sn-use-by-type-max-gathering",203},
        {"sn-minimum-boar-hunt-group-size",204},
        {"sn-minimum-amount-for-trading",216},
        {"sn-easiest-reaction-percentage",218},
        {"sn-easier-reaction-percentage",219},
        {"sn-hits-before-alliance-change",221},
        {"sn-allow-civilian-defense",225},
        {"sn-number-forward-builders",226},
        {"sn-number-forward-gatherers",220},
        {"sn-percent-attack-soldiers",227},
        {"sn-percent-attack-boats",228},
        {"sn-percent-attack-air",223},
        {"sn-do-not-scale-for-difficulty-level",229},
        {"sn-group-form-distance",230},
        {"sn-ignore-attack-group-under-attack",231},
        {"sn-gather-defense-units",232},
        {"sn-maximum-carbon-drop-distance",233},
        {"sn-maximum-food-drop-distance",234},
        {"sn-maximum-fish-boat-drop-distance",236},
        {"sn-maximum-nova-drop-distance",237},
        {"sn-maximum-metal-drop-distance",238},
        {"sn-garrison-rams",240},
        {"sn-percent-animal-garrison",6},
        {"sn-do-not-transport-from-same-zone",241},
        {"sn-gather-idle-soldiers-at-center",239},
        {"sn-special-attack-type2",107},
        {"sn-special-attack-type3",108},
        {"sn-desired-number-transport-escorts",66},
    };
    const auto found=ids.find(normalizeAiSymbol(name));
    return found==ids.end() ? -1 : found->second;
}

bool AiProgram::hasConstant(
    const std::string &name) const {
    int number = 0;
    return parseInteger(name, number) ||
           constants.count(
               normalizeAiSymbol(name)) != 0;
}

} // namespace swgb
