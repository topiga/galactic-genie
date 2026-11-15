// SPDX-License-Identifier: GPL-3.0-or-later
#include "rms_object_rows.h"
#include <algorithm>
#include <charconv>
#include <limits>
#include <string_view>

namespace swgb {
// Original4e3850..4e3cfb; saved rms-object-static-evidence-2026-11-12.json,
// object-command block SHA256 a1f6114957da9fc69b7ff25af06592d51ab1c6a2a26c565436a3ae9b1c06664c.
bool parseRmsObjectRow(const std::vector<RmsToken> &tokens, size_t &index,
                      RmsObjectRow &out, std::string *err) {
    if(err) err->clear();
    size_t cursor=index;
    auto fail=[&](const std::string &message) {
        if(err) {
            *err="RMS object row: "+message;
            if(cursor<tokens.size()) *err+=" at resource "+std::to_string(tokens[cursor].resourceId)+
                ":"+std::to_string(tokens[cursor].line);
        }
        return false;
    };
    auto number=[&](int32_t &value,int32_t minimum,int32_t maximum) {
        if(cursor>=tokens.size()) return fail("missing integer");
        std::string_view text=tokens[cursor].text;
        if(!text.empty() && text.front()=='+') {
            text.remove_prefix(1);
            if(text.empty() || text.front()=='-') return fail("invalid integer: "+tokens[cursor].text);
        }
        const auto result=std::from_chars(text.data(),text.data()+text.size(),value);
        if(result.ec!=std::errc{} || result.ptr!=text.data()+text.size() ||
           value<minimum || value>maximum) return fail("integer outside supported range: "+tokens[cursor].text);
        ++cursor;
        return true;
    };
    constexpr int32_t limit=std::numeric_limits<int32_t>::max();
    RmsObjectRow row;
    if(!number(row.unitId,0,32767)) return false;
    if(cursor==tokens.size() || tokens[cursor].text!="{") return fail("expected object block");
    ++cursor;
    while(cursor<tokens.size() && tokens[cursor].text!="}") {
        const auto &name=tokens[cursor].text;
        if(name=="set_scaling_to_map_size") { row.scaling=1;++cursor;continue; }
        if(name=="set_scaling_to_player_number") { row.scaling=2;++cursor;continue; }
        if(name=="set_gaia_object_only") { row.owner=0;++cursor;continue; }
        if(name=="set_place_for_every_player") { row.landId=1;++cursor;continue; }
        if(name=="match_player_civ") { row.matchPlayerCiv=1;++cursor;continue; }
        if(name=="set_loose_grouping" || name=="set_tight_grouping") {
            if(row.mode==0) { row.perGroup=row.groups;row.groups=1; }
            row.mode=name=="set_loose_grouping"?1:2;
            ++cursor;continue;
        }
        int32_t *field=nullptr,minimum=0,maximum=limit;
        if(name=="number_of_groups" || name=="number_of_objects") {}
        else if(name=="group_variance") field=&row.variance;
        else if(name=="group_placement_radius") field=&row.radius;
        else if(name=="terrain_to_place_on") { field=&row.terrain;minimum=-1;maximum=255; }
        else if(name=="place_on_specific_land_id") { field=&row.landId;maximum=limit-10; }
        else if(name=="min_distance_to_players") { field=&row.minDistance;minimum=-1; }
        else if(name=="max_distance_to_players") { field=&row.maxDistance;minimum=-1; }
        else if(name=="min_distance_group_placement") field=&row.spacing;
        else if(name=="temp_min_distance_group_placement") field=&row.temporarySpacing;
        else if(name=="max_distance_to_other_zones") field=&row.zoneDistance;
        else return fail("unsupported object directive "+name);
        ++cursor;
        int32_t value=0;
        if(!number(value,minimum,maximum)) return false;
        if(name=="number_of_groups") {
            if(row.mode==0) { row.perGroup=row.groups;row.mode=1; }
            row.groups=value;
        } else if(name=="number_of_objects") {
            (row.mode==0?row.groups:row.perGroup)=value;
        } else {
            if(name=="place_on_specific_land_id") value+=10;
            if((name=="min_distance_to_players" || name=="max_distance_to_players") && row.landId==-1)
                row.landId=-2;
            *field=value;
        }
    }
    if(cursor==tokens.size()) return fail("unterminated object block");
    index=cursor+1;
    out=row;
    return true;
}

bool finalizeRmsObjectRow(const RmsObjectRow &row, uint32_t width, uint32_t height,
                         uint32_t playerCount, RmsObjectRow &out, std::string *err) {
    if(err) err->clear();
    auto fail=[&](const char *message) {
        if(err) *err=std::string("RMS object row: ")+message;
        return false;
    };
    constexpr uint64_t limit=std::numeric_limits<int32_t>::max();
    if(!width || !height || width>limit || height>limit || playerCount>32766 ||
       row.mode>2 || row.scaling>2 || row.groups<0 || row.perGroup<0 || row.variance<0 || row.radius<0)
        return fail("invalid dimensions, player count or grouping fields");
    RmsObjectRow result=row;
    if(row.scaling) {
        const uint64_t factor=row.scaling==1?uint64_t(width)*height:playerCount;
        if(factor>limit || (row.groups && factor>limit/uint32_t(row.groups)))
            return fail("group scaling exceeds original signed arithmetic");
        const uint64_t product=factor*uint32_t(row.groups);
        result.groups=static_cast<int32_t>(std::max(uint64_t(1),row.scaling==1?product/10000:product));
    }
    out=result;
    return true;
}
} // namespace swgb
