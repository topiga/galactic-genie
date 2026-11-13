// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_map.h"
#include "rms_terrain.h"
#include "rms_terrain_cleanup.h"
#include "rms_early_forest.h"
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
    RmsResourceGroups grouping;
    grouping.mode=0;
    int32_t objectUnitId=348;
    std::vector<RmsLandGrowth> lands;
    std::vector<RmsTerrainRow> terrainRows;
    while (i < tokens.size()) {
        const std::string command = tokens[i++].text;
        if (command == "<PLAYER_SETUP>" && stage == 0 && !playerSetup) { playerSetup=true; continue; }
        // Original handler0x4e2c10 accepts this without changing the game setting.
        if (command == "random_placement" && stage == 0 && playerSetup) continue;
        if (command == "<LAND_GENERATION>" && stage == 0) { stage = 1; continue; }
        if (command == "<TERRAIN_GENERATION>" && stage == 1) { stage = 2; continue; }
        if (command == "<OBJECTS_GENERATION>" && (stage == 1 || stage == 2)) { stage = 3; continue; }
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
                else if (name == "other_zone_avoidance_distance" && value>=0 && value<=1024) land.separation=value;
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
        } else if (stage == 2 && command == "create_terrain") {
            if (terrainRows.size()==99 || !number(value) || value<0 || value>255 ||
                i==tokens.size() || tokens[i++].text!="{")
                return fail("expected terrain block, at most99 rows");
            RmsTerrainRow row;
            row.terrain=static_cast<uint8_t>(value);
            row.targetTiles=width;
            int32_t tileAmount=static_cast<int32_t>(width);
            int scaling=0;
            bool hasBase=false;
            while (i<tokens.size() && tokens[i].text!="}") {
                const auto name=tokens[i++].text;
                if (name=="set_scale_by_size") { scaling=1; continue; }
                if (name=="set_scale_by_groups") { scaling=2; continue; }
                if (name=="set_avoid_player_start_areas") { row.avoidPlayerStarts=true; continue; }
                if (!number(value)) return fail(name+" requires an integer");
                if (name=="base_terrain" && value>=0 && value<=255) {
                    row.baseTerrain=static_cast<uint8_t>(value); hasBase=true;
                }
                else if (name=="number_of_tiles" && value>=0 && uint32_t(value)<=width*height) tileAmount=value;
                else if (name=="land_percent" && value>=0 && value<=100) tileAmount=-value;
                else if (name=="number_of_clumps" && value>=0 && value<=999) row.clumps=value;
                else if (name=="spacing_to_other_terrain_types" && value>=0 && value<=1024) row.spacing=value;
                else if (name=="clumping_factor" && value>=0 && value<=100) row.clumping=value;
                else return fail("unsupported terrain parameter/value "+name);
            }
            if (i==tokens.size()) return fail("unterminated terrain block");
            ++i;
            if (!hasBase) return fail("terrain row requires explicit base_terrain");
            if (tileAmount<0 || scaling) {
                const uint64_t product=uint64_t(tileAmount<0?-tileAmount:tileAmount)*width*height;
                if (product>INT32_MAX) return fail("terrain scaling exceeds supported signed arithmetic");
                row.targetTiles=product/(tileAmount<0?100:10000);
            } else row.targetTiles=tileAmount;
            if (scaling==2) row.clumps=uint64_t(row.clumps)*width*height/10000;
            terrainRows.push_back(row);
        } else if (stage == 3 && command == "create_object" && !hasObjects) {
            if (!number(value) || (value!=348 && value!=66 && value!=102) || i==tokens.size() || tokens[i++].text!="{")
                return fail("only one Gaia348/66/102 object block is supported");
            objectUnitId=value;
            hasObjects=true;
            while (i<tokens.size() && tokens[i].text!="}") {
                const auto name=tokens[i++].text;
                if (name=="set_gaia_object_only") continue;
                if (name=="set_loose_grouping" || name=="set_tight_grouping") {
                    if (!grouping.mode) { grouping.perGroup=grouping.groups; grouping.groups=1; }
                    grouping.mode=name=="set_loose_grouping"?1:2;
                    continue;
                }
                if (!number(value) || value<0) return fail("unsupported object parameter/value " + name);
                if (name=="number_of_objects" && uint32_t(value)<=width*height) {
                    if (grouping.mode) grouping.perGroup=value; else grouping.groups=value;
                } else if (name=="number_of_groups" && uint32_t(value)<=width*height) {
                    if (!grouping.mode) { grouping.perGroup=grouping.groups; grouping.mode=1; }
                    grouping.groups=value;
                } else if (name=="group_variance" && value<=1024) grouping.variance=value;
                else if (name=="group_placement_radius" && value<=1024) grouping.radius=value;
                else return fail("unsupported object parameter/value " + name);
            }
            if (i==tokens.size()) return fail("unterminated object block");
            ++i;
        } else return fail("unsupported directive " + command);
    }
    if (base<0 || lands.empty()) return fail("explicit base terrain and at least one land required");
    auto safeTerrain = [&](uint8_t terrain, bool allowScenery=false) {
        if (terrain>52 || terrain>=data.terrainBlock.terrains.size()) return false;
        const auto &t=data.terrainBlock.terrains[terrain];
        const int alias=t.terrainToDraw;
        const bool validAlias=alias>=0 && size_t(alias)<data.terrainBlock.terrains.size() &&
            data.terrainBlock.terrains[alias].enabled==1 && data.terrainBlock.terrains[alias].terrainToDraw==-1;
        if (t.enabled!=1 || (alias!=-1 && !validAlias) || t.numTerrainUnitsUsed<0 || t.numTerrainUnitsUsed>30)
            return false;
        // Land/terrain rows write raw bytes; active bases need separate smoothing callbacks.
        if (allowScenery && (terrain==10 || terrain==13 || (terrain>=17 && terrain<=21) || terrain==49 || terrain==50)) return true;
        // Original 0x495020 skips the exact -1 sentinel before consulting other slot fields.
        for (int slot=0;slot<t.numTerrainUnitsUsed;++slot)
            if (t.terrainUnitId[slot]!=-1) return false;
        return true;
    };
    const bool earlyForest=base==10 || base==13 || (base>=17 && base<=21);
    if (!safeTerrain(static_cast<uint8_t>(base),earlyForest)) return fail("unsupported base terrain metadata");
    for (const auto &land:lands) if (!safeTerrain(land.terrain,true)) return fail("unsupported land terrain metadata");
    for (const auto &row:terrainRows)
        if (!safeTerrain(row.terrain,true) || !safeTerrain(row.baseTerrain,true)) return fail("unsupported terrain row metadata");
    if (earlyForest) {
        auto water=[](uint8_t id) { return id==1 || id==4 || id==22 || id==23; };
        for (const auto &land:lands) if (water(land.terrain))
            return fail("forest base with water requires later scenery setter callbacks");
        for (const auto &row:terrainRows) if (water(row.terrain) || water(row.baseTerrain))
            return fail("forest base with water requires later scenery setter callbacks");
    }
    RmsMap generated;
    std::vector<RmsTerrainProtection> protection;
    generated.width=width; generated.height=height;
    if (playerLands) {
        std::vector<RmsPlayerStart> starts;
        if (!generateRmsPlayerStarts(width,height,static_cast<int32_t>(lands[0].radius),playerCount,rng,starts,err)) return false;
        for (size_t owner=0;owner<starts.size();++owner) {
            lands[owner].centerX=static_cast<uint32_t>(starts[owner].x);
            lands[owner].centerY=static_cast<uint32_t>(starts[owner].y);
            generated.playerStarts.push_back(starts[owner]);
            protection.push_back({starts[owner].x,starts[owner].y,13,20});
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
    std::vector<uint32_t> earlyPaints;
    std::vector<RmsSceneryPlacement> early;
    if (!growRmsLands(width,height,static_cast<uint8_t>(base),lands,data.terrainBlock,rng,generated.terrain,err) ||
        !smoothRmsTerrain(width,height,static_cast<uint8_t>(base),generated.terrain,generated.terrain,err,
                          64*1024*1024,earlyForest?&earlyPaints:nullptr)) return false;
    if (earlyForest && !generateRmsEarlyForest(width,height,earlyPaints,data,rng,early,err,static_cast<uint8_t>(base))) return false;
    genieRandomDraw(rng);
    if (!generateRmsTerrain(width,height,terrainRows,rng,generated.terrain,err,protection)) return false;
    if (!smoothRmsTerrain(width,height,1,generated.terrain,generated.terrain,err) ||
        !cleanupRmsTerrain(width,height,generated.terrain,err)) return false;
    genieRandomDraw(rng);
    if (earlyForest) {
        if (!finishRmsEarlyForest(width,height,generated.terrain,data,early,rng,generated.scenery,err,static_cast<uint8_t>(base))) return false;
    } else if (!generateRmsScenery(width,height,generated.terrain,data,rng,generated.scenery,err)) return false;
    std::vector<uint32_t> occupied;
    for (const auto &tree:generated.scenery) if (tree.alive)
        occupied.push_back(static_cast<uint32_t>(tree.y)*width+static_cast<uint32_t>(tree.x));
    if (hasObjects) {
        const bool ok=grouping.mode ?
            generateRmsGroupedResourcePlacements(width,height,generated.terrain,data,objectUnitId,grouping,rng,generated.objects,err,occupied) :
            generateRmsResourcePlacements(width,height,generated.terrain,data,objectUnitId,grouping.groups,rng,generated.objects,err,occupied);
        if (!ok) return false;
    }
    genieRandomDraw(rng);
    randomState=rng;
    out=std::move(generated);
    return true;
}
} // namespace swgb
