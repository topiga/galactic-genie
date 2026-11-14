// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_map.h"
#include "rms_terrain.h"
#include "rms_terrain_cleanup.h"
#include "rms_early_forest.h"
#include "../core/genie_dat.h"
#include "../core/genie_random.h"
#include <algorithm>
#include <charconv>
#include <cmath>

namespace swgb {
bool generateRmsMap(const RmsScript &script, const std::unordered_set<std::string> &defines,
                    uint32_t width, uint32_t height, const dat::DatFile &data,
                    uint32_t &randomState, RmsMap &out, std::string *err, uint32_t playerCount,
                    const std::vector<uint16_t> &playerCivilizations) {
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
    bool ownedObjects = false;
    bool startingWorkers = false;
    bool startingScouts = false;
    bool nearNova = false;
    constexpr int scoutIds[]={550,552,556,554,258,558,927,932};
    int workerMinDistance=1,workerMaxDistance=3;
    bool startingStalagmites = false;
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
            if (!number(value) || value<0 || value>255) return fail("base_terrain requires an integer in0..255");
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
                if (name == "terrain_type" && value>=0 && value<=255) { land.terrain=static_cast<uint8_t>(value); terrain=true; }
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
                else if (name == "number_of_tiles" && value>=0) {
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
        } else if (stage==3 && command=="create_object" && startingScouts && !nearNova) {
            if (!number(value) || value!=66 || i==tokens.size() || tokens[i++].text!="{")
                return fail("only the verified near-Nova66 row may follow scouts");
            bool each=false,gaia=false,tight=false,count=false,inner=false,outer=false,spacing=false,zone=false;
            while (i<tokens.size() && tokens[i].text!="}") {
                const auto name=tokens[i++].text;
                if(name=="set_place_for_every_player") { each=true;continue; }
                if(name=="set_gaia_object_only") { gaia=true;continue; }
                if(name=="set_tight_grouping") { tight=true;continue; }
                if(!number(value)) return fail("missing near-Nova parameter value");
                if(name=="number_of_objects" && value==7) count=true;
                else if(name=="number_of_groups" && value==1) continue;
                else if(name=="group_placement_radius" && value==3) continue;
                else if(name=="min_distance_to_players" && value==12) inner=true;
                else if(name=="max_distance_to_players" && value==16) outer=true;
                else if(name=="min_distance_group_placement" && value==7) spacing=true;
                else if(name=="max_distance_to_other_zones" && value==7) zone=true;
                else return fail("unsupported near-Nova parameter/value "+name);
            }
            if(i==tokens.size() || !each || !gaia || !tight || !count || !inner || !outer || !spacing || !zone)
                return fail("near-Nova requires the verified tight7 per-player row");
            ++i;nearNova=true;
        } else if (stage==3 && command=="create_object" && startingWorkers && !startingScouts) {
            if (!number(value) || value!=550 || i==tokens.size() || tokens[i++].text!="{")
                return fail("only the starting550 scout row may follow workers");
            bool each=false,match=false,inner=false,outer=false;
            while (i<tokens.size() && tokens[i].text!="}") {
                const auto name=tokens[i++].text;
                if (name=="set_place_for_every_player") { each=true;continue; }
                if (name=="match_player_civ") { match=true;continue; }
                if (!number(value)) return fail("missing scout parameter value");
                if (name=="number_of_objects" && value==1) continue;
                if (name=="group_placement_radius" && value==3) continue;
                if (name=="min_distance_to_players" && value==7) inner=true;
                else if (name=="max_distance_to_players" && value==9) outer=true;
                else return fail("unsupported scout parameter/value "+name);
            }
            if (i==tokens.size() || !each || !match || !inner || !outer)
                return fail("scouts require every-player civilization matching and distances7..9");
            ++i;startingScouts=true;
        } else if (stage==3 && command=="create_object" && ownedObjects && !startingWorkers) {
            if (!number(value) || value!=83 || i==tokens.size() || tokens[i++].text!="{")
                return fail("only an83 worker row may follow owned109");
            // Original4e3850 initializes object quantity to1 before parsing attributes.
            bool count=true,each=false;
            int inner=-1,outer=-1,groupRadius=-1;
            while (i<tokens.size() && tokens[i].text!="}") {
                const auto name=tokens[i++].text;
                if (name=="set_place_for_every_player") { each=true; continue; }
                if (!number(value)) return fail("missing worker parameter value");
                if (name=="number_of_objects" && value==1) count=true;
                else if (name=="min_distance_to_players") inner=value;
                else if (name=="max_distance_to_players") outer=value;
                else if (name=="group_placement_radius" && value>=0) groupRadius=value;
                else return fail("unsupported worker parameter/value "+name);
            }
            const bool diagnostic=inner==1 && outer==3 && (groupRadius==-1 || groupRadius==18);
            const bool packaged=inner==6 && outer==6 && (groupRadius==-1 || groupRadius==3);
            if (i==tokens.size() || !count || !each || (!diagnostic && !packaged))
                return fail("workers require count1, every-player placement and verified distances1..3 or6..6");
            ++i; startingWorkers=true;workerMinDistance=inner;workerMaxDistance=outer;
        } else if (stage == 3 && command == "create_object" && !hasObjects) {
            if (!number(value) || (value!=348 && value!=66 && value!=102 && value!=109 && value!=573) || i==tokens.size() || tokens[i++].text!="{")
                return fail("expected Gaia348/66/102, owned109, or leading Gaia573 block");
            objectUnitId=value;
            hasObjects=true;
            ownedObjects=objectUnitId==109;
            if (ownedObjects || objectUnitId==573) {
                const bool stalagmite=objectUnitId==573;
                if (stalagmite && startingStalagmites) return fail("duplicate starting573 row");
                bool gaia=false;
                // Shared original object initializer also covers leading Gaia573.
                bool eachPlayer=false, count=true, inner=false, outer=false;
                while (i<tokens.size() && tokens[i].text!="}") {
                    const auto name=tokens[i++].text;
                    if (name=="set_place_for_every_player") { eachPlayer=true; continue; }
                    if (stalagmite && name=="set_gaia_object_only") { gaia=true; continue; }
                    if (!number(value)) return fail("missing owned109 parameter value");
                    if (name=="number_of_objects" && value==1) count=true;
                    else if (name=="min_distance_to_players" && value==0) inner=true;
                    else if (name=="max_distance_to_players" && value==0) outer=true;
                    else if (name!="group_placement_radius" || value!=18)
                        return fail("unsupported owned109 parameter/value " + name);
                }
                if (i==tokens.size()) return fail("unterminated owned109 block");
                ++i;
                if (!eachPlayer || !count || !inner || !outer)
                    return fail("owned109 requires every-player placement, count1 and explicit zero distances");
                if (stalagmite) {
                    if (!gaia) return fail("starting573 requires Gaia ownership");
                    startingStalagmites=true; hasObjects=false;
                }
                continue;
            }
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
    const bool ownedForest=ownedObjects && playerLands && base==41 && lands[0].terrain==9;
    if (startingStalagmites) {
        if (!ownedForest)
            return fail("starting573 requires forest Command Centers");
        if (data.civs.empty() || data.civs[0].units.size()<=573) return fail("missing starting573 prototype");
        const auto &u=data.civs[0].units[573];
        const auto *g=data.graphic(u.standingGraphic[0]);
        if (!u.exists || u.id!=573 || u.type!=10 || u.cls!=42 || u.hitPoints!=50 ||
            u.collisionSize[0]!=1 || u.collisionSize[1]!=1 || u.collisionSize[2]!=2 ||
            u.clearanceSize[0]!=1 || u.clearanceSize[1]!=1 || u.canBeBuiltOn!=1 ||
            u.lineOfSight || u.enabled || u.speed!=0 || u.trainSound!=-1 || u.terrainRestriction!=29 ||
            u.hillMode || u.flyMode || u.obstructionType!=2 || u.obstructionClass!=3 ||
            u.standingGraphic[0]!=8960 || u.dyingGraphic!=-1 || u.undeadGraphic!=-1 ||
            u.deadUnitId!=-1 || u.dyingSound!=-1 || u.enableAutoGather || u.createDoppelgangerOnDeath ||
            !g || g->sequenceType || g->frameCount!=1 || g->angleCount!=1 ||
            g->soundId!=-1 || !g->angleSounds.empty() || g->frameDuration!=.0465f || g->replayDelay!=0)
            return fail("unsupported starting573 cleanup metadata");
        for (const auto &resource:u.resourceStorages)
            if (resource.type!=-1 || resource.amount!=0 || resource.flag)
                return fail("starting573 resource callbacks are unsupported");
    }
    std::vector<uint8_t> foundations;
    if (ownedObjects) {
        if (width!=height || !playerLands || playerCivilizations.size()!=playerCount ||
            (!ownedForest && (base!=0 || !terrainRows.empty() || lands[0].terrain!=0)))
            return fail("owned109 requires explicit civilizations on supported square player lands");
        for (const auto ownedCiv:playerCivilizations) {
            if (!ownedCiv || ownedCiv>8 || ownedCiv>=data.civs.size() || data.civs[ownedCiv].units.size()<=109)
                return fail("owned109 requires an original playable civilization1..8");
            const auto &u=data.civs[ownedCiv].units[109];
            const int standing=ownedCiv<=6?2271+ownedCiv:7824+ownedCiv;
            const int construction=ownedCiv<=6?366+ownedCiv:7390+ownedCiv;
            const auto *graphic=data.graphic(standing), *buildingGraphic=data.graphic(construction);
            if (!u.exists || u.id!=109 || u.type!=dat::UT_Building || u.cls!=18 || u.hitPoints!=2500 ||
                u.lineOfSight!=8 || u.collisionSize[0]!=2 || u.collisionSize[1]!=2 || u.collisionSize[2]!=4 ||
                u.clearanceSize[0]!=2 || u.clearanceSize[1]!=2 || u.terrainRestriction!=4 || u.hillMode!=2 ||
                u.flyMode || u.canBeBuiltOn || u.trait || u.convertTerrain || u.obstructionType!=2 || u.obstructionClass!=3 ||
                u.speed!=0 || u.trackingUnit!=-1 || u.trackingUnitMode || u.trackingUnitDensity!=0 ||
                u.placementTerrain[0]!=-1 || u.placementTerrain[1]!=-1 ||
                u.placementSideTerrain[0]!=-1 || u.placementSideTerrain[1]!=-1 ||
                u.standingGraphic[0]!=standing || u.standingGraphic[1]!=-1 || u.constructionGraphic!=construction ||
                u.adjacentMode || u.graphicsAngle || u.disappearsWhenBuilt || u.stackUnitId!=-1 ||
                u.foundationTerrainId!=27 || u.oldOverlayId!=-1 || u.techId!=22 || u.transformUnit!=-1 ||
                u.headUnit!=621 || u.secondaryProjectileUnit!=54 || u.trainTime!=100 || u.trainSound!=-1 ||
                u.constructionSound!=597 || !graphic || graphic->sequenceType!=1 || graphic->frameCount!=8 ||
                graphic->angleCount!=1 || graphic->soundId!=-1 || !graphic->angleSounds.empty() ||
                !buildingGraphic || buildingGraphic->sequenceType!=11 || buildingGraphic->frameCount!=1 ||
                buildingGraphic->angleCount!=3 || buildingGraphic->soundId!=-1 || !buildingGraphic->angleSounds.empty())
                return fail("unsupported owned109 prototype or graphics");
            for (const auto &annex:u.annexes) if (annex.unitId!=-1)
                return fail("owned109 annexes are unsupported");
            if (data.terrainRestrictions.size()<=4 || data.terrainRestrictions[4].passableBuildableDmgMultiplier.empty() ||
                !(data.terrainRestrictions[4].passableBuildableDmgMultiplier[0]>0) ||
                !std::isfinite(data.terrainRestrictions[4].passableBuildableDmgMultiplier[0]))
                return fail("owned109 requires buildable grass");
            foundations.push_back(ownedCiv==2?4:ownedCiv==6?6:27);
        }
    }
    if (nearNova && (width!=32 || height!=32 || playerCount!=2 ||
        playerCivilizations!=std::vector<uint16_t>{1,1} || startingStalagmites))
        return fail("near-Nova currently requires two civilization1 owners on32-square grass or dry forest");
    if (startingWorkers) {
        for (const auto ownedCiv:playerCivilizations) {
            const auto &civ=data.civs[ownedCiv];
            if (civ.resources.size()<=84 || civ.resources[84]!=3 || civ.units.size()<=293 ||
                data.terrainRestrictions.size()<=25)
                return fail("unsupported worker startup resources");
            const auto &pass=data.terrainRestrictions[25].passableBuildableDmgMultiplier;
            for (int terrain:{0,4,6,9,10,27,41})
                if (size_t(terrain)>=pass.size() || !std::isfinite(pass[terrain]) || pass[terrain]<=0)
                    return fail("workers require connected passable grass and foundations");
            for (int id:{83,293}) {
                const auto &u=civ.units[id];
                const int standing=ownedCiv<=6?7016+ownedCiv:8624+ownedCiv;
                const auto *graphic=data.graphic(standing);
                if (!u.exists || u.id!=id || u.type!=dat::UT_Creatable || u.cls!=58 || u.hitPoints!=40 ||
                    u.collisionSize[0]!=.2f || u.collisionSize[1]!=.2f || u.collisionSize[2]!=2 ||
                    u.clearanceSize[0]!=.2f || u.clearanceSize[1]!=.2f || u.lineOfSight!=5 ||
                    u.terrainRestriction!=25 || u.headUnit!=-1 || u.trainSound!=570 || u.speed!=.8f ||
                    u.trackingUnit!=-1 || u.trackingUnitMode || u.trackingUnitDensity!=0 ||
                    u.flyMode || u.hillMode || u.trait!=16 || u.convertTerrain ||
                    u.standingGraphic[0]!=standing || !graphic || graphic->sequenceType!=7 ||
                    graphic->frameCount!=8 || graphic->angleCount!=8 || graphic->frameDuration!=1 ||
                    graphic->replayDelay!=3 || graphic->soundId!=-1 ||
                    !graphic->angleSounds.empty() || u.costs[0].type!=0 || u.costs[0].amount!=50 ||
                    (u.costs[0].flag&255)!=1 || u.resourceStorages[0].type!=4 ||
                    u.resourceStorages[0].amount!=-1 || u.resourceStorages[0].flag!=2 ||
                    u.resourceStorages[1].type!=11 || u.resourceStorages[1].amount!=1 ||
                    u.resourceStorages[1].flag!=2)
                    return fail("unsupported starting worker prototype or graphics");
            }
        }
    }
    if (startingScouts) {
        if (playerCount>2 || workerMinDistance!=6 || workerMaxDistance!=6)
            return fail("starting scouts require one or two owners and packaged worker distances6..6");
        constexpr int standing[]={6500,6510,6520,6515,6505,6525,9505,9510};
        constexpr int sounds[]={14,525,440,499,740,743,1133,1137};
        for (const auto civ:playerCivilizations) {
            const int id=scoutIds[civ-1],restriction=civ==2?7:25;
            if (data.civs[civ].units.size()<=size_t(id) || data.terrainRestrictions.size()<=size_t(restriction))
                return fail("missing scout prototype or terrain restriction");
            const auto &u=data.civs[civ].units[id];const auto *g=data.graphic(standing[civ-1]);
            if (!u.exists || u.id!=id || u.type!=dat::UT_Creatable || u.cls!=47 || u.hitPoints!=60 ||
                u.collisionSize[0]!=.25f || u.collisionSize[1]!=.25f || u.collisionSize[2]!=2 ||
                u.clearanceSize[0]!=.25f || u.clearanceSize[1]!=.25f || u.lineOfSight!=4 ||
                u.terrainRestriction!=restriction || u.headUnit!=-1 || u.trainSound!=sounds[civ-1] || u.speed!=2.2f ||
                u.trackingUnit!=-1 || u.trackingUnitMode || u.trackingUnitDensity!=0 ||
                u.flyMode || u.hillMode || u.trait!=(civ==2?40:(civ==5 || civ==8)?24:8) || u.convertTerrain ||
                u.standingGraphic[0]!=standing[civ-1] || !g || g->sequenceType!=(civ==4?6:7) ||
                g->frameCount!=(civ==2?8:1) || g->angleCount!=8 || g->frameDuration!=.2f ||
                g->replayDelay!=((civ==1 || civ==4 || civ==7)?0:3) || g->soundId!=-1 || !g->angleSounds.empty() ||
                u.costs[0].type!=1 || u.costs[0].amount!=50 || (u.costs[0].flag&255)!=1 ||
                u.resourceStorages[0].type!=4 || u.resourceStorages[0].amount!=-1 || u.resourceStorages[0].flag!=2 ||
                u.resourceStorages[1].type!=11 || u.resourceStorages[1].amount!=1 || u.resourceStorages[1].flag!=2)
                return fail("unsupported starting scout prototype or graphics");
            const auto &pass=data.terrainRestrictions[restriction].passableBuildableDmgMultiplier;
            for (int terrain:{0,4,6,9,10,27,41})
                if (size_t(terrain)>=pass.size() || !std::isfinite(pass[terrain]) || pass[terrain]<=0)
                    return fail("scouts require connected passable grass and foundations");
        }
    }
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
    for (const auto foundation:foundations)
        if (!safeTerrain(foundation)) return fail("unsupported owned109 foundation metadata");
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
    if (ownedObjects) {
        if (generated.playerStarts.size()!=playerCount) return fail("owned109 requires player starts");
        if (ownedForest) {
            if (std::any_of(generated.terrain.begin(),generated.terrain.end(),[](uint8_t t){return t!=41 && t!=9 && t!=6 && t!=10;}) ||
                std::any_of(generated.scenery.begin(),generated.scenery.end(),[](const auto &o){return o.unitId!=411 || !o.alive;}))
                return fail("owned forest starts require dry terrain and live411 scenery");
        } else if (!generated.scenery.empty() ||
                   std::any_of(generated.terrain.begin(),generated.terrain.end(),[](uint8_t tile){return tile!=0;}))
            return fail("owned109 requires scenery-free grass");
        // Fresh default startup retains zero owner visibility-query masks until after RMS.
        for (uint16_t owner=1;owner<=playerCount;++owner) {
            const auto &center=generated.playerStarts[owner-1];
            if (center.x<2 || center.y<2 || center.x>int32_t(width)-2 || center.y>int32_t(height)-2)
                return fail("owned109 footprint extends outside the map");
            if (ownedForest) {
                for (int y=center.y-2;y<center.y+2;++y) for (int x=center.x-2;x<center.x+2;++x) {
                    const auto tile=generated.terrain[size_t(y)*width+x];
                    if (tile!=9 && tile!=6 && tile!=10) return fail("owned forest starts require supported dry footprints");
                }
                // Forced radius0 CC placement retains non-clearable411 trees inside its footprint.
            }
        }
        for (uint16_t owner=1;owner<=playerCount;++owner) {
            const auto &center=generated.playerStarts[owner-1];
            // Radius0 has one candidate; original controller forces its final rejected gate too.
            for (int draw=0;draw<4;++draw) genieRandomDraw(rng);
            rng=genieRandomDraw(rng); // Completion reseeds the CRT with its fifth15-bit draw.
            const uint8_t foundation=foundations[owner-1];
            generated.ownedObjects.push_back({float(center.x),float(center.y),owner,
                                             playerCivilizations[owner-1],109,
                                             uint8_t(owner==1?255:27)});
            // Local completed109 paints its clearance footprint (557c10/557cf0).
            if (owner==1) for (int32_t y=center.y-2;y<center.y+2;++y)
                for (int32_t x=center.x-2;x<center.x+2;++x)
                    generated.terrain[size_t(y)*width+x]=foundation;
        }
        if (startingStalagmites) for (const auto &center:generated.playerStarts) {
            // shortcut: import after original owner-list deletion; intermediate state7/8 is not simulated.
            generated.scenery.push_back({573,float(center.x),float(center.y),0,0,false});
        }
        if (startingWorkers) for (uint16_t owner=1;owner<=playerCount;++owner) {
            const auto &center=generated.playerStarts[owner-1];
            std::vector<uint32_t> candidates;
            if (!generateRmsObjectCandidatesInArea(width,height,center.x,center.y,workerMaxDistance,rng,candidates,err)) return false;
            int remaining=3;
            int16_t unitId=83;
            for (const auto tile:candidates) {
                const int x=tile%width,y=tile/width;
                if (std::any_of(generated.playerStarts.begin(),generated.playerStarts.end(),[&](const auto &start) {
                    return std::abs(x-start.x)<workerMinDistance && std::abs(y-start.y)<workerMinDistance;
                })) continue;
                const float px=x+.5f,py=y+.5f;
                // Original RMS placement ignores existing workers, but blocks every owner's CC.
                const bool blocked=std::any_of(generated.ownedObjects.begin(),generated.ownedObjects.end(),[&](const auto &object) {
                    return object.unitId==109 && std::abs(px-object.x)<2.2f && std::abs(py-object.y)<2.2f;
                }) || std::binary_search(occupied.begin(),occupied.end(),tile);
                if (blocked && tile!=candidates.back()) continue;
                // Original forces the last rejected candidate without a gender draw or count decrement.
                if (!blocked && unitId==83 && genieRandomDraw(rng)>16383) unitId=293;
                uint8_t facet=0,frame=0;
                float delay=0;
                for (int draw=0;draw<7;++draw) {
                    const auto value=genieRandomDraw(rng);
                    if (draw==2) {
                        const float elapsed=float(value*11u)*(1.f/32767.f);
                        frame=uint8_t(std::min(7,int(elapsed)));
                        delay=std::max(0.f,elapsed-8.f);
                    }
                    if (draw==3) facet=uint8_t((value*8u/32767u+7)%8);
                }
                generated.ownedObjects.push_back({px,py,owner,playerCivilizations[owner-1],unitId,255,facet,frame,delay});
                if (blocked || !--remaining) break;
            }
        }
        if (startingScouts) for (uint16_t owner=1;owner<=playerCount;++owner) {
            const auto civ=playerCivilizations[owner-1];const int id=scoutIds[civ-1];
            const auto &u=data.civs[civ].units[id];const auto &g=*data.graphic(u.standingGraphic[0]);
            const auto &center=generated.playerStarts[owner-1];std::vector<uint32_t> candidates;
            if (!generateRmsObjectCandidatesInArea(width,height,center.x,center.y,9,rng,candidates,err)) return false;
            for (const auto tile:candidates) {
                const int x=tile%width,y=tile/width;
                if (std::any_of(generated.playerStarts.begin(),generated.playerStarts.end(),[&](const auto &start) {
                    return std::abs(x-start.x)<7 && std::abs(y-start.y)<7;
                })) continue;
                const float px=x+.5f,py=y+.5f;
                const bool blocked=std::any_of(generated.ownedObjects.begin(),generated.ownedObjects.end(),[&](const auto &o) {
                    return o.unitId==109 && std::abs(px-o.x)<2.25f && std::abs(py-o.y)<2.25f;
                }) || std::binary_search(occupied.begin(),occupied.end(),tile);
                if (blocked && tile!=candidates.back()) continue;
                // Original type70 construction: sprite/facing, action timers, then military timer41e410.
                genieRandomDraw(rng);genieRandomDraw(rng);
                uint8_t frame=0;float delay=0;
                if (g.sequenceType&1) {
                    const float cycle=g.frameCount*g.frameDuration+g.replayDelay;
                    const double elapsed=double(genieRandomDraw(rng))*cycle*(1.f/32767.f);
                    const int initial=int(elapsed/g.frameDuration);
                    frame=uint8_t(std::min(int(g.frameCount)-1,initial));
                    if (initial>=g.frameCount) {
                        const float hold=float(elapsed-double(g.frameCount-1)*g.frameDuration);
                        delay=std::max(0.f,hold-g.frameDuration);
                    }
                }
                const uint8_t facet=uint8_t((genieRandomDraw(rng)*8u/32767u+7)%8);
                for (int draw=0;draw<4;++draw) genieRandomDraw(rng);
                generated.ownedObjects.push_back({px,py,owner,civ,int16_t(id),255,facet,frame,delay});
                break;
            }
        }
        if (nearNova) {
            auto blockers=occupied;
            for(const auto &object:generated.ownedObjects) {
                if(object.unitId==109) {
                    for(int y=int(object.y)-2;y<int(object.y)+2;++y)
                        for(int x=int(object.x)-2;x<int(object.x)+2;++x) blockers.push_back(uint32_t(y)*width+x);
                } else blockers.push_back(uint32_t(object.y)*width+uint32_t(object.x));
            }
            RmsResourceGroups nova; nova.mode=2;nova.perGroup=7;
            if(!generateRmsGroupedResourcePlacements(width,height,generated.terrain,data,66,nova,rng,
                generated.resourcesAfterOwned,err,blockers,16000000,generated.playerStarts)) return false;
        }
    } else if (hasObjects) {
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
