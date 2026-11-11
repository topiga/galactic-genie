// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <filesystem>
#include <fstream>
#include <string>

namespace desktop {
struct Preferences {
    int music=100, effects=100, humanCiv=1, aiCiv=3, population=75;
    bool explored=false, conquest=true;
    bool valid() const {
        return music>=0 && music<=100 && effects>=0 && effects<=100 &&
               humanCiv>=1 && humanCiv<=8 && aiCiv>=1 && aiCiv<=8 &&
               population>=25 && population<=200 && population%25==0;
    }
};
inline bool readPreferences(const std::filesystem::path &path,Preferences &out) {
    std::ifstream file(path);Preferences parsed;int version=0,explored=0,conquest=0;
    if(!(file>>version>>parsed.music>>parsed.effects>>parsed.humanCiv>>parsed.aiCiv>>parsed.population>>explored) ||
       (version!=1 && version!=2) || (explored!=0 && explored!=1) || !parsed.valid()) return false;
    if(version==2 && (!(file>>conquest) || (conquest!=0 && conquest!=1))) return false;
    file>>std::ws;if(!file.eof()) return false;
    parsed.conquest=conquest!=0;parsed.explored=explored!=0;out=parsed;return true;
}
inline bool writePreferences(const std::filesystem::path &path,const Preferences &settings,std::string &error) {
    if(!settings.valid()) {error="Invalid settings";return false;}
    const auto pending=path.string()+".tmp";
    {
        std::ofstream file(pending,std::ios::trunc);
        file<<2<<' '<<settings.music<<' '<<settings.effects<<' '<<settings.humanCiv<<' '
            <<settings.aiCiv<<' '<<settings.population<<' '<<int(settings.explored)<<' '<<int(settings.conquest)<<'\n';
        file.close();
        if(!file) {error="Cannot write settings: "+path.string();std::error_code ec;std::filesystem::remove(pending,ec);return false;}
    }
    std::error_code ec;std::filesystem::rename(pending,path,ec);
    if(ec) {error="Cannot replace settings: "+ec.message();std::filesystem::remove(pending,ec);return false;}
    return true;
}
}
