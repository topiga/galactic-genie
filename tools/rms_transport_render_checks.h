// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <filesystem>
#include <fstream>
// Original54a643/54a6d8: transport layer19 uses ground coordinates, layer20 uses object coordinates.
static void rmsTransportRenderChecks(const std::string &dataDirectory) {
    struct Capture:swgb::SoftRenderer {
        std::vector<std::pair<swgb::Texture*,swgb::Quad>> quads;
        void draw(swgb::Texture *texture,const swgb::Quad &quad) override {
            quads.push_back({texture,quad});SoftRenderer::draw(texture,quad);
        }
    } renderer;
    swgb::Assets assets(&renderer);std::string error;
    check(assets.init(dataDirectory,&error),error.c_str());
    std::ifstream input(std::filesystem::path(__FILE__).parent_path()/"fixtures/cliff-full-start.rms");
    check(bool(input),"transport render fixture opens");
    std::string source((std::istreambuf_iterator<char>(input)),{});
    source+="\ncreate_object 1521 { set_place_for_every_player match_player_civ number_of_objects 1 min_distance_to_players 3 max_distance_to_players 6 }\n";
    swgb::RmsScript script;swgb::Game game(assets);uint32_t rng=1;
    check(script.load(60000,[&](int32_t,std::vector<uint8_t> &bytes){bytes.assign(source.begin(),source.end());return true;},
        swgb::RmsScript::Limits{},&error),error.c_str());
    check(game.initRms(script,{"FIXED_POSITIONS","TINY_MAP","RANDOM_MAP","2_PLAYERS","DEFAULT_RESOURCES"},
        120,rng,&error,2,{1,1}),error.c_str());
    game.setClassicHud(false);game.setZoom(1);check(game.lookAtObject(6236),"transport exists for rendering");
    game.render(renderer,960,544);
    float x,y;check(game.objectScreenPosition(6236,960,544,x,y),"transport projects");
    const float drop=assets.dat().terrainBlock.tileHalfHeight;
    check(x==480 && y==272-drop,"transport height uses half-tile scale");
    for(int id:{5460,5461}) {
        const auto *graphic=assets.dat().graphic(id);
        check(graphic && graphic->layer==(id==5460?19:20),"original transport graphic layers");
        const auto *sheet=assets.sheet(graphic->slp,assets.dat().playerColours[0].playerColorBase);
        bool found=false;
        if(sheet)for(const auto &frame:sheet->frames)for(const auto &entry:renderer.quads) {
            const auto &q=entry.second;
            if(entry.first!=frame.tex || q.w!=frame.w || q.h!=frame.h || q.v0!=frame.v || std::min(q.u0,q.u1)!=frame.u)continue;
            const float anchorX=q.x+(q.u1<q.u0?frame.w-frame.hotX:frame.hotX);
            if(std::abs(anchorX-x)<.001f && std::abs(q.y+frame.hotY-(id==5460?272:y))<.001f)found=true;
        }
        check(found,id==5460?"transport shadow remains on ground":"transport sprite uses airborne coordinates");
    }
    std::puts("PASS: original transport sprite, shadow and screen projection");
}
