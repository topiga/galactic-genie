// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Actual 495f80 + Gaia/411 constructors, two 120x120 prefixes overlaid with
// original generated hills, plus every original slope ID. Placement and RNG
// are independent of geometry for this prototype; height callbacks are recorded
// privately and are tested separately through Game height import.
static void checkElevatedSceneryOriginal(const swgb::dat::DatFile &data) {
    using namespace swgb;
    {
        // Seed 1, True.
        struct Run { uint8_t terrain; unsigned count; };
        const Run runs[] = {{41,1031}, {9,3}, {41,115}, {9,7}, {41,113}, {9,7}, {41,112}, {9,9}, {41,109}, {9,11}, {41,108}, {9,12}, {41,108}, {9,12}, {41,108}, {9,12}, {41,110}, {9,11}, {41,109}, {9,11}, {41,109}, {9,3}, {6,2}, {9,5}, {41,114}, {6,2}, {9,4}, {41,303}, {10,1}, {41,118}, {10,3}, {41,116}, {10,3}, {41,118}, {10,1}, {41,119}, {10,1}, {41,6023}, {9,5}, {41,115}, {9,2}, {6,1}, {9,5}, {41,112}, {9,1}, {6,3}, {9,4}, {41,109}, {9,5}, {6,1}, {9,6}, {41,108}, {9,12}, {41,108}, {9,12}, {41,108}, {9,12}, {41,109}, {9,11}, {41,109}, {9,12}, {41,109}, {9,11}, {41,109}, {9,11}, {41,115}, {9,2}, {41,92}, {10,1}, {41,119}, {10,3}, {41,115}, {10,3}, {41,116}, {10,3}, {41,597}, {10,1}, {41,118}, {10,3}, {41,115}, {10,4}, {41,118}, {10,1}, {41,413}, {10,1}, {41,118}, {10,4}, {41,117}, {10,4}, {41,118}, {10,1}, {41,1725}};
        std::vector<uint8_t> terrain;
        for (const auto &r:runs) terrain.insert(terrain.end(),r.count,r.terrain);
        uint32_t rng=1661911417u;
        std::vector<RmsSceneryPlacement> actual; std::string error;
        assert(generateRmsScenery(120,120,terrain,data,rng,actual,&error));
        assert(error.empty() && rng==2732732815u && actual.size()==38);
        struct Expected { int tile; uint8_t facet; };
        const Expected expected[] = {{2660,0}, {2779,10}, {2780,4}, {2781,3}, {2898,11}, {2899,10}, {2900,10}, {3019,2}, {3139,7}, {10582,10}, {10702,1}, {10703,1}, {10704,4}, {10820,6}, {10821,11}, {10822,10}, {10939,9}, {10940,4}, {10941,11}, {11539,0}, {11658,11}, {11659,1}, {11660,9}, {11776,1}, {11777,2}, {11778,6}, {11779,6}, {11898,2}, {12312,11}, {12431,0}, {12432,2}, {12433,11}, {12434,5}, {12552,11}, {12553,11}, {12554,11}, {12555,0}, {12674,3}};
        for (size_t i=0;i<actual.size();++i) {
            assert(actual[i].unitId==411 && actual[i].alive);
            assert(actual[i].x==float(expected[i].tile%120)+.5f);
            assert(actual[i].y==float(expected[i].tile/120)+.5f);
            assert(actual[i].facet==expected[i].facet);
        }
    }
    {
        // Seed 1, allSlopes.
        struct Run { uint8_t terrain; unsigned count; };
        const Run runs[] = {{10,17}, {6,14383}};
        std::vector<uint8_t> terrain;
        for (const auto &r:runs) terrain.insert(terrain.end(),r.count,r.terrain);
        uint32_t rng=1661911417u;
        std::vector<RmsSceneryPlacement> actual; std::string error;
        assert(generateRmsScenery(120,120,terrain,data,rng,actual,&error));
        assert(error.empty() && rng==3852746472u && actual.size()==17);
        struct Expected { int tile; uint8_t facet; };
        const Expected expected[] = {{0,0}, {1,10}, {2,4}, {3,3}, {4,11}, {5,10}, {6,10}, {7,2}, {8,7}, {9,10}, {10,1}, {11,1}, {12,4}, {13,6}, {14,11}, {15,10}, {16,9}};
        for (size_t i=0;i<actual.size();++i) {
            assert(actual[i].unitId==411 && actual[i].alive);
            assert(actual[i].x==float(expected[i].tile%120)+.5f);
            assert(actual[i].y==float(expected[i].tile/120)+.5f);
            assert(actual[i].facet==expected[i].facet);
        }
    }
    {
        // Seed 12345, True.
        struct Run { uint8_t terrain; unsigned count; };
        const Run runs[] = {{41,4398}, {9,4}, {41,115}, {9,9}, {41,111}, {9,12}, {41,109}, {9,11}, {41,109}, {9,11}, {41,108}, {9,11}, {41,109}, {9,6}, {6,1}, {9,4}, {41,109}, {9,5}, {6,3}, {9,3}, {41,109}, {9,6}, {6,1}, {9,4}, {41,110}, {9,10}, {41,110}, {9,7}, {41,114}, {9,6}, {41,61}, {10,1}, {41,55}, {9,3}, {41,61}, {10,3}, {41,116}, {10,5}, {41,118}, {10,1}, {41,307}, {10,1}, {41,118}, {10,3}, {41,117}, {10,2}, {41,118}, {10,2}, {41,119}, {10,1}, {41,1290}, {10,2}, {41,117}, {10,2}, {41,116}, {10,4}, {41,66}, {9,2}, {41,50}, {10,1}, {41,66}, {9,4}, {41,112}, {9,9}, {41,111}, {9,10}, {41,110}, {9,10}, {41,110}, {9,3}, {6,1}, {9,6}, {41,61}, {10,2}, {41,47}, {9,3}, {6,2}, {9,7}, {41,58}, {10,3}, {41,46}, {9,4}, {6,1}, {9,8}, {41,60}, {10,1}, {41,46}, {9,13}, {41,59}, {10,3}, {41,45}, {9,11}, {41,62}, {10,1}, {41,48}, {9,9}, {41,113}, {9,7}, {41,114}, {9,5}, {41,4422}};
        std::vector<uint8_t> terrain;
        for (const auto &r:runs) terrain.insert(terrain.end(),r.count,r.terrain);
        uint32_t rng=3766631823u;
        std::vector<RmsSceneryPlacement> actual; std::string error;
        assert(generateRmsScenery(120,120,terrain,data,rng,actual,&error));
        assert(error.empty() && rng==2087641909u && actual.size()==38);
        struct Expected { int tile; uint8_t facet; };
        const Expected expected[] = {{5786,4}, {5906,10}, {5907,3}, {5908,0}, {6025,11}, {6026,4}, {6027,1}, {6028,5}, {6029,4}, {6148,3}, {6456,2}, {6575,11}, {6576,7}, {6577,1}, {6695,4}, {6696,2}, {6815,8}, {6816,10}, {6936,3}, {8227,0}, {8228,8}, {8346,9}, {8347,5}, {8464,3}, {8465,6}, {8466,3}, {8467,11}, {8586,4}, {9200,2}, {9201,5}, {9319,8}, {9320,5}, {9321,2}, {9441,3}, {9560,10}, {9561,7}, {9562,8}, {9681,11}};
        for (size_t i=0;i<actual.size();++i) {
            assert(actual[i].unitId==411 && actual[i].alive);
            assert(actual[i].x==float(expected[i].tile%120)+.5f);
            assert(actual[i].y==float(expected[i].tile/120)+.5f);
            assert(actual[i].facet==expected[i].facet);
        }
    }
    {
        // Seed 12345, allSlopes.
        struct Run { uint8_t terrain; unsigned count; };
        const Run runs[] = {{10,17}, {6,14383}};
        std::vector<uint8_t> terrain;
        for (const auto &r:runs) terrain.insert(terrain.end(),r.count,r.terrain);
        uint32_t rng=3766631823u;
        std::vector<RmsSceneryPlacement> actual; std::string error;
        assert(generateRmsScenery(120,120,terrain,data,rng,actual,&error));
        assert(error.empty() && rng==3688670246u && actual.size()==17);
        struct Expected { int tile; uint8_t facet; };
        const Expected expected[] = {{0,4}, {1,10}, {2,3}, {3,0}, {4,11}, {5,4}, {6,1}, {7,5}, {8,4}, {9,3}, {10,2}, {11,11}, {12,7}, {13,1}, {14,4}, {15,2}, {16,8}};
        for (size_t i=0;i<actual.size();++i) {
            assert(actual[i].unitId==411 && actual[i].alive);
            assert(actual[i].x==float(expected[i].tile%120)+.5f);
            assert(actual[i].y==float(expected[i].tile/120)+.5f);
            assert(actual[i].facet==expected[i].facet);
        }
    }
}
