// SPDX-License-Identifier: GPL-3.0-or-later
#include "../../engine/game.h"
#include "../../render/soft_renderer.h"
#include "../../core/audio.h"
#include "menu.h"
#include <cstdlib>
#include <SDL.h>
#include <vorbis/vorbisfile.h>
#include <thread>
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>

namespace {
#ifdef __APPLE__
class DesktopRenderer : public swgb::SoftRenderer {
    desktop::NativeText text_{*this};
    float textScale_=1;
public:
    explicit DesktopRenderer(const std::string &root) {
        swgb::LanguageStrings strings; std::string error;
        if(!strings.load(root+"/language.dll",&error) || strings.get(102).empty())
            throw std::runtime_error("Missing gameplay font definition: "+error);
        text_.family(strings.get(102));
    }
    void beginFrame(int w,int h,float scale,uint8_t r,uint8_t g,uint8_t b) override {
        textScale_=scale;
        SoftRenderer::beginFrame(w,h,scale,r,g,b);
    }
    float measureInterfaceText(const std::string &value,int pixels) override { return text_.width(value,pixels,false); }
    void drawInterfaceText(const std::string &value,float x,float y,int pixels,uint8_t r,uint8_t g,uint8_t b) override {
        text_.drawColored(value,x,y,r,g,b,pixels,false,textScale_);
    }
};
#endif
class Audio {
    struct Voice {
        std::shared_ptr<swgb::AudioClip> clip; size_t at = 0;
        std::array<float,2> gain{1,1};
        int soundKey=0;
    };
    static std::array<float,2> soundGains(int pan,int volume) {
        pan=std::clamp(pan,-10000,10000); volume=std::clamp(volume,-10000,0);
        return {std::pow(10.f,(volume-std::max(0,pan))/2000.f),
                std::pow(10.f,(volume+std::min(0,pan))/2000.f)};
    }
    std::array<Voice, 16> voices_{};
    SDL_AudioDeviceID device_ = 0;
    std::atomic<int> musicGain_{100}, effectsGain_{100};
    std::atomic<unsigned> nonzero_{0}, musicSamples_{0}, musicLoops_{0};
    std::array<int16_t, 131072> musicRing_{};
    std::atomic<uint64_t> read_{0}, written_{0};
    std::atomic<bool> stopMusic_{false}, muted_{false};
    OggVorbis_File music_{};
    bool musicOpen_ = false;
    std::thread musicThread_;
    static void callback(void *ctx, Uint8 *stream, int bytes) {
        static_cast<Audio *>(ctx)->mix(reinterpret_cast<int16_t *>(stream), bytes / 2);
    }
public:
    Audio() {
        SDL_AudioSpec spec{};
        spec.freq = 44100; spec.format = AUDIO_S16SYS; spec.channels = 2;
        spec.samples = 1024; spec.callback = callback; spec.userdata = this;
        device_ = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);
        if (!device_) throw std::runtime_error(SDL_GetError());
        SDL_PauseAudioDevice(device_, 0);
    }
    ~Audio() {
        if (device_) SDL_CloseAudioDevice(device_);
        stopMusic_ = true;
        if (musicThread_.joinable()) musicThread_.join();
        if (musicOpen_) ov_clear(&music_);
    }
    void setVolumes(int music, int effects) {
        musicGain_=std::clamp(music,0,100); effectsGain_=std::clamp(effects,0,100);
    }
    void toggleMute() { muted_ = !muted_.load(); }
    bool muted() const { return muted_; }
    void startMusic(const std::string &path, bool testLoop) {
        if (ov_fopen(path.c_str(), &music_) != 0) throw std::runtime_error("Cannot open soundtrack: " + path);
        musicOpen_ = true;
        const auto *info = ov_info(&music_, -1);
        if (!info || info->channels != 2 || info->rate != 44100)
            throw std::runtime_error("Soundtrack must be 44100 Hz stereo Vorbis");
        if (testLoop && ov_pcm_seek(&music_, std::max<ogg_int64_t>(0, ov_pcm_total(&music_, -1) - 1024)) != 0)
            throw std::runtime_error("Music seek test failed");
        musicThread_ = std::thread([this] {
            std::array<int16_t, 2048> pcm{};
            int section = 0, consecutiveErrors = 0;
            while (!stopMusic_) {
                const auto write = written_.load(std::memory_order_relaxed);
                if (write - read_.load(std::memory_order_acquire) + pcm.size() > musicRing_.size()) {
                    SDL_Delay(2); continue;
                }
                const long bytes = ov_read(&music_, reinterpret_cast<char *>(pcm.data()), sizeof(pcm),
                                          SDL_BYTEORDER == SDL_BIG_ENDIAN, 2, 1, &section);
                if (bytes == 0) {
                    if (ov_pcm_seek(&music_, 0) != 0 || ++consecutiveErrors > 4) break;
                    musicLoops_++; continue;
                }
                if (bytes < 0) { if (++consecutiveErrors > 4) break; continue; }
                consecutiveErrors = 0;
                if (bytes % 4) break;
                const auto *current = ov_info(&music_, section);
                if (!current || current->channels != 2 || current->rate != 44100) break;
                for (size_t i = 0; i < size_t(bytes) / 2; ++i)
                    musicRing_[(write + i) % musicRing_.size()] = pcm[i];
                written_.store(write + bytes / 2, std::memory_order_release);
            }
            if (!stopMusic_) std::fprintf(stderr, "Soundtrack decoding stopped unexpectedly\n");
        });
    }
    bool musicVerified() const { return musicSamples_ > 0 && musicLoops_ > 0; }
    void mix(int16_t *out, int samples) {
        auto read = read_.load(std::memory_order_relaxed);
        const auto available = written_.load(std::memory_order_acquire);
        unsigned musicNonzero = 0, effectNonzero = 0;
        const int musicGain=musicGain_, effectsGain=effectsGain_;
        for (int i = 0; i < samples; ++i) {
            int total = 0;
            for (auto &v : voices_)
                if (v.clip && v.at < v.clip->samples.size()) {
                    total += int(std::lround(v.clip->samples[v.at]*v.gain[v.at%2])); ++v.at;
                }
            if (total) ++effectNonzero;
            int music = read < available ? musicRing_[read++ % musicRing_.size()] : 0;
            if (music) ++musicNonzero;
            out[i] = muted_ ? 0 : static_cast<int16_t>(std::clamp((total / 3) * effectsGain / 100 + (music / 8) * musicGain / 100, -32768, 32767));
        }
        read_.store(read, std::memory_order_release);
        nonzero_.fetch_add(effectNonzero, std::memory_order_relaxed);
        musicSamples_.fetch_add(musicNonzero, std::memory_order_relaxed);
    }
    unsigned mixedSamples() const { return nonzero_.load(); }
    void checkMixer() {
        SDL_PauseAudioDevice(device_, 1);
        auto clip = std::make_shared<swgb::AudioClip>();
        clip->samples = {3000, -3000, 6000, -6000};
        voices_[0] = {clip, 0}; voices_[1] = {clip, 0};
        int16_t out[6]{}; mix(out, 6);
        voices_ = {};
        if (out[0] != 2000 || out[1] != -2000 || out[2] != 4000 || out[3] != -4000 || out[4] || out[5])
            throw std::runtime_error("Mixer overlap/end check failed");
        voices_[0]={clip,0,soundGains(-2000,0)}; mix(out,4);
        if(out[0]!=1000 || out[1]!=-100) throw std::runtime_error("DirectSound left-pan check failed");
        voices_[0]={clip,0,soundGains(2000,-2000)}; mix(out,4);
        if(out[0]!=10 || out[1]!=-100) throw std::runtime_error("DirectSound cumulative pan/volume check failed");
        voices_={};
        setVolumes(0,50); voices_[0]={clip,0}; mix(out,4);
        if(out[0]!=500 || out[2]!=1000) throw std::runtime_error("Effects volume check failed");
        voices_={}; setVolumes(100,100);
        play(clip,0,0,50357); mix(out,2); play(clip,0,0,50357); mix(out,2);
        if(out[0]!=1000 || out[1]!=-1000 || std::count_if(voices_.begin(),voices_.end(),[](const Voice &v) {return v.clip!=nullptr;})!=1)
            throw std::runtime_error("Original sound-buffer restart check failed");
        voices_={}; musicRing_[0]=8000; musicRing_[1]=-8000; written_=2; read_=0;
        setVolumes(50,0); mix(out,2);
        if(out[0]!=500 || out[1]!=-500) throw std::runtime_error("Music volume check failed");
        musicRing_[2]=8000; written_=3; setVolumes(0,0); mix(out,1);
        if(out[0]) throw std::runtime_error("Zero volume check failed");
        read_=0; written_=0; setVolumes(100,100); musicSamples_=0;
        nonzero_ = 0;
        SDL_PauseAudioDevice(device_, 0);
    }
    float play(std::shared_ptr<swgb::AudioClip> clip, int pan=0, int volume=0, int soundKey=0) {
        if (clip->samples.empty() || clip->samples.size() > 44100 * 2 * 120) return 0;
        SDL_LockAudioDevice(device_);
        Voice *chosen=nullptr;
        if(soundKey) for(auto &voice:voices_) if(voice.soundKey==soundKey) {chosen=&voice;break;}
        if(!chosen) for(auto &voice:voices_) if(!voice.clip || voice.at>=voice.clip->samples.size()) {chosen=&voice;break;}
        const bool accepted=chosen!=nullptr;
        if(chosen) *chosen={clip,0,soundGains(pan,volume),soundKey};
        SDL_UnlockAudioDevice(device_);
        return accepted ? float(clip->frameCount()) / 44100 : 0;
    }
    float file(const std::string &path, bool wav) {
        auto clip = std::make_shared<swgb::AudioClip>(); std::string error;
        if (!(wav ? swgb::loadWav(path, *clip, &error) : swgb::loadMp3(path, *clip, &error))) {
            std::fprintf(stderr, "audio: %s\n", error.c_str()); return 0;
        }
        return play(clip);
    }
    void effect(const std::vector<uint8_t> &bytes, int pan=0, int volume=0, int soundKey=0) {
        auto clip = std::make_shared<swgb::AudioClip>(); std::string error;
        if (swgb::decodeWav(bytes, *clip, &error)) play(clip,pan,volume,soundKey);
        else std::fprintf(stderr, "audio: %s\n", error.c_str());
    }
};

