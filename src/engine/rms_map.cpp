// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_map.h"
#include "rms_object_rows.h"
#include "rms_terrain.h"
#include "rms_elevation.h"
#include "rms_connections.h"
#include "rms_zones.h"
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
    const auto isTree=[](int unit){return unit==348 || unit==349 || unit==350 || unit==411 || unit==413;};
    const auto isBigHunt=[](int unit){return unit==822 || unit==1249 || unit==1364;};
    const auto isPredator=[](int unit){return unit==126 || unit==812 || unit==1197 || unit==1250 || unit==1129;};
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
        if(allies && defines.count("FIXED_POSITIONS")) return fail("allied fixed-position starts remain unverified");
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
    // shortcut: expand after composed original cliff/terrain/startup comparisons.
    if(cliffStage) {
        // Verified packaged Reytha/Arid geometry; other cliff compositions remain gated.
        const bool packagedCliffGeometry=[&]() {
            const unsigned kind=base==14?1:base==26?2:0;
            const bool arid=kind!=0;
            if(width!=120 || height!=120 || (base!=41 && !arid) || !playerLands || playerCount!=2 ||
                lands.size()!=2 || playerZoneOverride || !ownedObjects || !startingWorkers ||
                !startingScouts || startingStalagmites || workerMinDistance!=6 || workerMaxDistance!=6 ||
                playerCivilizations!=std::vector<uint16_t>{1,1} || !defines.count("FIXED_POSITIONS") ||
                connectionStage || !elevationStage || elevationRows.size()!=7 || terrainRows.size()!=5 ||
                cliffSettings.minCount!=(arid?10:30) || cliffSettings.maxCount!=(arid?15:60) || cliffSettings.minLength!=(arid?3:5) ||
                cliffSettings.maxLength!=(arid?10:20) || cliffSettings.curl!=36 || cliffSettings.waterDistance!=2 ||
                cliffSettings.cliffDistance!=2) return false;
            for(size_t i=0;i<lands.size();++i) {
                const auto &land=lands[i];
                if(land.terrain!=base || land.radius!=9 || land.targetTiles!=1800 || land.zone!=int(i+1) ||
                    land.clumpingFactor!=8 || land.borderFuzziness!=15 || land.separation ||
                    land.leftBorder || land.topBorder || land.rightBorder!=-1 || land.bottomBorder!=-1 ||
                    placements[i].x!=-1 || placements[i].y!=-1 || placements[i].minPlacementDistance!=-1) return false;
            }
            const uint32_t terrain[3][5]={{10,9,6,6,44},{50,46,50,45,46},{19,46,19,45,14}}, tiles[3][5]={{1728,2016,720,432,288},{1728,2880,144,288,288},{1728,2880,144,288,288}};
            const uint32_t clumps[3][5]={{28,50,8,12,10},{17,16,7,24,30},{17,16,7,24,30}}, spacing[3][5]={{5,1,1,1,1},{5,0,3,1,1},{5,0,3,1,1}};
            for(size_t i=0;i<terrainRows.size();++i) {
                const auto &row=terrainRows[i];
                if(row.terrain!=terrain[kind][i] || row.baseTerrain!=base || row.targetTiles!=tiles[kind][i] ||
                    row.clumps!=clumps[kind][i] || row.spacing!=spacing[kind][i] || row.clumping!=20 ||
                    row.avoidPlayerStarts!=(i==0 || (arid && i==2)) || row.flatTerrainOnly ||
                    row.minHeight!=0 || row.maxHeight!=255) return false;
            }
            for(size_t i=0;i<elevationRows.size();++i) {
                const auto &row=elevationRows[i];
                if(row.elevation!=i+1 || row.baseHeight!=i || row.baseTerrain!=base ||
                    row.targetTiles!=(arid?6000u:2000u) || row.clumps!=(arid?28u:21u) || row.spacing!=(i==0?2u:1u)) return false;
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
                (playerLands ? (playerCount!=2 || lands.size()!=2 || playerZoneOverride ||
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
                    (ownedCliff ? (land.radius!=13 || land.targetTiles!=7200) :
                        playerLands ? (land.radius>10 || land.targetTiles>8000) : (land.radius || land.targetTiles)) ||
                    land.zone!=(playerLands?int(index+1):0) ||
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
    if(allies && ownedObjects) return fail("allied owned-object startup remains unverified");
    // shortcut: explicit player zones need an original owned-object replay before enabling their startup.
    if(playerZoneOverride && ownedObjects) return fail("owned startup with explicit player zones remains unverified");
    if(ownedObjects && lands.size()!=playerCount) {
        // shortcut: fixed neutral seed lands only; broaden after original mixed-land startup comparisons.
        if(width!=120 || height!=120 || base!=6 || playerLandBegin!=0 || playerCount!=2 ||
           lands.size()>playerCount+2 || cliffStage || elevationStage || connectionStage ||
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
    const bool ownedMixedForest=ownedObjects && playerLands && (base==6 || base==12 || base==14 || base==26 || base==41) && lands[0].terrain==base &&
        width==120 && height==120 && playerCount==2 && playerCivilizations.size()==2 &&
        std::all_of(playerCivilizations.begin(),playerCivilizations.end(),[](uint16_t civ){return civ>=1 && civ<=8;}) &&
        startingWorkers && startingScouts && workerMinDistance==6 && workerMaxDistance==6 &&
        !startingStalagmites;
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
    const bool airTransport=std::any_of(resourceRows.begin(),resourceRows.end(),[](const auto &row){return row.unitId==1521;});
    if(airTransport) {
        // shortcut: one Empire transport row on the compared flat120 profile; extend with original constructor/gate cases.
        if(!ownedMixedForest || base!=6 || playerCivilizations!=std::vector<uint16_t>{1,1} ||
           std::count_if(resourceRows.begin(),resourceRows.end(),[](const auto &row){return row.unitId==1521;})!=1 ||
           data.civs[1].units.size()<=1036 || data.terrainRestrictions.size()<=23)
            return fail("unsupported air transport startup profile");
        const auto &u=data.civs[1].units[1036];const auto *g=data.graphic(u.standingGraphic[0]);
        if(!u.exists || u.id!=1036 || u.type!=dat::UT_Creatable || u.cls!=59 || u.hitPoints!=150 ||
           u.collisionSize[0]!=.5f || u.collisionSize[1]!=.5f || u.collisionSize[2]!=3 ||
           u.clearanceSize[0]!=.5f || u.clearanceSize[1]!=.5f || u.terrainRestriction!=23 ||
           u.flyMode!=1 || u.hillMode || u.trait!=1 || u.headUnit!=-1 || u.trackingUnit!=-1 ||
           u.trackingUnitMode || u.trackingUnitDensity || u.convertTerrain || u.trainSound!=144 ||
           u.standingGraphic[0]!=5458 || !g || g->sequenceType!=7 || g->frameCount!=5 ||
           g->angleCount!=8 || g->frameDuration!=.85f || g->replayDelay || g->soundId!=-1 || !g->angleSounds.empty() ||
           u.costs[0].type!=0 || u.costs[0].amount!=35 || (u.costs[0].flag&255)!=1 ||
           u.costs[1].type!=1 || u.costs[1].amount!=125 || (u.costs[1].flag&255)!=1 ||
           u.resourceStorages[0].type!=4 || u.resourceStorages[0].amount!=-1 || u.resourceStorages[0].flag!=2 ||
           u.resourceStorages[1].type!=11 || u.resourceStorages[1].amount!=1 || u.resourceStorages[1].flag!=2)
            return fail("unsupported air transport prototype or graphics");
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
    if(connectionStage && earlyForest) return fail("connections with early forest objects require setter callbacks");
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
        // shortcut: bordered owned startup needs an original object-stage comparison before enabling it.
        if(ownedObjects && (land.leftBorder || land.topBorder || right!=int(width) || bottom!=int(height)))
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
        for(uint8_t terrain:generated.terrain) if(terrain!=10 && terrain!=18 && terrain!=20 && terrain!=21 && terrain!=19 && terrain!=50 && !safeTerrain(terrain))
            return fail("scenery startup on elevations is not verified");
    }
    if(airTransport && std::any_of(generated.elevation.begin(),generated.elevation.end(),[](uint8_t h){return h!=0;}))
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
                if(tile!=1 && tile!=2 && tile!=4 && tile!=5 && tile!=6 && tile!=9 && tile!=10 && tile!=12 && tile!=18 && tile!=20 && tile!=21 && tile!=22 && tile!=41 && tile!=44 && tile!=14 && tile!=45 && tile!=46 && tile!=50 && tile!=19 && tile!=26)
                    return fail("unsupported Forest startup terrain "+std::to_string(tile));
            for(const auto &tree:generated.scenery)
                if((!isTree(tree.unitId) && tree.unitId!=217) || !tree.alive) return fail("Forest startup requires live348/349/411/413 scenery");
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
            const auto civ=playerCivilizations[owner-1];const int id=air?1036:scoutIds[civ-1];
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
                std::vector<RmsObjectTarget> targets;
                if(!selectRmsObjectTargets(row,generated.lands,playerCount,targets,err))return false;
                if(targets.empty())continue;
                const bool ownedRow=std::any_of(targets.begin(),targets.end(),[](const auto &target){return target.owner>0;});
                if(ownedRow) {
                    // shortcut: compared worker/scout/transport profiles; extend other owned constructors with original comparisons.
                    const bool scout=row.unitId==550 && row.matchPlayerCiv;
                    const bool air=row.unitId==1521 && row.matchPlayerCiv;
                    if((row.unitId!=83 && !scout && !air) || (air && row.groups!=1) || row.mode || row.perGroup!=1 || row.variance || row.radius!=3 ||
                       row.terrain!=-1 || row.minDistance!=(air?3:scout?7:6) || row.maxDistance!=(scout?9:6) || row.spacing || row.temporarySpacing || row.zoneDistance)
                        return fail("unsupported additional owned object row");
                    auto staticBlockers=occupied;
                    for(const auto &object:generated.resourcesAfterOwned)
                        if(!object.owner && (object.unitId==59 || object.unitId==66 || object.unitId==102 || isTree(object.unitId)))
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
                if(row.unitId==594 || row.unitId==833 || row.unitId==1363 || row.unitId==1365 || isPredator(row.unitId) || isBigHunt(row.unitId)) {
                    rowBlockers=occupied;
                    for(const auto &object:generated.ownedObjects) if(object.unitId==109)
                        for(int y=int(object.y)-2;y<int(object.y)+2;++y)
                            for(int x=int(object.x)-2;x<int(object.x)+2;++x) rowBlockers.push_back(uint32_t(y)*width+x);
                    // Original herd gates ignore mobile units, holocrons and other animals.
                    for(const auto &object:generated.resourcesAfterOwned)
                        if(object.unitId==59 || object.unitId==66 || object.unitId==102 || isTree(object.unitId))
                            rowBlockers.push_back(uint32_t(object.y)*width+uint32_t(object.x));
                }
                std::vector<RmsTimberPlacement> placed;
                if(row.unitId==285) {
                    if(row.mode || row.groups!=5 || row.perGroup!=1 || row.variance || row.radius!=3 ||
                       row.landId!=-2 || row.minDistance!=25 || row.terrain!=-1 || row.spacing ||
                       row.temporarySpacing!=20 || row.zoneDistance)
                        return fail("unsupported holocron placement modifiers");
                    if(!generateRmsHolocronPlacements(width,height,generated.terrain,data,
                        generated.playerStarts,rng,placed,err,rowBlockers,&eligibility)) return false;
                } else {
                    RmsGlobalGroupPlacement global;
                    if(row.landId==-2 || row.landId>=0) {
                        for(const auto &land:generated.lands) global.landCenters.push_back(land.center);
                        global.minDistance=row.minDistance;global.spacing=row.spacing;
                    }
                    if(!generateRmsGroupedResourcePlacements(width,height,generated.terrain,data,int16_t(row.unitId),group,rng,
                        placed,err,rowBlockers,16000000,selectedStarts,
                        RmsResourceDistance::Near,&eligibility,(row.landId==-2 || row.landId>=0)?&global:nullptr,nullptr,&row)) return false;
                }
                // Original waves are nonblocking decorations; fish may share their tile.
                for(const auto &object:placed) if(object.unitId!=816 && object.unitId!=1585)
                    blockers.push_back(uint32_t(object.y)*width+uint32_t(object.x));
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
