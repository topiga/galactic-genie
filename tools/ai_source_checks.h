// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <memory>
static void aiSourceChecks(swgb::Assets &assets) {
    swgb::AiProgram program;std::string error,source;
    auto lifetime=std::make_shared<int>(0);
    auto loader=[lifetime](const std::string &name,std::string &text,std::string *err) {
        ++*lifetime;
        if(name=="entry.per")text="(load \"child\") (load \"CHILD\")";
        else if(name=="child.per")text="(load \"entry\") (defrule (true) => (set-goal 77 9))";
        else {if(err)*err="not in test package";return false;}
        return true;
    };
    check(program.load("entry.per",{},&error,loader),error.c_str());
    check(program.rules.size()==1&&program.files.size()==2&&*lifetime==2,
          "Source resolver follows includes once and terminates cycles");
    check(lifetime.use_count()==2,"Completed load releases the source resolver");
    check(program.loadSource("CHILD.per","(load \"child\") (defrule (true) => (set-goal 77 9))",{},&error,loader) &&
          program.rules.size()==1 && *lifetime==2,"Embedded root participates in case-insensitive cycle detection");
    check(!program.loadSource("embedded.per","(load \"missing\")",{},&error,loader)&&error=="not in test package",
          "Missing embedded include preserves resolver error without disk fallback");
    check(lifetime.use_count()==2,"Failed load releases the source resolver");
    check(!assets.readAiSource("../randomgame.per",source,&error)&&source.empty(),
          "Packaged AI rejects traversal");
    check(!assets.readAiSource("/tmp/randomgame.per",source,&error)&&source.empty(),
          "Packaged AI rejects absolute paths");
    check(!assets.readAiSource("unknown.per",source,&error)&&source.empty(),
          "Packaged AI rejects unknown names");
    if(assets.readAiSource("randomgame.per",source,&error)) {
        std::string upper;
        check(assets.readAiSource("RANDOMGAME.PER",upper,&error)&&source==upper,
              "Original packaged AI names are case insensitive");
        check(program.load("randomgame.per",{},&error,[&](const std::string &name,std::string &text,std::string *err) {
            return assets.readAiSource(name,text,err);
        }),error.c_str());
        check(!program.rules.empty()&&program.files.size()>1,"Packaged original AI resolves its complete include graph");
    }
    std::puts("PASS: AI source resolver, include deduplication, lifetime and packaged name boundary");
}