void checkMaskRendering() {
    swgb::SoftRenderer renderer;
    const uint8_t alpha[] = {0,255,128,64};
    auto *mask=renderer.createMaskTexture(2,2,alpha);
    renderer.beginFrame(2,2,1,0,0,0);
    renderer.drawTinted(mask,{0,0,2,2,0,0,2,2},200,100,50,128);
    const auto tinted=renderer.pixels();
    renderer.beginFrame(2,2,1,0,0,0);
    renderer.draw(mask,{0,0,2,2,0,0,2,2});
    const auto plain=renderer.pixels();
    renderer.destroyTexture(mask);
    for(int i=0;i<4;++i) {
        for(int c=0;c<3;++c) {
            const int color=c==0?200:c==1?100:50;
            if(tinted[i*4+c]!=color*(alpha[i]*128/255)/255 || plain[i*4+c]!=alpha[i])
                throw std::runtime_error("Mask texture sampling self-check failed");
        }
    }
}

void checkPreferences() {
    const auto directory=std::filesystem::temp_directory_path()/
        ("galactic-genie-preferences-"+std::to_string(SDL_GetPerformanceCounter()));
    if(!std::filesystem::create_directory(directory)) throw std::runtime_error("Cannot create settings test directory");
    const auto path=directory/"settings.txt";
    auto require=[](bool ok) {if(!ok) throw std::runtime_error("Preferences self-check failed");};
    desktop::Preferences settings{30,80,7,8,25,true},loaded;std::string error;
    require(!desktop::readPreferences(path,loaded));
    require(desktop::writePreferences(path,settings,error) && desktop::readPreferences(path,loaded));
    require(loaded.music==30 && loaded.effects==80 && loaded.humanCiv==7 && loaded.aiCiv==8 && loaded.population==25 && loaded.explored);
    {std::ofstream file(path);file<<"1 30 80 7 8 25 1\n";}
    require(desktop::readPreferences(path,loaded) && !loaded.conquest && loaded.explored);
    settings.music=60;require(desktop::writePreferences(path,settings,error));
    require(desktop::readPreferences(path,loaded) && loaded.music==60);
    settings.population=0;require(!desktop::writePreferences(path,settings,error));
    require(desktop::readPreferences(path,loaded) && loaded.population==25);
    for(const char *bad:{"", "1 30 80", "2 30 80 7 8 25 1", "1 101 80 7 8 25 1",
                        "1 30 80 0 8 25 1", "1 30 80 7 9 25 1", "1 30 80 7 8 26 1",
                        "1 30 80 7 8 25 2", "1 30 80 7 8 25 1 junk"}) {
        {std::ofstream file(path);file<<bad;}
        loaded=desktop::Preferences{};
        require(!desktop::readPreferences(path,loaded) && loaded.music==100 && loaded.humanCiv==1);
    }
    require(!desktop::writePreferences(directory/"missing"/"settings.txt",desktop::Preferences{},error));
    std::filesystem::remove(path);std::filesystem::remove(directory);
    std::fprintf(stderr,"PREFERENCES PASS: round trip, replacement, malformed data and write failures\n");
}

