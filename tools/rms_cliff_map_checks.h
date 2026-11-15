// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <iostream>
static void checkRmsCliffMap(const swgb::dat::DatFile &data) {
    using namespace swgb;
    const std::string land="<LAND_GENERATION> base_terrain 6 create_land { terrain_type 6 land_position 50 50 base_size 0 number_of_tiles 0 } ";
    const char *settings[]={
        "",
        "min_number_of_cliffs 5 max_number_of_cliffs 10 min_length_of_cliff 3 max_length_of_cliff 12 cliff_curliness 35 min_distance_cliffs 1 min_terrain_distance 1 ",
        "min_number_of_cliffs 0 max_number_of_cliffs 0 ",
        "min_number_of_cliffs 2 max_number_of_cliffs 2 min_length_of_cliff 0 max_length_of_cliff 0 cliff_curliness 0 min_terrain_distance 0 min_distance_cliffs 1 ",
        "min_number_of_cliffs 2 max_number_of_cliffs 2 min_length_of_cliff 1 max_length_of_cliff 1 cliff_curliness 100 min_terrain_distance 0 min_distance_cliffs 1 ",
        "min_number_of_cliffs 2 max_number_of_cliffs 2 min_length_of_cliff 2 max_length_of_cliff 2 cliff_curliness 100 min_terrain_distance 0 min_distance_cliffs 1 ",
        "min_number_of_cliffs 2 max_number_of_cliffs 2 min_length_of_cliff 3 max_length_of_cliff 3 cliff_curliness 100 min_terrain_distance 0 min_distance_cliffs 1 ",
        "min_number_of_cliffs 20 max_number_of_cliffs 20 min_length_of_cliff 30 max_length_of_cliff 40 cliff_curliness 100 min_terrain_distance 0 min_distance_cliffs 1 ",
        "min_number_of_cliffs 30 max_number_of_cliffs 40 min_length_of_cliff 3 max_length_of_cliff 30 cliff_curliness 0 min_terrain_distance 0 min_distance_cliffs 1 ",
        "min_number_of_cliffs 10 max_number_of_cliffs 20 min_length_of_cliff 3 max_length_of_cliff 20 cliff_curliness 50 min_terrain_distance 0 min_distance_cliffs 39 ",
        "min_number_of_cliffs 999 max_number_of_cliffs 999 min_length_of_cliff 0 max_length_of_cliff 2 cliff_curliness 100 min_terrain_distance 0 min_distance_cliffs 1 "
    };
    struct Case { int settings; uint32_t seed,rng,hash; size_t count; };
    // Actual original scheduler, land, cliffs, empty terrain and object stages.
    const Case cases[]={
        {0,1u,1150416911u,1363270808u,29},
        {0,12345u,1497412313u,959006638u,45},
        {0,4294967295u,2483271371u,1228384702u,21},
        {1,1u,1755895453u,1286295733u,55},
        {1,12345u,1772623844u,4126238780u,72},
        {1,4294967295u,2710870391u,602856561u,37},
        {2,1u,887571558u,2166136261u,0},
        {2,12345u,4126090032u,2166136261u,0},
        {2,4294967295u,2372191774u,2166136261u,0},
        {3,1u,2558158352u,2166136261u,0},
        {3,12345u,1025198890u,2166136261u,0},
        {3,4294967295u,3669407112u,2166136261u,0},
        {4,1u,2558158352u,2166136261u,0},
        {4,12345u,1025198890u,2166136261u,0},
        {4,4294967295u,3669407112u,2166136261u,0},
        {5,1u,2558158352u,2166136261u,0},
        {5,12345u,1025198890u,2166136261u,0},
        {5,4294967295u,3669407112u,2166136261u,0},
        {6,1u,2144960216u,3308703961u,8},
        {6,12345u,4114177586u,1754853289u,8},
        {6,4294967295u,1800325706u,4057489208u,8},
        {7,1u,2851926737u,1712362764u,420},
        {7,12345u,3986125029u,577991049u,394},
        {7,4294967295u,2171071764u,4241318935u,399},
        {8,1u,699734975u,4002586549u,423},
        {8,12345u,2369775109u,500297054u,522},
        {8,4294967295u,2297857372u,1813709853u,418},
        {9,1u,443464005u,3313979162u,17},
        {9,12345u,379872204u,4015112052u,10},
        {9,4294967295u,1562644537u,231032273u,7},
        {10,1u,2848727207u,2166136261u,0},
        {10,12345u,2365967353u,2166136261u,0},
        {10,4294967295u,4263982527u,2166136261u,0},
    };
    RmsScript script; RmsMap map; std::string error;
    auto load=[&](const std::string &text) {
        check(script.load(0,[&](int32_t id,std::vector<uint8_t> &out) {
            if(id) return false; out.assign(text.begin(),text.end()); return true;
        },{},&error),error.c_str());
    };
    for(const auto &c:cases) for(bool late:{false,true}) {
        const std::string cliff="<CLIFF_GENERATION> "+std::string(settings[c.settings]);
        load(land+(late?"<TERRAIN_GENERATION> <OBJECTS_GENERATION> "+cliff:cliff+"<TERRAIN_GENERATION> <OBJECTS_GENERATION> "));
        uint32_t rng=c.seed;
        check(generateRmsMap(script,{},120,120,data,rng,map,&error),error.c_str());
        uint32_t hash=2166136261u;
        for(const auto &o:map.cliffs)
            for(uint32_t v:{uint32_t(o.unit),uint32_t(o.facet),uint32_t(o.x*2),uint32_t(o.y*2),o.creationOrdinal})
                hash=(hash^v)*16777619u;
        if(rng!=c.rng || hash!=c.hash || map.cliffs.size()!=c.count) std::cerr<<"cliff settings "<<c.settings<<" seed "<<c.seed<<" rng "<<rng<<'/'<<c.rng<<" hash "<<hash<<'/'<<c.hash<<" count "<<map.cliffs.size()<<'/'<<c.count<<'\n';
        check(rng==c.rng && hash==c.hash && map.cliffs.size()==c.count,"original composed cliff objects and RNG");
        check(map.scenery.empty() && map.objects.empty() && map.ownedObjects.empty(),"cliff-only profile creates no later objects");
        for(auto t:map.terrain) check(t==6,"cliffs preserve grass terrain");
    }
    struct TerrainCase { int tiles,clumps,spacing,clumping; bool flat;
        uint32_t seed,rng,objects,terrain; size_t count; };
    const TerrainCase terrainCases[]={
        {30,3,1,10,true,1u,646082904u,1363270808u,3484421045u,29},
        {30,3,1,10,true,12345u,2638519242u,959006638u,3095328181u,45},
        {30,3,1,10,true,4294967295u,1620821508u,1228384702u,3173278085u,21},
        {2000,20,2,20,false,1u,3225468973u,1363270808u,365659973u,29},
        {2000,20,2,20,false,12345u,2742977745u,959006638u,1539307699u,45},
        {2000,20,2,20,false,4294967295u,1218767571u,1228384702u,679768181u,21},
        {8000,1,0,0,false,1u,4127068469u,1363270808u,1009525507u,29},
        {8000,1,0,0,false,12345u,2886244795u,959006638u,779798787u,45},
        {8000,1,0,0,false,4294967295u,137764506u,1228384702u,4214891795u,21},
    };
    for(const auto &c:terrainCases) {
        load(land+"<CLIFF_GENERATION> <TERRAIN_GENERATION> create_terrain 0 { base_terrain 6 number_of_tiles "+
            std::to_string(c.tiles)+" number_of_clumps "+std::to_string(c.clumps)+
            " spacing_to_other_terrain_types "+std::to_string(c.spacing)+" clumping_factor "+
            std::to_string(c.clumping)+(c.flat?" set_flat_terrain_only ":" ")+"}");
        uint32_t rng=c.seed;
        check(generateRmsMap(script,{},120,120,data,rng,map,&error),error.c_str());
        uint32_t objects=2166136261u,terrain=2166136261u;
        for(const auto &o:map.cliffs)
            for(uint32_t v:{uint32_t(o.unit),uint32_t(o.facet),uint32_t(o.x*2),uint32_t(o.y*2),o.creationOrdinal})
                objects=(objects^v)*16777619u;
        for(auto tile:map.terrain) terrain=(terrain^tile)*16777619u;
        if(rng!=c.rng || objects!=c.objects || terrain!=c.terrain)
            std::cerr<<"cliff/terrain "<<c.tiles<<' '<<c.seed<<" rng "<<rng<<'/'<<c.rng
                <<" objects "<<objects<<'/'<<c.objects<<" terrain "<<terrain<<'/'<<c.terrain<<'\n';
        check(rng==c.rng && objects==c.objects && terrain==c.terrain && map.cliffs.size()==c.count,
            "original cliff/grass composition objects, terrain and RNG");
        check(map.scenery.empty() && map.objects.empty(),"grass painting adds no scenery");
    }
    struct ElevationCase { int tiles,clumps; uint32_t seed,rng,objects,heights,slopes; size_t count; int levels=1; };
    const ElevationCase elevationCases[]={
        {400,8,1u,2790043587u,291107893u,73028170u,3169292009u,29},
        {400,8,12345u,3576478055u,1991487999u,1897583699u,1760096298u,55},
        {400,8,4294967295u,875337137u,1565698685u,3593664539u,3804388132u,20},
        {1000,2,1u,506459290u,1053432523u,2939230450u,463713662u,27},
        {1000,2,12345u,3863891629u,2160117617u,2456656715u,1555305034u,30},
        {1000,2,4294967295u,178198308u,1763354238u,92000700u,4288308406u,35},
        {8000,30,1u,2269132341u,3231355347u,1601223514u,3045030720u,47},
        {8000,30,12345u,356477077u,1829178315u,3368565405u,202350159u,42},
        {8000,30,4294967295u,3507099122u,2453755317u,2749370310u,3751637635u,50},
        {400,8,1u,2744178341u,3166680752u,2427225276u,4151966524u,30,2},
        {400,8,12345u,3143903008u,3975870492u,3655599846u,4093227016u,22,2},
        {400,8,4294967295u,2523226310u,1795454824u,1411226935u,1654172885u,47,2},
        {1000,2,1u,3426239401u,4023666330u,581908518u,3196936021u,23,2},
        {1000,2,12345u,3993901233u,538006811u,3093269219u,3699007432u,55,2},
        {1000,2,4294967295u,1185834593u,3105500748u,3249137414u,2080283094u,45,2},
        {8000,30,1u,2978000291u,3093696992u,4215580062u,301997494u,39,2},
        {8000,30,12345u,1306408363u,2779652543u,3218675948u,2919639480u,29,2},
        {8000,30,4294967295u,644339416u,4239108436u,3729770312u,236545581u,42,2},
        {400,8,1u,1166557347u,2868996674u,3011256799u,704240237u,44,3},
        {400,8,12345u,2075013762u,2179615068u,2429582657u,153959930u,25,3},
        {400,8,4294967295u,3574253452u,3722963227u,1879595097u,1438540761u,51,3},
        {1000,2,1u,4140240717u,644755107u,1558467066u,703627950u,46,3},
        {1000,2,12345u,3310335381u,3191389979u,2583580881u,147187923u,46,3},
        {1000,2,4294967295u,183039817u,405829136u,228976599u,3453017446u,34,3},
        {8000,30,1u,1425325917u,173898735u,3503654032u,904079192u,20,3},
        {8000,30,12345u,1395193335u,1273660848u,994739660u,4042009274u,18,3},
        {8000,30,4294967295u,2825231775u,3400683102u,1731752944u,4131699844u,44,3},
        {400,8,1u,1165343810u,1784602409u,1241919861u,3794864003u,38,7},
        {400,8,12345u,14680498u,4096189495u,2429582657u,153959930u,51,7},
        {400,8,4294967295u,2464239411u,3372067959u,1879595097u,1438540761u,38,7},
        {1000,2,1u,1435115100u,1307305007u,3486551398u,2107840337u,37,7},
        {1000,2,12345u,818655602u,199402233u,4049201087u,3398082362u,42,7},
        {1000,2,4294967295u,747771640u,3880729723u,3255158590u,1042687799u,25,7},
        {8000,30,1u,2215531909u,3523760796u,2333374578u,4221510755u,33,7},
        {8000,30,12345u,354546605u,1620451199u,692390202u,2626351737u,21,7},
        {8000,30,4294967295u,1598187674u,1009416997u,1909161120u,2865734476u,23,7},
    };
    for(const auto &c:elevationCases) for(bool late:{false,true}) {
        const std::string elevation="<ELEVATION_GENERATION> create_elevation "+std::to_string(c.levels)+" { base_terrain 6 number_of_tiles "+
            std::to_string(c.tiles)+" number_of_clumps "+std::to_string(c.clumps)+" } ";
        load(land+(late?"<CLIFF_GENERATION> "+elevation:elevation+"<CLIFF_GENERATION> "));
        uint32_t rng=c.seed;
        check(generateRmsMap(script,{},120,120,data,rng,map,&error),error.c_str());
        uint32_t objects=2166136261u,heights=2166136261u,slopes=2166136261u;
        for(const auto &o:map.cliffs)
            for(uint32_t v:{uint32_t(o.unit),uint32_t(o.facet),uint32_t(o.x*2),uint32_t(o.y*2),o.creationOrdinal})
                objects=(objects^v)*16777619u;
        for(auto h:map.elevation) heights=(heights^h)*16777619u;
        for(auto slope:map.slopes) slopes=(slopes^slope)*16777619u;
        if(rng!=c.rng || objects!=c.objects || heights!=c.heights || slopes!=c.slopes)
            std::cerr<<"cliff/elevation "<<c.tiles<<' '<<c.seed<<" rng "<<rng<<'/'<<c.rng
                <<" objects "<<objects<<'/'<<c.objects<<" heights "<<heights<<'/'<<c.heights
                <<" slopes "<<slopes<<'/'<<c.slopes<<'\n';
        check(rng==c.rng && objects==c.objects && heights==c.heights && slopes==c.slopes && map.cliffs.size()==c.count,
            "original elevation/cliff composition objects, heights, slopes and RNG");
        for(auto tile:map.terrain) check(tile==6,"elevated cliffs preserve grass terrain");
        check(map.scenery.empty() && map.objects.empty(),"elevated grass adds no scenery");
    }
    const char *elevationProfiles[]={
        "<ELEVATION_GENERATION> create_elevation 1 { base_terrain 6 number_of_tiles 400 number_of_clumps 8 } ",
        "<ELEVATION_GENERATION> create_elevation 1 { base_terrain 6 number_of_tiles 1000 number_of_clumps 2 } ",
        "<ELEVATION_GENERATION> create_elevation 1 { base_terrain 6 number_of_tiles 8000 number_of_clumps 30 } "};
    const char *terrainProfiles[]={
        "<TERRAIN_GENERATION> create_terrain 0 { base_terrain 6 number_of_tiles 30 number_of_clumps 3 spacing_to_other_terrain_types 1 clumping_factor 10 set_flat_terrain_only } ",
        "<TERRAIN_GENERATION> create_terrain 0 { base_terrain 6 number_of_tiles 2000 number_of_clumps 20 spacing_to_other_terrain_types 2 clumping_factor 20 } ",
        "<TERRAIN_GENERATION> create_terrain 0 { base_terrain 6 number_of_tiles 8000 number_of_clumps 1 spacing_to_other_terrain_types 0 clumping_factor 0 } "};
    struct ComposedCase { int elevation,paint; uint32_t seed,rng,objects,terrain,heights,slopes; size_t count; };
    const ComposedCase composedCases[]={
        {0,0,1u,1669067597u,291107893u,3322776917u,73028170u,3169292009u,29},
        {0,0,12345u,2766064017u,1991487999u,1926381653u,1897583699u,1760096298u,55},
        {0,0,4294967295u,4219911869u,1565698685u,525050579u,3593664539u,3804388132u,20},
        {0,1,1u,1414928774u,291107893u,179978549u,73028170u,3169292009u,29},
        {0,1,12345u,2607552378u,1991487999u,2991465267u,1897583699u,1760096298u,55},
        {0,1,4294967295u,2143772937u,1565698685u,1199486021u,3593664539u,3804388132u,20},
        {0,2,1u,1151360151u,291107893u,3177634035u,73028170u,3169292009u,29},
        {0,2,12345u,1138811651u,1991487999u,4155714179u,1897583699u,1760096298u,55},
        {0,2,4294967295u,2516137956u,1565698685u,1747737461u,3593664539u,3804388132u,20},
        {1,0,1u,3299953103u,1053432523u,3922290179u,2939230450u,463713662u,27},
        {1,0,12345u,3892837138u,2160117617u,3306366643u,2456656715u,1555305034u,30},
        {1,0,4294967295u,683634326u,1763354238u,2976045685u,92000700u,4288308406u,35},
        {1,1,1u,284299030u,1053432523u,1104645555u,2939230450u,463713662u,27},
        {1,1,12345u,2829043695u,2160117617u,2094912259u,2456656715u,1555305034u,30},
        {1,1,4294967295u,1517942319u,1763354238u,3737583109u,92000700u,4288308406u,35},
        {1,2,1u,610403274u,1053432523u,2367995939u,2939230450u,463713662u,27},
        {1,2,12345u,2206464246u,2160117617u,3139625685u,2456656715u,1555305034u,30},
        {1,2,4294967295u,4180202955u,1763354238u,248722979u,92000700u,4288308406u,35},
        {2,0,1u,1697189782u,3231355347u,681091941u,1601223514u,3045030720u,47},
        {2,0,12345u,2255696495u,1829178315u,1955985493u,3368565405u,202350159u,42},
        {2,0,4294967295u,3018555015u,2453755317u,1124132885u,2749370310u,3751637635u,50},
        {2,1,1u,3126289683u,3231355347u,700999331u,1601223514u,3045030720u,47},
        {2,1,12345u,2156550556u,1829178315u,308214373u,3368565405u,202350159u,42},
        {2,1,4294967295u,1674459729u,2453755317u,1794881765u,2749370310u,3751637635u,50},
        {2,2,1u,3481116158u,3231355347u,4248104451u,1601223514u,3045030720u,47},
        {2,2,12345u,4287602241u,1829178315u,921911525u,3368565405u,202350159u,42},
        {2,2,4294967295u,4083295235u,2453755317u,2518593189u,2749370310u,3751637635u,50},
    };
    for(const auto &c:composedCases) for(bool late:{false,true}) {
        load(land+(late?std::string(terrainProfiles[c.paint])+"<CLIFF_GENERATION> "+elevationProfiles[c.elevation]:
            std::string(elevationProfiles[c.elevation])+"<CLIFF_GENERATION> "+terrainProfiles[c.paint]));
        uint32_t rng=c.seed;
        check(generateRmsMap(script,{},120,120,data,rng,map,&error),error.c_str());
        uint32_t objects=2166136261u,terrain=2166136261u,heights=2166136261u,slopes=2166136261u;
        for(const auto &o:map.cliffs)
            for(uint32_t v:{uint32_t(o.unit),uint32_t(o.facet),uint32_t(o.x*2),uint32_t(o.y*2),o.creationOrdinal})
                objects=(objects^v)*16777619u;
        for(auto t:map.terrain) terrain=(terrain^t)*16777619u;
        for(auto h:map.elevation) heights=(heights^h)*16777619u;
        for(auto slope:map.slopes) slopes=(slopes^slope)*16777619u;
        if(rng!=c.rng || objects!=c.objects || terrain!=c.terrain || heights!=c.heights || slopes!=c.slopes)
            std::cerr<<"elevation/cliff/grass "<<c.elevation<<' '<<c.paint<<' '<<c.seed<<" mismatch\n";
        check(rng==c.rng && objects==c.objects && terrain==c.terrain && heights==c.heights && slopes==c.slopes && map.cliffs.size()==c.count,
            "original combined elevation/cliff/grass terrain, objects and RNG");
        check(map.scenery.empty() && map.objects.empty(),"combined elevated grass adds no scenery");
    }
    struct ForestCase { int tiles,clumps,spacing; uint32_t seed,rng,objects,terrain; size_t count,blocked; int terrainType=21; };
    const ForestCase forestCases[]={
        {2000,20,2,1u,1567879473u,3602929036u,3238789567u,2065,16},
        {2000,20,2,12345u,2603646791u,3248902699u,3671891854u,2035,67},
        {2000,20,2,4294967295u,1747975995u,3510501071u,4171639027u,2013,50},
        {8000,40,0,1u,3214010955u,2637590139u,1269346813u,8007,104},
        {8000,40,0,12345u,3846171459u,1918157620u,1588475975u,8008,145},
        {8000,40,0,4294967295u,3982945277u,256632634u,3738121163u,7996,115},
        {2000,20,2,1u,1567879473u,2602113488u,3363920733u,2065,16,18},
        {2000,20,2,12345u,2603646791u,404381693u,3490096497u,2035,67,18},
        {2000,20,2,4294967295u,1747975995u,2114254583u,1344200397u,2013,50,18},
        {8000,40,0,1u,3214010955u,1698228665u,2773107205u,8007,104,18},
        {8000,40,0,12345u,3846171459u,2354153889u,2669345853u,8008,145,18},
        {8000,40,0,4294967295u,3982945277u,1834129471u,1690993965u,7996,115,18},
        {2000,20,2,1u,1567879473u,2264503250u,3251077261u,2065,16,20},
        {2000,20,2,12345u,2603646791u,3165698723u,4082327271u,2035,67,20},
        {2000,20,2,4294967295u,1747975995u,2849488543u,1214617645u,2013,50,20},
        {8000,40,0,1u,3214010955u,4254848801u,390932597u,8007,104,20},
        {8000,40,0,12345u,3846171459u,3669998891u,2325653181u,8008,145,20},
        {8000,40,0,4294967295u,3982945277u,14491839u,913097181u,7996,115,20},
    };
    for(const auto &c:forestCases) for(bool late:{false,true}) {
        const std::string forest="<TERRAIN_GENERATION> create_terrain "+std::to_string(c.terrainType)+" { base_terrain 6 number_of_tiles "+
            std::to_string(c.tiles)+" number_of_clumps "+std::to_string(c.clumps)+
            " spacing_to_other_terrain_types "+std::to_string(c.spacing)+" clumping_factor 20 } ";
        load(land+(late?forest+"<CLIFF_GENERATION> ":"<CLIFF_GENERATION> "+forest));
        uint32_t rng=c.seed;
        check(generateRmsMap(script,{},120,120,data,rng,map,&error),error.c_str());
        uint32_t objects=2166136261u,terrain=2166136261u,nextId=0;
        auto mixObject=[&](int unit,int facet,float x,float y,uint32_t id) {
            for(uint32_t v:{uint32_t(unit),uint32_t(facet),uint32_t(x*2),uint32_t(y*2),id}) objects=(objects^v)*16777619u;
        };
        for(const auto &o:map.cliffs) { mixObject(o.unit,o.facet,o.x,o.y,o.creationOrdinal);nextId=o.creationOrdinal+1; }
        for(const auto &o:map.scenery) {check(o.alive,"forest tree survives startup");mixObject(o.unitId,o.facet,o.x,o.y,nextId++);}
        size_t forestTiles=0;
        for(auto t:map.terrain) {terrain=(terrain^t)*16777619u;forestTiles+=t==c.terrainType;}
        if(rng!=c.rng || objects!=c.objects || terrain!=c.terrain)
            std::cerr<<"cliff/forest "<<c.tiles<<' '<<c.seed<<" rng "<<rng<<'/'<<c.rng
                <<" objects "<<objects<<'/'<<c.objects<<" terrain "<<terrain<<'/'<<c.terrain<<'\n';
        check(rng==c.rng && objects==c.objects && terrain==c.terrain && map.cliffs.size()+map.scenery.size()==c.count,
            "original cliff/forest composition terrain, all objects and RNG");
        check(forestTiles-map.scenery.size()==c.blocked,"original cliff collision rejects centered trees without a draw");
    }
    const char *centerStages[]={"",
        "<TERRAIN_GENERATION> create_terrain 0 { base_terrain 6 number_of_tiles 2000 number_of_clumps 20 spacing_to_other_terrain_types 2 clumping_factor 20 } ",
        "<TERRAIN_GENERATION> create_terrain 18 { base_terrain 6 number_of_tiles 2000 number_of_clumps 20 spacing_to_other_terrain_types 2 clumping_factor 20 } ",
        "<TERRAIN_GENERATION> create_terrain 20 { base_terrain 6 number_of_tiles 2000 number_of_clumps 20 spacing_to_other_terrain_types 2 clumping_factor 20 } ",
        "<TERRAIN_GENERATION> create_terrain 21 { base_terrain 6 number_of_tiles 2000 number_of_clumps 20 spacing_to_other_terrain_types 2 clumping_factor 20 } ",
        "<ELEVATION_GENERATION> create_elevation 3 { base_terrain 6 number_of_tiles 8000 number_of_clumps 30 } "};
    struct CenterCase { int x,y,stage; uint32_t seed,rng,objects,terrain,heights,slopes; size_t count; };
    const CenterCase centerCases[]={
        {0,0,0,1u,958887679u,2892238716u,2736574149u,1017236165u,1017236165u,19},
        {0,0,0,12345u,1057779985u,1484396439u,2736574149u,1017236165u,1017236165u,20},
        {0,0,0,4294967295u,2142547864u,493507092u,2736574149u,1017236165u,1017236165u,49},
        {0,99,0,1u,958887679u,1563714726u,2736574149u,1017236165u,1017236165u,19},
        {0,99,0,12345u,4030079865u,1234926283u,2736574149u,1017236165u,1017236165u,22},
        {0,99,0,4294967295u,3494186659u,2139548678u,2736574149u,1017236165u,1017236165u,54},
        {99,0,0,1u,958887679u,2128969292u,2736574149u,1017236165u,1017236165u,19},
        {99,0,0,12345u,1057779985u,1484396439u,2736574149u,1017236165u,1017236165u,20},
        {99,0,0,4294967295u,321109722u,1660527627u,2736574149u,1017236165u,1017236165u,55},
        {99,99,0,1u,958887679u,1563714726u,2736574149u,1017236165u,1017236165u,19},
        {99,99,0,12345u,4030079865u,1234926283u,2736574149u,1017236165u,1017236165u,22},
        {99,99,0,4294967295u,3494186659u,3519820470u,2736574149u,1017236165u,1017236165u,54},
        {25,75,0,1u,3024936072u,2026548085u,2736574149u,1017236165u,1017236165u,34},
        {25,75,0,12345u,359641895u,3695282908u,2736574149u,1017236165u,1017236165u,45},
        {25,75,0,4294967295u,1695180386u,69635541u,2736574149u,1017236165u,1017236165u,22},
        {10,90,0,1u,164685784u,458080199u,2736574149u,1017236165u,1017236165u,48},
        {10,90,0,12345u,2829185419u,74125009u,2736574149u,1017236165u,1017236165u,39},
        {10,90,0,4294967295u,1534096359u,3034170892u,2736574149u,1017236165u,1017236165u,31},
        {0,0,2,1u,2856842116u,3594875341u,1218766021u,1017236165u,1017236165u,2061},
        {0,0,2,12345u,680095670u,15835905u,776767093u,1017236165u,1017236165u,2048},
        {0,0,3,1u,2856842116u,3685957767u,1805553301u,1017236165u,1017236165u,2061},
        {0,0,3,12345u,680095670u,3337948012u,1624742005u,1017236165u,1017236165u,2048},
        {0,0,4,1u,2856842116u,2071870919u,437704653u,1017236165u,1017236165u,2061},
        {0,0,4,12345u,680095670u,3249313513u,2739559489u,1017236165u,1017236165u,2048},
        {0,0,5,1u,2142864666u,1937344243u,2736574149u,3503654032u,904079192u,52},
        {0,0,5,12345u,2480161993u,3429309772u,2736574149u,994739660u,4042009274u,52},
        {99,99,2,1u,2856842116u,1511111591u,1218766021u,1017236165u,1017236165u,2061},
        {99,99,2,12345u,146210911u,901480770u,3489319301u,1017236165u,1017236165u,2048},
        {99,99,3,1u,2856842116u,2630832029u,1805553301u,1017236165u,1017236165u,2061},
        {99,99,3,12345u,146210911u,3888948564u,2083981221u,1017236165u,1017236165u,2048},
        {99,99,4,1u,2856842116u,2507402057u,437704653u,1017236165u,1017236165u,2061},
        {99,99,4,12345u,146210911u,2456507208u,1507891181u,1017236165u,1017236165u,2048},
        {99,99,5,1u,897748544u,1498033484u,2736574149u,3503654032u,904079192u,23},
        {99,99,5,12345u,3706374906u,3582843644u,2736574149u,994739660u,4042009274u,57},
        {25,75,2,1u,3704569038u,351469256u,3897783153u,1017236165u,1017236165u,2039},
        {25,75,2,12345u,2849074184u,1099627322u,3353255649u,1017236165u,1017236165u,1980},
        {25,75,3,1u,3704569038u,1379643965u,2735422039u,1017236165u,1017236165u,2039},
        {25,75,3,12345u,2849074184u,1643756671u,4009009687u,1017236165u,1017236165u,1980},
        {25,75,4,1u,3704569038u,699927899u,4082602038u,1017236165u,1017236165u,2039},
        {25,75,4,12345u,2849074184u,2385709371u,2360170338u,1017236165u,1017236165u,1980},
        {25,75,5,1u,288154613u,3676574710u,2736574149u,3503654032u,904079192u,19},
        {25,75,5,12345u,2907542172u,795436380u,2736574149u,994739660u,4042009274u,42},
        {0,0,1,1u,4142211138u,2892238716u,79417397u,1017236165u,1017236165u,19},
        {0,0,1,12345u,3846340714u,1484396439u,3768992485u,1017236165u,1017236165u,20},
        {99,99,1,1u,4142211138u,1563714726u,79417397u,1017236165u,1017236165u,19},
        {99,99,1,12345u,731058869u,1234926283u,842772325u,1017236165u,1017236165u,22},
        {25,75,1,1u,2649483491u,2026548085u,2107407075u,1017236165u,1017236165u,34},
        {25,75,1,12345u,1141075675u,3695282908u,1663737619u,1017236165u,1017236165u,45},
    };
    for(const auto &c:centerCases) for(bool late:{false,true}) {
        const std::string movedLand="<LAND_GENERATION> base_terrain 6 create_land { terrain_type 6 land_position "+
            std::to_string(c.x)+" "+std::to_string(c.y)+" base_size 0 number_of_tiles 0 } ";
        load(movedLand+(late?std::string(centerStages[c.stage])+"<CLIFF_GENERATION> ":
            "<CLIFF_GENERATION> "+std::string(centerStages[c.stage])));
        uint32_t rng=c.seed;
        check(generateRmsMap(script,{},120,120,data,rng,map,&error),error.c_str());
        uint32_t objects=2166136261u,terrain=2166136261u,heights=2166136261u,slopes=2166136261u,nextId=0;
        auto mixObject=[&](int unit,int facet,float x,float y,uint32_t id) {
            for(uint32_t v:{uint32_t(unit),uint32_t(facet),uint32_t(x*2),uint32_t(y*2),id}) objects=(objects^v)*16777619u;
        };
        for(const auto &o:map.cliffs) {mixObject(o.unit,o.facet,o.x,o.y,o.creationOrdinal);nextId=o.creationOrdinal+1;}
        for(const auto &o:map.scenery) mixObject(o.unitId,o.facet,o.x,o.y,nextId++);
        for(size_t i=0;i<map.terrain.size();++i) {
            terrain=(terrain^map.terrain[i])*16777619u;
            heights=(heights^(map.elevation.empty()?0:map.elevation[i]))*16777619u;
            slopes=(slopes^(map.slopes.empty()?0:map.slopes[i]))*16777619u;
        }
        if(rng!=c.rng || objects!=c.objects || terrain!=c.terrain || heights!=c.heights || slopes!=c.slopes)
            std::cerr<<"cliff land position "<<c.x<<','<<c.y<<" stage "<<c.stage<<" seed "<<c.seed<<" mismatch\n";
        check(rng==c.rng && objects==c.objects && terrain==c.terrain && heights==c.heights && slopes==c.slopes && map.cliffs.size()+map.scenery.size()==c.count,
            "original land-position cliff protection, composition and RNG");
    }
    const std::string playerLand="<LAND_GENERATION> base_terrain 6 create_player_lands { terrain_type 6 base_size 0 number_of_tiles 0 } ";
    struct PlayerCliffCase { int settings; uint32_t seed,rng,hash; size_t count; int starts[4]; int radius=0,tiles=0,terrain=-1; uint32_t terrainHash=0; bool avoid=false; };
    const PlayerCliffCase playerCases[]={
        {0,1u,1152740336u,2017768301u,44,{74,12,46,84}},
        {0,12345u,2690460253u,3186643419u,59,{12,79,84,41}},
        {0,4294967295u,467720497u,2706656024u,48,{84,36,12,84}},
        {1,1u,3791774962u,1277912413u,52,{74,12,46,84}},
        {1,12345u,2227754372u,2858416769u,74,{12,79,84,41}},
        {1,4294967295u,2656191654u,2396337780u,68,{84,36,12,84}},
        {2,1u,2650171262u,2166136261u,0,{74,12,46,84}},
        {2,12345u,1339939592u,2166136261u,0,{12,79,84,41}},
        {2,4294967295u,2952239670u,2166136261u,0,{84,36,12,84}},
        {0,1u,1003445079u,2619903990u,63,{72,14,46,82},3,1000},
        {0,12345u,182360986u,3442638069u,39,{14,77,82,41},3,1000},
        {0,4294967295u,992564139u,3626055600u,43,{82,36,14,82},3,1000},
        {0,1u,3087756617u,455463861u,54,{71,20,49,80},10,2000},
        {0,12345u,3833562541u,2728107736u,43,{20,76,80,44},10,2000},
        {0,4294967295u,1260596611u,249111388u,37,{80,40,20,80},10,2000},
        {0,1u,3293037729u,4232931309u,43,{74,12,46,84},0,8000},
        {0,12345u,4162111261u,4023500210u,59,{12,79,84,41},0,8000},
        {0,4294967295u,3951742847u,2131392925u,44,{84,36,12,84},0,8000},
        {0,1u,4152948138u,2017768301u,44,{74,12,46,84},0,0,0,188518405u},
        {0,12345u,2480538885u,3186643419u,59,{12,79,84,41},0,0,0,531955827u},
        {0,4294967295u,1740280934u,2706656024u,48,{84,36,12,84},0,0,0,2102344771u},
        {0,1u,1443741453u,1522498477u,2069,{74,12,46,84},0,0,21,2215424281u},
        {0,12345u,1591679402u,2149077088u,2042,{12,79,84,41},0,0,21,4168035080u},
        {0,4294967295u,1664064343u,2115634118u,2055,{84,36,12,84},0,0,21,2580829008u},
        {0,1u,1213600036u,2619903990u,63,{72,14,46,82},3,1000,0,2027666005u},
        {0,12345u,31766244u,3442638069u,39,{14,77,82,41},3,1000,0,2324343635u},
        {0,4294967295u,3191885255u,3626055600u,43,{82,36,14,82},3,1000,0,4202677285u},
        {0,1u,405893282u,844241075u,2053,{72,14,46,82},3,1000,21,2532306835u},
        {0,12345u,2188808205u,1283477720u,2078,{14,77,82,41},3,1000,21,1454294088u},
        {0,4294967295u,3434029358u,410909770u,2028,{82,36,14,82},3,1000,21,310295079u},
        {0,1u,4193257045u,455463861u,54,{71,20,49,80},10,2000,0,2048940035u},
        {0,12345u,1349579420u,2728107736u,43,{20,76,80,44},10,2000,0,1461021667u},
        {0,4294967295u,3609843621u,249111388u,37,{80,40,20,80},10,2000,0,2981063379u},
        {0,1u,3538274515u,204127149u,2052,{71,20,49,80},10,2000,21,2027068824u},
        {0,12345u,3054933990u,2616080202u,2077,{20,76,80,44},10,2000,21,320288952u},
        {0,4294967295u,1419233999u,241760370u,2063,{80,40,20,80},10,2000,21,762817966u},
        {0,1u,1721990103u,2017768301u,44,{74,12,46,84},0,0,0,889833443u,true},
        {0,12345u,30862206u,3186643419u,59,{12,79,84,41},0,0,0,4006789717u,true},
        {0,4294967295u,3511738797u,2706656024u,48,{84,36,12,84},0,0,0,1916798691u,true},
        {0,1u,2718820820u,883075635u,2011,{74,12,46,84},0,0,21,161259388u,true},
        {0,12345u,1755922927u,1539462727u,2098,{12,79,84,41},0,0,21,1249312809u,true},
        {0,4294967295u,492326509u,174818381u,2032,{84,36,12,84},0,0,21,4065029198u,true},
        {0,1u,3077030288u,2619903990u,63,{72,14,46,82},3,1000,0,847092901u,true},
        {0,12345u,9214133u,3442638069u,39,{14,77,82,41},3,1000,0,303706389u,true},
        {0,4294967295u,2188604217u,3626055600u,43,{82,36,14,82},3,1000,0,2907341827u,true},
        {0,1u,2281347096u,2116353144u,2055,{72,14,46,82},3,1000,21,459859241u,true},
        {0,12345u,4286372938u,76799251u,2070,{14,77,82,41},3,1000,21,891484783u,true},
        {0,4294967295u,3320408648u,4205153232u,2012,{82,36,14,82},3,1000,21,4229424896u,true},
        {0,1u,412506u,455463861u,54,{71,20,49,80},10,2000,0,2137811957u,true},
        {0,12345u,1997591378u,2728107736u,43,{20,76,80,44},10,2000,0,3520223331u,true},
        {0,4294967295u,2920508933u,249111388u,37,{80,40,20,80},10,2000,0,345214099u,true},
        {0,1u,2531097193u,1232709288u,2075,{71,20,49,80},10,2000,21,2634118633u,true},
        {0,12345u,2056567578u,2261193262u,2035,{20,76,80,44},10,2000,21,1017910930u,true},
        {0,4294967295u,919433396u,1410480440u,2086,{80,40,20,80},10,2000,21,1804893542u,true},
    };
    for(const auto &c:playerCases) for(bool late:{false,true}) {
        const std::string cliff="<CLIFF_GENERATION> "+std::string(settings[c.settings]);
        const std::string growingLand="<LAND_GENERATION> base_terrain 6 create_player_lands { terrain_type 6 base_size "+
            std::to_string(c.radius)+" number_of_tiles "+std::to_string(c.tiles)+" } ";
        const std::string painting="<TERRAIN_GENERATION> "+(c.terrain<0?std::string{}:
            "create_terrain "+std::to_string(c.terrain)+" { base_terrain 6 number_of_tiles 2000 number_of_clumps 20 spacing_to_other_terrain_types 2 clumping_factor 20 "+
            (c.avoid?"set_avoid_player_start_areas ":"")+"} ")+"<OBJECTS_GENERATION> ";
        load(growingLand+(late?painting+cliff:cliff+painting));
        uint32_t state=c.seed;
        check(generateRmsMap(script,{},120,120,data,state,map,&error),error.c_str());
        uint32_t hash=2166136261u,nextId=0,terrainHash=2166136261u;
        auto mix=[&](int unit,int facet,float x,float y,uint32_t id) {
            for(uint32_t v:{uint32_t(unit),uint32_t(facet),uint32_t(x*2),uint32_t(y*2),id}) hash=(hash^v)*16777619u;
        };
        for(const auto &o:map.cliffs) {mix(o.unit,o.facet,o.x,o.y,o.creationOrdinal);nextId=o.creationOrdinal+1;}
        for(const auto &o:map.scenery) mix(o.unitId,o.facet,o.x,o.y,nextId++);
        for(auto tile:map.terrain) terrainHash=(terrainHash^tile)*16777619u;
        if(state!=c.rng || hash!=c.hash) std::cerr<<"player cliffs "<<c.seed<<" settings "<<c.settings<<" rng "<<state<<'/'<<c.rng<<" hash "<<hash<<'/'<<c.hash<<'\n';
        check(state==c.rng && hash==c.hash && map.cliffs.size()+map.scenery.size()==c.count,"original player-land cliffs, scenery and RNG");
        check(map.playerStarts.size()==2 && map.lands.size()==2,"two finalized player lands");
        for(size_t i=0;i<2;++i)
            check(map.playerStarts[i].x==c.starts[2*i] && map.playerStarts[i].y==c.starts[2*i+1] &&
                map.lands[i].center.x==c.starts[2*i] && map.lands[i].center.y==c.starts[2*i+1] &&
                map.lands[i].owner==int(i+1),"original cliff player-start coordinates and ownership");
        if(c.terrain<0) for(auto tile:map.terrain) check(tile==6,"player cliff lands preserve DIRT1");
        else check(terrainHash==c.terrainHash,"original player-land cliff terrain painting");
        check((c.terrain==21 || map.scenery.empty()) && map.objects.empty() && map.ownedObjects.empty(),"cliff player lands imply no starting objects");
    }
    for(const char *stage:{"<ELEVATION_GENERATION> create_elevation 1 { base_terrain 6 number_of_tiles 400 }",
        "<TERRAIN_GENERATION> create_terrain 18 { base_terrain 6 number_of_tiles 2000 }"}) {
        load(playerLand+"<CLIFF_GENERATION> "+stage);
        uint32_t state=1;
        check(!generateRmsMap(script,{},120,120,data,state,map,&error) && state==1 && map.cliffs.empty(),
            "unverified player-land cliff compositions remain transactional");
    }
    for(const char *parameters:{"base_size 11 number_of_tiles 1000","base_size 3 number_of_tiles 8001"}) {
        load("<LAND_GENERATION> base_terrain 6 create_player_lands { terrain_type 6 "+std::string(parameters)+" } <CLIFF_GENERATION>");
        uint32_t state=1;
        check(!generateRmsMap(script,{},120,120,data,state,map,&error) && state==1 && map.cliffs.empty(),
            "unverified player-land cliff sizes remain transactional");
    }
    uint32_t rng=1;
    load(land+"<CLIFF_GENERATION> min_distance_cliffs 0");
    check(!generateRmsMap(script,{},120,120,data,rng,map,&error) && rng==1 && map.cliffs.empty(),"unsupported cliff spacing is transactional");
    load(land+"<CLIFF_GENERATION> <TERRAIN_GENERATION> create_terrain 3 { base_terrain 6 number_of_tiles 10 }");
    check(!generateRmsMap(script,{},120,120,data,rng,map,&error) && rng==1 && map.cliffs.empty(),"cliff/terrain combination remains gated");
    for(const char *rows:{
        "create_terrain 0 { base_terrain 6 number_of_tiles 10 set_avoid_player_start_areas }",
        "create_terrain 0 { base_terrain 6 number_of_tiles 10 } create_terrain 0 { base_terrain 6 number_of_tiles 20 }"}) {
        load(land+"<CLIFF_GENERATION> <TERRAIN_GENERATION> "+rows);
        check(!generateRmsMap(script,{},120,120,data,rng,map,&error) && rng==1 && map.cliffs.empty(),
            "unverified cliff terrain-row combinations remain transactional");
    }
    for(const char *elevation:{
        "<ELEVATION_GENERATION>",
        "<ELEVATION_GENERATION> create_elevation 2 { base_terrain 6 number_of_tiles 400 } <TERRAIN_GENERATION> create_terrain 0 { base_terrain 6 number_of_tiles 30 }",
        "<ELEVATION_GENERATION> create_elevation 1 { base_terrain 6 number_of_tiles 400 } create_elevation 1 { base_terrain 6 number_of_tiles 300 }"}) {
        load(land+"<CLIFF_GENERATION> "+elevation);
        check(!generateRmsMap(script,{},120,120,data,rng,map,&error) && rng==1 && map.cliffs.empty(),
            "unverified cliff elevation combinations remain transactional");
    }
    load(land+"<CLIFF_GENERATION> <ELEVATION_GENERATION> create_elevation 1 { base_terrain 6 number_of_tiles 400 } <TERRAIN_GENERATION> create_terrain 21 { base_terrain 6 number_of_tiles 2000 }");
    check(!generateRmsMap(script,{},120,120,data,rng,map,&error) && rng==1 && map.cliffs.empty(),
        "elevated cliff forest composition remains unverified");
    load("<LAND_GENERATION> base_terrain 6 create_land { terrain_type 6 base_size 0 number_of_tiles 0 } <CLIFF_GENERATION>");
    check(!generateRmsMap(script,{},120,120,data,rng,map,&error) && rng==1 && map.cliffs.empty(),
        "cliffs with randomly placed neutral land remain unverified");
    load(land+"<CLIFF_GENERATION> cliff_curliness nope");
    check(!generateRmsMap(script,{},120,120,data,rng,map,&error) && rng==1,"malformed cliff integer");
}
