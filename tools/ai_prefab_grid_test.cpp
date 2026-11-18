// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/engine/ai_prefab_grid.h"
#include <cassert>
#include <iostream>
uint32_t fingerprint(const swgb::AiBuildPlacementGrid &grid) {
    uint32_t value=2166136261u;for(uint8_t cell:grid.cells())value=(value^cell)*16777619u;return value;
}
int main() {
    // Complete-grid fingerprints from executed original53-bit x87 controls.
    {swgb::AiBuildPlacementGrid grid(32);grid.ownedBuilding(109,16.0f,16.0f,2.0f,6.0f,10.0f,10.0f);
     assert(fingerprint(grid)==463919817u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.ownedBuilding(70,16.0f,16.0f,1.0f,6.0f,10.0f,10.0f);
     assert(fingerprint(grid)==1237276385u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.ownedBuilding(68,16.0f,16.0f,2.0f,6.0f,10.0f,10.0f);
     assert(fingerprint(grid)==3145056629u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.ownedBuilding(12,16.0f,16.0f,2.0f,6.0f,10.0f,10.0f);
     assert(fingerprint(grid)==536633461u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.ownedBuilding(87,16.0f,16.0f,2.0f,6.0f,10.0f,10.0f);
     assert(fingerprint(grid)==404997749u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.ownedBuilding(335,16.0f,16.0f,2.0f,6.0f,10.0f,10.0f);
     assert(fingerprint(grid)==2503165557u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.ownedBuilding(335,16.0f,16.0f,2.0f,6.0f,10.0f,4.0f);
     assert(fingerprint(grid)==1550478581u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.ownedBuilding(109,1.5f,1.5f,2.0f,6.0f,10.0f,10.0f);
     assert(fingerprint(grid)==445941285u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.ownedBuilding(70,1.0f,1.0f,1.0f,6.0f,10.0f,10.0f);
     assert(fingerprint(grid)==3833906577u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.ownedBuilding(12,16.0f,16.0f,2.0f,6.0f,5.0f,10.0f);
     assert(fingerprint(grid)==27997701u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.resource(0,27,16.f,16.f,0.5f,0.5f,false);
     assert(fingerprint(grid)==503765118u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.resource(2,29,16.f,16.f,0.5f,0.5f,false);
     assert(fingerprint(grid)==503765118u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.resource(3,30,16.f,16.f,0.5f,0.5f,false);
     assert(fingerprint(grid)==503765118u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.resource(1,31,16.f,16.f,0.5f,0.5f,false);
     assert(fingerprint(grid)==900439493u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.resource(0,27,16.f,16.f,0.5f,0.5f,true);
     assert(fingerprint(grid)==900439493u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.terrain(16,16,0,0);
     assert(fingerprint(grid)==900439493u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.terrain(16,16,1,0);
     assert(fingerprint(grid)==545573246u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.terrain(16,16,22,0);
     assert(fingerprint(grid)==545573246u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.terrain(16,16,23,0);
     assert(fingerprint(grid)==545573246u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.terrain(16,16,21,0);
     assert(fingerprint(grid)==900439493u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.terrain(16,16,0,1);
     assert(fingerprint(grid)==3570042246u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.terrain(16,16,0,16384);
     assert(fingerprint(grid)==3570042246u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.terrain(16,16,0,4);
     assert(fingerprint(grid)==900439493u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.terrain(22,22,0,0);
     grid.ownedBuilding(109,16.f,16.f,2.0f,6.f,10.f,0.f);
     assert(fingerprint(grid)==463919817u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.terrain(22,22,0,0);
     grid.ownedBuilding(70,16.f,16.f,1.0f,6.f,10.f,0.f);
     assert(fingerprint(grid)==1237276385u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.terrain(22,22,1,0);
     grid.ownedBuilding(109,16.f,16.f,2.0f,6.f,10.f,0.f);
     assert(fingerprint(grid)==641988470u);}
    // Executed58e770 known-building controls, complete-grid fingerprints.
    {swgb::AiBuildPlacementGrid grid(32);grid.knownBuilding(109,16.0f,16.0f,2.0f,false,6.0f,6.0f,false);
     assert(fingerprint(grid)==1400240837u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.knownBuilding(70,16.0f,16.0f,1.f,false,6.0f,6.0f,false);
     assert(fingerprint(grid)==1436224289u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.knownBuilding(109,16.0f,16.0f,2.0f,true,6.0f,6.0f,false);
     assert(fingerprint(grid)==162827381u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.knownBuilding(79,16.0f,16.0f,2.0f,false,6.0f,6.0f,false);
     assert(fingerprint(grid)==1400240837u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.knownBuilding(82,16.0f,16.0f,2.0f,false,6.0f,6.0f,false);
     assert(fingerprint(grid)==1400240837u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.knownBuilding(109,16.0f,16.0f,2.0f,false,2.5f,6.0f,false);
     assert(fingerprint(grid)==255413374u);}
    {swgb::AiBuildPlacementGrid grid(32);
     assert(fingerprint(grid)==900439493u);}
    {swgb::AiBuildPlacementGrid grid(32);
     assert(fingerprint(grid)==900439493u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.knownBuilding(109,1.0f,1.0f,2.0f,false,6.0f,6.0f,false);
     assert(fingerprint(grid)==900439493u);}
    {swgb::AiBuildPlacementGrid grid(32);grid.knownBuilding(109,16.0f,16.0f,2.0f,false,6.0f,3.5f,true);
     assert(fingerprint(grid)==1107941217u);}
    // Nine actual category8 owned-building matrices, including clipped bounds.
    {swgb::AiBuildPlacementGrid grid(32,8);grid.ownedBuilding(109,16.0f,16.0f,2.0f,6.0f,10.f,10.f);
     assert(fingerprint(grid)==1737087157u);}
    {swgb::AiBuildPlacementGrid grid(32,8);grid.ownedBuilding(70,16.0f,16.0f,1.0f,6.0f,10.f,10.f);
     assert(fingerprint(grid)==1436224289u);}
    {swgb::AiBuildPlacementGrid grid(32,8);grid.ownedBuilding(68,16.0f,16.0f,2.0f,6.0f,10.f,10.f);
     assert(fingerprint(grid)==21307237u);}
    {swgb::AiBuildPlacementGrid grid(32,8);grid.ownedBuilding(12,16.0f,16.0f,2.0f,6.0f,10.f,10.f);
     assert(fingerprint(grid)==162827381u);}
    {swgb::AiBuildPlacementGrid grid(32,8);grid.ownedBuilding(87,16.0f,16.0f,2.0f,6.0f,10.f,10.f);
     assert(fingerprint(grid)==162827381u);}
    {swgb::AiBuildPlacementGrid grid(32,8);grid.ownedBuilding(50,16.0f,16.0f,1.5f,6.0f,10.f,10.f);
     assert(fingerprint(grid)==2944849142u);}
    {swgb::AiBuildPlacementGrid grid(32,8);grid.ownedBuilding(109,16.0f,16.0f,2.0f,3.0f,10.f,10.f);
     assert(fingerprint(grid)==2583539685u);}
    {swgb::AiBuildPlacementGrid grid(32,8);grid.ownedBuilding(68,16.0f,16.0f,2.0f,3.0f,10.f,10.f);
     assert(fingerprint(grid)==1244288423u);}
    {swgb::AiBuildPlacementGrid grid(32,8);grid.ownedBuilding(109,1.5f,1.5f,2.0f,6.0f,10.f,10.f);
     assert(fingerprint(grid)==317486272u);}
    std::cout<<"Original prefab and farm grid checks passed\n";
}
