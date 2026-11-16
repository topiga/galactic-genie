// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_map.h"
#include "rms_object_rows.h"
#include "rms_terrain.h"
#include "rms_elevation.h"
#include "rms_connections.h"
#include "rms_zones.h"
#include "rms_walls.h"
#include "rms_land_placement.h"
#include "rms_terrain_cleanup.h"
#include "rms_early_forest.h"
#include "../core/genie_dat.h"
#include "../core/genie_random.h"
#include <algorithm>
#include <charconv>
#include <cmath>
#include <unordered_map>

namespace swgb {
bool generateRmsMap(const RmsScript &script, const std::unordered_set<std::string> &defines,
                    uint32_t width, uint32_t height, const dat::DatFile &data,
                    uint32_t &randomState, RmsMap &out, std::string *err, uint32_t playerCount,
                    const std::vector<uint16_t> &playerCivilizations,
                    const std::vector<std::vector<uint8_t>> &diplomacy) {
    const auto isTree=[](int unit){return unit==348 || unit==349 || unit==350 || unit==351 || unit==411 || unit==413;};
    const auto isGaiaBuilding=[](int unit){return unit==1295 || unit==1052 || unit==1307 || (unit>=1184 && unit<=1188) || unit==275 || unit==518 || unit==519 || unit==524 || unit==536 || unit==537;};
    const auto isBigHunt=[](int unit){return unit==822 || unit==1249 || unit==1364;};
    const auto isPredator=[](int unit){return unit==1459 || unit==1461 || unit==1463 || unit==126 || unit==812 || unit==1197 || unit==1250 || unit==1129 || unit==1252 || unit==1251 || unit==1465 || unit==998 || unit==1033;};
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
    std::vector<int> connectionTeams(playerCount+1);
    bool allies=false;
    if(diplomacy.empty()) {
        for(uint32_t player=1;player<=playerCount;++player) connectionTeams[player]=static_cast<int>(player);
    } else {
        if(diplomacy.size()!=playerCount+1 || !resolveRmsConnectionTeams(diplomacy,connectionTeams,err))
            return fail("diplomacy must include Gaia and every configured player");
        for(uint32_t player=1;player<=playerCount;++player)
            for(uint32_t other=1;other<=playerCount;++other)
                allies|=player!=other && diplomacy[player][other]==0;
    }
    // Zone draws must precede later script random branches, including overwritten zones.
    std::unordered_map<size_t,std::vector<int>> randomLandZones;
    size_t emitted=0;
    uint32_t selectedLands=0;
    auto resolveZones=[&](const RmsToken &token,uint32_t &state) {
        if(token.text=="create_player_lands") selectedLands=playerCount;
        else if(token.text=="create_land") selectedLands=1;
        else if(token.text=="}") selectedLands=0;
        else if(token.text=="set_zone_randomly" && selectedLands) {
            auto &zones=randomLandZones[emitted];
            for(uint32_t n=0;n<selectedLands;++n)
                zones.push_back(static_cast<int>(genieRandomDraw(state)*playerCount/32767u)+2);
        }
        ++emitted;
    };
    if (!script.preprocess(defines,tokens,err,&rng,resolveZones)) return false;
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
    bool holocrons = false;
    std::vector<RmsObjectRow> resourceRows;
    constexpr int scoutIds[]={550,552,556,554,258,558,927,932};
    constexpr int transportIds[]={1036,1040,1046,1042,1038,1044,1521,1522};
    int workerMinDistance=1,workerMaxDistance=3,workerGroups=1,scoutGroups=1;
    bool startingStalagmites = false;
    bool playerLands = false;
    bool playerZoneOverride = false;
    bool playerSetup = false;
    RmsResourceGroups grouping;
    grouping.mode=0;
    int32_t objectUnitId=348;
    std::vector<RmsLandGrowth> lands;
    std::vector<RmsLandPlacement> placements;
    std::vector<RmsMapLand> landMetadata;
    size_t playerLandBegin=0;
    std::vector<RmsTerrainRow> terrainRows;
    std::vector<RmsElevationRow> elevationRows;
    bool elevationStage=false, cliffStage=false;
    RmsCliffSettings cliffSettings;
    std::vector<RmsConnectionCommand> connectionCommands;
    bool connectionStage=false;
    while (i < tokens.size()) {
        const std::string command = tokens[i++].text;
        if (command == "<PLAYER_SETUP>" && stage == 0 && !playerSetup) { playerSetup=true; continue; }
        // Original handler0x4e2c10 accepts this without changing the game setting.
        if (command == "random_placement" && stage == 0 && playerSetup) continue;
        if (command == "<LAND_GENERATION>" && stage == 0) { stage = 1; continue; }
        if (command == "<TERRAIN_GENERATION>" && stage>0) { stage = 2; continue; }
        if (command == "<OBJECTS_GENERATION>" && stage>0) { stage = 3; continue; }
        if (command == "<ELEVATION_GENERATION>" && stage>0) { stage=4; elevationStage=true; continue; }
        if (command == "<CONNECTION_GENERATION>" && stage>0) { stage=5; connectionStage=true; continue; }
        if (command == "<CLIFF_GENERATION>" && stage>0) { stage=6; cliffStage=true; continue; }
        int32_t value = 0;
        if (stage==6) {
            int *setting=nullptr;
            if(command=="min_number_of_cliffs") setting=&cliffSettings.minCount;
            else if(command=="max_number_of_cliffs") setting=&cliffSettings.maxCount;
            else if(command=="min_length_of_cliff") setting=&cliffSettings.minLength;
            else if(command=="max_length_of_cliff") setting=&cliffSettings.maxLength;
            else if(command=="cliff_curliness") setting=&cliffSettings.curl;
            else if(command=="min_distance_cliffs") setting=&cliffSettings.cliffDistance;
            else if(command=="min_terrain_distance") setting=&cliffSettings.waterDistance;
            if(!setting || !number(value)) return fail("unsupported cliff command or integer");
            *setting=value;
        } else if (stage == 1 && command == "base_terrain") {
            if (!number(value) || value<0 || value>255) return fail("base_terrain requires an integer in0..255");
            base = static_cast<uint8_t>(value);
        } else if (stage == 1 && (command == "create_land" || command == "create_player_lands")) {
            const bool players=command=="create_player_lands";
            if (players && playerLands) return fail("multiple player-land blocks remain unsupported");
            if (lands.size()+(players?playerCount:1)>64 || i == tokens.size() || tokens[i++].text != "{")
                return fail("expected land block, at most64 lands");
            RmsLandGrowth land;
            land.zone=0; // Original nonplayer create_land default.
            land.radius=3;
            land.targetTiles=width*height;
            land.borderFuzziness=20;
            bool terrain=false,position=false,zoneSpecified=false,teamZone=false;
            std::vector<int> randomZones;
            int landId=players?1:0,minPlacementDistance=-1;
            while (i < tokens.size() && tokens[i].text != "}") {
                const auto name = tokens[i++].text;
                if(name=="set_zone_by_team" && players) { teamZone=true;zoneSpecified=true;randomZones.clear();continue; }
                if(name=="set_zone_randomly") {
                    const auto found=randomLandZones.find(i-1);
                    if(found==randomLandZones.end() || found->second.size()!=(players?playerCount:1))
                        return fail("random zone requires a selected land block");
                    randomZones=found->second;land.zone=randomZones.front();
                    teamZone=false;zoneSpecified=true;continue;
                }
                if (!number(value)) return fail(name + " requires an integer");
                if (name == "terrain_type" && value>=0 && value<=255) { land.terrain=static_cast<uint8_t>(value); terrain=true; }
                else if (name == "land_position" && !players) {
                    int32_t y=0;
                    if (!number(y) || value<0 || value>99 || y<0 || y>99) return fail("positions must be 0..99");
                    land.centerX=width*static_cast<uint32_t>(value)/100;
                    land.centerY=height*static_cast<uint32_t>(y)/100;
                    position=true;
                } else if (name == "base_size" && value>=0 && value<=1024) land.radius=value;
                else if(name=="land_id" && !players && value>=0 && value<=32767) landId=value+10;
                else if(name=="min_placement_distance" && !players && value>=0 && value<=4096) minPlacementDistance=value;
                else if (value>=0 && value<=100 &&
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
                else if (name == "zone" && value>=-10 && value<=53) { land.zone=value+10;zoneSpecified=true;teamZone=false;randomZones.clear(); }
                else return fail("unsupported land parameter/value " + name);
            }
            if (i == tokens.size()) return fail("unterminated land block");
            ++i;
            if (!terrain) return fail("land requires explicit terrain");
            const RmsLandPlacement placement{position?int(land.centerX):-1,position?int(land.centerY):-1,
                int(land.radius),int(land.separation),land.leftBorder,land.topBorder,land.rightBorder,land.bottomBorder,minPlacementDistance};
            if (players) {
                playerLands=true;playerLandBegin=lands.size();playerZoneOverride=zoneSpecified;
                for (uint32_t owner=1;owner<=playerCount;++owner) {
                    if(!randomZones.empty()) land.zone=randomZones[owner-1];
                    else if(teamZone) {
                        uint32_t earlier=1;
                        while(earlier<owner && (diplomacy.empty() || diplomacy[earlier][owner]!=0)) ++earlier;
                        land.zone=earlier+1;
                    } else if(!zoneSpecified)land.zone=owner;
                    lands.push_back(land);placements.push_back(placement);
                    landMetadata.push_back({{},landId,int(owner)});
                }
            } else {
                lands.push_back(land);placements.push_back(placement);landMetadata.push_back({{},landId,0});
            }
        } else if (stage==4 && command=="create_elevation") {
            if (!number(value) || value<1 || value>7 || elevationRows.size()+value>99 ||
                i==tokens.size() || tokens[i++].text!="{") return fail("unsupported elevation block");
            const int levels=value;
            uint32_t target=width,clumps=1;
            int terrain=-1,scaling=0;
            while(i<tokens.size() && tokens[i].text!="}") {
                const auto name=tokens[i++].text;
                if(name=="set_scale_by_groups") { scaling=1;continue; }
                if(name=="set_scale_by_size") { scaling=2;continue; }
                if(!number(value)) return fail(name+" requires an integer");
                if(name=="base_terrain" && value>=0 && value<=255) terrain=value;
                else if(name=="number_of_tiles" && value>=0) target=value;
                else if(name=="number_of_clumps" && value>=0 && value<=999) clumps=value;
                else return fail("unsupported elevation parameter/value "+name);
            }
            if(i==tokens.size() || terrain<0) return fail("elevation requires a closed block and explicit base terrain");
            ++i;
            // Elevation scales tiles OR clumps, unlike the terrain handler.
            const uint64_t product=uint64_t(scaling==1?target:clumps)*width*height;
            if(scaling && product>INT32_MAX) return fail("elevation scaling exceeds supported signed arithmetic");
            if(scaling==1) target=product/10000;
            if(scaling==2) clumps=product/10000;
            for(int level=1;level<=levels;++level)
                elevationRows.push_back({target,static_cast<uint8_t>(level),clumps,level==1?2u:1u,
                    static_cast<uint8_t>(level-1),static_cast<uint8_t>(terrain)});
        } else if(stage==5 && (command=="create_connect_all_players_land" || command=="create_connect_teams_lands")) {
            if(connectionCommands.size()==99 || i==tokens.size() || tokens[i++].text!="{")
                return fail("expected connection block, at most99 rows");
            RmsConnectionCommand connection;
            connection.kind=command=="create_connect_all_players_land"?63:64;
            while(i<tokens.size() && tokens[i].text!="}") {
                const auto name=tokens[i++].text;
                int32_t terrain=0;
                if(!number(terrain) || terrain<0 || terrain>=99 || !number(value))
                    return fail("connection requires terrain0..98 and an integer value");
                auto &setting=connection.settings[terrain];
                if(name=="replace_terrain" && value>=0 && value<99) setting.replacement=value;
                else if(name=="terrain_cost" && value>=0 && value<=10000) setting.cost=static_cast<float>(value);
                else if(name=="terrain_size" && value>=0 && value<=1024) {
                    int32_t variance=0;
                    if(!number(variance) || variance<0 || variance>value) return fail("invalid connection width variance");
                    setting.width=value;setting.variance=variance;
                } else return fail("unsupported connection parameter/value "+name);
            }
            if(i==tokens.size()) return fail("unterminated connection block");
            ++i;connectionCommands.push_back(connection);
        } else if (stage == 2 && command == "create_terrain") {
            if (terrainRows.size()==99 || !number(value) || value<0 || value>255 ||
                i==tokens.size() || tokens[i++].text!="{")
                return fail("expected terrain block, at most99 rows");
            RmsTerrainRow row;
            row.terrain=static_cast<uint8_t>(value);
            row.flatTerrainOnly=value==1; // Original derived reader4e7bd0 defaults shallow-water patches to flat terrain.
            row.targetTiles=width;
            int32_t tileAmount=static_cast<int32_t>(width);
            int scaling=0;
            bool hasBase=false;
            while (i<tokens.size() && tokens[i].text!="}") {
                const auto name=tokens[i++].text;
                if (name=="set_scale_by_size") { scaling=1; continue; }
                if (name=="set_scale_by_groups") { scaling=2; continue; }
                if (name=="set_avoid_player_start_areas") { row.avoidPlayerStarts=true; continue; }
                if (name=="set_flat_terrain_only") { row.flatTerrainOnly=true; continue; }
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
        } else if (stage==3 && command=="{" && !resourceRows.empty()) {
            --i;
            if(!parseRmsObjectRow(tokens,i,resourceRows.back(),err)) return false;
        } else if (stage==3 && command=="create_object" && startingScouts) {
            RmsObjectRow row;
            if(!parseRmsObjectRow(tokens,i,row,err)) return false;
            if((row.matchPlayerCiv && row.unitId!=550 && row.unitId!=1521) || row.owner>0)
                return fail("unsupported resource owner or land selector");
            if(row.landId < -2) return fail("unsupported resource land selector");
            holocrons |= row.unitId==285;
            resourceRows.push_back(row);
            nearNova=!holocrons && width==32;
        } else if (stage==3 && command=="create_object" && startingWorkers && !startingScouts) {
            RmsObjectRow row;
            if(!parseRmsObjectRow(tokens,i,row,err))return false;
            if(row.unitId!=550 || row.owner!=-1 || row.landId!=1 || !row.matchPlayerCiv || row.mode || row.perGroup!=1 ||
               row.variance || row.radius!=3 || row.minDistance!=7 || row.maxDistance!=9 || row.terrain!=-1 ||
               row.spacing || row.temporarySpacing || row.zoneDistance)
                return fail("scouts require ungrouped every-player civilization matching and distances7..9");
            RmsObjectRow finalized;
            if(!finalizeRmsObjectRow(row,width,height,playerCount,finalized,err))return false;
            scoutGroups=finalized.groups;startingScouts=true;
        } else if (stage==3 && command=="create_object" && ownedObjects && !startingWorkers) {
            RmsObjectRow row;
            if(!parseRmsObjectRow(tokens,i,row,err))return false;
            const bool diagnostic=row.minDistance==1 && row.maxDistance==3 && (row.radius==3 || row.radius==18);
            const bool packaged=row.minDistance==6 && row.maxDistance==6 && row.radius==3;
            if(row.unitId!=83 || row.owner!=-1 || row.landId!=1 || row.matchPlayerCiv || row.mode || row.perGroup!=1 ||
               row.variance || row.terrain!=-1 || row.spacing || row.temporarySpacing || row.zoneDistance || (!diagnostic && !packaged))
                return fail("workers require ungrouped every-player placement and verified distances1..3 or6..6");
            RmsObjectRow finalized;
            if(!finalizeRmsObjectRow(row,width,height,playerCount,finalized,err))return false;
            workerGroups=finalized.groups;startingWorkers=true;workerMinDistance=row.minDistance;workerMaxDistance=row.maxDistance;
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
                    else if (name!="group_placement_radius" || (value!=18 && (!ownedObjects || value!=14)))
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
    const bool flatTatooine=[&]() {
        if(width!=120 || height!=120 || base!=14 || !playerLands || playerLandBegin || lands.size()!=2 ||
           playerCount!=2 || playerZoneOverride || !ownedObjects || !startingWorkers || !startingScouts ||
           workerGroups!=1 || scoutGroups!=1 || workerMinDistance!=6 || workerMaxDistance!=6 || startingStalagmites ||
           playerCivilizations!=std::vector<uint16_t>{1,1} || !defines.count("FIXED_POSITIONS") ||
           cliffStage || elevationStage || connectionStage || terrainRows.size()!=5)return false;
        for(size_t n=0;n<lands.size();++n) {
            const auto &land=lands[n];const auto &position=placements[n];const auto &metadata=landMetadata[n];
            if(land.terrain!=14 || land.radius!=9 || land.targetTiles!=1800 || land.zone!=int(n+1) ||
               land.clumpingFactor!=8 || land.borderFuzziness!=15 || land.separation ||
               land.leftBorder || land.topBorder || land.rightBorder!=-1 || land.bottomBorder!=-1 ||
               position.x!=-1 || position.y!=-1 || position.minPlacementDistance!=-1 ||
               metadata.id!=1 || metadata.owner!=int(n+1))return false;
        }
        const uint32_t terrain[]={50,46,50,45,46},tiles[]={1728,2880,144,288,288},clumps[]={17,16,7,24,30},spacing[]={5,0,3,1,1};
        for(size_t n=0;n<terrainRows.size();++n) {
            const auto &row=terrainRows[n];
            if(row.terrain!=terrain[n] || row.baseTerrain!=14 || row.targetTiles!=tiles[n] || row.clumps!=clumps[n] ||
               row.spacing!=spacing[n] || row.clumping!=20 || row.avoidPlayerStarts!=(n==0 || n==2) ||
               row.flatTerrainOnly!=(row.terrain==1) || row.minHeight || row.maxHeight!=255)return false;
        }
        return true;
    }();
    const bool mixedLandCliffs=[&]() {
        const bool water=base==6;
        if(width!=120 || height!=120 || (base!=0 && !water) || !playerLands || playerLandBegin!=2 ||
           playerCount!=2 || lands.size()!=4 || playerZoneOverride==water || !ownedObjects ||
           !startingWorkers || !startingScouts || workerGroups!=1 || scoutGroups!=1 ||
           workerMinDistance!=6 || workerMaxDistance!=6 || startingStalagmites ||
           playerCivilizations!=std::vector<uint16_t>{1,1} || !defines.count("FIXED_POSITIONS") ||
           !cliffStage || !elevationStage || connectionStage || terrainRows.size()!=6 || elevationRows.size()!=12 ||
           cliffSettings.minCount!=5 || cliffSettings.maxCount!=10 || cliffSettings.minLength!=4 ||
           cliffSettings.maxLength!=10 || cliffSettings.curl!=10 || cliffSettings.waterDistance!=2 || cliffSettings.cliffDistance!=3)return false;
        for(size_t n=0;n<lands.size();++n) {
            const auto &land=lands[n];const auto &position=placements[n];const auto &metadata=landMetadata[n];
            const bool neutral=n<2;const int border=n==0?18:n==1?48:0;
            if(land.terrain!=(water?(n==0?1:n==1?0:12):(neutral?6:0)) || land.radius!=(neutral?3u:9u) ||
               land.targetTiles!=(n==0?12240u:n==1?720u:3600u) || land.zone!=(n==0?26:n==1?25:int(n)-(water?1:0)) ||
               land.clumpingFactor!=8 || land.borderFuzziness!=20 || land.separation!=(neutral?0u:7u) ||
               land.leftBorder!=border || land.topBorder!=border || land.rightBorder!=(neutral?120-border:-1) ||
               land.bottomBorder!=(neutral?120-border:-1) || position.x!=-1 || position.y!=-1 ||
               position.minPlacementDistance!=(n==0?100:-1) || metadata.owner!=(neutral?0:int(n-1)) ||
               metadata.id!=(n==0?0:n==1?32:1))return false;
        }
        const uint32_t terrain[2][6]={{18,18,1,22,45,12},{22,23,22,21,21,6}};
        const uint32_t bases[2][6]={{0,0,0,1,6,0},{1,22,23,12,12,12}};
        const uint32_t tiles[2][6]={{1728,720,144,144,1152,288},{5760,3600,144,1440,576,288}};
        const uint32_t clumps[2][6]={{14,28,4,4,24,24},{3,6,5,14,28,24}},spacing[2][6]={{5,5,1,1,1,1},{1,2,1,3,3,1}};
        for(size_t n=0;n<terrainRows.size();++n) {
            const auto &row=terrainRows[n];
            if(row.terrain!=terrain[water][n] || row.baseTerrain!=bases[water][n] || row.targetTiles!=tiles[water][n] ||
               row.clumps!=clumps[water][n] || row.spacing!=spacing[water][n] || row.clumping!=20 ||
               row.avoidPlayerStarts!=(water?(n==3 || n==4):n<4) || row.flatTerrainOnly!=(row.terrain==1 || (!water && (n==2 || n==3))) || row.minHeight || row.maxHeight!=255)return false;
        }
        for(size_t n=0;n<elevationRows.size();++n) {
            const auto &row=elevationRows[n];const auto level=n<5?n:n-5;
            if(row.elevation!=level+1 || row.baseHeight!=level || row.baseTerrain!=(water?(n<5?12u:0u):(n<5?0u:6u)) ||
               row.targetTiles!=2000 || row.clumps!=20 || row.spacing!=(level==0?2u:1u))return false;
        }
        return true;
    }();
    const bool lakeCliffs=[&]() {
        const int biome=base==0?0:base==41?1:base==26?2:-1;
        if(biome<0 || width!=120 || height!=120 || !playerLands || playerLandBegin!=1 || lands.size()!=3 ||
           playerCount!=2 || !playerZoneOverride || !ownedObjects || !startingWorkers || !startingScouts ||
           workerGroups!=1 || scoutGroups!=1 || workerMinDistance!=6 || workerMaxDistance!=6 || startingStalagmites ||
           playerCivilizations!=std::vector<uint16_t>{1,1} || !defines.count("FIXED_POSITIONS") ||
           !cliffStage || !elevationStage || connectionStage || terrainRows.size()!=8 || elevationRows.size()!=12 ||
           cliffSettings.minCount!=5 || cliffSettings.maxCount!=10 || cliffSettings.minLength!=4 || cliffSettings.maxLength!=10 ||
           cliffSettings.curl!=10 || cliffSettings.waterDistance!=2 || cliffSettings.cliffDistance!=3)return false;
        for(size_t n=0;n<lands.size();++n) {
            const auto &land=lands[n];const auto &position=placements[n];const auto &metadata=landMetadata[n];
            const bool neutral=n==0;
            if(land.terrain!=(neutral?1u:uint32_t(base)) || land.radius!=(neutral?3u:9u) ||
               land.targetTiles!=(neutral?12240u:3600u) || land.zone!=(neutral?26:int(n+1)) ||
               land.clumpingFactor!=8 || land.borderFuzziness!=(neutral?40u:20u) || land.separation!=(neutral?0u:7u) ||
               land.leftBorder!=(neutral?12:0) || land.topBorder!=(neutral?12:0) ||
               land.rightBorder!=(neutral?108:-1) || land.bottomBorder!=(neutral?108:-1) ||
               position.x!=-1 || position.y!=-1 || position.minPlacementDistance!=-1 ||
               metadata.id!=(neutral?0:1) || metadata.owner!=int(n))return false;
        }
        const uint32_t terrain[3][8]={{22,23,22,18,12,18,6,45},{22,23,22,10,9,10,6,44},{22,23,22,19,46,50,45,14}};
        const uint32_t tiles[]={5760,3600,144,1296,864,144,288,288},clumps[]={3,6,6,17,8,4,24,30},spacing[]={1,3,1,5,0,3,0,0};
        for(size_t n=0;n<terrainRows.size();++n) {
            const auto &row=terrainRows[n];
            if(row.terrain!=terrain[biome][n] || row.baseTerrain!=(n==0?1u:n==1?22u:n==2?23u:uint32_t(base)) ||
               row.targetTiles!=tiles[n] || row.clumps!=clumps[n] || row.spacing!=spacing[n] || row.clumping!=20 ||
               row.avoidPlayerStarts!=(n==3 || n==5) || row.flatTerrainOnly!=(row.terrain==1) || row.minHeight || row.maxHeight!=255)return false;
        }
        for(size_t n=0;n<elevationRows.size();++n) {
            const auto &row=elevationRows[n];const auto level=n<5?n:n-5;
            if(row.elevation!=level+1 || row.baseHeight!=level || row.baseTerrain!=uint32_t(base) ||
               row.targetTiles!=2000 || row.clumps!=20 || row.spacing!=(level==0?2u:1u))return false;
        }
        return true;
    }();
    const bool migrationCliffs=[&]() {
        if(width!=120 || height!=120 || base!=1 || !playerLands || playerLandBegin || lands.size()!=3 ||
           playerCount!=2 || playerZoneOverride || !ownedObjects || !startingWorkers || !startingScouts ||
           workerGroups!=1 || scoutGroups!=1 || workerMinDistance!=6 || workerMaxDistance!=6 || startingStalagmites ||
           playerCivilizations!=std::vector<uint16_t>{1,1} || !defines.count("FIXED_POSITIONS") ||
           !cliffStage || !elevationStage || connectionStage || terrainRows.size()!=7 || elevationRows.size()!=7 ||
           cliffSettings.minCount!=3 || cliffSettings.maxCount!=5 || cliffSettings.minLength!=4 || cliffSettings.maxLength!=5 ||
           cliffSettings.curl!=10 || cliffSettings.waterDistance!=2 || cliffSettings.cliffDistance!=3)return false;
        const uint32_t ground=lands[0].terrain;
        if(ground!=0 && ground!=12 && ground!=41)return false;
        for(size_t n=0;n<lands.size();++n) {
            const auto &land=lands[n];const auto &position=placements[n];const auto &metadata=landMetadata[n];const bool neutral=n==2;
            if(land.terrain!=ground || land.radius!=(neutral?3u:9u) || land.targetTiles!=(neutral?5760u:1080u) ||
               land.zone!=(neutral?0:int(n+1)) || land.clumpingFactor!=(neutral?8u:15u) ||
               land.borderFuzziness!=(neutral?10u:7u) || land.separation!=(neutral?10u:7u) ||
               position.x!=-1 || position.y!=-1 || position.minPlacementDistance!=-1 ||
               metadata.id!=(neutral?31:1) || metadata.owner!=(neutral?0:int(n+1)))return false;
            if(neutral) {
                if(!((land.leftBorder==15 && land.rightBorder==108) || (land.leftBorder==12 && land.rightBorder==105)) ||
                   !((land.topBorder==15 && land.bottomBorder==108) || (land.topBorder==12 && land.bottomBorder==105)))return false;
            } else if(land.leftBorder || land.topBorder || land.rightBorder!=-1 || land.bottomBorder!=-1)return false;
        }
        const uint32_t forest=ground==0?18:ground==12?20:10;
        const uint32_t terrain[]={22,23,forest,ground==0?12u:9u,forest,6,45},tiles[]={8640,1440,864,1296,144,288,288},clumps[]={10,6,20,8,4,24,30},spacing[]={2,5,5,0,3,0,0};
        for(size_t n=0;n<terrainRows.size();++n) {
            const auto &row=terrainRows[n];
            if(row.terrain!=terrain[n] || row.baseTerrain!=(n==0?1u:n==1?22u:ground) || row.targetTiles!=tiles[n] ||
               row.clumps!=clumps[n] || row.spacing!=spacing[n] || row.clumping!=20 || row.avoidPlayerStarts!=(n==2 || n==4) ||
               row.flatTerrainOnly!=(row.terrain==1) || row.minHeight || row.maxHeight!=255)return false;
        }
        for(size_t n=0;n<elevationRows.size();++n) {
            const auto &row=elevationRows[n];
            if(row.elevation!=n+1 || row.baseHeight!=n || row.baseTerrain!=ground || row.targetTiles!=2000 ||
               row.clumps!=20 || row.spacing!=(n==0?2u:1u))return false;
        }
        return true;
    }();
    const bool connectedCliffs=[&]() {
        if(width!=120 || height!=120 || base!=1 || !playerLands || playerLandBegin || lands.size()!=2 ||
           playerCount!=2 || !playerZoneOverride || !ownedObjects || !startingWorkers || !startingScouts ||
           workerGroups!=1 || scoutGroups!=1 || workerMinDistance!=6 || workerMaxDistance!=6 || startingStalagmites ||
           playerCivilizations!=std::vector<uint16_t>{1,1} || !defines.count("FIXED_POSITIONS") ||
           !cliffStage || !elevationStage || !connectionStage || terrainRows.size()!=8 || elevationRows.size()!=7 ||
           connectionCommands.size()!=1 || connectionCommands[0].kind!=63 ||
           cliffSettings.minCount!=5 || cliffSettings.maxCount!=8 || cliffSettings.minLength!=4 || cliffSettings.maxLength!=10 ||
           cliffSettings.curl!=10 || cliffSettings.waterDistance!=2 || cliffSettings.cliffDistance!=3)return false;
        const uint32_t ground=lands[0].terrain;
        const int biome=ground==12?0:ground==9?1:ground==6?2:ground==41?3:-1;
        if(biome<0)return false;
        for(size_t n=0;n<lands.size();++n) {
            const auto &land=lands[n];const auto &position=placements[n];const auto &metadata=landMetadata[n];
            if(land.terrain!=ground || land.radius!=13 || land.targetTiles!=6840 || land.clumpingFactor!=8 ||
               land.borderFuzziness!=15 || land.zone<2 || land.zone>4 ||
               (land.separation!=7 && land.separation!=10) || land.separation!=lands[0].separation ||
               land.leftBorder || land.topBorder || land.rightBorder!=-1 || land.bottomBorder!=-1 ||
               position.x!=-1 || position.y!=-1 || position.minPlacementDistance!=-1 || metadata.id!=1 || metadata.owner!=int(n+1))return false;
        }
        const uint32_t forest[]={20,21,18,10},overlay[]={9,12,9,9},soil[]={6,6,12,6},last[]={45,9,6,44};
        const uint32_t terrain[]={22,23,forest[biome],forest[biome],overlay[biome],forest[biome],soil[biome],last[biome]};
        const uint32_t tiles[]={144,1440,1296,1152,864,144,288,288},clumps[]={2,10,5,14,8,4,24,30},spacing[]={2,2,5,5,0,3,0,0};
        for(size_t n=0;n<terrainRows.size();++n) {
            const auto &row=terrainRows[n];
            if(row.terrain!=terrain[n] || row.baseTerrain!=(n==0?1u:n==1?22u:ground) ||
               row.targetTiles!=tiles[n] || row.clumps!=clumps[n] || row.spacing!=spacing[n] || row.clumping!=20 ||
               row.avoidPlayerStarts!=(n==2 || n==3 || n==5) || row.flatTerrainOnly!=(row.terrain==1) || row.minHeight || row.maxHeight!=255)return false;
        }
        for(size_t n=0;n<elevationRows.size();++n) {
            const auto &row=elevationRows[n];
            if(row.elevation!=n+1 || row.baseHeight!=n || row.baseTerrain!=ground || row.targetTiles!=2000 ||
               row.clumps!=20 || row.spacing!=(n==0?2u:1u))return false;
        }
        RmsConnectionSettings expected{};
        for(int terrain:{1,22,23}) {expected[terrain].replacement=4;expected[terrain].width=2;expected[terrain].variance=1;}
        for(int terrain:{1,2,18,20,10,21})expected[terrain].cost=7;
        expected[22].cost=9;expected[23].cost=15;expected[4].cost=3;
        for(int terrain:{12,9,41})expected[terrain].cost=2;
        for(int terrain:{12,9,41,6,18,20,10,21,45,44})expected[terrain].width=0;
        for(size_t n=0;n<expected.size();++n) {
            const auto &a=expected[n],&b=connectionCommands[0].settings[n];
            if(a.cost!=b.cost || a.width!=b.width || a.variance!=b.variance || a.replacement!=b.replacement)return false;
        }
        return true;
    }();
    const bool motherlode=[&]() {
        if(width!=120 || height!=120 || (base!=46 && base!=0 && base!=41) || !playerLands || playerLandBegin!=2 || lands.size()!=4 ||
           playerCount!=2 || !playerZoneOverride || !ownedObjects || !startingWorkers || !startingScouts ||
           workerGroups!=1 || scoutGroups!=1 || workerMinDistance!=6 || workerMaxDistance!=6 || startingStalagmites ||
           playerCivilizations!=std::vector<uint16_t>{1,1} || !defines.count("FIXED_POSITIONS") ||
           cliffStage || !elevationStage || connectionStage || terrainRows.size()!=6 || elevationRows.size()!=7)return false;
        const uint32_t ground=static_cast<uint32_t>(base),forest=base==46?50:base==0?18:10;
        for(size_t n=0;n<lands.size();++n) {
            const auto &land=lands[n];const auto &position=placements[n];const auto &metadata=landMetadata[n];const bool neutral=n<2;
            const int border=n==0?24:n==1?48:0;
            if(land.terrain!=(n==0?forest:n==1?1:ground) || land.radius!=(neutral?3u:7u) ||
               land.targetTiles!=(n==0?8640u:n==1?720u:2160u) || land.zone!=(n==0?26:n==1?25:11) ||
               land.clumpingFactor!=8 || land.borderFuzziness!=(n==0?90u:n==1?20u:10u) || land.separation!=(neutral?0u:5u) ||
               land.leftBorder!=border || land.topBorder!=border || land.rightBorder!=(neutral?120-border:-1) ||
               land.bottomBorder!=(neutral?120-border:-1) || position.x!=-1 || position.y!=-1 || position.minPlacementDistance!=-1 ||
               metadata.id!=(neutral?0:1) || metadata.owner!=(neutral?0:int(n-1)))return false;
        }
        const uint32_t terrain[]={22,23,forest,base==46?45u:base==0?12u:9u,base==46?14u:6u,base==41?44u:45u};
        const uint32_t bases[]={1,22,ground,ground,ground,ground};
        const uint32_t tiles[]={288,144,2160,1296,288,288},clumps[]={3,6,14,8,24,30},spacing[]={1,5,5,1,1,1};
        for(size_t n=0;n<terrainRows.size();++n) {
            const auto &row=terrainRows[n];
            if(row.terrain!=terrain[n] || row.baseTerrain!=bases[n] || row.targetTiles!=tiles[n] || row.clumps!=clumps[n] ||
               row.spacing!=spacing[n] || row.clumping!=20 || row.avoidPlayerStarts!=(n==2) ||
               row.flatTerrainOnly!=(row.terrain==1) || row.minHeight || row.maxHeight!=255)return false;
        }
        for(size_t n=0;n<elevationRows.size();++n) {
            const auto &row=elevationRows[n];
            if(row.elevation!=n+1 || row.baseHeight!=n || row.baseTerrain!=ground || row.targetTiles!=2000 ||
               row.clumps!=20 || row.spacing!=(n==0?2u:1u))return false;
        }
        return true;
    }();
    const bool centralLakeCliffs=[&]() {
        if(width!=120 || height!=120 || (base!=0 && base!=26) || !playerLands || playerLandBegin!=1 || lands.size()!=3 ||
           playerCount!=2 || !playerZoneOverride || !ownedObjects || !startingWorkers || !startingScouts ||
           workerGroups!=1 || scoutGroups!=1 || workerMinDistance!=6 || workerMaxDistance!=6 || startingStalagmites ||
           playerCivilizations!=std::vector<uint16_t>{1,1} || !defines.count("FIXED_POSITIONS") ||
           !cliffStage || !elevationStage || connectionStage || terrainRows.size()!=10 || elevationRows.size()!=7 ||
           cliffSettings.minCount!=5 || cliffSettings.maxCount!=8 || cliffSettings.minLength!=4 || cliffSettings.maxLength!=10 ||
           cliffSettings.curl!=10 || cliffSettings.waterDistance!=2 || cliffSettings.cliffDistance!=3)return false;
        for(size_t n=0;n<lands.size();++n) {
            const auto &land=lands[n];const auto &position=placements[n];const auto &metadata=landMetadata[n];const bool neutral=n==0;
            if(land.terrain!=(neutral?1:base) || land.radius!=(neutral?3u:7u) || land.targetTiles!=(neutral?11520u:2160u) ||
               land.zone!=(neutral?26:11) || land.clumpingFactor!=8 || land.borderFuzziness!=(neutral?40u:10u) ||
               land.separation!=(neutral?2u:5u) || land.leftBorder!=(neutral?18:0) || land.topBorder!=(neutral?18:0) ||
               land.rightBorder!=(neutral?96:-1) || land.bottomBorder!=(neutral?96:-1) ||
               position.x!=-1 || position.y!=-1 || position.minPlacementDistance!=-1 ||
               metadata.id!=(neutral?0:1) || metadata.owner!=int(n))return false;
        }
        const uint32_t ground=static_cast<uint32_t>(base);
        const uint32_t terrain[]={22,23,23,22,1,base==26?19u:18u,base==26?46u:12u,base==26?50u:18u,base==26?45u:6u,base==26?14u:45u};
        const uint32_t bases[]={1,22,22,23,22,ground,ground,ground,ground,ground};
        const uint32_t tiles[]={5760,720,144,144,144,1296,1296,144,288,288},clumps[]={3,6,30,30,30,17,8,4,24,30},spacing[]={1,5,1,1,1,5,1,3,1,1};
        for(size_t n=0;n<terrainRows.size();++n) {
            const auto &row=terrainRows[n];
            if(row.terrain!=terrain[n] || row.baseTerrain!=bases[n] || row.targetTiles!=tiles[n] || row.clumps!=clumps[n] ||
               row.spacing!=spacing[n] || row.clumping!=20 || row.avoidPlayerStarts!=(n==5 || n==7) ||
               row.flatTerrainOnly!=(row.terrain==1) || row.minHeight || row.maxHeight!=255)return false;
        }
        for(size_t n=0;n<elevationRows.size();++n) {
            const auto &row=elevationRows[n];
            if(row.elevation!=n+1 || row.baseHeight!=n || row.baseTerrain!=ground || row.targetTiles!=2000 ||
               row.clumps!=20 || row.spacing!=(n==0?2u:1u))return false;
        }
        return true;
    }();
    const bool alliedRoads=[&]() {
        if(width!=120 || height!=120 || base!=1 || !playerLands || playerLandBegin || lands.size()!=2 ||
           playerCount!=2 || !playerZoneOverride || !ownedObjects || !startingWorkers || !startingScouts ||
           workerGroups!=1 || scoutGroups!=1 || workerMinDistance!=6 || workerMaxDistance!=6 || startingStalagmites ||
           playerCivilizations!=std::vector<uint16_t>{1,1} || !defines.count("FIXED_POSITIONS") ||
           diplomacy!=std::vector<std::vector<uint8_t>>{{0,3,3},{0,0,0},{0,0,0}} ||
           !cliffStage || !elevationStage || !connectionStage || terrainRows.size()!=11 || elevationRows.size()!=7 ||
           connectionCommands.size()!=1 || connectionCommands[0].kind!=64 ||
           cliffSettings.minCount!=3 || cliffSettings.maxCount!=5 || cliffSettings.minLength!=4 || cliffSettings.maxLength!=8 ||
           cliffSettings.curl!=10 || cliffSettings.waterDistance!=2 || cliffSettings.cliffDistance!=3)return false;
        const uint32_t ground=lands[0].terrain;
        if(ground!=26 && ground!=12 && ground!=0)return false;
        for(size_t n=0;n<lands.size();++n) {
            const auto &land=lands[n];const auto &position=placements[n];const auto &metadata=landMetadata[n];
            if(land.terrain!=ground || land.radius!=12 || land.targetTiles!=3240 || land.zone!=2 ||
               land.clumpingFactor!=8 || land.borderFuzziness!=15 || land.separation!=22 ||
               position.x!=-1 || position.y!=-1 || position.minPlacementDistance!=-1 || metadata.id!=1 || metadata.owner!=int(n+1) ||
               !((land.leftBorder==10 && land.rightBorder==112) || (land.leftBorder==8 && land.rightBorder==110)) ||
               !((land.topBorder==10 && land.bottomBorder==112) || (land.topBorder==8 && land.bottomBorder==110)))return false;
        }
        const uint32_t firstForest=ground==26?19:ground==12?50:10,secondForest=ground==26?50:ground==12?20:18;
        const uint32_t patch=ground==26?46:ground==12?9:12;
        const uint32_t terrain[]={22,22,23,23,22,1,firstForest,patch,secondForest,ground==26?45u:6u,ground==26?14u:45u};
        const uint32_t tiles[]={5760,144,2880,144,144,144,1152,1296,144,288,288},clumps[]={10,30,8,30,30,30,11,10,4,24,30},spacing[]={2,1,3,1,1,1,5,1,3,1,1};
        const uint32_t bases[]={1,1,22,22,23,22,ground,ground,ground,ground,ground};
        for(size_t n=0;n<terrainRows.size();++n) {
            const auto &row=terrainRows[n];
            if(row.terrain!=terrain[n] || row.baseTerrain!=bases[n] || row.targetTiles!=tiles[n] || row.clumps!=clumps[n] ||
               row.spacing!=spacing[n] || row.clumping!=20 || row.avoidPlayerStarts!=(n==6 || n==8) ||
               row.flatTerrainOnly!=(row.terrain==1) || row.minHeight || row.maxHeight!=255)return false;
        }
        for(size_t n=0;n<elevationRows.size();++n) {
            const auto &row=elevationRows[n];
            if(row.elevation!=n+1 || row.baseHeight!=n || row.baseTerrain!=ground || row.targetTiles!=1200 ||
               row.clumps!=20 || row.spacing!=(n==0?2u:1u))return false;
        }
        RmsConnectionSettings expected{};
        const int road=ground==26?38:ground==12?25:24;
        for(int terrain:{int(ground),int(patch),ground==26?14:6,45,ground==26?50:ground==12?10:18})expected[terrain].replacement=road;
        for(int terrain:{1,22,23,2})expected[terrain].replacement=ground;
        expected[1].cost=5;expected[22].cost=7;expected[23].cost=15;expected[45].cost=5;expected[2].cost=4;
        for(int terrain:{18,20,50})expected[terrain].cost=7;
        for(int terrain:{0,12,9,6,14,46,26})expected[terrain].cost=2;
        for(int terrain:{24,25,38})expected[terrain].cost=1;
        for(int terrain:{1,23}){expected[terrain].width=6;expected[terrain].variance=1;}
        for(int terrain:{0,12,9,26}){expected[terrain].width=1;expected[terrain].variance=0;}
        for(int terrain:{18,20,50}){expected[terrain].width=3;expected[terrain].variance=1;}
        for(int terrain:{6,14,46,45,24,25,38}){expected[terrain].width=0;expected[terrain].variance=0;}
        for(size_t n=0;n<expected.size();++n) {
            const auto &a=expected[n],&b=connectionCommands[0].settings[n];
            if(a.cost!=b.cost || a.width!=b.width || a.variance!=b.variance || a.replacement!=b.replacement)return false;
        }
        return true;
    }();
    // Two mutual allies use the same fixed permutation; larger teams remain unverified.
    if(allies && defines.count("FIXED_POSITIONS") && !alliedRoads) return fail("allied fixed-position starts remain unverified");
    const bool basinCliffs=[&]() {
        if(width!=120 || height!=120 || base!=1 || !playerLands || playerLandBegin!=1 || lands.size()!=3 ||
           playerCount!=2 || !playerZoneOverride || !ownedObjects || !startingWorkers || !startingScouts ||
           workerGroups!=1 || scoutGroups!=1 || workerMinDistance!=6 || workerMaxDistance!=6 || startingStalagmites ||
           playerCivilizations!=std::vector<uint16_t>{1,1} || !defines.count("FIXED_POSITIONS") ||
           !cliffStage || !elevationStage || !connectionStage || terrainRows.size()!=12 || elevationRows.size()!=7 ||
           connectionCommands.size()!=1 || connectionCommands[0].kind!=63 ||
           cliffSettings.minCount!=5 || cliffSettings.maxCount!=8 || cliffSettings.minLength!=4 || cliffSettings.maxLength!=10 ||
           cliffSettings.curl!=10 || cliffSettings.waterDistance!=2 || cliffSettings.cliffDistance!=3)return false;
        for(size_t n=0;n<lands.size();++n) {
            const auto &land=lands[n];const auto &position=placements[n];const auto &metadata=landMetadata[n];const bool neutral=n==0;
            if(land.terrain || land.radius!=(neutral?7u:9u) || land.targetTiles!=(neutral?1440u:3600u) ||
               (neutral?land.zone!=0:(land.zone<2 || land.zone>4)) || land.clumpingFactor!=8 ||
               land.borderFuzziness!=(neutral?20u:10u) || land.separation!=(neutral?0u:5u) ||
               position.x!=-1 || position.y!=-1 || position.minPlacementDistance!=-1 ||
               metadata.id!=(neutral?0:1) || metadata.owner!=int(n))return false;
            if(neutral) {
                if(land.leftBorder!=30 || land.topBorder!=30 || land.rightBorder!=90 || land.bottomBorder!=90)return false;
            } else if(!((land.leftBorder==9 && land.rightBorder==113) || (land.leftBorder==7 && land.rightBorder==111)) ||
                      !((land.topBorder==9 && land.bottomBorder==113) || (land.topBorder==7 && land.bottomBorder==111)))return false;
        }
        const uint32_t terrain[]={22,22,23,23,22,1,22,18,12,18,6,45},bases[]={1,1,22,22,23,22,1,0,0,12,0,0};
        const uint32_t tiles[]={5760,144,2880,144,144,144,144,1008,1296,144,288,288},clumps[]={3,30,5,30,30,30,30,14,8,4,24,30},spacing[]={2,1,5,1,1,1,1,5,0,3,0,0};
        for(size_t n=0;n<terrainRows.size();++n) {
            const auto &row=terrainRows[n];
            if(row.terrain!=terrain[n] || row.baseTerrain!=bases[n] || row.targetTiles!=tiles[n] || row.clumps!=clumps[n] ||
               row.spacing!=spacing[n] || row.clumping!=20 || row.avoidPlayerStarts!=(n==7 || n==9) ||
               row.flatTerrainOnly!=(row.terrain==1) || row.minHeight || row.maxHeight!=255)return false;
        }
        for(size_t n=0;n<elevationRows.size();++n) {
            const auto &row=elevationRows[n];
            if(row.elevation!=n+1 || row.baseHeight!=n || row.baseTerrain || row.targetTiles!=2000 ||
               row.clumps!=20 || row.spacing!=(n==0?2u:1u))return false;
        }
        RmsConnectionSettings expected{};
        for(int terrain:{1,22,23}){expected[terrain].replacement=4;expected[terrain].width=3;expected[terrain].variance=1;}
        for(int terrain:{1,18,2})expected[terrain].cost=7;
        expected[22].cost=9;expected[23].cost=15;expected[4].cost=3;
        for(int terrain:{0,12,6,45})expected[terrain].cost=2;
        for(int terrain:{0,12,6,45,18})expected[terrain].width=0;
        for(size_t n=0;n<expected.size();++n) {
            const auto &a=expected[n],&b=connectionCommands[0].settings[n];
            if(a.cost!=b.cost || a.width!=b.width || a.variance!=b.variance || a.replacement!=b.replacement)return false;
        }
        return true;
    }();
    const bool coastalCliffs=[&]() {
        if(width!=120 || height!=120 || base!=1 || !playerLands || playerLandBegin || lands.size()!=2 ||
           playerCount!=2 || !playerZoneOverride || !ownedObjects || !startingWorkers || !startingScouts ||
           workerGroups!=1 || scoutGroups!=1 || workerMinDistance!=6 || workerMaxDistance!=6 || startingStalagmites ||
           playerCivilizations!=std::vector<uint16_t>{1,1} || !defines.count("FIXED_POSITIONS") ||
           !cliffStage || !elevationStage || !connectionStage || terrainRows.size()!=11 || elevationRows.size()!=7 ||
           connectionCommands.size()!=1 || connectionCommands[0].kind!=63 ||
           cliffSettings.minCount!=5 || cliffSettings.maxCount!=8 || cliffSettings.minLength!=4 || cliffSettings.maxLength!=10 ||
           cliffSettings.curl!=10 || cliffSettings.waterDistance!=2 || cliffSettings.cliffDistance!=3)return false;
        const uint32_t ground=lands[0].terrain;
        if(ground!=26 && ground!=41 && ground!=9 && ground!=12 && ground!=0)return false;
        const bool zaloriis=ground==26,kashyyyk=ground==9 && terrainRows[6].terrain==21;
        for(size_t n=0;n<lands.size();++n) {
            const auto &land=lands[n];const auto &position=placements[n];const auto &metadata=landMetadata[n];
            if(land.terrain!=ground || land.radius!=8 || (land.targetTiles!=3744 && land.targetTiles!=4320 && land.targetTiles!=4680) ||
               land.targetTiles!=lands[0].targetTiles || land.zone!=11 || land.clumpingFactor!=15 || land.borderFuzziness!=15 || land.separation!=7 ||
               position.x!=-1 || position.y!=-1 || position.minPlacementDistance!=-1 || metadata.id!=1 || metadata.owner!=int(n+1))return false;
            const std::array<int,4> border{land.leftBorder,land.topBorder,land.rightBorder,land.bottomBorder};
            if(border!=std::array<int,4>{7,9,-1,113} && border!=std::array<int,4>{7,7,111,-1} &&
               border!=std::array<int,4>{7,0,113,111} && border!=std::array<int,4>{0,7,111,113})return false;
            if(land.leftBorder!=lands[0].leftBorder || land.topBorder!=lands[0].topBorder ||
               land.rightBorder!=lands[0].rightBorder || land.bottomBorder!=lands[0].bottomBorder)return false;
        }
        const uint32_t forest=zaloriis?19:ground==41?10:kashyyyk?21:ground==9?10:ground==12?20:18;
        const uint32_t patch=zaloriis?46:(ground==41 || ground==12)?9:12;
        const uint32_t terrain[]={22,22,23,23,22,1,forest,patch,forest,zaloriis?45u:6u,zaloriis?14u:ground==41?44u:kashyyyk?12u:45u};
        const uint32_t bases[]={1,1,22,22,23,22,ground,ground,ground,ground,ground};
        const uint32_t tiles[]={5760,144,2880,144,144,144,1296,zaloriis?1152u:(ground==41 || ground==9)?864u:1296u,144,288,288};
        const uint32_t clumps[]={10,30,8,30,30,30,14,zaloriis?12u:8u,4,24,30},spacing[]={2,1,3,1,1,1,5,0,3,0,0};
        for(size_t n=0;n<terrainRows.size();++n) {
            const auto &row=terrainRows[n];
            if(row.terrain!=terrain[n] || row.baseTerrain!=bases[n] || row.targetTiles!=tiles[n] || row.clumps!=clumps[n] ||
               row.spacing!=spacing[n] || row.clumping!=20 || row.avoidPlayerStarts!=(n==6 || n==8) ||
               row.flatTerrainOnly!=(row.terrain==1) || row.minHeight || row.maxHeight!=255)return false;
        }
        for(size_t n=0;n<elevationRows.size();++n) {
            const auto &row=elevationRows[n];
            if(row.elevation!=n+1 || row.baseHeight!=n || row.baseTerrain!=ground || row.targetTiles!=2000 ||
               row.clumps!=20 || row.spacing!=(n==0?2u:1u))return false;
        }
        RmsConnectionSettings expected{};
        for(int terrain:{1,22,23}){expected[terrain].replacement=4;expected[terrain].width=2;expected[terrain].variance=1;}
        for(int terrain:{1,18,20,10,19,21,2})expected[terrain].cost=7;
        expected[22].cost=9;expected[23].cost=15;expected[4].cost=3;
        for(int terrain:{44,6,45,0,12,9,41,14,46})expected[terrain].cost=2;
        for(int terrain:{0,12,9,41,6,14,46,44,45,18,20,10,19,21})expected[terrain].width=0;
        for(size_t n=0;n<expected.size();++n) {
            const auto &a=expected[n],&b=connectionCommands[0].settings[n];
            if(a.cost!=b.cost || a.width!=b.width || a.variance!=b.variance || a.replacement!=b.replacement)return false;
        }
        return true;
    }();
    const bool spaceMass=[&]() {
        if(width!=120 || height!=120 || base!=16 || !playerLands || playerLandBegin!=1 || lands.size()!=3 ||
           playerCount!=2 || !playerZoneOverride || !ownedObjects || !startingWorkers || !startingScouts ||
           workerGroups!=1 || scoutGroups!=1 || workerMinDistance!=6 || workerMaxDistance!=6 || startingStalagmites ||
           playerCivilizations!=std::vector<uint16_t>{1,1} || !defines.count("FIXED_POSITIONS") ||
           cliffStage || !elevationStage || !connectionStage || terrainRows.size()!=6 || elevationRows.size()!=27 ||
           connectionCommands.size()!=1 || connectionCommands[0].kind!=63)return false;
        for(size_t n=0;n<lands.size();++n) {
            const auto &land=lands[n];const auto &position=placements[n];const auto &metadata=landMetadata[n];const bool neutral=n==0;
            if(land.terrain!=42 || land.radius!=(neutral?7u:9u) || land.targetTiles!=4320u ||
               (neutral?land.zone!=0:(land.zone<2 || land.zone>4)) || land.clumpingFactor!=8 || land.borderFuzziness!=(neutral?20u:10u) ||
               land.separation!=(neutral?0u:5u) || position.x!=-1 || position.y!=-1 || position.minPlacementDistance!=-1 ||
               metadata.id!=(neutral?0:1) || metadata.owner!=int(n))return false;
            if(neutral) {
                if(land.leftBorder!=30 || land.topBorder!=30 || land.rightBorder!=90 || land.bottomBorder!=90)return false;
            } else {
                if(!((land.leftBorder==9 && land.rightBorder==113) || (land.leftBorder==7 && land.rightBorder==111)) ||
                   !((land.topBorder==9 && land.bottomBorder==113) || (land.topBorder==7 && land.bottomBorder==111)) ||
                   land.leftBorder!=lands[1].leftBorder || land.rightBorder!=lands[1].rightBorder ||
                   land.topBorder!=lands[1].topBorder || land.bottomBorder!=lands[1].bottomBorder)return false;
            }
        }
        const uint32_t terrain[]={50,44,50,14,33,44},bases[]={42,42,44,44,42,14};
        const uint32_t tiles[]={1008,1152,144,288,1008,576},clumps[]={14,8,4,24,5,30},spacing[]={5,0,3,0,1,0};
        for(size_t n=0;n<terrainRows.size();++n) {
            const auto &row=terrainRows[n];
            if(row.terrain!=terrain[n] || row.baseTerrain!=bases[n] || row.targetTiles!=tiles[n] || row.clumps!=clumps[n] ||
               row.spacing!=spacing[n] || row.clumping!=20 || row.avoidPlayerStarts!=(n==0 || n==2) ||
               row.flatTerrainOnly || row.minHeight || row.maxHeight!=255)return false;
        }
        size_t elevationIndex=0;
        const uint32_t counts[]={2,3,4,5,6,7},amounts[]={20000,1000,1000,1000,1000,2000},groups[]={1,28,25,23,20,20};
        for(size_t group=0;group<6;++group)for(uint32_t level=1;level<=counts[group];++level) {
            const auto &row=elevationRows[elevationIndex++];
            if(row.elevation!=level || row.baseHeight!=level-1 || row.baseTerrain!=42 || row.targetTiles!=amounts[group] ||
               row.clumps!=groups[group] || row.spacing!=(level==1?2u:1u))return false;
        }
        RmsConnectionSettings expected{};
        expected[16].replacement=42;expected[16].width=3;expected[16].variance=1;expected[16].cost=15;
        expected[50].cost=7;expected[44].cost=2;expected[14].cost=3;
        for(int terrain:{50,42,44,14})expected[terrain].width=0;
        for(size_t n=0;n<expected.size();++n) {
            const auto &a=expected[n],&b=connectionCommands[0].settings[n];
            if(a.cost!=b.cost || a.width!=b.width || a.variance!=b.variance || a.replacement!=b.replacement)return false;
        }
        return true;
    }();
    const bool geonosisCliffs=[&]() {
        if(width!=120 || height!=120 || base!=26 || !playerLands || playerLandBegin!=1 || lands.size()!=3 ||
           playerCount!=2 || !playerZoneOverride || !ownedObjects || !startingWorkers || !startingScouts ||
           workerGroups!=1 || scoutGroups!=1 || workerMinDistance!=6 || workerMaxDistance!=6 || startingStalagmites ||
           playerCivilizations!=std::vector<uint16_t>{1,1} || !defines.count("FIXED_POSITIONS") ||
           !cliffStage || elevationStage || connectionStage || terrainRows.size()!=4 ||
           cliffSettings.minCount!=5 || cliffSettings.maxCount!=10 || cliffSettings.minLength!=8 || cliffSettings.maxLength!=25 ||
           cliffSettings.curl!=1 || cliffSettings.waterDistance!=2 || cliffSettings.cliffDistance!=2)return false;
        for(size_t n=0;n<lands.size();++n) {
            const auto &land=lands[n];const auto &position=placements[n];const auto &metadata=landMetadata[n];const bool neutral=n==0;
            if(land.terrain!=26 || land.radius!=(neutral?7u:9u) || land.targetTiles!=(neutral?4320u:2160u) ||
               (neutral?land.zone!=0:(land.zone<2 || land.zone>4)) || land.clumpingFactor!=8 || land.borderFuzziness!=(neutral?20u:10u) ||
               land.separation!=(neutral?0u:5u) || position.x!=-1 || position.y!=-1 || position.minPlacementDistance!=-1 ||
               metadata.id!=(neutral?0:1) || metadata.owner!=int(n))return false;
            if(neutral) {
                if(land.leftBorder!=30 || land.topBorder!=30 || land.rightBorder!=90 || land.bottomBorder!=90)return false;
            } else {
                if(!((land.leftBorder==9 && land.rightBorder==113) || (land.leftBorder==7 && land.rightBorder==111)) ||
                   !((land.topBorder==9 && land.bottomBorder==113) || (land.topBorder==7 && land.bottomBorder==111)) ||
                   land.leftBorder!=lands[1].leftBorder || land.rightBorder!=lands[1].rightBorder ||
                   land.topBorder!=lands[1].topBorder || land.bottomBorder!=lands[1].bottomBorder)return false;
            }
        }
        const uint32_t terrain[]={50,46,50,46},bases[]={26,26,44,26};
        const uint32_t tiles[]={2160,1440,1152,720},clumps[]={14,8,4,24},spacing[]={5,0,3,0};
        for(size_t n=0;n<terrainRows.size();++n) {
            const auto &row=terrainRows[n];
            if(row.terrain!=terrain[n] || row.baseTerrain!=bases[n] || row.targetTiles!=tiles[n] || row.clumps!=clumps[n] ||
               row.spacing!=spacing[n] || row.clumping!=20 || row.avoidPlayerStarts!=(n==0 || n==2) ||
               row.flatTerrainOnly || row.minHeight || row.maxHeight!=255)return false;
        }
        return true;
    }();
    const bool eredennCliffs=[&]() {
        if(width!=120 || height!=120 || base!=32 || !playerLands || playerLandBegin || lands.size()!=3 ||
           playerCount!=2 || playerZoneOverride || !ownedObjects || !startingWorkers || !startingScouts ||
           workerGroups!=1 || scoutGroups!=1 || workerMinDistance!=6 || workerMaxDistance!=6 || startingStalagmites ||
           playerCivilizations!=std::vector<uint16_t>{1,1} || !defines.count("FIXED_POSITIONS") ||
           !cliffStage || !elevationStage || connectionStage || terrainRows.size()!=6 || elevationRows.size()!=4 ||
           cliffSettings.minCount!=5 || cliffSettings.maxCount!=8 || cliffSettings.minLength!=4 || cliffSettings.maxLength!=10 ||
           cliffSettings.curl!=10 || cliffSettings.waterDistance!=2 || cliffSettings.cliffDistance!=3)return false;
        for(size_t n=0;n<lands.size();++n) {
            const auto &land=lands[n];const auto &position=placements[n];const auto &metadata=landMetadata[n];const bool neutral=n==2;
            if(land.terrain!=(neutral?1u:32u) || land.radius!=(neutral?3u:9u) || land.targetTiles!=(neutral?288u:1800u) ||
               land.zone!=(neutral?0:int(n+1)) || land.clumpingFactor!=8 || land.borderFuzziness!=(neutral?20u:15u) || land.separation ||
               land.leftBorder || land.topBorder || land.rightBorder!=-1 || land.bottomBorder!=-1 ||
               position.x!=-1 || position.y!=-1 || position.minPlacementDistance!=-1 || metadata.id!=(neutral?0:1) || metadata.owner!=(neutral?0:int(n+1)))return false;
        }
        const uint32_t terrain[]={33,33,13,47,22,35},bases[]={32,32,32,33,1,32};
        const uint32_t tiles[]={1728,144,1440,1440,432,720},clumps[]={17,11,30,30,1,30},spacing[]={5,3,1,1,1,1};
        for(size_t n=0;n<terrainRows.size();++n) {
            const auto &row=terrainRows[n];
            if(row.terrain!=terrain[n] || row.baseTerrain!=bases[n] || row.targetTiles!=tiles[n] || row.clumps!=clumps[n] ||
               row.spacing!=spacing[n] || row.clumping!=20 || row.avoidPlayerStarts!=(n!=4) ||
               row.flatTerrainOnly || row.minHeight || row.maxHeight!=255)return false;
        }
        for(size_t n=0;n<elevationRows.size();++n) {
            const auto &row=elevationRows[n];
            if(row.elevation!=n+1 || row.baseHeight!=n || row.baseTerrain!=32 || row.targetTiles!=6000 ||
               row.clumps!=28 || row.spacing!=(n==0?2u:1u))return false;
        }
        return true;
    }();
    const bool tundraCliffs=[&]() {
        if(width!=120 || height!=120 || base!=1 || !playerLands || playerLandBegin || lands.size()!=2 ||
           playerCount!=2 || playerZoneOverride || !ownedObjects || !startingWorkers || !startingScouts ||
           workerGroups!=1 || scoutGroups!=1 || workerMinDistance!=6 || workerMaxDistance!=6 || startingStalagmites ||
           playerCivilizations!=std::vector<uint16_t>{1,1} || !defines.count("FIXED_POSITIONS") ||
           !cliffStage || !elevationStage || connectionStage || terrainRows.size()!=14 || elevationRows.size()!=5 ||
           cliffSettings.minCount!=5 || cliffSettings.maxCount!=10 || cliffSettings.minLength!=3 || cliffSettings.maxLength!=7 ||
           cliffSettings.curl!=36 || cliffSettings.waterDistance!=2 || cliffSettings.cliffDistance!=2)return false;
        for(size_t n=0;n<lands.size();++n) {
            const auto &land=lands[n];const auto &position=placements[n];const auto &metadata=landMetadata[n];
            if(land.terrain!=32 || land.radius!=15 || land.targetTiles!=5760 || land.zone!=int(n+1) ||
               land.clumpingFactor!=8 || land.borderFuzziness!=5 || land.separation ||
               position.x!=-1 || position.y!=-1 || position.minPlacementDistance!=-1 || metadata.id!=1 || metadata.owner!=int(n+1))return false;
            const std::array<int,4> border{land.leftBorder,land.topBorder,land.rightBorder,land.bottomBorder};
            if(border!=std::array<int,4>{14,0,106,-1} && border!=std::array<int,4>{0,14,-1,106})return false;
            if(land.leftBorder!=lands[0].leftBorder || land.topBorder!=lands[0].topBorder ||
               land.rightBorder!=lands[0].rightBorder || land.bottomBorder!=lands[0].bottomBorder)return false;
        }
        const uint32_t terrain[]={22,23,22,1,35,35,35,35,13,13,47,47,33,33};
        const uint32_t bases[]={1,22,23,22,1,32,32,32,32,32,32,32,47,47};
        const uint32_t tiles[]={1440,720,144,1440,288,576,144,144,1728,720,1296,720,288,288};
        const uint32_t clumps[]={10,8,5,5,10,1,1,1,11,28,20,24,24,10},spacing[]={1,3,1,1,1,1,1,1,5,5,1,1,1,1};
        for(size_t n=0;n<terrainRows.size();++n) {
            const auto &row=terrainRows[n];
            if(row.terrain!=terrain[n] || row.baseTerrain!=bases[n] || row.targetTiles!=tiles[n] || row.clumps!=clumps[n] ||
               row.spacing!=spacing[n] || row.clumping!=20 || row.avoidPlayerStarts!=(n>=5 && n<=9) ||
               row.flatTerrainOnly!=(n>=3 && n<=7) || row.minHeight || row.maxHeight!=255)return false;
        }
        for(size_t n=0;n<elevationRows.size();++n) {
            const auto &row=elevationRows[n];
            if(row.elevation!=n+1 || row.baseHeight!=n || row.baseTerrain!=32 || row.targetTiles!=2000 ||
               row.clumps!=28 || row.spacing!=(n==0?2u:1u))return false;
        }
        return true;
    }();
    const bool arenaForest=[&]() {
        if(width!=120 || height!=120 || (base!=21 && base!=20 && base!=10) || !playerLands || playerLandBegin!=1 || lands.size()!=3 ||
           playerCount!=2 || playerZoneOverride || !ownedObjects || !startingWorkers || !startingScouts ||
           workerGroups!=1 || scoutGroups!=1 || workerMinDistance!=6 || workerMaxDistance!=6 || startingStalagmites ||
           playerCivilizations!=std::vector<uint16_t>{1,1} || !defines.count("FIXED_POSITIONS") ||
           cliffStage || elevationStage || !connectionStage || terrainRows.size()!=4 ||
           connectionCommands.size()!=1 || connectionCommands[0].kind!=63)return false;
        const uint32_t ground=base==21?6:base==20?12:9;
        for(size_t n=0;n<lands.size();++n) {
            const auto &land=lands[n];const auto &position=placements[n];const auto &metadata=landMetadata[n];const bool neutral=n==0;
            const int border=neutral?21:3;
            if(land.terrain!=ground || land.radius!=(neutral?10u:12u) || land.targetTiles!=(neutral?8640u:1440u) ||
               land.zone!=(neutral?11:int(n)) || land.clumpingFactor!=8 || land.borderFuzziness!=(neutral?20u:1u) ||
               land.separation!=(neutral?0u:30u) || land.leftBorder!=border || land.topBorder!=border ||
               land.rightBorder!=120-border || land.bottomBorder!=120-border || position.x!=-1 || position.y!=-1 ||
               position.minPlacementDistance!=-1 || metadata.id!=(neutral?0:1) || metadata.owner!=int(n))return false;
        }
        const uint32_t terrain[]={uint32_t(base),base==20?9u:12u,base==21?0u:6u,base==21?12u:45u},tiles[]={144,576,288,288},clumps[]={4,8,24,30};
        for(size_t n=0;n<terrainRows.size();++n) {
            const auto &row=terrainRows[n];
            if(row.terrain!=terrain[n] || row.baseTerrain!=ground || row.targetTiles!=tiles[n] || row.clumps!=clumps[n] ||
               row.spacing!=(n==0?5u:0u) || row.avoidPlayerStarts!=(n==0) || row.clumping!=20 ||
               row.flatTerrainOnly || row.minHeight || row.maxHeight!=255)return false;
        }
        RmsConnectionSettings expected{};
        for(int terrain:{1,22,23}){expected[terrain].replacement=4;expected[terrain].width=3;expected[terrain].variance=1;}
        for(int terrain:{10,20,21}){expected[terrain].replacement=terrain==10?9:terrain==20?12:6;expected[terrain].width=6;expected[terrain].variance=1;expected[terrain].cost=7;}
        expected[1].cost=7;expected[22].cost=9;expected[23].cost=15;expected[4].cost=3;expected[45].cost=7;
        for(int terrain:{0,6,9,12})expected[terrain].cost=2;
        for(int terrain:{0,2,6,9,12,45})expected[terrain].width=0;
        for(size_t n=0;n<expected.size();++n) {
            const auto &a=expected[n],&b=connectionCommands[0].settings[n];
            if(a.cost!=b.cost || a.width!=b.width || a.variance!=b.variance || a.replacement!=b.replacement)return false;
        }
        return true;
    }();
    const bool resourceIslands=[&]() {
        const bool watermass=playerZoneOverride;
        if(width!=120 || height!=120 || base!=1 || !playerLands || playerLandBegin || lands.size()!=4 ||
           playerCount!=2 || !ownedObjects || !startingWorkers || !startingScouts ||
           workerGroups!=1 || scoutGroups!=1 || workerMinDistance!=6 || workerMaxDistance!=6 || startingStalagmites ||
           playerCivilizations!=std::vector<uint16_t>{1,1} || !defines.count("FIXED_POSITIONS") ||
           cliffStage || elevationStage || connectionStage)return false;
        const uint32_t ground=lands[0].terrain;
        if(ground!=12 && ground!=26 && (watermass?(ground!=6 && ground!=41):ground!=0))return false;
        for(size_t n=0;n<lands.size();++n) {
            const auto &land=lands[n];const auto &position=placements[n];const auto &metadata=landMetadata[n];const bool neutral=n>=2;
            const int border=neutral?6:watermass?4:8;
            if(land.terrain!=ground || land.radius!=(neutral?3u:watermass?12u:9u) || land.targetTiles!=(neutral?144u:2520u) ||
               land.clumpingFactor!=8 || land.borderFuzziness!=(neutral?10u:7u) || land.separation!=((neutral || watermass)?7u:9u) ||
               land.leftBorder!=border || land.topBorder!=border || land.rightBorder!=120-border || land.bottomBorder!=120-border ||
               position.x!=-1 || position.y!=-1 || position.minPlacementDistance!=-1 ||
               metadata.id!=(neutral?(n==2?30:33):1) || metadata.owner!=(neutral?0:int(n+1)))return false;
            if(neutral?land.zone!=0:watermass?(land.zone!=18 && (land.zone<2 || land.zone>4)):land.zone!=int(n+1))return false;
        }
        if(watermass && (lands[0].zone==18)!=(lands[1].zone==18))return false;
        std::vector<std::array<uint32_t,6>> rows;
        if(watermass) {
            rows={{22,1,5760,10,2,0},{22,1,144,30,1,0},{23,22,2880,8,3,0},{23,22,144,30,1,0},{22,23,144,30,1,0},{1,22,144,30,1,0},
                  {ground==12?20u:ground==6?21u:ground==41?10u:19u,ground,ground==12?864u:432u,ground==26?7u:14u,5,1}};
            if(ground==26)rows.push_back({50,ground,432,7,5,1});
            rows.push_back({ground==26?46u:ground==6?12u:9u,ground,ground==12?2160u:288u,ground==12?24u:ground==26?10u:16u,0,0});
            rows.push_back({ground==26?45u:ground==6?0u:6u,ground,144,24,0,0});
        } else {
            rows={{22,1,5760,10,2,0},{23,22,2880,6,3,0},{ground==26?50u:ground==12?20u:18u,ground,864,21,4,1},
                  {ground==26?46u:5u,ground,288,10,1,0},{ground==26?45u:ground==12?9u:12u,ground,144,24,1,0}};
        }
        if(terrainRows.size()!=rows.size())return false;
        for(size_t n=0;n<terrainRows.size();++n) {
            const auto &row=terrainRows[n];const auto &expected=rows[n];
            if(row.terrain!=expected[0] || row.baseTerrain!=expected[1] || row.targetTiles!=expected[2] || row.clumps!=expected[3] ||
               row.spacing!=expected[4] || row.avoidPlayerStarts!=bool(expected[5]) || row.clumping!=20 ||
               row.flatTerrainOnly!=(row.terrain==1) || row.minHeight || row.maxHeight!=255)return false;
        }
        return true;
    }();
    const bool connectedIslands=[&]() {
        if(width!=120 || height!=120 || base!=1 || !playerLands || playerLandBegin || lands.size()!=5 ||
           playerCount!=2 || playerZoneOverride || !ownedObjects || !startingWorkers || !startingScouts ||
           workerGroups!=1 || scoutGroups!=1 || workerMinDistance!=6 || workerMaxDistance!=6 || startingStalagmites ||
           playerCivilizations!=std::vector<uint16_t>{1,1} || !defines.count("FIXED_POSITIONS") || cliffStage ||
           !elevationStage || !connectionStage || terrainRows.size()!=7 || elevationRows.size()!=7 ||
           connectionCommands.size()!=1 || connectionCommands[0].kind!=63)return false;
        const uint32_t ground=lands[0].terrain;const int biome=ground==46?0:ground==12?1:ground==6?2:-1;
        if(biome<0)return false;
        const uint32_t patch[]={45,9,12};
        for(size_t n=0;n<lands.size();++n) {
            const auto &land=lands[n];const auto &position=placements[n];const auto &metadata=landMetadata[n];const bool neutral=n>=2;
            if(land.terrain!=(n==4?patch[biome]:ground) || land.radius!=(neutral?3u:11u) ||
               land.targetTiles!=(neutral?144u:4680u) || land.zone!=(neutral?0:int(n+1)) ||
               land.clumpingFactor!=(neutral?8u:15u) || land.borderFuzziness!=(neutral?10u:7u) || land.separation!=(neutral?7u:5u) ||
               position.x!=-1 || position.y!=-1 || position.minPlacementDistance!=-1 ||
               metadata.id!=(neutral?(n==2?30:35):1) || metadata.owner!=(neutral?0:int(n+1)))return false;
            if(neutral) {
                if(land.leftBorder!=6 || land.topBorder!=6 || land.rightBorder!=114 || land.bottomBorder!=114)return false;
            } else if(!((land.leftBorder==9 && land.rightBorder==-1) || (land.leftBorder==0 && land.rightBorder==111)) ||
                      !((land.topBorder==9 && land.bottomBorder==-1) || (land.topBorder==0 && land.bottomBorder==111)))return false;
        }
        const uint32_t terrain[3][7]={{22,23,19,45,50,14,45},{22,23,20,9,20,6,45},{22,23,21,12,21,0,12}};
        const uint32_t tiles[]={2160,720,1296,1152,144,288,288},clumps[]={20,10,14,8,4,24,30},spacing[]={2,2,5,0,3,0,0};
        for(size_t n=0;n<terrainRows.size();++n) {
            const auto &row=terrainRows[n];
            if(row.terrain!=terrain[biome][n] || row.baseTerrain!=(n==0?1u:n==1?22u:ground) || row.targetTiles!=tiles[n] ||
               row.clumps!=clumps[n] || row.spacing!=spacing[n] || row.clumping!=20 || row.avoidPlayerStarts!=(n==2 || n==4) ||
               row.flatTerrainOnly!=(row.terrain==1) || row.minHeight || row.maxHeight!=255)return false;
        }
        for(size_t n=0;n<elevationRows.size();++n) {
            const auto &row=elevationRows[n];
            if(row.elevation!=n+1 || row.baseHeight!=n || row.baseTerrain!=ground || row.targetTiles!=1000 ||
               row.clumps!=20 || row.spacing!=(n==0?2u:1u))return false;
        }
        RmsConnectionSettings expected{};
        for(int terrain:{1,22,23}){expected[terrain].replacement=4;expected[terrain].width=3;expected[terrain].variance=1;}
        for(int terrain:{1,21,20,2})expected[terrain].cost=7;
        expected[22].cost=9;expected[23].cost=15;expected[4].cost=3;expected[6].cost=3;
        for(int terrain:{45,0,12,9})expected[terrain].cost=2;
        for(int terrain:{0,12,9,21,20,46,14,6,45})expected[terrain].width=0;
        for(size_t n=0;n<expected.size();++n) {
            const auto &a=expected[n],&b=connectionCommands[0].settings[n];
            if(a.cost!=b.cost || a.width!=b.width || a.variance!=b.variance || a.replacement!=b.replacement)return false;
        }
        return true;
    }();
    const bool fortressCliffs=[&]() {
        if(width!=120 || height!=120 || base!=1 || playerCount!=2 || playerCivilizations!=std::vector<uint16_t>{1,1} ||
           !defines.count("FIXED_POSITIONS") || !playerLands || lands.size()!=2 || playerLandBegin || !playerZoneOverride ||
           !ownedObjects || !startingWorkers || !startingScouts || startingStalagmites || workerGroups!=1 || scoutGroups!=1 ||
           workerMinDistance!=6 || workerMaxDistance!=6 || !cliffStage || !elevationStage || connectionStage ||
           terrainRows.size()!=11 || elevationRows.size()!=8 || cliffSettings.minCount!=5 || cliffSettings.maxCount!=10 ||
           cliffSettings.minLength!=3 || cliffSettings.maxLength!=10 || cliffSettings.curl!=36 || cliffSettings.waterDistance!=2 || cliffSettings.cliffDistance!=2)return false;
        const uint32_t ground=lands[0].terrain;const bool reytha=ground==41,zaloriis=ground==26;
        if(ground!=0 && !reytha && !zaloriis)return false;
        for(size_t n=0;n<lands.size();++n) {
            const auto &l=lands[n];const auto &at=placements[n];const auto &m=landMetadata[n];
            if(l.terrain!=ground || l.radius!=20 || l.targetTiles!=6120 || l.zone!=11 || l.clumpingFactor!=8 || l.borderFuzziness!=15 || l.separation!=7 ||
               l.leftBorder || l.topBorder || l.rightBorder!=-1 || l.bottomBorder!=-1 || at.x!=-1 || at.y!=-1 || at.minPlacementDistance!=-1 || m.id!=1 || m.owner!=int(n+1))return false;
        }
        constexpr uint32_t rows[][6]={{22,1,5760,10,2,0},{22,1,144,30,1,0},{23,22,2880,8,3,0},{23,22,144,30,1,0},{22,23,144,30,1,0},{1,22,144,30,1,0},{18,0,1296,14,1,1},{12,0,1152,14,0,0},{18,0,144,4,1,1},{6,0,288,24,0,0},{45,0,288,30,0,0}};
        for(size_t n=0;n<terrainRows.size();++n) {
            const auto &r=terrainRows[n];const auto &e=rows[n];
            constexpr uint32_t zaloriisTerrain[]={19,46,50,45,14};
            const uint32_t terrain=zaloriis && n>=6?zaloriisTerrain[n-6]:reytha && n>=6?(e[0]==18?10:e[0]==12?9:e[0]==45?44:e[0]):e[0];
            if(r.terrain!=terrain || r.baseTerrain!=(n>=6?ground:e[1]) || r.targetTiles!=e[2] || r.clumps!=e[3] || r.spacing!=e[4] || r.avoidPlayerStarts!=bool(e[5]) ||
               r.clumping!=20 || r.flatTerrainOnly!=(r.terrain==1) || r.minHeight || r.maxHeight!=255)return false;
        }
        for(size_t n=0;n<elevationRows.size();++n) {
            const auto &r=elevationRows[n];const size_t level=n<3?n+1:n-2;
            if(r.elevation!=level || r.baseHeight!=level-1 || r.baseTerrain!=ground || r.targetTiles!=(n<3?864u:720u) || r.clumps!=(n<3?10u:5u) || r.spacing!=(level==1?2u:1u))return false;
        }
        return true;
    }();
    // shortcut: expand after composed original cliff/terrain/startup comparisons.
    if(cliffStage && !mixedLandCliffs && !lakeCliffs && !centralLakeCliffs && !migrationCliffs && !basinCliffs && !alliedRoads && !connectedCliffs && !fortressCliffs && !coastalCliffs && !geonosisCliffs && !eredennCliffs && !tundraCliffs && !resourceIslands) {
        // Verified packaged Reytha/Arid/Hoth/Kessel geometry; other cliff compositions remain gated.
        const bool packagedCliffGeometry=[&]() {
            const unsigned kind=base==26 && terrainRows.size()==5 && terrainRows[0].terrain==46?4:base==32?3:base==14?1:base==26?2:0;
            const bool sparseCliffs=kind!=0, hoth=kind==3;
            if(width!=120 || height!=120 || (base!=41 && !sparseCliffs) || !playerLands || playerCount!=2 ||
                lands.size()!=2 || playerZoneOverride || !ownedObjects || !startingWorkers ||
                !startingScouts || startingStalagmites || workerMinDistance!=6 || workerMaxDistance!=6 ||
                playerCivilizations!=std::vector<uint16_t>{1,1} || !defines.count("FIXED_POSITIONS") ||
                connectionStage || !elevationStage || elevationRows.size()!=7 || terrainRows.size()!=(hoth?4u:5u) ||
                cliffSettings.minCount!=(sparseCliffs?10:30) || cliffSettings.maxCount!=(sparseCliffs?15:60) || cliffSettings.minLength!=(sparseCliffs?3:5) ||
                cliffSettings.maxLength!=(sparseCliffs?10:20) || cliffSettings.curl!=36 || cliffSettings.waterDistance!=2 ||
                cliffSettings.cliffDistance!=2) return false;
            for(size_t i=0;i<lands.size();++i) {
                const auto &land=lands[i];
                if(land.terrain!=base || land.radius!=9 || land.targetTiles!=1800 || land.zone!=int(i+1) ||
                    land.clumpingFactor!=8 || land.borderFuzziness!=15 || land.separation ||
                    land.leftBorder || land.topBorder || land.rightBorder!=-1 || land.bottomBorder!=-1 ||
                    placements[i].x!=-1 || placements[i].y!=-1 || placements[i].minPlacementDistance!=-1) return false;
            }
            const uint32_t terrain[5][5]={{10,9,6,6,44},{50,46,50,45,46},{19,46,19,45,14},{49,49,33,34},{46,19,19,14,14}}, tiles[5][5]={{1728,2016,720,432,288},{1728,2880,144,288,288},{1728,2880,144,288,288},{1728,144,1440,1440},{1440,11520,1440,288,288}};
            const uint32_t clumps[5][5]={{28,50,8,12,10},{17,16,7,24,30},{17,16,7,24,30},{17,7,30,30},{16,17,7,24,30}}, spacing[5][5]={{5,1,1,1,1},{5,0,3,1,1},{5,0,3,1,1},{5,3,1,1},{0,5,3,10,1}};
            for(size_t i=0;i<terrainRows.size();++i) {
                const auto &row=terrainRows[i];
                if(row.terrain!=terrain[kind][i] || row.baseTerrain!=(kind==4 && (i==1 || i==4)?46u:base) || row.targetTiles!=tiles[kind][i] ||
                    row.clumps!=clumps[kind][i] || row.spacing!=spacing[kind][i] || row.clumping!=20 ||
                    row.avoidPlayerStarts!=(kind==4?(i==1 || i==2):(i==0 || (hoth?i==1:sparseCliffs && i==2))) || row.flatTerrainOnly!=(row.terrain==1) ||
                    row.minHeight!=0 || row.maxHeight!=255) return false;
            }
            for(size_t i=0;i<elevationRows.size();++i) {
                const auto &row=elevationRows[i];
                if(row.elevation!=i+1 || row.baseHeight!=i || row.baseTerrain!=base ||
                    row.targetTiles!=(sparseCliffs?6000u:2000u) || row.clumps!=(sparseCliffs?28u:21u) || row.spacing!=(i==0?2u:1u)) return false;
            }
            return true;
        }();
        if(!packagedCliffGeometry) {
            const bool ownedCliff=ownedObjects && startingWorkers && startingScouts &&
                workerMinDistance==6 && workerMaxDistance==6 && !startingStalagmites &&
                playerCivilizations.size()==2 &&
                std::all_of(playerCivilizations.begin(),playerCivilizations.end(),[](auto civ){return civ>=1 && civ<=8;}) &&
                terrainRows.size()==1 &&
                terrainRows[0].terrain==21 && terrainRows[0].targetTiles==6000 && terrainRows[0].clumps==60 &&
                terrainRows[0].spacing==2 && terrainRows[0].clumping==20 && terrainRows[0].avoidPlayerStarts &&
                !terrainRows[0].flatTerrainOnly;
            const bool ownedCliffElevation=ownedCliff && defines.count("FIXED_POSITIONS") &&
                playerCivilizations==std::vector<uint16_t>{1,1} && !elevationRows.empty() &&
                ((elevationRows[0].targetTiles==400 && elevationRows[0].clumps==8) ||
                 (elevationRows[0].targetTiles==8000 && elevationRows[0].clumps==30));
            if(width!=120 || height!=120 || base!=6 ||
                (playerLands ? (playerCount!=2 || (lands.size()!=2 && !ownedCliff) || (playerZoneOverride && !ownedCliff) ||
                    (elevationStage && !ownedCliffElevation && (hasObjects || !terrainRows.empty()))) : lands.size()!=1) ||
                (hasObjects && !ownedCliff) || connectionStage || terrainRows.size()>1 ||
                (elevationStage && (elevationRows.empty() || elevationRows.size()>7 ||
                    (elevationRows.size()>1 && !ownedCliffElevation && !terrainRows.empty()))) ||
                std::any_of(terrainRows.begin(),terrainRows.end(),[&](const auto &row) {
                    return (row.terrain!=0 && ((row.terrain!=18 && row.terrain!=20 && row.terrain!=21) || (elevationStage && !ownedCliffElevation))) || row.baseTerrain!=6 || (row.avoidPlayerStarts && !playerLands) ||
                        row.minHeight!=0 || row.maxHeight!=255;
                }))
                return fail("cliffs require a verified120 DIRT1 land profile");
            for(size_t index=0;index<lands.size();++index) {
                const auto &land=lands[index];
                if(land.terrain!=6 || (!playerLands && (placements[index].x<0 || placements[index].y<0)) ||
                    (ownedCliff ? (landMetadata[index].owner ? (land.radius!=13 || land.targetTiles!=7200) : (land.radius || land.targetTiles)) :
                        playerLands ? (land.radius>10 || land.targetTiles>8000) : (land.radius || land.targetTiles)) ||
                    (!playerZoneOverride && land.zone!=(playerLands?landMetadata[index].owner:0)) ||
                    land.clumpingFactor!=8 || land.borderFuzziness!=20 || land.separation ||
                    land.leftBorder || land.topBorder || land.rightBorder!=-1 || land.bottomBorder!=-1)
                    return fail("cliffs require verified land size and growth defaults");
            }
            for(size_t index=0;index<elevationRows.size();++index) {
                const auto &row=elevationRows[index];
                if(row.elevation!=index+1 || row.baseHeight!=index || row.baseTerrain!=6 ||
                    row.targetTiles!=elevationRows[0].targetTiles || row.clumps!=elevationRows[0].clumps)
                    return fail("cliffs require one verified grass elevation block");
            }
        }
    }
    if(cliffStage) {
        if(data.civs.empty() || data.civs[0].units.size()<=273) return fail("missing original cliff units");
        static constexpr float cliffSizes[10][2]={{1.5f,1.5f},{.5f,1.5f},{1.5f,.5f},{1,1.5f},{1,1.5f},
            {1.5f,1},{1.5f,1},{1,1},{1,1},{1.5f,1.5f}};
        for(int id=264;id<=273;++id) {
            const auto &unit=data.civs[0].units[id];
            const auto *graphic=data.graphic(5256+id-264);
            if(!unit.exists || unit.id!=id || unit.type!=10 || unit.standingGraphic[0]!=5256+id-264 ||
                unit.cls!=22 || unit.collisionSize[data.footprintAxesSwapped?1:0]!=cliffSizes[id-264][0] ||
                unit.collisionSize[data.footprintAxesSwapped?0:1]!=cliffSizes[id-264][1] || unit.collisionSize[2]!=.1f ||
                unit.canBeBuiltOn || unit.obstructionType!=2 || unit.obstructionClass!=6 ||
                unit.trainSound!=-1 || unit.deadUnitId!=-1 || !graphic || graphic->sequenceType ||
                graphic->frameCount!=1 || graphic->angleCount!=25 || graphic->soundId!=-1 ||
                !graphic->deltas.empty() || !graphic->angleSounds.empty())
                return fail("unsupported original cliff master or graphic");
        }
    }
    if(allies && ownedObjects && !alliedRoads) return fail("allied owned-object startup remains unverified");
    if(playerZoneOverride && ownedObjects && !spaceMass && !motherlode && !mixedLandCliffs && !lakeCliffs && !centralLakeCliffs && !migrationCliffs && !basinCliffs && !alliedRoads && !connectedCliffs && !fortressCliffs && !coastalCliffs && !geonosisCliffs && !eredennCliffs && !tundraCliffs && !resourceIslands) {
        // shortcut: explicit zones on the compared flat120 Forest profile; extend geometry with original startup replays.
        if(width!=120 || height!=120 || base!=6 || playerCount!=2 || lands.size()!=2 ||
           elevationStage || connectionStage || !defines.count("FIXED_POSITIONS") ||
           playerCivilizations!=std::vector<uint16_t>{1,1} || !startingWorkers || !startingScouts ||
           workerMinDistance!=6 || workerMaxDistance!=6 || startingStalagmites || terrainRows.size()!=1)
            return fail("unsupported explicit-zone owned startup profile");
        for(const auto &land:lands)if(land.terrain!=6 || land.radius!=13 || land.targetTiles!=7200)
            return fail("unsupported explicit-zone player land profile");
        const auto &row=terrainRows.front();
        if(row.terrain!=21 || row.baseTerrain!=6 || row.targetTiles!=6000 || row.clumps!=60 ||
           row.spacing!=2 || row.clumping!=20 || !row.avoidPlayerStarts || row.flatTerrainOnly!=(row.terrain==1) ||
           row.minHeight || row.maxHeight!=255)
            return fail("unsupported explicit-zone terrain profile");
    }
    if(ownedObjects && lands.size()!=playerCount && !spaceMass && !motherlode && !mixedLandCliffs && !lakeCliffs && !centralLakeCliffs && !migrationCliffs && !basinCliffs && !connectedIslands && !resourceIslands && !arenaForest && !eredennCliffs && !geonosisCliffs) {
        // shortcut: fixed neutral seed lands only; broaden after original mixed-land startup comparisons.
        if(width!=120 || height!=120 || base!=6 || playerLandBegin!=0 || playerCount!=2 ||
           lands.size()>playerCount+(cliffStage?1:2) || elevationStage || connectionStage ||
           playerZoneOverride || !defines.count("FIXED_POSITIONS") ||
           playerCivilizations!=std::vector<uint16_t>{1,1} || !startingWorkers || !startingScouts)
            return fail("mixed-land owned-object startup remains unverified");
        for(size_t n=playerCount;n<lands.size();++n) {
            const auto &land=lands[n];const auto &position=placements[n];
            if(land.terrain!=6 || land.radius || land.targetTiles || land.zone || land.separation ||
               position.x<0 || position.y<0 || landMetadata[n].owner)
                return fail("mixed-land owned startup requires fixed neutral seed lands");
        }
    }
    const bool ownedForest=ownedObjects && playerLands && base==41 && lands[0].terrain==9;
    const bool ownedMixedForest=spaceMass || geonosisCliffs || eredennCliffs || tundraCliffs || arenaForest || resourceIslands || coastalCliffs || fortressCliffs || motherlode || mixedLandCliffs || lakeCliffs || centralLakeCliffs || migrationCliffs || basinCliffs || alliedRoads || connectedCliffs || connectedIslands || (ownedObjects && playerLands && (base==6 || base==12 || base==14 || base==26 || base==32 || base==41) && lands[0].terrain==base &&
        width==120 && height==120 && playerCount==2 && playerCivilizations.size()==2 &&
        std::all_of(playerCivilizations.begin(),playerCivilizations.end(),[](uint16_t civ){return civ>=1 && civ<=8;}) &&
        startingWorkers && startingScouts && workerMinDistance==6 && workerMaxDistance==6 &&
        !startingStalagmites);
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
            (!ownedForest && !ownedMixedForest && (base!=0 || !terrainRows.empty() || lands[0].terrain!=0)))
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
    if (nearNova && (defines.count("FIXED_POSITIONS") || width!=32 || height!=32 || playerCount!=2 ||
        playerCivilizations!=std::vector<uint16_t>{1,1} || startingStalagmites))
        return fail("near-Nova currently requires randomized starts for two civilization1 owners on32-square grass or dry forest");
    if(holocrons && ((!ownedMixedForest && defines.count("FIXED_POSITIONS")) || (width!=64 && width!=120) || height!=width ||
        playerCount!=2 || (!ownedMixedForest && playerCivilizations!=std::vector<uint16_t>{1,1}) || startingStalagmites ||
        (ownedForest && width!=120)))
        return fail("holocrons require a verified two-player grass or Forest startup profile");
    if(holocrons && resourceRows.size()>1 && width!=120)
        return fail("stock resource sequence requires verified120-square starts");
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
    const auto ownedBuilding=std::find_if(resourceRows.begin(),resourceRows.end(),[](const auto &row){return row.unitId==87 || row.unitId==70 || row.unitId==50 || row.unitId==79 || row.unitId==117;});
    if(ownedBuilding!=resourceRows.end()) {
        // shortcut: verified Empire authored/Fortress compositions; extend other rows and boundary fallback with original traces.
        if(!fortressCliffs && !arenaForest && (width!=120 || height!=120 || base!=6 || playerCount!=2 || playerCivilizations!=std::vector<uint16_t>{1,1} ||
           !defines.count("FIXED_POSITIONS") || !ownedObjects || !startingWorkers || !startingScouts || startingStalagmites ||
           workerGroups!=1 || scoutGroups!=1 || workerMinDistance!=6 || workerMaxDistance!=6 ||
           !cliffStage || elevationStage || connectionStage || lands.size()!=2 || playerLandBegin || playerZoneOverride ||
           terrainRows.size()!=1 || terrainRows[0].terrain!=21 || terrainRows[0].baseTerrain!=6 || terrainRows[0].targetTiles!=6000 ||
           terrainRows[0].flatTerrainOnly || terrainRows[0].minHeight || terrainRows[0].maxHeight!=255 ||
           terrainRows[0].clumps!=60 || terrainRows[0].spacing!=2 || terrainRows[0].clumping!=20 || !terrainRows[0].avoidPlayerStarts ||
           cliffSettings.minCount!=3 || cliffSettings.maxCount!=9 || cliffSettings.minLength!=5 || cliffSettings.maxLength!=9 ||
           cliffSettings.curl!=36 || cliffSettings.waterDistance!=2 || cliffSettings.cliffDistance!=2)) return fail("unsupported owned-building startup geometry");
        if(!fortressCliffs && !arenaForest)for(const auto &land:lands)
            if(land.terrain!=6 || land.radius!=13 || land.targetTiles!=7200 || land.leftBorder || land.topBorder ||
               land.rightBorder!=-1 || land.bottomBorder!=-1 || land.clumpingFactor!=8 || land.borderFuzziness!=20 || land.separation) return fail("unsupported owned-building player lands");
        const bool basicMixed=resourceRows.size()==3 && resourceRows[0].unitId==87 && resourceRows[0].minDistance==8 &&
                         resourceRows[1].unitId==70 && resourceRows[1].mode==1 && resourceRows[2].unitId==50 && resourceRows[2].groups==5 && resourceRows[2].minDistance==2;
        const bool defendedMixed=resourceRows.size()==4 && resourceRows[0].unitId==79 && resourceRows[0].groups==4 &&
                                 resourceRows[1].unitId==70 && resourceRows[1].mode==1 && resourceRows[2].unitId==87 && resourceRows[2].minDistance==8 &&
                                 resourceRows[3].unitId==50 && resourceRows[3].groups==5 && resourceRows[3].minDistance==2;
        const bool mixed=basicMixed || defendedMixed || fortressCliffs || arenaForest;
        if(fortressCliffs) {
            std::vector<RmsObjectRow> expected={
                {285,-1,0,0,1,0,5,3,-1,-2,25,-1,0,20,0,0},
                {59,-1,2,0,6,0,1,3,0,1,10,12,3,0,0,0},
                {66,-1,2,0,5,0,1,3,0,1,11,15,7,0,0,0},
                {102,-1,2,0,5,0,1,2,0,1,11,15,7,0,0,0},
                {102,-1,2,0,4,0,1,2,0,1,20,26,7,0,0,0},
                {126,-1,0,0,1,0,2,3,0,1,35,40,12,0,0,0},
                {816,-1,0,1,1,0,6,3,-1,-1,-1,-1,0,0,0,0},
                {1585,22,0,0,1,0,8,3,-1,-1,-1,-1,0,0,0,0},
                {79,-1,0,0,1,0,4,3,-1,1,10,13,6,0,0,0},
                {117,-1,0,0,1,0,9999,3,-1,1,16,20,0,0,0,0},
                {70,-1,1,0,4,0,1,3,-1,1,10,12,0,0,0,0},
                {87,-1,0,0,1,0,1,3,-1,1,8,14,0,0,0,0},
                {50,-1,0,0,1,0,5,3,-1,1,2,5,0,0,0,0},
                {348,-1,0,0,1,0,3,3,0,1,8,10,2,0,0,0},
                {348,-1,0,1,1,0,30,3,-1,-1,-1,-1,0,0,0,0},
                {822,-1,1,0,4,0,1,3,0,1,30,35,3,0,0,0},
                {455,-1,0,1,1,0,2,3,-1,-1,-1,-1,0,0,4,0},
                {456,-1,0,1,1,0,3,3,-1,-1,-1,-1,0,0,4,0},
                {451,-1,0,1,1,0,3,3,-1,-1,-1,-1,0,0,4,0},
                {69,-1,0,1,1,0,10,3,-1,-1,-1,-1,3,0,0,0},
            };
            if(lands[0].terrain!=0) {
                const bool reytha=lands[0].terrain==41;
                expected[1].perGroup=12;expected[2].perGroup=7;
                expected.insert(expected.begin()+3,{{66,-1,2,0,4,0,1,3,0,1,18,26,0,0,0,0},{66,-1,2,0,4,0,1,3,0,1,25,35,0,0,0,0}});
                expected[7].unitId=reytha?812:1129;
                for(auto &row:expected) {
                    if(row.unitId==348)row.unitId=reytha?411:1161;
                    if(!reytha && row.unitId==822)row.unitId=1249;
                }
                if(!reytha)expected.erase(std::remove_if(expected.begin(),expected.end(),[](const auto &row){return row.unitId==59 || row.unitId==816;}),expected.end());
            }
            const auto extraResources=std::find_if(expected.begin(),expected.end(),[](const auto &row){return row.unitId==816 || row.unitId==1585;});
            if(defines.count("TINY_MAP"))expected.insert(extraResources,{{66,-1,2,0,3,0,2,2,-1,-2,40,-1,9,0,0,0},{102,-1,2,0,4,1,1,2,-1,-2,30,-1,9,0,0,0}});
            // The original food draws may omit forage/hunt rows entirely.
            for(auto it=expected.begin();it!=expected.end();) {
                auto &row=*it;
                const bool forage=row.unitId==59,hunt=row.unitId==822 || row.unitId==1249;
                if(!forage && !hunt){++it;continue;}
                const auto found=std::find_if(resourceRows.begin(),resourceRows.end(),[&](const auto &actual) {
                    return forage?actual.unitId==59:(actual.unitId==row.unitId || (lands[0].terrain!=0 && actual.unitId==1364));
                });
                if(found==resourceRows.end()){it=expected.erase(it);continue;}
                if((forage && (found->perGroup==3 || found->perGroup==6 || found->perGroup==12)) ||
                   (hunt && (found->perGroup==1 || found->perGroup==2 || found->perGroup==4))) {
                    row.unitId=found->unitId;row.perGroup=found->perGroup;
                }
                ++it;
            }
            if(resourceRows.size()!=std::size(expected))return fail("unsupported fortress object composition");
            for(size_t n=0;n<resourceRows.size();++n) {
                const auto &a=resourceRows[n];auto b=expected[n];
                if(b.unitId==126 && a.unitId==812)b.unitId=812;
                if(a.unitId!=b.unitId || a.terrain!=b.terrain || a.mode!=b.mode || a.scaling!=b.scaling || a.perGroup!=b.perGroup || a.variance!=b.variance ||
                   a.groups!=b.groups || a.radius!=b.radius || a.owner!=b.owner || a.landId!=b.landId || a.minDistance!=b.minDistance || a.maxDistance!=b.maxDistance ||
                   a.spacing!=b.spacing || a.temporarySpacing!=b.temporarySpacing || a.zoneDistance!=b.zoneDistance || a.matchPlayerCiv!=b.matchPlayerCiv)
                    return fail("unsupported fortress object row");
            }
        }
        for(const auto &row:resourceRows) {
            if((fortressCliffs || arenaForest) && row.unitId!=79 && row.unitId!=117 && row.unitId!=70 && row.unitId!=87 && row.unitId!=50)continue;
            if(arenaForest && row.unitId!=117)return fail("unverified Arena owned building");
            if(!mixed && row.unitId!=ownedBuilding->unitId)return fail("unsupported mixed owned buildings");
            if(row.unitId==117) {
                if((!fortressCliffs && !arenaForest && resourceRows.size()!=1) || row.owner!=-1 || row.landId!=1 || row.matchPlayerCiv || row.scaling ||
                   row.groups!=9999 || row.perGroup!=1 || row.mode || row.variance || row.radius!=3 || row.terrain!=-1 ||
                   row.minDistance!=(arenaForest?18:16) || row.maxDistance!=(arenaForest?22:20) || row.spacing || row.temporarySpacing || row.zoneDistance)
                    return fail("unsupported wall startup composition");
                if(data.terrainRestrictions.size()<=25)return fail("missing wall path restriction");
                for(int id:{117,64,88,81,95}) {
                    if(data.civs[1].units.size()<=117)return fail("missing wall or gate prototype");
                    const auto &u=data.civs[1].units[id];const auto *g=data.graphic(u.standingGraphic[0]);
                    const bool wall=id==117,gate=id==64 || id==88;
                    const int graphic=wall?238:id==64?1084:id==88?1195:1111;
                    const int head=wall?-1:(id==64 || id==81)?487:490;
                    if(!u.exists || u.id!=id || u.type!=dat::UT_Building || u.cls!=(wall?6:8) || u.hitPoints!=(wall?1500:2750) ||
                       u.collisionSize[data.footprintAxesSwapped?1:0]!=(id==64?1.f:.5f) || u.collisionSize[data.footprintAxesSwapped?0:1]!=(id==88?1.f:.5f) || u.collisionSize[2]!=4 ||
                       u.clearanceSize[data.footprintAxesSwapped?1:0]!=(id==64?2.f:.5f) || u.clearanceSize[data.footprintAxesSwapped?0:1]!=(id==88?2.f:.5f) ||
                       u.speed || u.flyMode || u.hillMode || u.trait || u.convertTerrain || u.canBeBuiltOn || u.terrainRestriction!=10 ||
                       u.obstructionType!=2 || u.obstructionClass!=(gate?5:4) || u.adjacentMode!=1 || u.graphicsAngle ||
                       u.foundationTerrainId!=-1 || u.oldOverlayId!=-1 || u.stackUnitId!=-1 || u.pileUnit!=-1 || u.disappearsWhenBuilt ||
                       u.techId!=(wall?10:-1) || u.headUnit!=head || u.transformUnit!=(id==64?78:id==88?91:-1) ||
                       u.trainSound!=-1 || u.constructionSound!=(wall?228:541) || u.standingGraphic[0]!=graphic || u.standingGraphic[1]!=-1 ||
                       u.defaultTaskId!=-1 || size_t(id)>=data.unitHeaders.size() || !data.unitHeaders[id].tasks.empty() ||
                       !g || g->frameCount!=1 || g->angleCount!=(wall?5:1) || g->sequenceType!=(wall?2:0) ||
                       g->frameDuration || g->replayDelay || g->soundId!=-1 || !g->angleSounds.empty())
                        return fail("unsupported wall or gate lifecycle metadata");
                    if(gate) {
                        const auto suffix=u.name.rfind("CLOS");
                        if(suffix==std::string::npos || suffix+4!=u.name.size() ||
                           data.civs[1].units[u.transformUnit].name!=u.name.substr(0,suffix)+"OPEN" ||
                           data.civs[1].units[id==64?81:95].name!=u.name.substr(0,suffix)+"END")
                            return fail("unsupported native gate companion names");
                    }
                    for(size_t n=0;n<u.annexes.size();++n) {
                        const auto &a=u.annexes[n];const bool post=gate && n<2;
                        const float dx=post && id==64?(n==0?-1.5f:1.5f):0,dy=post && id==88?(n==0?1.5f:-1.5f):0;
                        if(a.unitId!=(post?(id==64?81:95):-1) || a.misplacementX!=dx || a.misplacementY!=dy)
                            return fail("unsupported wall gate annex layout");
                    }
                    for(size_t n=0;n<u.costs.size();++n) {
                        const auto &c=u.costs[n];const bool paid=wall && n==0;
                        if(c.type!=(paid?2:-1) || c.amount!=(paid?5:0) || c.flag!=(paid?1:0))return fail("unsupported wall completion cost");
                    }
                    for(const auto &storage:u.resourceStorages)
                        if(storage.type!=-1 || storage.amount || storage.flag)return fail("unsupported wall completion storage");
                }
                continue;
            }
            const int buildingId=row.unitId;const bool prefab=buildingId==70,farm=buildingId==50,tower=buildingId==79;
            const float radius=tower?.5f:prefab?1.f:1.5f;
            const bool quantity=prefab?((row.mode==0 && row.perGroup==1) || (row.mode==1 && row.perGroup==4)):(!row.mode && row.perGroup==1);
            const bool range=tower?(row.minDistance==10 && row.maxDistance==13):farm?((row.minDistance==2 && row.maxDistance==5) || (!row.minDistance && !row.maxDistance)):prefab?(row.minDistance==10 && row.maxDistance==12):((row.minDistance==8 && row.maxDistance==14) || (!row.minDistance && !row.maxDistance));
            const bool supported=row.owner==-1 && row.landId==1 && !row.matchPlayerCiv && !row.scaling && (row.groups==1 || (farm && row.groups==5) || (tower && row.groups==4)) &&
                   quantity && range && (mixed || !(farm || tower) || resourceRows.size()==1) && !row.variance && row.radius==3 && row.terrain==-1 && row.spacing==(tower?6:0) && !row.temporarySpacing && !row.zoneDistance;
            if(!supported)return fail("unsupported owned-building startup composition");
            if(data.civs.size()<=1 || data.civs[1].units.size()<=size_t(buildingId) || data.terrainRestrictions.size()<=4)
                return fail("missing owned-building prototype");
            const auto &u=data.civs[1].units[buildingId];const auto *g=data.graphic(u.standingGraphic[0]);
            if(!u.exists || u.id!=buildingId || u.type!=dat::UT_Building || u.cls!=(tower?10:farm?7:18) || u.hitPoints!=(tower?1250:farm?480:prefab?800:1500) ||
               u.collisionSize[0]!=radius || u.collisionSize[1]!=radius || u.collisionSize[2]!=(farm?0:prefab?2:4) ||
               u.clearanceSize[0]!=radius || u.clearanceSize[1]!=radius || u.terrainRestriction!=4 || u.hillMode!=((prefab || farm || tower)?0:3) ||
               u.speed || u.flyMode || u.canBeBuiltOn || u.trait || u.convertTerrain || u.obstructionType!=(farm?0:2) || u.obstructionClass!=(farm?0:3) ||
               u.standingGraphic[0]!=(tower?397:farm?1688:prefab?883:4075) || u.standingGraphic[1]!=-1 || u.constructionGraphic!=(tower?342:farm?-1:prefab?348:355) || u.foundationTerrainId!=(farm?7:27) ||
               u.costs[0].type!=(tower?2:1) || u.costs[0].amount!=(tower?125:farm?50:prefab?25:160) || (u.costs[0].flag&255)!=1 ||
               u.costs[1].type!=(tower?1:-1) || (tower && (u.costs[1].amount!=25 || (u.costs[1].flag&255)!=1)) || u.costs[2].type!=-1 || u.techId!=(tower?11:farm?20:prefab?16:31) || u.trainSound!=-1 || u.constructionSound!=(tower?227:farm?587:prefab?591:598) || u.adjacentMode!=(tower?1:0) || u.graphicsAngle ||
               u.disappearsWhenBuilt || u.stackUnitId!=-1 || u.oldOverlayId!=-1 || u.transformUnit!=-1 || u.headUnit!=-1 ||
               u.placementTerrain[0]!=-1 || u.placementTerrain[1]!=-1 || u.placementSideTerrain[0]!=-1 || u.placementSideTerrain[1]!=-1 ||
               !g || g->sequenceType!=(tower?2:farm?0:prefab?6:1) || g->frameCount!=((farm || tower)?1:prefab?3:2) || g->angleCount!=(tower?8:1) || g->frameDuration!=((prefab || farm || tower)?0.f:.4f) || g->replayDelay ||
               g->soundId!=-1 || !g->angleSounds.empty()) return fail("unsupported owned-building lifecycle metadata");
            for(const auto &annex:u.annexes)if(annex.unitId!=-1)return fail("owned-building annexes are unsupported");
            for(size_t n=0;n<u.resourceStorages.size();++n) {
                const auto &storage=u.resourceStorages[n];const bool population=prefab && n==0,food=farm && n==0;
                if(storage.type!=(food?0:population?4:-1) || storage.amount!=(food?15:population?5:0) || storage.flag!=(population?4:0))
                    return fail("unsupported owned-building completion storage");
            }
        }
    }
    const bool airTransport=std::any_of(resourceRows.begin(),resourceRows.end(),[](const auto &row){return row.unitId==1521;});
    if(airTransport) {
        // shortcut: bounded flat120 and Space Mass profiles; extend with original startup comparisons.
        if(!ownedMixedForest || (base!=6 && !spaceMass) ||
           std::count_if(resourceRows.begin(),resourceRows.end(),[](const auto &row){return row.unitId==1521;})!=1 ||
           data.terrainRestrictions.size()<=23)
            return fail("unsupported air transport startup profile");
        constexpr int standing[]={5458,5471,5488,5482,5465,5495,8984,8987};
        constexpr int sounds[]={144,969,538,965,971,963,418,692};
        for(const auto civ:playerCivilizations) {
            const int id=transportIds[civ-1];
            if(data.civs[civ].units.size()<=size_t(id))return fail("missing air transport prototype");
            const auto &u=data.civs[civ].units[id];const auto *g=data.graphic(u.standingGraphic[0]);
            if(!u.exists || u.id!=id || u.type!=dat::UT_Creatable || u.cls!=59 || u.hitPoints!=150 ||
               u.collisionSize[0]!=.5f || u.collisionSize[1]!=.5f || u.collisionSize[2]!=((civ==2 || civ==5 || civ==6 || civ==8)?4:3) ||
               u.clearanceSize[0]!=.5f || u.clearanceSize[1]!=.5f || u.terrainRestriction!=23 ||
               u.flyMode!=1 || u.hillMode || u.trait!=1 || u.headUnit!=-1 || u.trackingUnit!=-1 ||
               u.trackingUnitMode || u.trackingUnitDensity || u.convertTerrain || u.trainSound!=sounds[civ-1] ||
               u.lineOfSight!=5 || u.speed!=.75f || u.canBeBuiltOn!=1 || u.obstructionType!=5 || u.obstructionClass!=2 ||
               u.standingGraphic[0]!=standing[civ-1] || !g || g->sequenceType!=7 || g->frameCount!=(civ==2?8:5) ||
               g->angleCount!=8 || g->frameDuration!=(civ==2?.5f:.85f) || g->replayDelay || g->soundId!=-1 || !g->angleSounds.empty() ||
               u.costs[0].type!=0 || u.costs[0].amount!=35 || (u.costs[0].flag&255)!=1 ||
               u.costs[1].type!=1 || u.costs[1].amount!=125 || (u.costs[1].flag&255)!=1 ||
               u.resourceStorages[0].type!=4 || u.resourceStorages[0].amount!=-1 || u.resourceStorages[0].flag!=2 ||
               u.resourceStorages[1].type!=11 || u.resourceStorages[1].amount!=1 || u.resourceStorages[1].flag!=2)
                return fail("unsupported air transport prototype or graphics");
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
    for(const auto &connection:connectionCommands) for(const auto &setting:connection.settings)
        if(setting.replacement>=0 && !safeTerrain(static_cast<uint8_t>(setting.replacement),true))
            return fail("unsupported connection replacement terrain metadata");
    if(connectionStage && earlyForest && !arenaForest) return fail("unverified early forest connection profile");
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
        const auto &land=lands[playerLandBegin];
        const int right=land.rightBorder<0?int(width):land.rightBorder;
        const int bottom=land.bottomBorder<0?int(height):land.bottomBorder;
        if(right<=land.leftBorder || bottom<=land.topBorder) return fail("empty player-land border rectangle");
        // shortcut: bordered startup remains limited to compared water-map profiles.
        if(ownedObjects && !spaceMass && !connectedIslands && !basinCliffs && !alliedRoads && !coastalCliffs && !geonosisCliffs && !eredennCliffs && !tundraCliffs && !resourceIslands && !arenaForest && (land.leftBorder || land.topBorder || right!=int(width) || bottom!=int(height)))
            return fail("owned startup on bordered player lands remains unverified");
        const auto placeStarts=defines.count("FIXED_POSITIONS")?generateRmsFixedPlayerStarts:generateRmsPlayerStarts;
        if (!placeStarts(uint32_t(right-land.leftBorder),uint32_t(bottom-land.topBorder),
                         static_cast<int32_t>(land.radius),playerCount,rng,starts,err)) return false;
        for (size_t owner=0;owner<starts.size();++owner) {
            starts[owner].x+=land.leftBorder;starts[owner].y+=land.topBorder;
            placements[playerLandBegin+owner].x=starts[owner].x;
            placements[playerLandBegin+owner].y=starts[owner].y;
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
    if(std::any_of(placements.begin(),placements.end(),[](const auto &p){return p.x<0 || p.y<0;}) &&
        !resolveRmsLandPositions(width,height,placements,rng,err)) return false;
    for(size_t index=0;index<lands.size();++index) {
        lands[index].centerX=uint32_t(placements[index].x);lands[index].centerY=uint32_t(placements[index].y);
        landMetadata[index].center={placements[index].x,placements[index].y};
    }
    generated.lands=landMetadata;
    genieRandomDraw(rng); // Scheduler logging before its first stage.
    std::vector<uint32_t> earlyPaints;
    std::vector<RmsSceneryPlacement> early;
    if (!growRmsLands(width,height,static_cast<uint8_t>(base),lands,data.terrainBlock,rng,generated.terrain,err) ||
        !smoothRmsTerrain(width,height,static_cast<uint8_t>(base),generated.terrain,generated.terrain,err,
                          64*1024*1024,earlyForest?&earlyPaints:nullptr)) return false;
    if (earlyForest && !generateRmsEarlyForest(width,height,earlyPaints,data,rng,early,err,static_cast<uint8_t>(base))) return false;
    genieRandomDraw(rng);
    if(elevationStage) {
        generated.elevation.assign(generated.terrain.size(),0);
        generated.slopes.assign(generated.terrain.size(),0);
        std::vector<uint8_t> mask;
        std::vector<RmsElevationProtection> elevationProtection;
        for(const auto &p:protection) elevationProtection.push_back({p.x,p.y,p.radius,p.weight});
        if(!generateRmsElevation(width,height,elevationRows,generated.terrain,rng,
            generated.elevation,generated.slopes,mask,elevationProtection,err)) return false;
        genieRandomDraw(rng);
    }
    if(cliffStage) {
        const auto heights=generated.elevation.empty()?std::vector<uint8_t>(generated.terrain.size(),0):generated.elevation;
        std::vector<RmsCliffProtection> cliffProtection;
        for(const auto &land:lands) cliffProtection.push_back({int(land.centerX),int(land.centerY),15});
        if(!generateRmsCliffs(width,generated.terrain,heights,cliffSettings,
            cliffProtection,rng,generated.cliffs,err))
            return false;
        genieRandomDraw(rng);
    }
    if (!generateRmsTerrain(width,height,terrainRows,rng,generated.terrain,err,protection,
        generated.elevation,generated.slopes)) return false;
    if (!smoothRmsTerrain(width,height,1,generated.terrain,generated.terrain,err) ||
        !cleanupRmsTerrain(width,height,generated.terrain,err)) return false;
    genieRandomDraw(rng);
    if(connectionStage) {
        // Original492ab0 smooths terrain1; a dry raster and no replacement1 cannot invoke its setters.
        if(earlyForest && std::find(generated.terrain.begin(),generated.terrain.end(),1)!=generated.terrain.end())
            return fail("early forest connections require dry terrain");
        std::vector<RmsConnectionLand> connectionLands;
        for(size_t owner=0;owner<lands.size();++owner)
            connectionLands.push_back({{int(lands[owner].centerX),int(lands[owner].centerY)},landMetadata[owner].owner});
        std::vector<RmsConnectionRecord> records;
        if(!resolveRmsConnectionRecords(connectionCommands,connectionLands,connectionTeams,records,err) ||
            !generateRmsConnections(width,height,generated.terrain,records,rng,err)) return false;
        genieRandomDraw(rng);
    }
    if(std::any_of(generated.elevation.begin(),generated.elevation.end(),[](uint8_t h){return h!=0;})) {
        if((hasObjects && !ownedObjects) || (ownedObjects && !ownedMixedForest) || earlyForest) return fail("object startup on elevations is not verified");
        for(uint8_t terrain:generated.terrain) if(terrain!=10 && terrain!=18 && terrain!=20 && terrain!=21 && terrain!=19 && terrain!=50 && terrain!=49 && !((tundraCliffs || eredennCliffs) && terrain==13) && !safeTerrain(terrain))
            return fail("scenery startup on elevations is not verified");
    }
    if(airTransport && !spaceMass && std::any_of(generated.elevation.begin(),generated.elevation.end(),[](uint8_t h){return h!=0;}))
        return fail("transport startup on elevations is not verified");
    std::vector<uint8_t> sceneryOccupied;
    if(!generated.cliffs.empty()) {
        sceneryOccupied.resize(generated.terrain.size(),0);
        for(const auto &cliff:generated.cliffs) {
            const auto &size=data.civs[0].units[cliff.unit].collisionSize;
            const float halfX=size[data.footprintAxesSwapped?1:0],halfY=size[data.footprintAxesSwapped?0:1];
            // Original48e180 uses strict collision-box overlap, not clearance dimensions.
            const int x0=std::max(0,int(std::floor(cliff.x-halfX))),x1=std::min(int(width),int(std::ceil(cliff.x+halfX)));
            const int y0=std::max(0,int(std::floor(cliff.y-halfY))),y1=std::min(int(height),int(std::ceil(cliff.y+halfY)));
            for(int y=y0;y<y1;++y) for(int x=x0;x<x1;++x) sceneryOccupied[size_t(y)*width+x]=1;
        }
    }
    if (earlyForest) {
        if (!finishRmsEarlyForest(width,height,generated.terrain,data,early,rng,generated.scenery,err,static_cast<uint8_t>(base))) return false;
    } else if (!generateRmsScenery(width,height,generated.terrain,data,rng,generated.scenery,err,sceneryOccupied)) return false;
    std::vector<uint32_t> occupied;
    for (const auto &tree:generated.scenery) if (tree.alive)
        occupied.push_back(static_cast<uint32_t>(tree.y)*width+static_cast<uint32_t>(tree.x));
    for(size_t tile=0;tile<sceneryOccupied.size();++tile)
        if(sceneryOccupied[tile]) occupied.push_back(static_cast<uint32_t>(tile));
    if (ownedObjects) {
        RmsObjectRow startupRow;startupRow.landId=1;
        std::vector<RmsObjectTarget> startupTargets;
        if(!selectRmsObjectTargets(startupRow,generated.lands,playerCount,startupTargets,err))return false;
        if(startupTargets.size()!=playerCount)return fail("owned startup requires one selected land per player");
        for(const auto &target:startupTargets)if(target.owner<=0 || target.owner>int(playerCount))
            return fail("owned startup requires player-owned selected lands");
        if (generated.playerStarts.size()!=playerCount) return fail("owned109 requires player starts");
        if (ownedForest) {
            if (std::any_of(generated.terrain.begin(),generated.terrain.end(),[](uint8_t t){return t!=41 && t!=9 && t!=6 && t!=10;}) ||
                std::any_of(generated.scenery.begin(),generated.scenery.end(),[](const auto &o){return o.unitId!=411 || !o.alive;}))
                return fail("owned forest starts require dry terrain and live411 scenery");
        } else if (ownedMixedForest) {
            for(auto tile:generated.terrain)
                if(!(spaceMass && (tile==16 || tile==42)) && !(alliedRoads && (tile==24 || tile==25 || tile==38)) && tile!=0 && tile!=1 && tile!=2 && tile!=4 && tile!=5 && tile!=6 && tile!=9 && tile!=10 && tile!=12 && tile!=18 && tile!=20 && tile!=21 && tile!=22 && tile!=23 && tile!=41 && tile!=44 && tile!=14 && tile!=45 && tile!=46 && tile!=50 && tile!=19 && tile!=26 && tile!=32 && tile!=33 && tile!=34 && tile!=49 && !((tundraCliffs || eredennCliffs) && (tile==13 || tile==35 || tile==37 || tile==47)))
                    return fail("unsupported Forest startup terrain "+std::to_string(tile));
            for(const auto &tree:generated.scenery)
                if((!isTree(tree.unitId) && tree.unitId!=217 && tree.unitId!=1341) || (!tree.alive && !arenaForest)) return fail("unsupported Forest startup scenery");
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
        for (const auto &target:startupTargets) {
            const auto owner=uint16_t(target.owner);
            const auto &center=generated.lands[target.landIndex].center;
            // Radius0 has one candidate; original controller forces its final rejected gate too.
            for (int draw=0;draw<4;++draw) genieRandomDraw(rng);
            rng=genieRandomDraw(rng); // Completion reseeds the CRT with its fifth15-bit draw.
            const auto centerTerrain=generated.terrain[size_t(center.y)*width+center.x];
            const bool snow=centerTerrain==32 || centerTerrain==33 || centerTerrain==34 || centerTerrain==36 || centerTerrain==47;
            // 554953/557d8c: center-tile snow flag changes raw27 before civilization overrides.
            const uint8_t foundation=snow?36:foundations[owner-1];
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
        RmsZoneCache startZones;
        auto startZone=[&](const RmsPlayerStart &center,int restriction,uint32_t &index) {
            auto pass=data.terrainRestrictions[restriction].passableBuildableDmgMultiplier;
            for(float value:pass) if(!std::isfinite(value)) return fail("starting units require finite passability");
            const auto terrain=generated.terrain[size_t(center.y)*width+center.x];
            if(terrain>=pass.size()) return fail("starting terrain lacks passability metadata");
            pass[terrain]=1;
            return startZones.lookup(width,generated.terrain,pass,index,err);
        };
        auto placeWorkers=[&](uint16_t owner,const RmsPlayerStart &center,int minimum,int maximum,int requested,
                              const std::vector<uint32_t> &blockedTiles,const std::vector<uint8_t> *eligibility,auto emit) {
            std::vector<uint32_t> candidates;
            if (!generateRmsObjectCandidatesInArea(width,height,center.x,center.y,maximum,rng,candidates,err)) return false;
            if(eligibility && !eligibility->empty())candidates.erase(std::remove_if(candidates.begin(),candidates.end(),
                [&](uint32_t tile){return !(*eligibility)[tile];}),candidates.end());
            uint32_t zoneIndex;
            if(!startZone(center,25,zoneIndex)) return false;
            const auto &zones=startZones.entry(zoneIndex)->zones.tiles;
            const auto zone=zones[size_t(center.y)*width+center.x];
            const auto &pass=data.terrainRestrictions[25].passableBuildableDmgMultiplier;
            int remaining=requested;
            if(!remaining)return true;
            int16_t unitId=83;
            for (const auto tile:candidates) {
                if(zones[tile]!=zone) continue;
                const int x=tile%width,y=tile/width;
                if (std::any_of(generated.lands.begin(),generated.lands.end(),[&](const auto &land) {
                    return std::abs(x-land.center.x)<minimum && std::abs(y-land.center.y)<minimum;
                })) continue;
                const float px=x+.5f,py=y+.5f;
                // Original RMS placement ignores existing workers, but blocks every owner's CC.
                const bool blocked=pass[generated.terrain[tile]]<=.05f || std::any_of(generated.ownedObjects.begin(),generated.ownedObjects.end(),[&](const auto &object) {
                    return object.unitId==109 && std::abs(px-object.x)<2.2f && std::abs(py-object.y)<2.2f;
                }) || std::binary_search(blockedTiles.begin(),blockedTiles.end(),tile);
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
                emit(RmsOwnedPlacement{px,py,owner,playerCivilizations[owner-1],unitId,255,facet,frame,delay});
                if (blocked || !--remaining) break;
            }
            return true;
        };
        if (startingWorkers) for (const auto &target:startupTargets)
            if(!placeWorkers(uint16_t(target.owner),generated.lands[target.landIndex].center,workerMinDistance,workerMaxDistance,workerGroups==1?3:workerGroups,
                occupied,nullptr,[&](const auto &object){generated.ownedObjects.push_back(object);}))return false;
        auto placeMobile=[&](uint16_t owner,const RmsPlayerStart &center,int requested,bool air,const std::vector<uint32_t> &blockedTiles,
                            const std::vector<uint8_t> *eligibility,auto emit) {
            const auto civ=playerCivilizations[owner-1];const int id=air?transportIds[civ-1]:scoutIds[civ-1];
            const auto &u=data.civs[civ].units[id];const auto &g=*data.graphic(u.standingGraphic[0]);
            std::vector<uint32_t> candidates;
            if (!generateRmsObjectCandidatesInArea(width,height,center.x,center.y,air?6:9,rng,candidates,err)) return false;
            if(eligibility && !eligibility->empty())candidates.erase(std::remove_if(candidates.begin(),candidates.end(),
                [&](uint32_t tile){return !(*eligibility)[tile];}),candidates.end());
            uint32_t zoneIndex;
            const auto restriction=data.civs[civ].units[id].terrainRestriction;
            if(!startZone(center,restriction,zoneIndex)) return false;
            const auto &zones=startZones.entry(zoneIndex)->zones.tiles;
            const auto zone=zones[size_t(center.y)*width+center.x];
            const auto &pass=data.terrainRestrictions[restriction].passableBuildableDmgMultiplier;
            int remaining=requested;
            if(!remaining)return true;
            for (const auto tile:candidates) {
                if(zones[tile]!=zone) continue;
                const int x=tile%width,y=tile/width;
                if (std::any_of(generated.lands.begin(),generated.lands.end(),[&](const auto &land) {
                    return std::abs(x-land.center.x)<(air?3:7) && std::abs(y-land.center.y)<(air?3:7);
                })) continue;
                const float px=x+.5f,py=y+.5f;
                bool blocked=pass[generated.terrain[tile]]<=.05f;
                if(air) {
                    const auto overlaps=[&](int otherId,int otherOwner,float ox,float oy) {
                        const auto &other=data.civs[otherOwner?playerCivilizations[otherOwner-1]:0].units[otherId];
                        const float collisionZ=other.cls==27 || (other.type==dat::UT_Building && other.collisionSize[2]==0 && other.id!=50)?1:other.collisionSize[2];
                        if(other.flyMode!=1 && (other.type==dat::UT_Creatable || other.canBeBuiltOn==1 ||
                           other.collisionSize[0]<=0 || other.collisionSize[1]<=0 || collisionZ<=0))return false;
                        return std::abs(px-ox)<.5f+other.collisionSize[data.footprintAxesSwapped?1:0] &&
                               std::abs(py-oy)<.5f+other.collisionSize[data.footprintAxesSwapped?0:1];
                    };
                    for(const auto &o:generated.cliffs)blocked|=overlaps(o.unit,0,o.x,o.y);
                    for(const auto &o:generated.scenery)if(o.alive)blocked|=overlaps(o.unitId,0,o.x,o.y);
                    for(const auto &o:generated.ownedObjects)blocked|=overlaps(o.unitId,o.owner,o.x,o.y);
                    for(const auto &o:generated.resourcesAfterOwned)blocked|=overlaps(o.unitId,o.owner,o.x,o.y);
                } else blocked|=std::any_of(generated.ownedObjects.begin(),generated.ownedObjects.end(),[&](const auto &o) {
                    return o.unitId==109 && std::abs(px-o.x)<2.25f && std::abs(py-o.y)<2.25f;
                }) || std::binary_search(blockedTiles.begin(),blockedTiles.end(),tile);
                if (blocked && tile!=candidates.back()) continue;
                // Flying transports skip the ground military retimer after the shared type70 constructor.
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
                for (int draw=0;draw<(air?3:4);++draw) genieRandomDraw(rng);
                emit(RmsOwnedPlacement{px,py,owner,civ,int16_t(id),255,facet,frame,delay,air?1.f:0.f});
                if(blocked || !--remaining)break;
            }
            return true;
        };
        if (startingScouts) for (const auto &target:startupTargets)
            if(!placeMobile(uint16_t(target.owner),generated.lands[target.landIndex].center,scoutGroups,false,occupied,nullptr,
                [&](const auto &object){generated.ownedObjects.push_back(object);}))return false;
        if (!resourceRows.empty()) {
            auto appendFootprint=[&](const auto &object,std::vector<uint32_t> &tiles) {
                const auto &size=data.civs[object.owner?playerCivilizations[object.owner-1]:0].units[object.unitId].collisionSize;
                const float rx=size[data.footprintAxesSwapped?1:0],ry=size[data.footprintAxesSwapped?0:1];
                for(int y=std::max(0,int(std::floor(object.y-ry)));y<std::min(int(height),int(std::ceil(object.y+ry)));++y)
                    for(int x=std::max(0,int(std::floor(object.x-rx)));x<std::min(int(width),int(std::ceil(object.x+rx)));++x)
                        tiles.push_back(uint32_t(y)*width+x);
            };
            auto blockers=occupied;
            for(const auto &object:generated.ownedObjects) {
                if(object.unitId==109) {
                    for(int y=int(object.y)-2;y<int(object.y)+2;++y)
                        for(int x=int(object.x)-2;x<int(object.x)+2;++x) blockers.push_back(uint32_t(y)*width+x);
                } else blockers.push_back(uint32_t(object.y)*width+uint32_t(object.x));
            }
            for(const auto &object:generated.resourcesAfterOwned)
                blockers.push_back(uint32_t(object.y)*width+uint32_t(object.x));
            // Original spacing exclusions survive successive resource rows.
            std::vector<uint8_t> eligibility;
            for(const auto &sourceRow:resourceRows) {
                RmsObjectRow row;
                if(!finalizeRmsObjectRow(sourceRow,width,height,playerCount,row,err)) return false;
                // shortcut: one or two trailing Hoth/Zaloriis mountain rows; extend after mixed later-row comparisons.
                if(row.unitId==1165 || row.unitId==1166 || row.unitId==1019 || row.unitId==1020) {
                    const auto begin=resourceRows.begin()+(&sourceRow-resourceRows.data());
                    if((row.unitId<1100?(base!=26 && !(base==14 && row.unitId==1020)):base!=32) || !cliffStage || resourceRows.end()-begin>2 ||
                       !std::all_of(begin,resourceRows.end(),[&](const auto &later) {
                           return base==32?(later.unitId==1165 || later.unitId==1166):(later.unitId==1019 || later.unitId==1020);
                       }))return fail("mountains require trailing verified cliff-map rows");
                }
                if(row.unitId==1307 && (base!=26 || !cliffStage || terrainRows.size()!=5 || terrainRows[0].terrain!=46))
                    return fail("Gaia mines require the verified Kessel profile");
                if(isGaiaBuilding(row.unitId) && row.unitId!=1307 &&
                   ((row.unitId==1295 || row.unitId==1052)?(base!=12 || cliffStage || !elevationStage || terrainRows.size()!=7):(base!=14 || (!cliffStage && !flatTatooine))))
                    return fail("Gaia Tatooine buildings require a verified profile");
                if(row.unitId==938 && (base!=14 || (!cliffStage && !flatTatooine)))
                    return fail("Sandcrawler startup requires the verified Tatooine profile");
                std::vector<RmsObjectTarget> targets;
                if(!selectRmsObjectTargets(row,generated.lands,playerCount,targets,err))return false;
                if(targets.empty())continue;
                const bool ownedRow=std::any_of(targets.begin(),targets.end(),[](const auto &target){return target.owner>0;});
                if(ownedRow) {
                    if(row.unitId==117) {
                        RmsWallInput input;input.width=width;input.height=height;input.terrain=generated.terrain;
                        input.minimumDistance=row.minDistance;input.maximumDistance=row.maxDistance;
                        input.restrictions=data.terrainRestrictions[25].passableBuildableDmgMultiplier;
                        auto obstacle=[&](int id,int owner,float x,float y) {
                            const auto &u=data.civs[owner?playerCivilizations[owner-1]:0].units[id];
                            const float rx=u.collisionSize[data.footprintAxesSwapped?1:0],ry=u.collisionSize[data.footprintAxesSwapped?0:1];
                            input.objects.push_back({owner,x,y,rx,ry,u.obstructionType,u.obstructionClass,u.cls});
                            if(id==109)input.commandCenters.push_back({owner,x,y,rx,ry});
                        };
                        for(const auto &o:generated.cliffs)obstacle(o.unit,0,o.x,o.y);
                        for(const auto &o:generated.scenery)if(o.alive)obstacle(o.unitId,0,o.x,o.y);
                        for(const auto &o:generated.ownedObjects)obstacle(o.unitId,o.owner,o.x,o.y);
                        for(const auto &o:generated.resourcesAfterOwned)obstacle(o.unitId,o.owner,o.x,o.y);
                        RmsWallBoundary boundary;
                        if(!generateRmsWallBoundary(input,boundary,err))return false;
                        auto &placed=generated.resourcesAfterOwned;
                        for(const auto &target:targets) {
                            if(target.owner<1 || target.owner>2 || target.landIndex<0)return fail("walls require player lands");
                            const auto &center=generated.lands[target.landIndex].center;
                            std::vector<uint32_t> unused;
                            // Original4de830 consumes this queue before dispatching4df5b0.
                            if(!generateRmsObjectCandidatesInArea(width,height,center.x,center.y,row.maxDistance,rng,unused,err))return false;
                            std::array<int,4> gates{};
                            auto create=[&](int id,float x,float y,uint32_t parent=UINT32_MAX) {
                                RmsTimberPlacement object{x,y,uint8_t(id==117?2:7),0,int16_t(id),0,0,uint16_t(target.owner)};
                                object.annexParentIndex=parent;placed.push_back(object);
                            };
                            for(const auto &segment:boundary.segments[target.owner]) {
                                if(!segment.active)continue;
                                int x=segment.fromX,y=segment.fromY,skip=0;
                                const int ex=segment.toX,ey=segment.toY;
                                if(std::max(std::abs(x-ex),std::abs(y-ey))>=4) {
                                    const int direction=y>ey?0:(x<ex && y<ey)?1:x<ex?2:3;
                                    if(gates[direction]<2) {
                                        // shortcut: diagonal gate constructors await original controlled comparisons.
                                        if(direction<2)return fail("unverified diagonal wall gate");
                                        ++gates[direction];const uint32_t parent=uint32_t(placed.size());
                                        const float gx=x+(direction==2?2.f:.5f),gy=y+(direction==2?.5f:2.f);
                                        create(direction==2?64:88,gx,gy);
                                        if(direction==2) {create(81,gx-1.5f,gy,parent);create(81,gx+1.5f,gy,parent);}
                                        else {create(95,gx,gy+1.5f,parent);create(95,gx,gy-1.5f,parent);}
                                        for(int draw=0;draw<9;++draw)genieRandomDraw(rng);
                                        for(int completion=0;completion<3;++completion)rng=genieRandomDraw(rng);
                                        skip=4;
                                    }
                                }
                                for(;;) {
                                    if(skip)--skip;
                                    else {
                                        create(117,x+.5f,y+.5f);
                                        for(int draw=0;draw<4;++draw)genieRandomDraw(rng);
                                        rng=genieRandomDraw(rng);
                                    }
                                    bool moved=false;
                                    if(x<ex){++x;moved=true;}
                                    if(y>ey){--y;moved=true;}else if(y<ey){++y;moved=true;}
                                    if(!moved)break;
                                }
                            }
                        }
                        // Original555f80 tries a same-unit neighbor before a class8 gate/post.
                        std::vector<std::vector<size_t>> cells(size_t(width)*height);
                        for(size_t n=0;n<placed.size();++n) {
                            const auto &o=placed[n];
                            if(o.x<0 || o.y<0 || o.x>=width || o.y>=height)return fail("wall object outside map");
                            cells[size_t(o.y)*width+size_t(o.x)].push_back(n);
                        }
                        constexpr int dx[]={0,0,-1,1,-1,1,-1,1},dy[]={-1,1,0,0,-1,-1,1,1};
                        for(auto &o:placed)if(o.unitId==117) {
                            bool neighbors[8]{};
                            for(int d=0;d<8;++d) {
                                const int x=int(o.x)+dx[d],y=int(o.y)+dy[d];
                                if(x<0 || y<0 || x>=int(width) || y>=int(height))continue;
                                const auto &cell=cells[size_t(y)*width+x];
                                auto match=std::find_if(cell.begin(),cell.end(),[&](size_t n){return placed[n].unitId==117;});
                                if(match==cell.end())match=std::find_if(cell.begin(),cell.end(),[&](size_t n){return data.civs[1].units[placed[n].unitId].cls==8;});
                                neighbors[d]=match!=cell.end() && placed[*match].owner==o.owner;
                            }
                            const auto &n=neighbors;
                            o.facet=n[0]&&n[1]&&!n[2]&&!n[3]?1:n[2]&&n[3]&&!n[0]&&!n[1]?0:n[4]&&n[7]&&!n[5]&&!n[6]?3:n[5]&&n[6]&&!n[4]&&!n[7]?4:2;
                        }
                        continue;
                    }
                    if(row.unitId==87 || row.unitId==70 || row.unitId==50 || row.unitId==79) {
                        const bool prefab=row.unitId==70,farm=row.unitId==50,tower=row.unitId==79;
                        const uint8_t foundation=farm?7:27;
                        const float radius=tower?.5f:prefab?1.f:1.5f,offset=prefab?0.f:.5f;
                        const int low=tower?0:-1,high=(prefab || tower)?1:2;
                        const auto &pass=data.terrainRestrictions[4].passableBuildableDmgMultiplier;
                        for(auto terrain:generated.terrain)if(terrain>=pass.size() || !std::isfinite(pass[terrain]))
                            return fail("invalid owned-building terrain restriction");
                        auto outside=[&](int x,int y){return x+low<0 || y+low<0 || x+high>int(width) || y+high>int(height);};
                        auto blocked=[&](uint32_t tile,uint16_t targetOwner) {
                            const int x=tile%width,y=tile/width;
                            if(outside(x,y))return true;
                            for(int yy=y+low;yy<y+high;++yy)for(int xx=x+low;xx<x+high;++xx)
                                if(pass[generated.terrain[size_t(yy)*width+xx]]<=.05f)return true;
                            if(row.unitId==87 && !generated.elevation.empty()) {
                                const int reference=generated.elevation[size_t(y+high-1)*width+x+low];
                                for(int yy=y+low;yy<y+high;++yy)for(int xx=x+low;xx<x+high;++xx)
                                    if((xx==x+low || yy==y+high-1) && std::abs(int(generated.elevation[size_t(yy)*width+xx])-reference)>1)return true;
                            }
                            const float px=x+offset,py=y+offset;
                            auto overlap=[&](int id,uint16_t owner,float ox,float oy) {
                                const auto &u=data.civs[owner?playerCivilizations[owner-1]:0].units[id];
                                if(u.canBeBuiltOn==1 || u.collisionSize[0]<=0 || u.collisionSize[1]<=0)return false;
                                const float z=u.collisionSize[2];const bool building=u.type==dat::UT_Building && u.id!=50;
                                const float hardZ=u.cls==27 || (building && z==0)?1:z;
                                const float lateZ=z==0 && (building || u.cls==23 || u.cls==25)?1:z;
                                const bool mobile=u.type>=30 && u.speed>0;
                                // Original hard collision precedes the state2 mobile exclusions in its late pass.
                                const bool hard=!farm && hardZ>0;
                                const bool late=!(mobile && (farm || !owner || owner==targetOwner)) && (lateZ>0 || u.id==50 || u.cls==27);
                                if(!hard && !late)return false;
                                return std::abs(px-ox)<radius+u.collisionSize[data.footprintAxesSwapped?1:0] &&
                                       std::abs(py-oy)<radius+u.collisionSize[data.footprintAxesSwapped?0:1];
                            };
                            for(const auto &o:generated.cliffs)if(overlap(o.unit,0,o.x,o.y))return true;
                            for(const auto &o:generated.scenery)if(o.alive && overlap(o.unitId,0,o.x,o.y))return true;
                            for(const auto &o:generated.ownedObjects)if(overlap(o.unitId,o.owner,o.x,o.y))return true;
                            for(const auto &o:generated.resourcesAfterOwned)if(overlap(o.unitId,o.owner,o.x,o.y))return true;
                            return false;
                        };
                        if(eligibility.empty())eligibility.assign(size_t(width)*height,1);
                        for(const auto &target:targets) {
                            if(target.owner<1 || target.owner>2 || target.landIndex<0)return fail("owned buildings require selected player lands");
                            const auto &center=generated.lands[target.landIndex].center;
                            std::vector<uint32_t> candidates;
                            if(!generateRmsObjectCandidatesInArea(width,height,center.x,center.y,row.maxDistance,rng,candidates,err))return false;
                            candidates.erase(std::remove_if(candidates.begin(),candidates.end(),[&](uint32_t tile){return !eligibility[tile];}),candidates.end());
                            uint32_t zoneIndex;
                            if(!startZone(center,4,zoneIndex))return false;
                            const auto &zones=startZones.entry(zoneIndex)->zones.tiles;
                            const auto zone=zones[size_t(center.y)*width+center.x];
                            auto create=[&](uint32_t tile) {
                                const int x=tile%width,y=tile/width;
                                if(outside(x,y))return fail("owned-building forced boundary placement remains unverified");
                                for(int draw=0;draw<(tower?5:prefab?6:3);++draw)genieRandomDraw(rng);
                                rng=genieRandomDraw(rng); // Actual554951 completes four/six/seven-draw construction for87/50,79,70.
                                const uint16_t owner=uint16_t(target.owner);
                                // Spacing keeps79 isolated: original556160 overrides its random heading with facet2.
                                generated.resourcesAfterOwned.push_back({x+offset,y+offset,uint8_t(tower?2:0),0,int16_t(row.unitId),0,0,owner,0,uint8_t(owner==1?255:foundation)});
                                if(owner==1)for(int yy=y+low;yy<y+high;++yy)for(int xx=x+low;xx<x+high;++xx)
                                    generated.terrain[size_t(yy)*width+xx]=foundation;
                                return true;
                            };
                            int remainingGroups=row.groups;
                            for(size_t candidateIndex=0;candidateIndex<candidates.size();++candidateIndex) {
                                const auto tile=candidates[candidateIndex];
                                const int x=tile%width,y=tile/width;
                                if(zones[tile]!=zone || std::any_of(generated.lands.begin(),generated.lands.end(),[&](const auto &land) {
                                    return std::abs(x-land.center.x)<row.minDistance && std::abs(y-land.center.y)<row.minDistance;
                                }))continue;
                                if(!row.mode) {
                                    if(blocked(tile,uint16_t(target.owner)) && (farm || candidateIndex+1<candidates.size()))continue;
                                    // Last-candidate fallback retains overlapping world objects;4de1b0 only unlinks queue nodes.
                                    if(!create(tile))return false;
                                } else {
                                    if(blocked(tile,uint16_t(target.owner)))continue;
                                    std::vector<uint32_t> members;
                                    if(!generateRmsObjectCandidatesInArea(width,height,x,y,row.radius,rng,members,err))return false;
                                    members.erase(std::remove_if(members.begin(),members.end(),[&](uint32_t member){return !eligibility[member];}),members.end());
                                    genieRandomDraw(rng); // Original loose groups consume a quantity draw even with zero variance.
                                    int remaining=row.perGroup;
                                    for(auto member:members) {
                                        if(blocked(member,uint16_t(target.owner)))continue;
                                        if(!create(member))return false;
                                        if(!--remaining)break;
                                    }
                                }
                                if(row.spacing) {
                                    const auto close=[&](uint32_t at){return std::abs(int(at%width)-x)<=row.spacing && std::abs(int(at/width)-y)<=row.spacing;};
                                    candidates.erase(std::remove_if(candidates.begin()+candidateIndex+1,candidates.end(),close),candidates.end());
                                    for(int yy=std::max(0,y-row.spacing);yy<=std::min(int(height)-1,y+row.spacing);++yy)
                                        for(int xx=std::max(0,x-row.spacing);xx<=std::min(int(width)-1,x+row.spacing);++xx)
                                            if(pass[generated.terrain[size_t(yy)*width+xx]]>0)eligibility[size_t(yy)*width+xx]=0;
                                }
                                if(!--remainingGroups)break;
                            }
                        }
                        continue;
                    }
                    // shortcut: compared worker/scout/transport profiles; extend other owned constructors with original comparisons.
                    const bool scout=row.unitId==550 && row.matchPlayerCiv;
                    const bool air=row.unitId==1521 && row.matchPlayerCiv;
                    if((row.unitId!=83 && !scout && !air) || row.mode || row.perGroup!=1 || row.variance || row.radius!=3 ||
                       row.terrain!=-1 || row.minDistance!=(air?3:scout?7:6) || row.maxDistance!=(scout?9:6) || row.spacing || row.temporarySpacing || row.zoneDistance)
                        return fail("unsupported additional owned object row");
                    auto staticBlockers=occupied;
                    for(const auto &object:generated.resourcesAfterOwned)
                        if(!object.owner && isGaiaBuilding(object.unitId))appendFootprint(object,staticBlockers); else if(!object.owner && (object.unitId==1117 || object.unitId==615 || object.unitId==59 || object.unitId==66 || object.unitId==102 || object.unitId==1160 || object.unitId==1161 || object.unitId==1170 || object.unitId==1171 || isTree(object.unitId)))
                            staticBlockers.push_back(uint32_t(object.y)*width+uint32_t(object.x));
                    std::sort(staticBlockers.begin(),staticBlockers.end());
                    staticBlockers.erase(std::unique(staticBlockers.begin(),staticBlockers.end()),staticBlockers.end());
                    for(const auto &target:targets) {
                        if(target.owner<=0 || target.landIndex<0)return fail("additional units require owned selected lands");
                        auto emit=[&](const auto &object) {
                            generated.resourcesAfterOwned.push_back({object.x,object.y,object.facet,0,object.unitId,
                                object.animationFrame,object.initialIdleDelay,object.owner,object.heightAboveGround});
                            blockers.push_back(uint32_t(object.y)*width+uint32_t(object.x));
                        };
                        const auto &center=generated.lands[target.landIndex].center;
                        const bool placed=(scout || air)?placeMobile(uint16_t(target.owner),center,row.groups,air,staticBlockers,&eligibility,emit):
                            placeWorkers(uint16_t(target.owner),center,6,6,row.groups==1?3:row.groups,staticBlockers,&eligibility,emit);
                        if(!placed)return false;
                    }
                    continue;
                }
                std::vector<RmsPlayerStart> selectedStarts;
                for(const auto &target:targets) {
                    if(target.owner!=0)return fail("owned resource construction remains unsupported");
                    if(target.landIndex>=0)selectedStarts.push_back(generated.lands[target.landIndex].center);
                }
                const RmsResourceGroups group{row.mode,uint32_t(row.groups),uint32_t(row.perGroup),uint32_t(row.variance),uint32_t(row.radius)};
                if(!selectedStarts.empty())row.owner=0;
                auto rowBlockers=blockers;
                if(row.unitId==1203 || row.unitId==938 || row.unitId==594 || row.unitId==833 || row.unitId==1363 || row.unitId==1365 || row.unitId==1367 || row.unitId==1471 || row.unitId==1475 || isPredator(row.unitId) || isBigHunt(row.unitId)) {
                    rowBlockers=occupied;
                    for(const auto &object:generated.ownedObjects) if(object.unitId==109)
                        for(int y=int(object.y)-2;y<int(object.y)+2;++y)
                            for(int x=int(object.x)-2;x<int(object.x)+2;++x) rowBlockers.push_back(uint32_t(y)*width+x);
                    // Original herd gates ignore mobile units, holocrons and other animals.
                    for(const auto &object:generated.resourcesAfterOwned)
                        if(isGaiaBuilding(object.unitId) || object.unitId==1296)appendFootprint(object,rowBlockers); else if(object.unitId==1117 || object.unitId==615 || object.unitId==59 || object.unitId==66 || object.unitId==102 || object.unitId==1160 || object.unitId==1161 || object.unitId==1170 || object.unitId==1171 || isTree(object.unitId))
                            rowBlockers.push_back(uint32_t(object.y)*width+uint32_t(object.x));
                }
                if(isGaiaBuilding(row.unitId) || row.unitId==1296) {
                    rowBlockers=occupied;
                    for(const auto &object:generated.resourcesAfterOwned) {
                        const auto &prototype=data.civs[0].units[object.unitId];
                        if(object.owner || (!prototype.canBeBuiltOn && prototype.collisionSize[2]>0)) {
                            if(isGaiaBuilding(object.unitId) || object.unitId==1296)appendFootprint(object,rowBlockers);
                            else rowBlockers.push_back(uint32_t(object.y)*width+uint32_t(object.x));
                        }
                    }
                }
                for(const auto &object:generated.resourcesAfterOwned)
                    if(object.owner && data.civs[playerCivilizations[object.owner-1]].units[object.unitId].type==dat::UT_Building)
                        appendFootprint(object,rowBlockers);
                std::vector<RmsTimberPlacement> placed;
                if(row.unitId==285) {
                    const bool selected=migrationCliffs && row.landId==31 && selectedStarts.size()==1;
                    if(row.mode || row.groups!=5 || row.perGroup!=1 || row.variance || row.radius!=3 ||
                       (!selected && row.landId!=-2) || row.minDistance!=(arenaForest?30:25) || row.terrain!=(arenaForest?int(lands[0].terrain):-1) || row.spacing!=(arenaForest?5:0) ||
                       row.temporarySpacing!=(arenaForest?0:20) || row.zoneDistance)
                        return fail("unsupported holocron placement modifiers");
                    std::vector<RmsPlayerStart> landCenters;
                    for(const auto &land:generated.lands) landCenters.push_back(land.center);
                    if(!generateRmsHolocronPlacements(width,height,generated.terrain,data,
                        landCenters,rng,placed,err,rowBlockers,&eligibility,selected?&selectedStarts[0]:nullptr,arenaForest?&row:nullptr)) return false;
                } else {
                    RmsGlobalGroupPlacement global;
                    if(row.landId==-2 || row.landId>=0) {
                        for(const auto &land:generated.lands) global.landCenters.push_back(land.center);
                        global.minDistance=row.minDistance;global.spacing=row.spacing;
                    }
                    if(!generateRmsGroupedResourcePlacements(width,height,generated.terrain,data,int16_t(row.unitId),group,rng,
                        placed,err,rowBlockers,16000000,selectedStarts,
                        RmsResourceDistance::Near,&eligibility,(row.landId==-2 || row.landId>=0)?&global:nullptr,nullptr,&row,&generated.elevation)) return false;
                }
                // Original48e8b0 ignores buildable-over scenery; zero-height aquatic objects and plants also do not block.
                for(const auto &object:placed) if(!data.civs[0].units[object.unitId].canBeBuiltOn && object.unitId!=816 && object.unitId!=1585 && object.unitId!=69 && object.unitId!=458 && object.unitId!=456 && object.unitId!=450 && object.unitId!=451 && object.unitId!=455 && object.unitId!=1120 && object.unitId!=1122 && object.unitId!=1123 && (object.unitId<1329 || object.unitId>1334)) {
                    if(object.unitId==589 || object.unitId==1017 || object.unitId==1018 || object.unitId==1296 || object.unitId==1019 || object.unitId==1020 || object.unitId==1165 || object.unitId==1166 || isGaiaBuilding(object.unitId))
                        appendFootprint(object,blockers);
                    else blockers.push_back(uint32_t(object.y)*width+uint32_t(object.x));
                }
                generated.resourcesAfterOwned.insert(generated.resourcesAfterOwned.end(),placed.begin(),placed.end());
            }
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