int run(const std::string &root, bool smoke) {
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window(
        SDL_CreateWindow("Galactic Genie — Skirmish | B: actions | Space: pause | F8: mute",
                         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 544, 0), SDL_DestroyWindow);
    if (!window) throw std::runtime_error(SDL_GetError());
#ifdef __APPLE__
    DesktopRenderer renderer(root);
#else
    swgb::SoftRenderer renderer;
#endif
#ifdef __APPLE__
    if(smoke) {
        renderer.beginFrame(200,50,1,0,0,0);
        renderer.drawInterfaceText("Éclaireur",10,5,14,210,160,90);
        const auto unzoomed=renderer.pixels();
        const float measured=renderer.measureInterfaceText("Éclaireur",14);
        renderer.beginFrame(200,50,2,0,0,0);
        renderer.drawInterfaceText("Éclaireur",5,2.5f,14,210,160,90);
        if(renderer.pixels()!=unzoomed || renderer.measureInterfaceText("Éclaireur",14)!=measured || measured<=0)
            throw std::runtime_error("Gameplay typography changed with world zoom");
        desktop::NativeText text(renderer); text.family("Arial");
        renderer.beginFrame(200,40,1,0,0,0);
        text.draw("Éditeur de cartes",0,0,true,14,false);
        if(text.width("Éditeur de cartes",14,false)<=text.width("Éditeur",14,false) ||
           std::none_of(renderer.pixels().begin(),renderer.pixels().end(),[](uint8_t v){return v>0 && v<255;}))
            throw std::runtime_error("Original-family UTF-8 font self-check failed");
        renderer.beginFrame(40,40,1,0,0,0);
        text.draw("T",0,0,true,20,false);
        int first=-1,last=-1;
        for(int y=0;y<40;++y) {
            int ink=0;
            for(int x=0;x<40;++x) if(renderer.pixels()[(y*40+x)*4]>127) ++ink;
            if(ink) { if(first<0) first=ink; last=ink; }
        }
        if(first<=last) throw std::runtime_error("Interface glyph orientation check failed");
        bool rejected=false;
        try { text.draw(std::string(1,char(0xff)),0,0,true); } catch(const std::runtime_error &) { rejected=true; }
        if(!rejected) throw std::runtime_error("Invalid UTF-8 was accepted");
    }
#endif
    swgb::Assets assets(&renderer); std::string error;
    if (!assets.init(root + "/Data", &error)) throw std::runtime_error(error);
    desktop::Preferences settings;std::filesystem::path settingsPath;
    if(!smoke) {
        char *directory=SDL_GetPrefPath("Galactic Genie","Clone Campaigns");
        if(directory) {
            settingsPath=std::filesystem::path(directory)/"settings.txt";SDL_free(directory);
            std::error_code ec;
            if(!desktop::readPreferences(settingsPath,settings) && std::filesystem::exists(settingsPath,ec))
                std::fprintf(stderr,"Invalid settings file; using defaults: %s\n",settingsPath.string().c_str());
        } else std::fprintf(stderr,"Settings directory unavailable: %s\n",SDL_GetError());
    }
    desktop::Menu menu(renderer, root,settings);
    auto saveSettings=[&]() {
        if(smoke || settingsPath.empty()) return;
        std::string failure;
        if(!desktop::writePreferences(settingsPath,menu.preferences(),failure)) {
            std::fprintf(stderr,"%s\n",failure.c_str());
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING,"Settings were not saved",failure.c_str(),window.get());
        }
    };
    Audio audio;audio.setVolumes(settings.music,settings.effects);
    if (smoke) { audio.checkMixer(); checkMaskRendering(); checkPreferences(); }
    swgb::Game game(assets);
    game.setKeyboardMouseControls(true);
    game.setClassicHud(true);
    game.setLogger([](const std::string &message) { std::fprintf(stderr, "%s\n", message.c_str()); });
    game.setSoundPlayer([&](const std::string &name) {
        return audio.file(root + "/Sound/Scenario/" + name + ".mp3", false);
    });
    game.setAmbientSoundPlayer([&](const std::string &name) {
        return audio.file(root + "/Sound/Terrain/" + name + ".wav", true);
    });
    game.setInterfaceSoundPlayer([&](int id) {
        std::vector<uint8_t> bytes; if (assets.readSoundResource(id, bytes)) audio.effect(bytes);
    });
    game.setSpatialInterfaceSoundPlayer([&](int id,int pan,int volume) {
        std::vector<uint8_t> bytes; if(assets.readSoundResource(id,bytes)) audio.effect(bytes,pan,volume,id);
    });
    uint32_t choice = 0;
    game.setSpatialUnitSoundPlayer([&](int id,int civ,int pan,int volume) {
        std::vector<uint8_t> bytes;
        if(assets.readSound(id,civ,choice++,bytes)) audio.effect(bytes,pan,volume);
    });
    game.setUnitSoundPlayer([&](int id, int civ) {
        std::vector<uint8_t> bytes;
        if (assets.readSound(id, civ, choice++, bytes)) audio.effect(bytes);
    });
    if (smoke) {
        std::vector<uint8_t> wave;
        if (!assets.readSoundResource(50331, wave)) throw std::runtime_error("Smoke click WAV missing");
        audio.effect(wave);
    }
    if (smoke) {
        auto key = [&](SDL_Keycode code) {
            SDL_Event event{}; event.type = SDL_KEYDOWN; event.key.keysym.sym = code;
            return menu.event(event, audio.muted());
        };
        auto require = [](bool ok) { if (!ok) throw std::runtime_error("Menu navigation self-check failed"); };
        menu.render(false); require(renderer.savePng("launcher-smoke.png"));
        key(SDLK_RETURN); require(menu.screen() == desktop::Screen::Main);
        menu.render(false); require(renderer.savePng("menu-smoke.png"));
        const auto menuBackground=renderer.pixels();
        SDL_Event disabled{}; disabled.type=SDL_MOUSEBUTTONUP; disabled.button.button=SDL_BUTTON_LEFT;
        disabled.button.x=280; disabled.button.y=410;
        require(menu.event(disabled,false)==desktop::Action::None && menu.screen()==desktop::Screen::Main);
        key(SDLK_RETURN); require(menu.screen() == desktop::Screen::SinglePlayer);
        menu.render(false); require(renderer.savePng("single-player-smoke.png"));
        const size_t buttonInterior=(220*800+460)*4;
        require(std::equal(menuBackground.begin()+buttonInterior,menuBackground.begin()+buttonInterior+4,
                           renderer.pixels().begin()+buttonInterior));
        SDL_Event buttonPress{};buttonPress.type=SDL_MOUSEBUTTONDOWN;buttonPress.button.button=SDL_BUTTON_LEFT;
        buttonPress.button.x=600;buttonPress.button.y=228;
        menu.event(buttonPress,false);menu.render(false);require(renderer.savePng("single-player-pressed-smoke.png"));
        buttonPress.type=SDL_MOUSEBUTTONUP;buttonPress.button.x=790;menu.event(buttonPress,false);
        require(menu.screen()==desktop::Screen::SinglePlayer);
        SDL_Event click{}; click.type=SDL_MOUSEBUTTONUP; click.button.button=SDL_BUTTON_LEFT;
        click.button.x=25; click.button.y=490;
        menu.event(click,false); require(menu.screen()==desktop::Screen::SinglePlayer);
        click.type=SDL_MOUSEBUTTONDOWN; menu.event(click,false);
        click.type=SDL_MOUSEBUTTONUP; click.button.x=790; menu.event(click,false);
        require(menu.screen()==desktop::Screen::SinglePlayer);
        click.button.x=25; click.type=SDL_MOUSEBUTTONDOWN; menu.event(click,false);
        SDL_Event lostFocus{}; lostFocus.type=SDL_WINDOWEVENT; lostFocus.window.event=SDL_WINDOWEVENT_FOCUS_LOST;
        menu.event(lostFocus,false); click.type=SDL_MOUSEBUTTONUP; menu.event(click,false);
        require(menu.screen()==desktop::Screen::SinglePlayer);
        click.type=SDL_MOUSEBUTTONDOWN; menu.event(click,false);
        click.type=SDL_MOUSEBUTTONUP; menu.event(click,false); require(menu.screen()==desktop::Screen::Options);
        click.type=SDL_MOUSEBUTTONDOWN; menu.event(click,false);
        click.type=SDL_MOUSEBUTTONUP; menu.event(click,false); key(SDLK_ESCAPE); require(menu.screen()==desktop::Screen::Main);
        key(SDLK_RETURN); require(menu.screen()==desktop::Screen::SinglePlayer);
        key(SDLK_RETURN); require(menu.screen() == desktop::Screen::Setup);
        require(!menu.initiallyExplored());
        menu.render(false); require(renderer.savePng("setup-smoke.png"));
        key(SDLK_TAB); key(SDLK_TAB); key(SDLK_RETURN);
        require(menu.buttons(false).size()==9);
        menu.event(lostFocus,false); require(menu.buttons(false).size()==7 && !menu.initiallyExplored());
        key(SDLK_RETURN);
        click.button.x=500; click.button.y=450; click.type=SDL_MOUSEBUTTONDOWN; menu.event(click,false);
        click.type=SDL_MOUSEBUTTONUP; menu.event(click,false);
        require(menu.buttons(false).size()==7 && !menu.initiallyExplored());
        key(SDLK_RETURN);
        menu.render(false); require(renderer.savePng("setup-reveal-smoke.png"));
        key(SDLK_DOWN); key(SDLK_RETURN);
        require(menu.initiallyExplored() && menu.screen()==desktop::Screen::Setup);
        key(SDLK_ESCAPE); require(menu.screen()==desktop::Screen::SinglePlayer);
        key(SDLK_RETURN); require(menu.initiallyExplored());
        click.button.x=700; click.button.y=302; click.type=SDL_MOUSEBUTTONDOWN; menu.event(click,false);
        click.type=SDL_MOUSEBUTTONUP; menu.event(click,false);
        key(SDLK_ESCAPE); require(menu.screen()==desktop::Screen::Setup && menu.initiallyExplored());
        key(SDLK_RETURN);
        click.button.y=330; click.type=SDL_MOUSEBUTTONDOWN; menu.event(click,false);
        click.type=SDL_MOUSEBUTTONUP; menu.event(click,false);
        require(!menu.initiallyExplored());
        key(SDLK_TAB); key(SDLK_RETURN); // Human civilization dropdown.
        require(menu.buttons(false).size()==15);
        menu.render(false); require(renderer.savePng("setup-civilizations-smoke.png"));
        click.button.x=330;click.button.y=175;click.type=SDL_MOUSEBUTTONDOWN;menu.event(click,false);
        click.type=SDL_MOUSEBUTTONUP;menu.event(click,false);
        require(menu.humanCiv()==3 && menu.aiCiv()==3 && menu.buttons(false).size()==7);
        key(SDLK_RETURN);for(int i=0;i<4;++i) key(SDLK_DOWN);key(SDLK_RETURN);
        require(menu.humanCiv()==7);
        key(SDLK_TAB);key(SDLK_RETURN);for(int i=0;i<5;++i) key(SDLK_DOWN);key(SDLK_RETURN);
        require(menu.aiCiv()==8);
        key(SDLK_ESCAPE);key(SDLK_RETURN);
        require(menu.humanCiv()==7 && menu.aiCiv()==8);
        // Restore the original fixture for the existing desktop combat/HUD checks.
        key(SDLK_TAB);key(SDLK_TAB);key(SDLK_TAB);key(SDLK_RETURN);
        for(int i=0;i<6;++i) key(SDLK_UP);key(SDLK_RETURN);
        key(SDLK_TAB);key(SDLK_RETURN);for(int i=0;i<5;++i) key(SDLK_UP);key(SDLK_RETURN);
        require(menu.humanCiv()==1 && menu.aiCiv()==3 && menu.populationLimit()==75);
        key(SDLK_TAB);key(SDLK_RETURN);key(SDLK_UP);key(SDLK_UP);key(SDLK_RETURN);
        require(menu.populationLimit()==25);
        key(SDLK_RETURN);key(SDLK_DOWN);key(SDLK_DOWN);key(SDLK_RETURN);
        require(menu.populationLimit()==75 && menu.conquest());
        key(SDLK_TAB);key(SDLK_RETURN);key(SDLK_DOWN);key(SDLK_RETURN);
        require(!menu.conquest());
        key(SDLK_ESCAPE); require(menu.screen()==desktop::Screen::SinglePlayer);
        key(SDLK_ESCAPE); require(menu.screen()==desktop::Screen::Main);
        key(SDLK_DOWN); key(SDLK_RETURN); require(menu.screen()==desktop::Screen::Options);
        require(key(SDLK_RETURN)==desktop::Action::Volume && menu.musicVolume()==90 && menu.effectsVolume()==100);
        for(int i=0;i<20;++i) key(SDLK_RETURN);
        require(menu.musicVolume()==0);
        key(SDLK_RIGHT); key(SDLK_TAB);
        for(int i=0;i<20;++i) key(SDLK_RETURN);
        require(menu.musicVolume()==100);
        key(SDLK_TAB); require(key(SDLK_RETURN)==desktop::Action::Volume && menu.effectsVolume()==90);
        require(key(SDLK_ESCAPE)==desktop::Action::Volume);
        require(menu.musicVolume()==100 && menu.effectsVolume()==100 && menu.screen()==desktop::Screen::Main);
        key(SDLK_DOWN); key(SDLK_RETURN);
        key(SDLK_RETURN); // Preview music at 90%.
        for(int i=0;i<5;++i) key(SDLK_TAB);
        require(key(SDLK_RETURN)==desktop::Action::SaveSettings); // OK retains the preview.
        require(menu.screen()==desktop::Screen::Main && menu.musicVolume()==90);
        key(SDLK_DOWN); key(SDLK_RETURN); key(SDLK_RETURN);
        require(menu.musicVolume()==80);
        // Re-selecting Options must not replace the cancellation baseline.
        click.button.x=25; click.button.y=490; click.type=SDL_MOUSEBUTTONDOWN; menu.event(click,false);
        click.type=SDL_MOUSEBUTTONUP; menu.event(click,false);
        require(key(SDLK_ESCAPE)==desktop::Action::Volume && menu.musicVolume()==90);
        key(SDLK_DOWN); key(SDLK_RETURN); key(SDLK_RETURN);
        click.button.x=320; click.button.y=230; click.type=SDL_MOUSEBUTTONDOWN; menu.event(click,false);
        click.type=SDL_MOUSEBUTTONUP;
        require(menu.event(click,false)==desktop::Action::Volume);
        require(menu.screen()==desktop::Screen::SinglePlayer && menu.musicVolume()==90);
        key(SDLK_RETURN); require(key(SDLK_RETURN)==desktop::Action::Start);
        menu.pause(); require(menu.overlay() && menu.width()==960 && menu.height()==544);
        key(SDLK_DOWN); key(SDLK_DOWN); key(SDLK_RETURN);
        require(menu.overlay() && menu.screen()==desktop::Screen::Options);
        key(SDLK_RETURN); require(menu.musicVolume()==80);
        for(int i=0;i<6;++i) key(SDLK_TAB);
        require(key(SDLK_RETURN)==desktop::Action::Volume);
        require(menu.screen()==desktop::Screen::Pause && menu.musicVolume()==90);
        key(SDLK_DOWN); key(SDLK_DOWN); key(SDLK_RETURN);
        key(SDLK_RETURN);
        for(int i=0;i<5;++i) key(SDLK_TAB);
        key(SDLK_RETURN);
        require(menu.screen()==desktop::Screen::Pause && menu.musicVolume()==80);
        key(SDLK_RETURN);
        require(menu.screen()==desktop::Screen::Resign);
        key(SDLK_ESCAPE); require(menu.screen()==desktop::Screen::Pause);
        key(SDLK_DOWN); key(SDLK_RETURN); require(menu.screen()==desktop::Screen::Restart);
        key(SDLK_ESCAPE); require(menu.screen()==desktop::Screen::Pause);
        key(SDLK_DOWN); key(SDLK_RETURN); require(key(SDLK_RETURN)==desktop::Action::Start);
        require(menu.playing());
        if (!game.initSkirmish(0x5A17, &error, menu.initiallyExplored(),menu.humanCiv(),menu.aiCiv(),menu.populationLimit(),menu.conquest())) throw std::runtime_error(error);
        require(!menu.initiallyExplored() && game.tileVisibilityForTesting(60,2)==0);
        game.render(renderer,960,544);
        float workerX,workerY; require(game.objectScreenPosition(8,960,544,workerX,workerY));
        SDL_Event workerClick{}; workerClick.type=SDL_MOUSEBUTTONDOWN; workerClick.button.button=SDL_BUTTON_LEFT;
        workerClick.button.x=int(workerX);workerClick.button.y=int(workerY)-12;
        require(SDL_PushEvent(&workerClick)==1);
        workerClick.type=SDL_MOUSEBUTTONUP; require(SDL_PushEvent(&workerClick)==1);
        std::fprintf(stderr, "MENU PASS: launcher, main, disabled mode, setup/back, shared navigation, options, start, pause/resign/restart/cancel\n");
    }
    std::fprintf(stderr, "Ready: %zu objects. Left click/select, drag/box, right click/command, arrows/pan, wheel/zoom, Tab/actions, Q/E/pages, Space/pause, Escape/menu back, Cmd-Q/quit.\n", game.activeObjectCount());
    audio.startMusic(root + "/MUSIC/Track02.ogg", smoke);
    bool running = true, paused = false, held = false, moved = false, lastPaused = false, lastMuted = false;
    float startX = 0, startY = 0;
    Uint64 previous = SDL_GetPerformanceCounter();
    int frames = 0;
    while (running) {
        swgb::InputState input;
        const bool wasPlaying = menu.playing();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_q && (event.key.keysym.mod & KMOD_GUI)) running = false;
            if (!menu.playing()) {
                if (event.type == SDL_KEYDOWN && !event.key.repeat && event.key.keysym.sym == SDLK_F8) audio.toggleMute();
                const auto before = menu.screen();
                const auto action = menu.event(event, audio.muted());
                if (menu.screen() != before || action != desktop::Action::None) {
                    std::vector<uint8_t> wave;
                    if (assets.readSoundResource(50331, wave)) audio.effect(wave);
                }
                if (action == desktop::Action::Quit) running = false;
                if (action == desktop::Action::Mute) audio.toggleMute();
                if (action == desktop::Action::Volume) audio.setVolumes(menu.musicVolume(),menu.effectsVolume());
                if (action == desktop::Action::SaveSettings) saveSettings();
                if (action == desktop::Action::Start) {
                    if (!game.initSkirmish(0x5A17, &error, menu.initiallyExplored(),menu.humanCiv(),menu.aiCiv(),menu.populationLimit(),menu.conquest())) throw std::runtime_error(error);
                    saveSettings();
                    paused = false; held = false; moved = false;
                }
                if (action == desktop::Action::Resign) game.resignSkirmish();
                continue;
            }
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                paused = true; held = false; moved = false;
            }
            if (event.type == SDL_KEYDOWN && !event.key.repeat) {
                const auto key = event.key.keysym.sym;
                if (key == SDLK_q && (event.key.keysym.mod & KMOD_GUI)) running = false;
                if (key == SDLK_SPACE) paused = !paused;
                if(key>=SDLK_0 && key<=SDLK_9 && !(event.key.keysym.mod & (KMOD_GUI|KMOD_ALT|KMOD_SHIFT))) {
                    input.controlGroup=int(key-SDLK_0);
                    input.storeControlGroup=(event.key.keysym.mod & KMOD_CTRL)!=0;
                }
                input.healPressed = key == SDLK_y;
                input.menuBack = key == SDLK_ESCAPE;
                input.cycleAttackMode = key == SDLK_TAB || key == SDLK_b;
                input.actionTabLeft = key == SDLK_q;
                input.actionTabRight = key == SDLK_e;
                input.toggleDebug = key == SDLK_m;
                if (key == SDLK_F8) audio.toggleMute();
                if (key == SDLK_F10 || (key == SDLK_ESCAPE && !game.actionMenuOpenForTesting() && !game.classicHudSubmenuOpen() && !game.healingCursorActive())) {
                    menu.pause(); held = false; moved = false; input = {}; continue;
                }
                if (key == SDLK_RETURN && game.matchResult() >= 0) {
                    menu.main(); paused = false; input = {}; continue;
                }
                input.menuUp = key == SDLK_UP; input.menuDown = key == SDLK_DOWN;
                input.menuLeft = key == SDLK_LEFT; input.menuRight = key == SDLK_RIGHT;
                input.menuActivate = key == SDLK_RETURN;
            }
            if (event.type == SDL_MOUSEWHEEL) input.zoomStep = event.wheel.y > 0 ? 1 : event.wheel.y < 0 ? -1 : 0;
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                held = true; input.pointerPressed=true; moved = false; startX = event.button.x; startY = event.button.y;
                input.pointerX=event.button.x; input.pointerY=event.button.y;
            }
            if (event.type == SDL_MOUSEMOTION && held &&
                std::hypot(event.motion.x - startX, event.motion.y - startY) > 5) moved = true;
            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT && held) {
                held = false; input.boxSelectCommit = moved; input.selectPressed = !moved;
                input.addToSelection=(SDL_GetModState() & KMOD_SHIFT)!=0;
                input.pointerX=event.button.x; input.pointerY=event.button.y;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {
                input.commandPressed = true; input.pointerX=event.button.x; input.pointerY=event.button.y;
            }
        }
        int x, y; SDL_GetMouseState(&x, &y);
        // Button actions use their event position, not a later cursor poll.
        if(!input.pointerPressed && !input.selectPressed && !input.boxSelectCommit && !input.commandPressed) {
            input.pointerX = x; input.pointerY = y;
        }
        input.cursorVisible = !smoke && (SDL_GetWindowFlags(window.get()) & SDL_WINDOW_MOUSE_FOCUS) != 0;
        input.pointerHeld=held;
        input.boxSelectActive = held && moved;
        input.boxStartX = startX; input.boxStartY = startY; input.boxEndX = input.boxSelectCommit?input.pointerX:x; input.boxEndY = input.boxSelectCommit?input.pointerY:y;
        const auto keys = SDL_GetKeyboardState(nullptr);
        input.scrollX = keys[SDL_SCANCODE_RIGHT] - keys[SDL_SCANCODE_LEFT];
        input.scrollY = keys[SDL_SCANCODE_DOWN] - keys[SDL_SCANCODE_UP];
        const auto now = SDL_GetPerformanceCounter();
        const float dt = std::min(0.05, double(now - previous) / SDL_GetPerformanceFrequency()); previous = now;
        if (!running) break;
        int windowWidth, windowHeight;
        SDL_GetWindowSize(window.get(), &windowWidth, &windowHeight);
        if (windowWidth != menu.width() || windowHeight != menu.height())
            SDL_SetWindowSize(window.get(), menu.width(), menu.height());
        if (!menu.playing()) SDL_SetWindowTitle(window.get(), "Galactic Genie — Clone Campaigns");
        if (menu.playing() && (!wasPlaying || paused != lastPaused || audio.muted() != lastMuted)) {
            std::string title = paused ? "Galactic Genie — PAUSED | Space: resume" :
                                        "Galactic Genie — Skirmish | B: actions | Space: pause";
            title += audio.muted() ? " | MUTED (F8)" : " | F8: mute";
            SDL_SetWindowTitle(window.get(), title.c_str());
            lastPaused = paused; lastMuted = audio.muted();
        }
        if (menu.playing() && wasPlaying) {
            if(paused) game.updatePaused(dt,input);
            else game.update(smoke ? 1.f/30 : dt,input);
        }
        if (game.takeHudMenuRequest()) { menu.pause(); held=false; }
        if (input.selectPressed || input.commandPressed || input.cycleAttackMode || input.boxSelectCommit)
            std::fprintf(stderr, "input: select=%d command=%d actions=%d box=%d paused=%d selected=%zu pointer=%.0f,%.0f polled=%d,%d\n",
                         input.selectPressed, input.commandPressed, input.cycleAttackMode, input.boxSelectCommit,
                         paused, game.selectedObjectCount(),input.pointerX,input.pointerY,x,y);
        if (menu.playing() || menu.overlay()) game.render(renderer, 960, 544);
        if (!menu.playing()) { renderer.beginOverlay(); menu.render(audio.muted()); }
        SDL_Surface *surface = SDL_GetWindowSurface(window.get());
        if (!surface) throw std::runtime_error(SDL_GetError());
        if (SDL_LockSurface(surface) != 0) throw std::runtime_error(SDL_GetError());
        const int converted = SDL_ConvertPixels(menu.width(), menu.height(), SDL_PIXELFORMAT_RGBA32, renderer.pixels().data(),
                                                menu.width() * 4, surface->format->format, surface->pixels, surface->pitch);
        SDL_UnlockSurface(surface);
        if (converted != 0 || SDL_UpdateWindowSurface(window.get()) != 0) throw std::runtime_error(SDL_GetError());
        if(smoke && frames==0) {
            if(!game.objectSelected(8)) throw std::runtime_error("Button event position was replaced by mouse polling");
            game.clearSelectionForTesting();
        }
        if (smoke && ++frames == 60) {
            if(!game.selectObjectForTesting(game.commandCenterId(1))) throw std::runtime_error("HUD fixture selection failed");
            game.render(renderer,960,544);
            if(!renderer.savePng("hud-selection-smoke.png")) throw std::runtime_error("Cannot save selection HUD frame");
            if(!game.selectObjectForTesting(6)) throw std::runtime_error("HUD worker fixture missing");
            swgb::InputState actions; actions.cycleAttackMode=true;
            game.update(0,actions); game.render(renderer,960,544);
            if(!game.classicHudSubmenuOpen() || !renderer.savePng("hud-actions-smoke.png")) throw std::runtime_error("HUD action fixture failed");
            actions={}; actions.menuBack=true; game.update(0,actions);
            if(!game.selectObjectsForTesting({11,12,13,14})) throw std::runtime_error("Formation fixture selection failed");
            game.render(renderer,960,544);
            if(!renderer.savePng("hud-formations-smoke.png")) throw std::runtime_error("Formation frame failed");
            if(!game.garrisonForTesting(11,game.commandCenterId(1)) || !game.garrisonForTesting(12,game.commandCenterId(1))) throw std::runtime_error("Garrison fixture failed");
            for(int i=0;i<1800 && game.garrisonedCount(game.commandCenterId(1))<2;++i) game.update(1.f/30,{});
            if(game.garrisonedCount(game.commandCenterId(1))<2) throw std::runtime_error("Garrison entry timed out");
            game.selectObjectForTesting(game.commandCenterId(1)); game.render(renderer,960,544);
            if(!renderer.savePng("hud-garrison-smoke.png")) throw std::runtime_error("Garrison frame failed");
            swgb::InputState portraitPress; portraitPress.pointerX=portraitPress.boxStartX=333*1.2f;
            portraitPress.pointerY=portraitPress.boxStartY=340+62*1.2f; portraitPress.pointerPressed=true; portraitPress.pointerHeld=true;
            game.update(0,portraitPress); game.render(renderer,960,544);
            if(!renderer.savePng("hud-garrison-pressed-smoke.png")) throw std::runtime_error("Pressed garrison frame failed");
            portraitPress.pointerPressed=false; portraitPress.pointerHeld=false; portraitPress.pointerX=440*1.2f; game.update(0,portraitPress);

            swgb::InputState alarm; alarm.pointerX=108*1.2f; alarm.pointerY=340+146*1.2f; alarm.selectPressed=true;
            game.update(0,alarm);
            for(int i=0;i<1800 && game.garrisonedCount(game.commandCenterId(1))<7;++i) game.update(1.f/30,{});
            if(game.garrisonedCount(game.commandCenterId(1))<7 || game.classicHudIconForTesting(11)!=64) throw std::runtime_error("Alarm shelter fixture failed");
            game.render(renderer,960,544);
            if(!renderer.savePng("hud-alarm-smoke.png")) throw std::runtime_error("Alarm frame failed");
            alarm.pointerX=149*1.2f; alarm.pointerY=340+105*1.2f;
            game.update(0,alarm);
            if(game.garrisonedCount(game.commandCenterId(1))!=2 || !game.alarmActiveForTesting(game.commandCenterId(1))) throw std::runtime_error("Return-to-work must leave soldiers and bell state unchanged");
            for(int id:{50357,50358}) {
                std::vector<uint8_t> wave;
                if(!assets.readSoundResource(id,wave) || wave.empty()) throw std::runtime_error("Original alarm WAV missing");
            }
            alarm.pointerX=108*1.2f; alarm.pointerY=340+146*1.2f; game.update(0,alarm);
            game.clearSelectionForTesting(); game.render(renderer,960,544);
            const float frozenTime=game.matchTime();
            const size_t frozenObjects=game.activeObjectCount();
            menu.pause(); renderer.beginOverlay(); menu.render(audio.muted());
            if(!renderer.savePng("pause-smoke.png")) throw std::runtime_error("Cannot save pause frame");
            SDL_Event nav{}; nav.type=SDL_KEYDOWN; nav.key.keysym.sym=SDLK_DOWN; menu.event(nav,false); menu.event(nav,false);
            nav.key.keysym.sym=SDLK_RETURN; menu.event(nav,false);
            game.render(renderer,960,544); renderer.beginOverlay(); menu.render(audio.muted());
            if(!renderer.savePng("options-smoke.png") || game.matchTime()!=frozenTime || game.activeObjectCount()!=frozenObjects)
                throw std::runtime_error("Overlay changed match state");
            menu.play(); game.render(renderer,960,544);
            if (!renderer.savePng("desktop-smoke.png")) throw std::runtime_error("Cannot write smoke frame");
            if (!audio.musicVerified()) throw std::runtime_error("Music stream/EOF-loop check failed");
            if (!audio.mixedSamples()) throw std::runtime_error("Audio callback produced no PCM");
            std::fprintf(stderr, "SMOKE PASS: 60 update/render/present frames, effects + streamed music + EOF loop verified, %zu objects\n", game.activeObjectCount());
            running = false;
        }
        SDL_Delay(10);
    }
    return 0;
}
}

int main(int argc, char **argv) {
    if (argc > 3 || (argc == 3 && std::string(argv[2]) != "--smoke")) {
        std::fprintf(stderr, "usage: swgb-desktop <Game directory> [--smoke]\n"); return 2;
    }
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        std::fprintf(stderr, "%s\n", SDL_GetError()); return 1;
    }
    int result = 1;
    try {
        const char *home = std::getenv("HOME");
        const char *configured = std::getenv("SWGB_GAME_DIR");
        std::string root = argc >= 2 ? argv[1] : configured ? configured :
            home ? std::string(home) + "/Library/Application Support/Galactic Genie/Game-CloneCampaigns" : "";
        if (root.empty()) throw std::runtime_error("Set SWGB_GAME_DIR to your installed Game directory");
        result = run(root, argc == 3);
    }
    catch (const std::exception &e) {
        std::fprintf(stderr, "%s\n", e.what());
        if (argc != 3) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Galactic Genie", e.what(), nullptr);
    }
    SDL_Quit(); return result;
}
