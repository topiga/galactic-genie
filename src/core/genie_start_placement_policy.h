// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

namespace swgb {
// Original target object/master inputs. This is the empty-owner109/83/293
// policy only, not arbitrary building placement or editor interaction.
struct GenieStartObstacle {
    int id=0,type=0,unitClass=0,owner=0,state=2;
    float halfX=0,halfY=0,height=0,speed=0;
    bool canBeBuiltOn=false,pendingBuilding=false;
    bool hasTile=false;
    int terrainId=0; // Terrain byte at object[0x28]+5; object+28 points to its tile.
};

// 5c7b50 for109; caller supplies both directed alliance results from4c0450.
// Spatial overlap and excluded-object filtering are separate predicates.
inline bool genieStartCenterObstacle(const GenieStartObstacle &o,int owner,
                                     bool mutualAlliance,int gameMode) {
    if (o.state>=7) return false;
    const bool ordinaryBuilding=o.type==80 && o.unitClass!=34 && o.unitClass!=36;
    if (ordinaryBuilding && o.pendingBuilding && !mutualAlliance) return false;
    if (o.type>=30 && o.state==2 && o.speed>0) {
        if ((mutualAlliance && gameMode!=6) || o.owner==0 || o.owner==owner)
            return false;
    }
    if (o.canBeBuiltOn) return false;
    if (o.unitClass==31 && o.state>2 && o.hasTile &&
        !(o.terrainId==10 || o.terrainId==13 ||
          (o.terrainId>=17 && o.terrainId<=21))) return false;
    float height=o.height;
    if (height==0 && ((o.type==80 && o.id!=50) ||
                      o.unitClass==23 || o.unitClass==25)) height=1;
    return o.halfX>0 && o.halfY>0 &&
           (height>0 || o.id==50 || o.unitClass==27);
}

// 5c92e0 for groundworkers83/293 with arg12=0: other workers DO block.
// Candidate flyMode is0, so the both-flying shortcut is inapplicable.
inline bool genieStartWorkerObstacle(const GenieStartObstacle &o) {
    if (o.state>=7 || o.canBeBuiltOn) return false;
    if (o.type==80 && o.unitClass!=34 && o.unitClass!=36 && o.pendingBuilding)
        return false;
    float height=o.height;
    if (o.unitClass==27 || (height==0 && o.type==80 && o.id!=50)) height=1;
    return o.halfX>0 && o.halfY>0 && height>0;
}
} // namespace swgb
