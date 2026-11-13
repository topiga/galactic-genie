// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_map.h"
#include "../core/genie_dat.h"
#include "../core/genie_random.h"
#include <algorithm>
#include <charconv>

namespace swgb {
bool generateRmsMap(const RmsScript &script, const std::unordered_set<std::string> &defines,
                    uint32_t width, uint32_t height, const dat::DatFile &data,
                    uint32_t &randomState, RmsMap &out, std::string *err, uint32_t playerCount) {
    out = {};
    if (err) err->clear();
    std::vector<RmsToken> tokens;
    uint32_t rng = randomState;
    size_t i = 0;
    auto fail = [&](const std::string &reason) {
        if (err) {
            *err = "RMS map: " + reason;
            if (i && i <= tokens.size()) *err += " at resource " + std::to_string(tokens[i-1].resourceId) +
                                               ":" + std::to_string(tokens[i-1].line);
        }
        return false;
    };
    if (!width || !height || width > 1024 || height > 1024) return fail("dimensions must be 1..1024");
    if (!playerCount || playerCount>8) return fail("player count must be 1..8");
    if (defines.count("FIXED_POSITIONS")) return fail("FIXED_POSITIONS requires the unsupported nonrandomized game mode");
    if (!script.preprocess(defines,tokens,err,&rng)) return false;
    auto number = [&](int32_t &value) {
        if (i == tokens.size()) return false;
        const auto &text = tokens[i++].text;
        const auto result = std::from_chars(text.data(),text.data()+text.size(),value);
        return result.ec == std::errc{} && result.ptr == text.data()+text.size();
    };
    int stage = 0, base = -1;
    bool hasObjects = false;
    bool playerLands = false;
    bool playerSetup = false;
    uint32_t objectCount = 1;
    std::vector<RmsLandGrowth> lands;
    while (i < tokens.size()) {
        const std::string command = tokens[i++].text;
        if (command == "<PLAYER_SETUP>" && stage == 0 && !playerSetup) { playerSetup=true; continue; }
        // Original handler0x4e2c10 accepts this without changing the game setting.
        if (command == "random_placement" && stage == 0 && playerSetup) continue;
        if (command == "<LAND_GENERATION>" && stage == 0) { stage = 1; continue; }
        if (command == "<OBJECTS_GENERATION>" && stage == 1) { stage = 2; continue; }
        int32_t value = 0;
        if (stage == 1 && command == "base_terrain") {
            if (!number(value)) return fail("base_terrain requires an integer");
            base = static_cast<uint8_t>(value);
        } else if (stage == 1 && (command == "create_land" || command == "create_player_lands")) {
            const bool players=command=="create_player_lands";
            if (playerLands || (players && !lands.empty())) return fail("player lands cannot yet be mixed with other land blocks");
            if (lands.size() == 64 || i == tokens.size() || tokens[i++].text != "{")
                return fail("expected land block, at most64 lands");
            RmsLandGrowth land;
            land.zone=0; // Original nonplayer create_land default.
            land.radius=3;
            land.targetTiles=width*height;
            land.borderFuzziness=20;
            bool terrain=false,position=false;
            while (i < tokens.size() && tokens[i].text != "}") {
                const auto name = tokens[i++].text;
                if (!number(value)) return fail(name + " requires an integer");
                if (name == "terrain_type") { land.terrain=static_cast<uint8_t>(value); terrain=true; }
                else if (name == "land_position" && !players) {
                    int32_t y=0;
                    if (!number(y) || value<0 || value>99 || y<0 || y>99) return fail("positions must be 0..99");
                    land.centerX=width*static_cast<uint32_t>(value)/100;
                    land.centerY=height*static_cast<uint32_t>(y)/100;
                    position=true;
                } else if (name == "base_size" && value>=0 && value<=1024) land.radius=value;
                else if (!players && value>=0 && value<=100 &&
                         (name=="left_border" || name=="right_border" ||
                          name=="top_border" || name=="bottom_border")) {
                    // Right/bottom subtract a truncated margin, not round the remaining percentage.
                    if (name=="left_border") land.leftBorder=width*value/100;
                    else if (name=="right_border") land.rightBorder=width-width*value/100;
                    else if (name=="top_border") land.topBorder=height*value/100;
                    else land.bottomBorder=height-height*value/100;
                }
                else if (name == "number_of_tiles" && value>=0 && uint32_t(value)<=width*height) {
                    land.targetTiles=value;
                } else if (name == "land_percent" && value>=0 && value<=100) {
                    land.targetTiles=width*height*static_cast<uint32_t>(value)/(100u*(players?playerCount:1u));
                } else if (name == "clumping_factor" && value>=0 && value<=100) land.clumpingFactor=value;
                else if (name == "border_fuzziness" && value>=0 && value<=255) land.borderFuzziness=value;
                else if (name == "other_zone_avoidance_distance" && value==0) {}
                else if (name == "zone" && !players && value>=-10 && value<=53) land.zone=value+10;
                else return fail("unsupported land parameter/value " + name);
            }
            if (i == tokens.size()) return fail("unterminated land block");
            ++i;
            if (!terrain || (!players && !position)) return fail("land requires explicit terrain and nonplayer position");
            if (players) {
                playerLands=true;
                for (uint32_t owner=1;owner<=playerCount;++owner) { land.zone=owner; lands.push_back(land); }
            } else lands.push_back(land);
        } else if (stage == 2 && command == "create_object" && !hasObjects) {
            if (!number(value) || value!=348 || i==tokens.size() || tokens[i++].text!="{")
                return fail("only one default timber348 object block is supported");
            hasObjects=true;
            while (i<tokens.size() && tokens[i].text!="}") {
                const auto name=tokens[i++].text;
                if (name!="number_of_objects" || !number(value) || value<0 || uint32_t(value)>width*height)
                    return fail("unsupported object parameter/value " + name);
                objectCount=static_cast<uint32_t>(value);
            }
            if (i==tokens.size()) return fail("unterminated object block");
            ++i;
        } else return fail("unsupported directive " + command);
    }
    if (base<0 || lands.empty()) return fail("explicit base terrain and at least one land required");
    auto safeTerrain = [&](uint8_t terrain) {
        if ((terrain!=0 && terrain!=3) || terrain>=data.terrainBlock.terrains.size()) return false;
        const auto &t=data.terrainBlock.terrains[terrain];
        return t.enabled==1 && t.terrainToDraw==-1 && t.numTerrainUnitsUsed==0;
    };
    if (!safeTerrain(static_cast<uint8_t>(base))) return fail("unsupported base terrain metadata; require plain terrain0/3");
    for (const auto &land:lands) if (!safeTerrain(land.terrain)) return fail("unsupported land terrain metadata");
    RmsMap generated;
    generated.width=width; generated.height=height;
    if (playerLands) {
        std::vector<RmsPlayerStart> starts;
        if (!generateRmsPlayerStarts(width,height,static_cast<int32_t>(lands[0].radius),playerCount,rng,starts,err)) return false;
        for (size_t owner=0;owner<starts.size();++owner) {
            lands[owner].centerX=static_cast<uint32_t>(starts[owner].x);
            lands[owner].centerY=static_cast<uint32_t>(starts[owner].y);
            generated.playerStarts.push_back(starts[owner]);
        }
    } else {
        // Original setup still consumes permutation/offset draws without player lands.
        bool slots[8]={};
        size_t draws=0, unique=0;
        while (unique<8) {
            if (++draws>65536) return fail("player-order randomization work limit exceeded");
            const uint32_t slot=std::min(7u,genieRandomDraw(rng)*8u/32767u);
            if (!slots[slot]) { slots[slot]=true; ++unique; }
        }
        genieRandomDraw(rng);
    }
    genieRandomDraw(rng); // Scheduler logging before its first stage.
    if (!growRmsLands(width,height,static_cast<uint8_t>(base),lands,data.terrainBlock,rng,generated.terrain,err) ||
        !smoothRmsTerrain(width,height,static_cast<uint8_t>(base),generated.terrain,generated.terrain,err)) return false;
    genieRandomDraw(rng);
    // The auto-enabled empty terrain stage is a proven no-op on terrain0/3.
    genieRandomDraw(rng);
    if (hasObjects && !generateRmsTimberPlacements(width,height,generated.terrain,data,objectCount,rng,generated.timber,err))
        return false;
    genieRandomDraw(rng);
    randomState=rng;
    out=std::move(generated);
    return true;
}
} // namespace swgb
