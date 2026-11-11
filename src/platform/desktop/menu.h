// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../../engine/assets.h"
#include "../../engine/font_atlas.h"
#include <SDL.h>
#include <stdexcept>
#include <algorithm>
#include <array>
#include <sstream>
#include "native_text.h"
#include "preferences.h"
#include "../../core/language_strings.h"

namespace desktop {
enum class Screen { Launcher, Main, SinglePlayer, Setup, Options, Help, Playing, Pause, Resign, Restart };
enum class Action { None, Start, Quit, Mute, Resign, Volume, SaveSettings };
struct Button { SDL_Rect rect; std::string label; int command; bool enabled = true; };

// Desktop navigation uses the original installed art; no game assets are bundled.
class Menu {
    swgb::Renderer &renderer_;
    swgb::LanguageStrings strings_;
    std::array<uint8_t,3> mainFocus_, setupFocus_, pauseFocus_, optionsFocus_;
    std::array<swgb::Rgba,6> mainBevel_{};
    int captionPixels_=11, buttonPixels_=15, dialogPixels_=17;
    bool captionBold_=true, buttonBold_=false, dialogBold_=false;
    const std::string &original(int id) const {
        const auto &value=strings_.get(id);
        if(value.empty()) throw std::runtime_error("Missing interface string: "+std::to_string(id));
        return value;
    }
    std::string singleLineOriginal(int id) const {
        auto value=original(id);std::replace(value.begin(),value.end(),'\n',' ');return value;
    }
#ifdef __APPLE__
    NativeText nativeText_;
#endif
    swgb::Texture *background_ = nullptr, *logo_ = nullptr, *launcher_ = nullptr, *font_ = nullptr, *button_ = nullptr, *focused_ = nullptr, *setup_ = nullptr, *pauseArt_ = nullptr, *optionsArt_ = nullptr, *singleHighlight_ = nullptr, *optionsHighlight_ = nullptr, *zone_ = nullptr, *singlePressed_ = nullptr, *optionsPressed_ = nullptr;
    Screen screen_ = Screen::Launcher, return_ = Screen::Launcher;
    int focus_ = 0, pressed_ = -1, hovered_ = -1, music_ = 100, effects_ = 100;
    int savedMusic_ = 100, savedEffects_ = 100;
    bool initiallyExplored_ = false, conquest_=true;
    int setupOpen_=0, humanCiv_=1, aiCiv_=3, populationLimit_=75;
    static constexpr int Back = 0, Play = 1, Setup = 2, Options = 3, Help = 4,
                         Quit = 5, Mute = 6, Resume = 7, Resign = 8, Confirm = 9, MusicDown = 10, MusicUp = 11, EffectsDown = 12, EffectsUp = 13, SinglePlayer = 14, Restart = 15, ConfirmRestart = 16, ApplyOptions = 17, RevealMap = 18, RevealStandard = 19, RevealExplored = 20, HumanCiv = 21, AiCiv = 22, Population = 23, Victory = 24, Conquest = 25, CommandCenter = 26, CivChoice = 1000, PopulationChoice = 2000;
    swgb::Texture *sprite(swgb::Slp &slp, const swgb::Palette &palette, int frame) {
        swgb::SlpImage img; std::string error;
        if (!slp.decode(frame, img, &error)) throw std::runtime_error(error);
        std::vector<uint8_t> rgba(size_t(img.width) * img.height * 4);
        swgb::colorize(img, palette, {}, rgba.data(), img.width);
        return renderer_.createTexture(img.width, img.height, rgba.data());
    }
    void image(swgb::Texture *texture, float x, float y) {
        if (texture) renderer_.draw(texture, {x,y,float(texture->width),float(texture->height),0,0,float(texture->width),float(texture->height)});
    }
    void text(const std::string &value, float x, float y, bool bright = true) {
#ifdef __APPLE__
        nativeText_.draw(value,x,y,bright,captionPixels_,captionBold_);
#else
        const auto &face = swgb::font::faces[2];
        for (unsigned char c : value) {
            if (c < 32 || c > 126) continue;
            const auto &g = face.glyphs[c - 32];
            swgb::Quad q{x + g.xoff, y + g.yoff, float(g.w), float(g.h),float(g.x),float(g.y),float(g.x+g.w),float(g.y+g.h)};
            swgb::Quad shadow=q; shadow.x+=1; shadow.y+=1;
            renderer_.drawTinted(font_,shadow,0,0,0,255);
            renderer_.drawTinted(font_, q, bright?245:140, bright?245:150, bright?245:160,255);
            x += g.advance;
        }
#endif
    }
    void set(Screen s) { setupOpen_=0; screen_ = s; focus_ = 0; pressed_ = -1; hovered_ = -1; }
    void closeSetupChoices() {
        focus_=setupOpen_==HumanCiv?3:setupOpen_==AiCiv?4:setupOpen_==Population?5:setupOpen_==Victory?6:2;setupOpen_=0;
    }
public:
    Menu(swgb::Renderer &renderer, const std::string &root, const Preferences &settings = {}) : renderer_(renderer)
#ifdef __APPLE__
        , nativeText_(renderer)
#endif
    {
        if(!settings.valid()) throw std::runtime_error("Invalid menu settings");
        music_=settings.music;effects_=settings.effects;humanCiv_=settings.humanCiv;aiCiv_=settings.aiCiv;
        populationLimit_=settings.population;initiallyExplored_=settings.explored;conquest_=settings.conquest;
        std::string languageError;
        if(!strings_.load(root+"/language.dll",&languageError) ||
           !strings_.load(root+"/language_x1.dll",&languageError,true))
            throw std::runtime_error(languageError);
#ifdef __APPLE__
        nativeText_.family(original(102));
        auto fontRole=[&](int id,int &pixels,bool &bold) {
            if(original(id)!=original(102)) throw std::runtime_error("Mixed interface font families are not supported yet");
            const auto &size=original(id+1); size_t consumed=0;
            const int points=std::stoi(size,&consumed);
            if(consumed!=size.size() || points<1 || points>72 || (original(id+2)!="N" && original(id+2)!="B"))
                throw std::runtime_error("Invalid interface font definition: "+std::to_string(id));
            // shortcut: font-role assignments and 96-DPI sizing need runtime GDI confirmation.
            pixels=int(std::lround(points*96.0/72.0)); bold=original(id+2)=="B";
        };
        fontRole(119,captionPixels_,captionBold_);
        fontRole(146,buttonPixels_,buttonBold_);
        fontRole(110,dialogPixels_,dialogBold_);
#endif
        // Read expansion first, matching Assets' resource precedence.
        swgb::ResourceSet resources; std::string error;
        resources.add(swgb::findFileNoCase(root + "/Data", "interfac_x1.drs"));
        resources.add(swgb::findFileNoCase(root + "/Data", "interfac.drs"));
        std::vector<uint8_t> bytes; swgb::Palette palette; swgb::Slp slp;
        if (!resources.read(53237, bytes) || !palette.parse(bytes, &error) ||
            !resources.read(53233, bytes) || !slp.parse(std::move(bytes), &error))
            throw std::runtime_error("Clone Campaigns menu resources missing: " + error);
        background_ = sprite(slp, palette, 0); logo_ = sprite(slp, palette, 49);
        singleHighlight_ = sprite(slp,palette,36);
        singlePressed_ = sprite(slp,palette,35);
        optionsHighlight_ = sprite(slp,palette,12);
        optionsPressed_ = sprite(slp,palette,11);
        zone_ = sprite(slp,palette,18);
        auto originalArt = [&](int slpId, int paletteId) {
            swgb::Palette p; swgb::Slp sheet;
            if(!resources.read(paletteId,bytes) || !p.parse(bytes,&error) ||
               !resources.read(slpId,bytes) || !sheet.parse(bytes,&error))
                throw std::runtime_error("Interface resource missing: " + std::to_string(slpId) + " " + error);
            return sprite(sheet,p,0);
        };
        auto focusColor=[&](int id) -> std::array<uint8_t,3> {
            if(!resources.read(id,bytes)) throw std::runtime_error("Missing interface design: "+std::to_string(id));
            std::istringstream file(std::string(bytes.begin(),bytes.end())); std::string line;
            while(std::getline(file,line)) {
                std::istringstream fields(line); std::string key; fields>>key;
                if(key!="focus_color1") continue;
                int red,green,blue;
                if(!(fields>>red>>green>>blue) || red<0 || red>255 || green<0 || green>255 || blue<0 || blue>255)
                    throw std::runtime_error("Invalid interface focus color");
                return {uint8_t(red),uint8_t(green),uint8_t(blue)};
            }
            throw std::runtime_error("Interface design has no focus color");
        };
        mainFocus_=focusColor(53232); setupFocus_=focusColor(50053);
        if(!resources.read(53232,bytes)) throw std::runtime_error("Missing main menu design");
        std::istringstream mainDesign(std::string(bytes.begin(),bytes.end()));
        std::string designLine; bool foundBevel=false;
        while(std::getline(mainDesign,designLine)) {
            std::istringstream fields(designLine); std::string key; fields>>key;
            if(key!="bevel_colors") continue;
            for(auto &color:mainBevel_) {
                int index;
                if(!(fields>>index) || index<0 || index>255) throw std::runtime_error("Invalid menu bevel color");
                color=palette[uint8_t(index)];
            }
            foundBevel=true;break;
        }
        if(!foundBevel) throw std::runtime_error("Missing menu bevel colors");
        pauseFocus_=focusColor(50015); optionsFocus_=focusColor(50018);
        setup_=originalArt(50104,50533);
        pauseArt_=originalArt(50222,50500);
        optionsArt_=originalArt(50225,50500);
        auto loadBmp = [&](const char *name, bool transparent) -> swgb::Texture * {
            SDL_Surface *bmp = SDL_LoadBMP((root + "/Launcher/" + name).c_str());
            if (!bmp) return nullptr;
            SDL_Surface *rgba = SDL_ConvertSurfaceFormat(bmp, SDL_PIXELFORMAT_RGBA32, 0);
            SDL_FreeSurface(bmp);
            if (!rgba) throw std::runtime_error(SDL_GetError());
            std::vector<uint8_t> pixels(size_t(rgba->w)*rgba->h*4);
            for(int y=0;y<rgba->h;++y) {
                const auto *row=static_cast<uint8_t *>(rgba->pixels)+y*rgba->pitch;
                std::copy(row,row+rgba->w*4,pixels.data()+size_t(y)*rgba->w*4);
            }
            if(transparent) for(size_t i=0;i<pixels.size();i+=4)
                if(pixels[i]==0 && pixels[i+1]==0 && pixels[i+2]==0) pixels[i+3]=0;
            auto *texture=renderer_.createTexture(rgba->w,rgba->h,pixels.data());
            SDL_FreeSurface(rgba);
            return texture;
        };
        launcher_=loadBmp("Product.bmp",false);
        button_=loadBmp("unselect.bmp",true);
        focused_=loadBmp("focused.bmp",true);
#ifndef __APPLE__
        const auto &face = swgb::font::faces[2];
        font_ = renderer_.createMaskTexture(face.atlasW, face.atlasH, face.alpha);
#endif
    }
    ~Menu() { for (auto *t : {background_, logo_, launcher_, font_, button_, focused_, setup_, pauseArt_, optionsArt_, singleHighlight_, optionsHighlight_, zone_, singlePressed_, optionsPressed_}) if (t) renderer_.destroyTexture(t); }
    Screen screen() const { return screen_; }
    bool playing() const { return screen_ == Screen::Playing; }
    Preferences preferences() const {return {music_,effects_,humanCiv_,aiCiv_,populationLimit_,initiallyExplored_,conquest_};}
    bool conquest() const {return conquest_;}
    int populationLimit() const {return populationLimit_;}
    int humanCiv() const {return humanCiv_;}
    int aiCiv() const {return aiCiv_;}
    bool initiallyExplored() const { return initiallyExplored_; }
    int musicVolume() const { return music_; }
    int effectsVolume() const { return effects_; }
    bool overlay() const {
        return screen_ == Screen::Pause || screen_ == Screen::Resign || screen_ == Screen::Restart ||
               (screen_ == Screen::Options && return_ == Screen::Pause);
    }
    int width() const { return screen_ == Screen::Launcher ? 501 : (playing() || overlay()) ? 960 : 800; }
    int height() const { return screen_ == Screen::Launcher ? 482 : (playing() || overlay()) ? 544 : 600; }
    void pause() { if (playing()) set(Screen::Pause); }
    void main() { set(Screen::Main); }
    void play() { set(Screen::Playing); }
    std::vector<Button> pageButtons(bool muted) const {
        switch (screen_) {
        case Screen::Launcher: return {{{120,175,260,50},"Start Game",Play},{{120,240,260,50},"Help",Help},{{120,305,260,50},original(9506),Options},{{120,370,260,50},"Exit",Quit}};
        case Screen::Main: return {{{275,190,136,101},original(9500),SinglePlayer},{{12,345,144,171},original(9506),Options},{{3,540,88,48},original(9509),Quit},{{275,90,120,40},original(9503),Help,false},{{50,205,160,40},original(9505),Help,false},{{235,397,150,40},original(9501),Help,false},{{235,550,150,40},original(9504),Help,false},{{105,143,100,40},original(9502),Help,false}};
        case Screen::SinglePlayer: return {{{441,208,324,40},original(9226),Setup},
            {{441,92,324,40},original(9222),Help,false},
            {{441,150,324,40},original(9224),Help,false},
            {{441,266,324,40},original(9229),Help,false},
            {{441,324,324,40},original(9223),Help,false},{{445,420,300,40},original(9306),Back}};
        case Screen::Setup: {
            std::vector<Button> controls={{{35,554,165,35},original(9710),Play},{{205,554,165,35},original(9275),Back},
                {{650,289,135,28},original(initiallyExplored_?9756:9755),RevealMap},
                {{260,80,160,28},original(10230+humanCiv_),HumanCiv},
                {{260,152,160,28},original(10230+aiCiv_),AiCiv},
                {{650,260,135,28},std::to_string(populationLimit_),Population},
                {{640,347,150,28},original(conquest_?4321:13016),Victory}};
            if(setupOpen_==RevealMap) {
                controls.push_back({{650,317,135,28},original(9755),RevealStandard});
                controls.push_back({{650,345,135,28},original(9756),RevealExplored});
            } else if(setupOpen_==Victory) {
                controls.push_back({{640,375,150,28},original(4321),Conquest});
                controls.push_back({{640,403,150,28},original(13016),CommandCenter});
            } else if(setupOpen_==Population) {
                for(int count=25;count<=200;count+=25)
                    controls.push_back({{650,288+(count/25-1)*28,135,28},std::to_string(count),PopulationChoice+count});
            } else if(setupOpen_==HumanCiv || setupOpen_==AiCiv) {
                const int top=setupOpen_==HumanCiv?108:180;
                for(int civ=1;civ<=8;++civ) controls.push_back({{260,top+(civ-1)*28,160,28},original(10230+civ),CivChoice+civ});
            }
            return controls;
        }
        case Screen::Options: {
            const int x=overlay()?330:455;
            return {{{x,210,65,38},"-",MusicDown},{{x+235,210,65,38},"+",MusicUp},
                    {{x,280,65,38},"-",EffectsDown},{{x+235,280,65,38},"+",EffectsUp},
                    {{x,345,300,38},muted?"Unmute (F8)":"Mute (F8)",Mute},{{x,405,145,38},original(4001),ApplyOptions},{{x+155,405,145,38},original(9275),Back}};
        }
        case Screen::Help: return {{{465,425,260,45},original(9306),Back}};
        case Screen::Pause: return {{{367,116,226,28},original(9273),Resign},
            {{367,156,226,28},original(9272),Help,false},{{367,190,226,28},original(9276),Help,false},
            {{367,224,226,28},original(9279),Restart},{{367,258,226,28},original(9289),Help,false},
            {{367,292,226,28},original(9506),Options},{{367,326,226,28},original(9508),Help,false},
            {{367,366,226,28},original(9275),Resume}};
        case Screen::Restart: return {{{350,300,260,45},original(9279),ConfirmRestart},{{350,365,260,45},original(9275),Back}};
        case Screen::Resign: return {{{350,300,260,45},original(9280),Confirm},{{350,365,260,45},original(9275),Back}};
        default: return {};
        }
    }
    std::vector<Button> buttons(bool muted) const {
        auto controls=pageButtons(muted);
        if(screen_==Screen::SinglePlayer || (screen_==Screen::Options && return_==Screen::Main)) {
            controls.insert(controls.end(),{
                {{275,190,136,101},original(9500),SinglePlayer},
                {{12,345,144,171},original(9506),Options},{{3,540,88,48},original(9509),Quit},
                {{275,90,120,40},original(9503),Help,false},{{50,205,160,40},original(9505),Help,false},
                {{235,397,150,40},original(9501),Help,false},{{235,550,150,40},original(9504),Help,false},
                {{105,143,100,40},original(9502),Help,false}});
        }
        return controls;
    }
    Action activate(int command) {
        if(command>CivChoice && command<=CivChoice+8 && (setupOpen_==HumanCiv || setupOpen_==AiCiv)) {
            (setupOpen_==HumanCiv?humanCiv_:aiCiv_)=command-CivChoice;closeSetupChoices();return Action::None;
        }
        if(command>=PopulationChoice+25 && command<=PopulationChoice+200 && setupOpen_==Population) {
            populationLimit_=command-PopulationChoice;closeSetupChoices();return Action::None;
        }
        const bool cancelOptions=screen_==Screen::Options &&
            (command==Back || command==SinglePlayer || command==Quit);
        if(cancelOptions) {music_=savedMusic_;effects_=savedEffects_;}
        switch(command) {
        case Back: if(screen_==Screen::SinglePlayer) set(Screen::Main); else if(screen_==Screen::Setup) {set(Screen::SinglePlayer); return_=Screen::Main;} else set(return_); break;
        case Play: if (screen_ == Screen::Launcher) set(Screen::Main); else {play(); return Action::Start;} break;
        case SinglePlayer: return_ = Screen::Main; set(Screen::SinglePlayer); break;
        case Setup: return_ = Screen::SinglePlayer; set(Screen::Setup); break;
        case Options:
            if(screen_!=Screen::Options) {
                return_ = screen_==Screen::SinglePlayer?Screen::Main:screen_;
                savedMusic_=music_;savedEffects_=effects_;
            }
            set(Screen::Options); break;
        case ApplyOptions: set(return_); return Action::SaveSettings;
        case RevealMap: case HumanCiv: case AiCiv: case Population: case Victory:
            if(setupOpen_==command) closeSetupChoices();
            else {
                setupOpen_=command;
                focus_=7+(command==RevealMap?int(initiallyExplored_):command==HumanCiv?humanCiv_-1:command==AiCiv?aiCiv_-1:command==Victory?int(!conquest_):populationLimit_/25-1);
            }
            break;
        case Conquest: case CommandCenter:
            conquest_=command==Conquest;closeSetupChoices();break;
        case RevealStandard: case RevealExplored:
            initiallyExplored_=command==RevealExplored;closeSetupChoices();break;
        case Help: return_ = screen_; set(Screen::Help); break;
        case Quit: return Action::Quit;
        case Mute: return Action::Mute;
        case MusicDown: music_=std::max(0,music_-10); return Action::Volume;
        case MusicUp: music_=std::min(100,music_+10); return Action::Volume;
        case EffectsDown: effects_=std::max(0,effects_-10); return Action::Volume;
        case EffectsUp: effects_=std::min(100,effects_+10); return Action::Volume;
        case Resume: play(); break;
        case Resign: return_ = Screen::Pause; set(Screen::Resign); break;
        case Confirm: play(); return Action::Resign;
        case Restart: return_ = Screen::Pause; set(Screen::Restart); break;
        case ConfirmRestart: play(); return Action::Start;
        }
        return cancelOptions?Action::Volume:Action::None;
    }
    Action event(const SDL_Event &event, bool muted) {
        const auto controls = buttons(muted);
        if (controls.empty()) return Action::None;
        if (event.type == SDL_MOUSEMOTION) {
            hovered_=-1;
            SDL_Point p{event.motion.x,event.motion.y};
            for (size_t i=0;i<controls.size();++i) if(controls[i].enabled && SDL_PointInRect(&p,&controls[i].rect)) hovered_=focus_=int(i);
        }
        if(event.type==SDL_WINDOWEVENT && event.window.event==SDL_WINDOWEVENT_FOCUS_LOST) {
            pressed_=-1;
            if(setupOpen_) closeSetupChoices();
        }
        if(event.type==SDL_MOUSEBUTTONDOWN && event.button.button==SDL_BUTTON_LEFT) {
            pressed_=-1;
            SDL_Point p{event.button.x,event.button.y};
            const SDL_Rect popupBounds=setupOpen_==RevealMap?SDL_Rect{650,289,135,84}:
                setupOpen_==Population?SDL_Rect{650,260,135,252}:
                setupOpen_==Victory?SDL_Rect{640,347,150,84}:
                SDL_Rect{260,setupOpen_==HumanCiv?80:152,160,252};
            if(setupOpen_ && !SDL_PointInRect(&p,&popupBounds)) closeSetupChoices();
            for(size_t i=controls.size();i-->0;) if(controls[i].enabled && SDL_PointInRect(&p,&controls[i].rect)) {
                hovered_=pressed_=focus_=int(i); break;
            }
        }
        if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
            const int pressed=pressed_; pressed_=-1;
            SDL_Point p{event.button.x,event.button.y};
            if(pressed>=0 && pressed<int(controls.size()) && controls[pressed].enabled && SDL_PointInRect(&p,&controls[pressed].rect))
                return activate(controls[pressed].command);
        }
        if(event.type == SDL_KEYDOWN && !event.key.repeat) {
            pressed_=-1;
            auto key=event.key.keysym.sym;
            if(key==SDLK_ESCAPE) {
                if(setupOpen_) {closeSetupChoices();return Action::None;}
                if(screen_==Screen::Pause) play();
                else if(screen_==Screen::Main) set(Screen::Launcher);
                else if(screen_!=Screen::Launcher) return activate(Back);
            } else if(key==SDLK_TAB || key==SDLK_DOWN || key==SDLK_UP) {
                int step=key==SDLK_UP || (key==SDLK_TAB && (event.key.keysym.mod & KMOD_SHIFT))?-1:1;
                do { focus_=(focus_+step+int(controls.size()))%int(controls.size()); } while(!controls[focus_].enabled);
            } else if(key==SDLK_RETURN || key==SDLK_SPACE) return activate(controls[focus_].command);
        }
        return Action::None;
    }
    void render(bool muted) {
        if(!overlay()) renderer_.beginFrame(width(),height(),1,16,38,51);
        if(overlay()) {
            if(screen_==Screen::Options) image(optionsArt_,144,35);
            else if(screen_==Screen::Pause) image(pauseArt_,333,108);
            else renderer_.draw(pauseArt_,{322,100,350,365,0,0,float(pauseArt_->width),float(pauseArt_->height)});
            if(screen_!=Screen::Pause) text(screen_==Screen::Options?original(9431):screen_==Screen::Restart?original(9279):original(9280),380,125);
            if(screen_==Screen::Restart) text("Discard this match and start again?",345,210);
            if(screen_==Screen::Resign) text("This ends your current match.",350,210);
        } else if(screen_==Screen::Launcher) {
            image(launcher_,0,0);
            if(!launcher_) text("STAR WARS GALACTIC BATTLEGROUNDS",70,75);
        } else if(screen_==Screen::Setup) {
            image(setup_,0,0);
            text(original(9226),230,25);
            text(original(9680),40,60); text(original(9681),275,60); text("Player",405,60);
            text("Player",40,88); text("1",440,88);
            text(original(9688),40,130);
            text(original(9683),40,160); text("2",440,160);
            const char *labels[]={"Game Type","Map Type","Location","Map Size","Difficulty","Other Players","Resources","Population","Reveal Map","Initial Tech Level","Victory"};
            const char *values[]={"Test Skirmish","Fixed land","Test map","64 x 64","Custom AI","1","1200 each","By housing","Explored","Tech Level 1","Command Center"};
            for(int i=0;i<11;++i) {
                text(i==8?original(9724):i==7?original(9747):i==10?original(10019):labels[i],512,65+i*29,false);
                if(i!=8 && i!=7 && i!=10) text(values[i],650,65+i*29);
            }
            text("Choose factions, population, visibility and victory.",395,510);
            text("Other settings describe this test skirmish.",395,532);
        } else {
            image(background_,0,0); image(logo_,3,5); image(zone_,75,91);
            const auto navigation=buttons(muted);
            const Button *focused = focus_ < int(navigation.size()) ? &navigation[focus_] : nullptr;
            int highlighted=screen_==Screen::SinglePlayer?SinglePlayer:screen_==Screen::Options?Options:-1;
            if(focused && focused->rect.x<400) highlighted=focused->command;
            if(highlighted==SinglePlayer) image(pressed_==focus_ && hovered_==pressed_?singlePressed_:singleHighlight_,275,190);
            if(highlighted==Options) image(pressed_==focus_ && hovered_==pressed_?optionsPressed_:optionsHighlight_,12,345);
            if(screen_==Screen::SinglePlayer) {
#ifdef __APPLE__
                nativeText_.draw(original(9500),600-nativeText_.width(original(9500),dialogPixels_,dialogBold_)/2.f,15,true,dialogPixels_,dialogBold_);
#else
                text(original(9500),445,18);
#endif
                text("Campaigns and saved games are unavailable.",430,530,false);
            } else if(screen_==Screen::Options) {
                text(original(9506),445,18);
            } else if(screen_==Screen::Help) {
                text("HOW TO PLAY",465,125);
                int y=180;
                for(const auto *line:{"Left click: select / drag: group", "Right click: order / cancel placement", "B: actions / Q, E: action tabs", "Arrows: camera / wheel: zoom", "Space: pause / Escape: game menu", "F8: mute / Command-Q: quit", "Ctrl+0..9: store / 0..9: recall"}) {text(line,435,y);y+=30;}
            } else if(screen_==Screen::Pause) text("GAME PAUSED",465,125);
            else if(screen_==Screen::Resign) {text("RESIGN GAME?",465,125);text("This ends your current match.",445,210);}
        }
        if(screen_==Screen::Options) {
            const int x=overlay()?330:455;
            text(singleLineOriginal(9435),x,185); text(std::to_string(music_)+"%",x+125,222);
            text(singleLineOriginal(9438),x,255); text(std::to_string(effects_)+"%",x+125,292);
            text("Preview changes; OK saves them.",x,155,false);
        }
        const auto controls=buttons(muted);
        for(size_t i=0;i<controls.size();++i) {
            const auto &b=controls[i]; const auto &r=b.rect;
            const bool mainModeButton=screen_==Screen::SinglePlayer && r.x>=420;
            const bool pushed=int(i)==pressed_ && hovered_==pressed_;
            if(screen_==Screen::Launcher && button_) {
                image(int(i)==focus_ && focused_?focused_:button_,r.x-40,r.y-9);
            } else if(mainModeButton) {
                for(int edge=0;edge<3;++edge) {
                    const auto &top=mainBevel_[edge];
                    const auto &bottom=mainBevel_[5-edge];
                    const int inset=pushed && edge==2?1:0;
                    renderer_.fillRect(r.x+1+edge-inset,r.y+edge,r.w-1-2*edge+inset,1,top.r,top.g,top.b,255);
                    renderer_.fillRect(r.x+r.w-1-edge,r.y+1+edge,1,r.h-2-2*edge+inset,top.r,top.g,top.b,255);
                    if(!pushed || edge<2) {
                        renderer_.fillRect(r.x+edge,r.y+r.h-1-edge,r.w-2*edge,1,bottom.r,bottom.g,bottom.b,255);
                        renderer_.fillRect(r.x+edge,r.y+edge,1,r.h-1-2*edge,bottom.r,bottom.g,bottom.b,255);
                    }
                }
            } else if(screen_!=Screen::Main && !(r.x<400 && (screen_==Screen::SinglePlayer || (screen_==Screen::Options && !overlay())))) {
                renderer_.fillRect(r.x,r.y,r.w,r.h,48,111,145,255);
                renderer_.fillRect(r.x+1,r.y+1,r.w-2,r.h-2,0,0,0,255);
                renderer_.fillRect(r.x+3,r.y+3,r.w-6,r.h-6,0,42,53,255);
                for(int y=r.y+5;y<r.y+r.h-3;y+=4) renderer_.fillRect(r.x+3,y,r.w-6,1,0,12,20,255);
            }
            const bool navigation = r.x<420 && (screen_==Screen::Main || screen_==Screen::SinglePlayer || (screen_==Screen::Options && return_==Screen::Main));
            if(!mainModeButton && int(i)==focus_ && b.enabled && (!navigation || (b.command!=SinglePlayer && b.command!=Options))) renderer_.fillRect(r.x,r.y+r.h-3,r.w,2,102,152,203,255);
            if(b.command==RevealMap || b.command==HumanCiv || b.command==AiCiv || b.command==Population || b.command==Victory) for(int row=0;row<4;++row)
                renderer_.fillRect(r.x+r.w-14+row,r.y+12+row,7-2*row,1,102,152,203,255);
            float textX=r.x+8;
            const bool setupChoice=screen_==Screen::Setup && b.command!=Play && b.command!=Back;
            const int fontPixels=(navigation || setupChoice)?captionPixels_:(screen_==Screen::Pause || mainModeButton)?dialogPixels_:buttonPixels_;
            const bool fontBold=(navigation || setupChoice)?captionBold_:(screen_==Screen::Pause || mainModeButton)?dialogBold_:buttonBold_;
            if(!navigation) {
                int length=0;
#ifdef __APPLE__
                length=nativeText_.width(b.label,fontPixels,fontBold);
#else
                for(unsigned char c:b.label) if(c>=32 && c<=126) length+=swgb::font::faces[2].glyphs[c-32].advance;
#endif
                textX=r.x+(r.w-length)/2.f;
            }
            float textY=r.y+(screen_==Screen::Pause?7:12);
            if(navigation && b.command==SinglePlayer) {
#ifdef __APPLE__
                textX=275+(136-nativeText_.width(b.label,captionPixels_,captionBold_))/2.f;
#else
                textX=283;
#endif
                textY=211;
            }
            if(navigation && b.command==Options) {textX=43; textY=367;}
#ifdef __APPLE__
            if(!navigation) textY=r.y+(r.h-nativeText_.height(b.label,fontPixels,fontBold))/2.f;
            if(mainModeButton && pushed) {textX+=1;textY-=1;}
            const auto &focusColor=screen_==Screen::Setup?setupFocus_:overlay()?(screen_==Screen::Options?optionsFocus_:pauseFocus_):mainFocus_;
            if(mainModeButton) {
                const auto color=!b.enabled?std::array<uint8_t,3>{147,142,138}:int(i)==focus_?focusColor:std::array<uint8_t,3>{255,255,255};
                nativeText_.drawColored(b.label,textX,textY,color[0],color[1],color[2],fontPixels,fontBold,1,b.enabled,-1);
            } else nativeText_.draw(b.label,textX,textY,b.enabled,fontPixels,fontBold,int(i)==focus_ && b.enabled,focusColor);
#else
            text(b.label,textX,textY,b.enabled);
#endif
        }
        renderer_.endFrame();
    }
};
}
