// SPDX-License-Identifier: GPL-3.0-or-later
// Actual original 0x4d9950 stage; only final visual-refresh calls skipped.
// Private rms-elevation-stage-oracle-2026-11-14.json, CC EXE c01d78a0...
static void checkElevationStage() {
    struct Case {uint32_t seed,size,profile,levels,heightsHash,slopesHash,maskHash,finalRng;};
    const Case cases[] = {
        {1u,5u,0u,1u,2434127348u,2414772159u,2414772159u,304070270u},
        {1u,5u,0u,4u,2492192915u,2414772159u,2414772159u,101146934u},
        {1u,5u,0u,7u,1514670350u,2414772159u,2414772159u,2473540626u},
        {1u,5u,1u,1u,2414772159u,2414772159u,2414772159u,1303876527u},
        {1u,5u,1u,4u,2414772159u,2414772159u,2414772159u,1303876527u},
        {1u,5u,1u,7u,2414772159u,2414772159u,2414772159u,1303876527u},
        {1u,5u,2u,1u,2414772159u,2414772159u,3351582011u,1u},
        {1u,5u,2u,4u,2414772159u,2414772159u,3351582011u,1u},
        {1u,5u,2u,7u,2414772159u,2414772159u,3351582011u,1u},
        {1u,9u,0u,1u,375737396u,1555472262u,4192459231u,1061836850u},
        {1u,9u,0u,4u,544960429u,559237243u,4192459231u,789369644u},
        {1u,9u,0u,7u,544960429u,559237243u,4192459231u,789369644u},
        {1u,9u,1u,1u,4192459231u,4192459231u,4192459231u,2298630877u},
        {1u,9u,1u,4u,4192459231u,4192459231u,4192459231u,2298630877u},
        {1u,9u,1u,7u,4192459231u,4192459231u,4192459231u,2298630877u},
        {1u,9u,2u,1u,1735815149u,3203616519u,4007083119u,433661943u},
        {1u,9u,2u,4u,3344416704u,3973483424u,506763444u,310991220u},
        {1u,9u,2u,7u,3344416704u,3973483424u,506763444u,293252728u},
        {1u,16u,0u,1u,422363852u,1550721652u,3869364677u,2574251963u},
        {1u,16u,0u,4u,2647634097u,2121588215u,3869364677u,1822780656u},
        {1u,16u,0u,7u,2647634097u,2121588215u,3869364677u,1822780656u},
        {1u,16u,1u,1u,2533767794u,582072344u,3869364677u,126145313u},
        {1u,16u,1u,4u,2533767794u,582072344u,3869364677u,1911746471u},
        {1u,16u,1u,7u,2533767794u,582072344u,3869364677u,1911746471u},
        {1u,16u,2u,1u,2141935104u,829986241u,1797404746u,1924233093u},
        {1u,16u,2u,4u,2482175769u,9942586u,1797404746u,66682888u},
        {1u,16u,2u,7u,2482175769u,9942586u,1797404746u,66682888u},
        {12345u,5u,0u,1u,2434127348u,2414772159u,2414772159u,4028089366u},
        {12345u,5u,0u,4u,2492192915u,2414772159u,2414772159u,1216421628u},
        {12345u,5u,0u,7u,1514670350u,2414772159u,2414772159u,334622001u},
        {12345u,5u,1u,1u,2414772159u,2414772159u,2414772159u,3960859687u},
        {12345u,5u,1u,4u,2414772159u,2414772159u,2414772159u,3960859687u},
        {12345u,5u,1u,7u,2414772159u,2414772159u,2414772159u,3960859687u},
        {12345u,5u,2u,1u,2414772159u,2414772159u,3351582011u,12345u},
        {12345u,5u,2u,4u,2414772159u,2414772159u,3351582011u,12345u},
        {12345u,5u,2u,7u,2414772159u,2414772159u,3351582011u,12345u},
        {12345u,9u,0u,1u,542287959u,1569569864u,4192459231u,372314759u},
        {12345u,9u,0u,4u,3750530785u,2255705835u,4192459231u,67123581u},
        {12345u,9u,0u,7u,3750530785u,2255705835u,4192459231u,67123581u},
        {12345u,9u,1u,1u,4192459231u,4192459231u,4192459231u,2546000789u},
        {12345u,9u,1u,4u,4192459231u,4192459231u,4192459231u,2546000789u},
        {12345u,9u,1u,7u,4192459231u,4192459231u,4192459231u,2546000789u},
        {12345u,9u,2u,1u,3347618393u,3963503437u,3486871682u,1792318582u},
        {12345u,9u,2u,4u,2411975697u,2225733765u,695121913u,3054369549u},
        {12345u,9u,2u,7u,2411975697u,2225733765u,695121913u,3054369549u},
        {12345u,16u,0u,1u,1224978597u,3846188264u,3869364677u,1063484074u},
        {12345u,16u,0u,4u,271317204u,932142735u,3869364677u,1855912166u},
        {12345u,16u,0u,7u,271317204u,932142735u,3869364677u,2331077716u},
        {12345u,16u,1u,1u,105649266u,3275750257u,3869364677u,2723506741u},
        {12345u,16u,1u,4u,105649266u,3275750257u,3869364677u,2141972095u},
        {12345u,16u,1u,7u,105649266u,3275750257u,3869364677u,2141972095u},
        {12345u,16u,2u,1u,1612844108u,2887116499u,1392599289u,431133128u},
        {12345u,16u,2u,4u,1612844108u,2887116499u,1392599289u,3238541470u},
        {12345u,16u,2u,7u,1612844108u,2887116499u,1392599289u,3238541470u},
        {4294967295u,5u,0u,1u,2434127348u,2414772159u,2414772159u,1154355463u},
        {4294967295u,5u,0u,4u,2492192915u,2414772159u,2414772159u,3778237396u},
        {4294967295u,5u,0u,7u,1514670350u,2414772159u,2414772159u,263543913u},
        {4294967295u,5u,1u,1u,2414772159u,2414772159u,2414772159u,2926932541u},
        {4294967295u,5u,1u,4u,2414772159u,2414772159u,2414772159u,2926932541u},
        {4294967295u,5u,1u,7u,2414772159u,2414772159u,2414772159u,2926932541u},
        {4294967295u,5u,2u,1u,2414772159u,2414772159u,3351582011u,4294967295u},
        {4294967295u,5u,2u,4u,2414772159u,2414772159u,3351582011u,4294967295u},
        {4294967295u,5u,2u,7u,2414772159u,2414772159u,3351582011u,4294967295u},
        {4294967295u,9u,0u,1u,62657752u,4124444067u,4192459231u,3837363403u},
        {4294967295u,9u,0u,4u,599501939u,4235376079u,4192459231u,2314526674u},
        {4294967295u,9u,0u,7u,599501939u,4235376079u,4192459231u,2314526674u},
        {4294967295u,9u,1u,1u,4192459231u,4192459231u,4192459231u,889435131u},
        {4294967295u,9u,1u,4u,4192459231u,4192459231u,4192459231u,889435131u},
        {4294967295u,9u,1u,7u,4192459231u,4192459231u,4192459231u,889435131u},
        {4294967295u,9u,2u,1u,2033183699u,35566056u,383401595u,3715874732u},
        {4294967295u,9u,2u,4u,3247818906u,3026084777u,383401595u,2871799934u},
        {4294967295u,9u,2u,7u,3247818906u,3026084777u,383401595u,2871799934u},
        {4294967295u,16u,0u,1u,3000846411u,3523892352u,3869364677u,795693139u},
        {4294967295u,16u,0u,4u,3000846411u,3523892352u,3869364677u,3349289363u},
        {4294967295u,16u,0u,7u,3000846411u,3523892352u,3869364677u,3349289363u},
        {4294967295u,16u,1u,1u,1872165829u,3056255429u,3869364677u,2503559016u},
        {4294967295u,16u,1u,4u,1872165829u,3056255429u,3869364677u,2980725896u},
        {4294967295u,16u,1u,7u,1872165829u,3056255429u,3869364677u,2980725896u},
        {4294967295u,16u,2u,1u,978957166u,1710381155u,1392599289u,2371765394u},
        {4294967295u,16u,2u,4u,1297601557u,4177943861u,1392599289u,2860815158u},
        {4294967295u,16u,2u,7u,1297601557u,4177943861u,1392599289u,2860815158u},
    };
    for (const auto &c:cases) {
        std::vector<uint8_t> terrain(c.size*c.size,6),heights(terrain.size(),5),slopes(terrain.size(),0),mask;
        std::vector<swgb::RmsElevationProtection> protection;
        if (c.profile==2) protection.push_back({static_cast<int32_t>(c.size/2),static_cast<int32_t>(c.size/2),3,20});
        if (c.profile==1) for (uint32_t y=0;y<c.size;++y) for (uint32_t x=0;x<c.size;x+=4) terrain[y*c.size+x]=0;
        std::vector<swgb::RmsElevationRow> rows;
        for (uint32_t level=1;level<=c.levels;++level)
            rows.push_back({30,static_cast<uint8_t>(level),3,level==1?2u:1u,static_cast<uint8_t>(level-1),6});
        uint32_t rng=c.seed;std::string error;
        assert(swgb::generateRmsElevation(c.size,c.size,rows,terrain,rng,heights,slopes,mask,protection,&error));
        assert(hash(heights)==c.heightsHash && hash(slopes)==c.slopesHash && hash(mask)==c.maskHash && rng==c.finalRng);
    }
    // Actual120x120 Forest4+7 finalized row groups, both live protection centers.
    // Uniformterrain6 and worker-entry seed1: a controlled stage, not the live whole map.
    {
        std::vector<uint8_t> terrain(120*120,6),heights(terrain.size(),5),slopes(terrain.size()),mask;
        std::vector<swgb::RmsElevationRow> rows;
        for (uint32_t level=1;level<=4;++level)
            rows.push_back({1008,static_cast<uint8_t>(level),10,level==1?2u:1u,static_cast<uint8_t>(level-1),6});
        for (uint32_t level=1;level<=7;++level)
            rows.push_back({6192,static_cast<uint8_t>(level),7,level==1?2u:1u,static_cast<uint8_t>(level-1),6});
        uint32_t rng=1;std::string error;
        assert(swgb::generateRmsElevation(120,120,rows,terrain,rng,heights,slopes,mask,
            {{19,36,13,20},{42,96,13,20}},&error));
        assert(hash(heights)==696616561u && hash(slopes)==1739620962u &&
            hash(mask)==712632647u && rng==1103906832u);
    }
    // Actual zero-row4d9950 keeps initial height/slope/RNG, but rebuilds protection.
    {
        std::vector<uint8_t> terrain(81,6),heights(81,5),slopes(81,9),mask(81,99);
        uint32_t rng=12345;std::string error;
        assert(swgb::generateRmsElevation(9,9,{},terrain,rng,heights,slopes,mask,{{4,4,3,20}},&error));
        assert(rng==12345 && heights==std::vector<uint8_t>(81,5) && slopes==std::vector<uint8_t>(81,9));
        assert(mask[4*9+4]==60 && mask[4*9+5]==40 && mask[4*9+6]==20 && mask[0]==0);
    }
    // A failure in a later row must not expose the earlier row's successful mutations.
    std::vector<uint8_t> terrain(25,6),heights(25,5),slopes(25,9),mask(25,7);
    const auto beforeHeights=heights,beforeSlopes=slopes,beforeMask=mask;
    uint32_t rng=1;std::string error;
    assert(!swgb::generateRmsElevation(5,5,{{10,1,1,2,0,6},{10,8,1,2,1,6}},terrain,rng,heights,slopes,mask,{},&error));
    assert(rng==1 && heights==beforeHeights && slopes==beforeSlopes && mask==beforeMask && !error.empty());
}
