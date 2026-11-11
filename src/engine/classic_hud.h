// SPDX-License-Identifier: GPL-3.0-or-later
// Included by game.cpp: desktop presentation using the installed 800px HUD sheets.
namespace {
struct ClassicGarrisonLayout {
    size_t first, count, columns=4;
    float step=38;
    ClassicGarrisonLayout(size_t occupants,bool busy):first(occupants>size_t(busy?19:40)?occupants-size_t(busy?19:40):0),count(occupants-first) {
        const size_t rows=busy?1:3;
        if(count>rows*columns) {columns=(count+rows-1)/rows;step=114.f/float(columns-1);}
    }
    std::array<float,2> position(size_t index) const {
        const size_t cell=count-1-(index-first);
        return {float(int(275+float(cell%columns)*step)),42+float(cell/columns)*41};
    }
    int hit(float x,float y) const {
        // Later portraits are painted on top where original compressed columns overlap.
        for(size_t i=first+count;i>first;--i) {
            const auto p=position(i-1);
            if(x>=p[0] && x<p[0]+40 && y>=p[1] && y<p[1]+40) return int(i-1);
        }
        return -1;
    }
};
struct ClassicHudLayout {
    float scale, top;
    ClassicHudLayout(int w,int h):scale(w/800.f),top(h-170*scale) {}
    float x(float value) const { return value*scale; }
    float y(float value) const { return top+value*scale; }
    int slot(float px,float py) const {
        const float x=px/scale-6, y=(py-top)/scale-3;
        if(x<0 || y<0 || x>=164 || y>=164) return -1;
        return int(y/41)*4+int(x/41);
    }
    bool mapPoint(float px,float py,int size,float &tx,float &ty) const {
        const float dx=(px/scale-586)/133, dy=((py-top)/scale-98)/67;
        if(std::abs(dx)+std::abs(dy)>1) return false;
        tx=std::clamp((dy+1+dx)*size/2,0.f,size-0.001f);
        ty=std::clamp((dy+1-dx)*size/2,0.f,size-0.001f);
        return true;
    }
    std::array<float,2> project(float tx,float ty,int size) const {
        return {x(586+(tx-ty)*133/size),y(98+((tx+ty)/size-1)*67)};
    }
};
}

void Game::syncClassicHud() {
    const auto selected=selectedObjectsInOrder(false);
    const uint32_t subject=selected.empty()?0:selected.front()->spawnId;
    if(subject!=classicHudSubject_) {
        classicHudSubject_=subject; classicHudRoot_=true; classicGarrisonPressed_=0;
    }
}

std::array<Game::HudButton,16> Game::classicHudButtons() {
    syncClassicHud();
    std::array<HudButton,16> slots{};
    Object *subject=findObject(classicHudSubject_);
    if(!subject || subject->player!=localPlayer_ || !isInspectable(*subject)) return slots;
    if(healingCursorActive()) {
        slots[14].kind=HudButton::CancelTarget; slots[14].icon=0; slots[14].label="Cancel";
        return slots;
    }
    if(gatherPointBuildingId_==subject->spawnId) {
        // Original mode 0x6f replaces the command grid with remove/cancel.
        slots[0].kind=HudButton::RemoveRally; slots[0].icon=46;
        slots[0].label=buildingCommandTitle(*subject,BuildingCommand::RemoveGatherPoint);
        slots[14].kind=HudButton::CancelTarget; slots[14].icon=0; slots[14].label="Cancel";
        return slots;
    }
    const int civ=civilizationForPlayer(subject->player);
    const int iconSet=std::max(1,int(assets_.dat().civs[size_t(civ)].iconSet));
    std::vector<HudButton> entries;
    auto units=[&](ActionMenuTab tab,const std::vector<const dat::Unit *> &options) {
        for(size_t i=0;i<options.size();++i) {
            const auto *u=options[i];
            HudButton b; b.kind=HudButton::Action; b.tab=tab; b.option=i;
            b.slp=(u->type==dat::UT_Building?kBuildingIconSlpBase:kUnitIconSlpBase)+iconSet-1;
            b.icon=u->iconId; b.preferred=int(u->buttonId)-(u->buttonId>=17 && u->buttonId<=31?17:1); b.label=unitDisplayName(*u);
            const char *names[]={"Food","Carbon","Ore","Nova"};
            for(const auto &cost:u->costs) if(cost.flag && cost.type>=0 && cost.type<4 && cost.amount>0)
                b.label+="  "+std::string(names[cost.type])+": "+std::to_string(cost.amount);
            entries.push_back(b);
        }
    };
    if(!classicHudRoot_) {
        units(classicHudTab_,buildingOptions(*subject,classicHudTab_));
    } else {
        if(isWorker(*subject)) {
            for(auto tab:{ActionMenuTab::Economy,ActionMenuTab::Military,ActionMenuTab::Defense}) {
                const auto options=buildingOptions(*subject,tab);
                if(options.empty()) continue;
                HudButton b; b.kind=HudButton::Tab; b.tab=tab;
                b.preferred=tab==ActionMenuTab::Economy?0:tab==ActionMenuTab::Military?1:2;
                b.icon=tab==ActionMenuTab::Economy?30:tab==ActionMenuTab::Military?31:29;
                b.label=tab==ActionMenuTab::Economy?"Economic buildings":tab==ActionMenuTab::Military?"Military buildings":"Defensive buildings";
                entries.push_back(b);
            }
        } else if(subject->unit->type==dat::UT_Building) {
            units(ActionMenuTab::Units,productionOptions(*subject));
            const auto technologies=researchOptions(*subject);
            for(size_t i=0;i<technologies.size();++i) {
                const auto &tech=assets_.dat().techs[size_t(technologies[i])];
                HudButton b; b.kind=HudButton::Action; b.tab=ActionMenuTab::Research; b.option=i;
                b.slp=kTechnologyIconSlpBase+iconSet-1; b.icon=tech.iconId; b.preferred=int(tech.buttonId)-1;
                b.label=technologyDisplayName(technologies[i]); entries.push_back(b);
            }
        }
        if(subject->unit->type==dat::UT_Building) {
            const auto commands=buildingCommands(*subject);
            for(size_t i=0;i<commands.size();++i) {
                HudButton b; b.kind=HudButton::Action; b.tab=ActionMenuTab::Commands; b.option=i;
                b.preferred=commands[i]==BuildingCommand::Alarm?14:commands[i]==BuildingCommand::ReturnToWork?11:commands[i]==BuildingCommand::ToggleGate?0:commands[i]==BuildingCommand::Eject?12:13;
                if(commands[i]==BuildingCommand::RemoveGatherPoint) continue;
                if(commands[i]==BuildingCommand::Eject && garrisonedCount(*subject,false)==0) continue;
                b.icon=buildingCommandIcon(*subject,commands[i]); b.label=buildingCommandTitle(*subject,commands[i]); entries.push_back(b);
            }
        } else {
            const auto commands=unitCommands(*subject);
            for(size_t i=0;i<commands.size();++i) {
                HudButton b; b.kind=HudButton::Action; b.tab=ActionMenuTab::Commands; b.option=i;
                if(isWorker(*subject) && commands[i]!=UnitCommand::Stop) continue;
                if(canAttackGround(*subject) && commands[i]!=UnitCommand::Stop && commands[i]!=UnitCommand::AttackGround && commands[i]!=UnitCommand::Unload) continue;
                b.preferred=commands[i]==UnitCommand::Unload?13:commands[i]==UnitCommand::Stop?3:commands[i]==UnitCommand::Guard?1:commands[i]==UnitCommand::Follow?2:0;
                b.icon=unitCommandIcon(commands[i]); b.label=unitCommandTitle(commands[i]); entries.push_back(b);
            }
            if(!isWorker(*subject) && canAttack(*subject)) for(size_t i=0;i<4;++i) {
                HudButton b; b.kind=HudButton::Action; b.tab=ActionMenuTab::Stances; b.option=i; b.preferred=4+int(i);
                b.icon=int(size_t(subject->attackMode)==i?kStanceActiveIcons[i]:kStanceIcons[i]);
                b.label=assets_.localizedString(kStanceNameStrings[i]); entries.push_back(b);
            }
            if(hasAnyGarrisonTask(*subject)) {
                HudButton b; b.kind=HudButton::Garrison; b.preferred=12; b.icon=int(kCommandGarrisonIcon); b.label="Garrison"; entries.push_back(b);
            }
            if(isWorker(*subject)) {
                HudButton b; b.kind=HudButton::Repair; b.preferred=7; b.icon=int(kCommandRepairIcon); b.label="Repair"; entries.push_back(b);
            }
        }
        const auto selected=selectedObjectsInOrder(false);
        const auto military=std::count_if(selected.begin(),selected.end(),[&](const Object *o) {
            return o->player==localPlayer_ && !isWorker(*o) && o->unit->speed>0 && o->unit->type>=dat::UT_Combatant;
        });
        if(military>1) {
            // 0x502d20: slots 8..11; original help strings establish the formation names.
            const FormationType types[]={FormationType::Line,FormationType::Box,FormationType::Staggered,FormationType::Flank};
            const int normal[]={40,43,63,62},active[]={65,66,68,67};
            const int help[]={41021,41020,41022,41023};
            for(int i=0;i<4;++i) {
                HudButton b; b.kind=HudButton::Formation; b.option=size_t(types[i]); b.preferred=8+i;
                b.icon=selectedFormation_==types[i]?active[i]:normal[i]; b.label=assets_.localizedString(help[i]);
                const auto end=b.label.find("\n"); if(end!=std::string::npos) b.label.resize(end);
                for(size_t tag=b.label.find("<b>");tag!=std::string::npos;tag=b.label.find("<b>")) b.label.erase(tag,3);
                entries.push_back(b);
            }
        }
        if(subject->unit->type!=dat::UT_Building) {
            HudButton b; b.kind=HudButton::Destroy; b.preferred=14; b.icon=int(kCommandDestroyIcon); b.label="Delete selected unit"; entries.push_back(b);
        }
    }
    // Original constructors use these DAT slots directly; empty cells remain empty.
    for(const auto &button:entries) if(button.preferred>=0 && button.preferred<15)
        slots[size_t(button.preferred)]=button;
    if(!classicHudRoot_) {
        // 0x504490/0x504900/0x504b80 and action 12: Economy -> Military -> Defense.
        slots[13].kind=HudButton::Tab; slots[13].icon=32;
        slots[13].tab=classicHudTab_==ActionMenuTab::Economy?ActionMenuTab::Military:
            classicHudTab_==ActionMenuTab::Military?ActionMenuTab::Defense:ActionMenuTab::Economy;
        slots[13].label="Next building category";
        slots[14].kind=HudButton::Root; slots[14].icon=0; slots[14].label="Back";
    }
    return slots;
}

std::vector<Game::Object *> Game::classicGarrisonMembers() {
    std::vector<Object *> members;
    const Object *subject=findObject(classicHudSubject_);
    if(!subject || subject->player!=localPlayer_) return members;
    for(auto &object:objects_) if(object.active && object.garrisonedInId==int32_t(subject->spawnId)) members.push_back(&object);
    return members;
}

void Game::handleClassicHudInput(InputState &in,bool allowOrders) {
    if(in.menuBack && healingCursorActive()) {
        unitCommandCursorActive_=false; unitCommandJustBegun_=false; in.menuBack=false;
    }
    syncClassicHud();
    Object *subject=findObject(classicHudSubject_);
    if(in.menuBack && !classicHudRoot_) {classicHudRoot_=true; in.menuBack=false;}
    if(in.cycleAttackMode) {
        if(subject && isWorker(*subject)) {classicHudRoot_=!classicHudRoot_; classicHudTab_=ActionMenuTab::Economy;}
        in.cycleAttackMode=false;
    }
    const ClassicHudLayout layout(in.screenW,in.screenH);
    if(in.boxStartY>=layout.top) {in.boxSelectActive=false; in.boxSelectCommit=false;}
    else {in.boxEndY=std::min(in.boxEndY,layout.top-1);}
    if(in.pointerY<layout.top) {if(!in.pointerHeld) classicGarrisonPressed_=0; return;}
    const auto members=classicGarrisonMembers();
    const ClassicGarrisonLayout garrison(members.size(),subject && !subject->productionQueue.empty());
    if(in.pointerPressed) {
        const int hit=garrison.hit(in.boxStartX/layout.scale,(in.boxStartY-layout.top)/layout.scale);
        classicGarrisonPressed_=selectedObjectsInOrder(false).size()==1 && hit>=0?members[size_t(hit)]->spawnId:0;
    }

    const bool clicked=in.selectPressed || in.pointerTap;
    const float x=in.pointerX/layout.scale,y=(in.pointerY-layout.top)/layout.scale;
    float tx,ty;
    if(layout.mapPoint(in.pointerX,in.pointerY,mapSize_,tx,ty)) {
        if(clicked) lookAt(tx,ty);
        if(in.commandPressed && allowOrders) {
            float sx,sy; toScreen(tx,ty,sx,sy);
            commandAtScreen((sx-camX_)*zoom_+in.screenW/2.f,(sy-camY_)*zoom_+in.screenH/2.f,in.screenW,in.screenH);
        }
    } else if(clicked && x>=743 && x<793 && y>=37 && y<56) {
        classicHudMenuRequested_=true;
    } else if(clicked && subject && subject->player==localPlayer_) {
        const auto selected=selectedObjectsInOrder(false);
        if(selected.size()>1 && x>=185 && x<425 && y>=82 && y<162) {
            const size_t index=size_t((y-82)/40)*6+size_t((x-185)/40);
            if(index<selected.size()) {
                Object *chosen=selected[index]; clearSelection(); selectObject(*chosen);
                if(playUnitSound_ && chosen->unit->selectionSound>=0)
                    playUnitSound_(chosen->unit->selectionSound,civilizationForPlayer(chosen->player));
            }
        } else if(selected.size()==1 && garrison.hit(x,y)>=0) {
            const size_t index=size_t(garrison.hit(x,y));
            if(allowOrders && (in.pointerTap || classicGarrisonPressed_==members[index]->spawnId)) {
                const std::string name=unitDisplayName(*members[index]->unit);
                const bool ejected=ejectGarrisonedUnit(*subject,*members[index]);
                statusMessage_=ejected?"EJECTED "+name:"NO CLEAR EXIT"; statusTime_=2;
                playInterfaceFeedback(ejected?kInterfaceButtonSound:kInterfaceCannotDoSound);
            }
        } else if(selected.size()==1 && x>=185 && x<435 && y>=123 && y<162) {
            const size_t index=size_t((x-185)/48);
            if(allowOrders && index<subject->productionQueue.size()) cancelProductionItem(*subject,index);
        } else {
            const int slot=layout.slot(in.pointerX,in.pointerY);
            if(slot>=0) {
                const auto b=classicHudButtons()[size_t(slot)];
                if(b.kind==HudButton::Tab) {classicHudTab_=b.tab; classicHudRoot_=false;}
                if(b.kind==HudButton::Root) {classicHudRoot_=true;}
                if(b.kind==HudButton::CancelTarget) {gatherPointBuildingId_=0; gatherPointJustBegun_=false; unitCommandCursorActive_=false; unitCommandJustBegun_=false;}
                if(allowOrders && b.kind==HudButton::RemoveRally) {
                    executeBuildingCommand(*subject,BuildingCommand::RemoveGatherPoint);
                    gatherPointBuildingId_=0; gatherPointJustBegun_=false;
                }
                if(allowOrders && b.kind==HudButton::Formation) { applyFormation(FormationType(b.option)); playInterfaceFeedback(kInterfaceButtonSound); }
                if(allowOrders && b.kind==HudButton::Action) {
                    actionMenuObjectId_=subject->spawnId; actionMenuTab_=b.tab;
                    activateActionMenuOption(b.option);
                    actionMenuOpen_=false; actionMenuObjectId_=0;
                }
                if(allowOrders && (b.kind==HudButton::Garrison || b.kind==HudButton::Repair || b.kind==HudButton::Destroy)) {
                    const float offset=b.kind==HudButton::Garrison?20:b.kind==HudButton::Repair?72:124;
                    handleSelectionPanelClick(std::max(500.f,in.screenW-300.f)+offset,in.screenH-kSelectionPanelHeight+25,in.screenW,in.screenH);
                }
            }
        }
    }
    if(!in.pointerHeld) classicGarrisonPressed_=0;
    // UI gestures must never select, place, scroll or issue world orders underneath it.
    in.selectPressed=in.pointerTap=in.commandPressed=false;
    in.cursorVisible=false; in.zoomStep=0;
}

void Game::renderClassicHud(Renderer &r,int width,int height) {
    syncClassicHud();
    const ClassicHudLayout layout(width,height);
    const float unit=1/zoom_;
    auto rect=[&](float x,float y,float w,float h,uint8_t red,uint8_t green,uint8_t blue) {
        r.fillRect(layout.x(x)*unit,layout.y(y)*unit,w*layout.scale*unit,h*layout.scale*unit,red,green,blue,255);
    };
    auto text=[&](const std::string &s,float x,float y,float size=11) {
        drawBitmapText(r,{s},layout.x(x)*unit,layout.y(y)*unit,size*layout.scale*unit/9);
    };
    auto icon=[&](int slp,int index,float x,float y,float size) {
        const auto *sheet=assets_.interfaceSheet(slp);
        if(!sheet || index<0 || size_t(index)>=sheet->frames.size()) return;
        const auto &frame=sheet->frames[size_t(index)];
        if(frame.w<=0 || frame.h<=0) return;
        const float factor=std::min(size/frame.w,size/frame.h);
        r.draw(frame.tex,{layout.x(x+(size-frame.w*factor)/2)*unit,layout.y(y+(size-frame.h*factor)/2)*unit,
            frame.w*factor*layout.scale*unit,frame.h*factor*layout.scale*unit,frame.u,frame.v,frame.u+frame.w,frame.v+frame.h});
    };
    auto croppedIcon=[&](int slp,int index,float x,float y,float cx,float cy,float cw,float ch) {
        const auto *sheet=assets_.interfaceSheet(slp);
        if(!sheet || index<0 || size_t(index)>=sheet->frames.size()) return;
        const auto &f=sheet->frames[size_t(index)];
        const float left=std::max(x,cx),top=std::max(y,cy),right=std::min(x+f.w,cx+cw),bottom=std::min(y+f.h,cy+ch);
        if(right<=left || bottom<=top) return;
        r.draw(f.tex,{layout.x(left)*unit,layout.y(top)*unit,(right-left)*layout.scale*unit,(bottom-top)*layout.scale*unit,
            f.u+left-x,f.v+top-y,f.u+right-x,f.v+bottom-y});
    };
    const int civ=civilizationForPlayer(localPlayer_);
    const int iconSet=std::max(1,int(assets_.dat().civs[size_t(civ)].iconSet));
    const auto *skin=assets_.interfaceSheet(51100+std::clamp(iconSet,1,8));
    if(skin && !skin->frames.empty()) {
        const auto &f=skin->frames.front();
        r.draw(f.tex,{0,layout.top*unit,width*unit,170*layout.scale*unit,f.u,f.v,f.u+f.w,f.v+f.h});
    }
    int level=1; for(int id=1;id<=3;++id) if(researchedTechs_[size_t(localPlayer_)].count(id)) level=id+1;
    text("Tech "+std::to_string(level),280,4);
    const int resources[]={1,0,3,2};
    for(int i=0;i<4;++i) text(std::to_string(int(resource(localPlayer_,resources[i]))),469+i*73,5,10);
    text(std::to_string(int(populationUsed(localPlayer_)))+"/"+std::to_string(int(populationCapacity(localPlayer_))),758,5,9);
    // Terrain and objects use the same isometric transform as minimap input.
    const auto &terrains=assets_.dat().terrainBlock.terrains;
    for(int y=31;y<165;++y) {
        const float halfWidth=138*(1-std::abs((y+.5f-98)/67));
        for(int x=int(std::ceil(586-halfWidth));x<586+halfWidth;x+=2) {
            float tx,ty;
            if(!layout.mapPoint(layout.x(x+.5f),layout.y(y+.5f),mapSize_,tx,ty)) continue;
            const int terrain=rememberedTerrainAt(int(tx),int(ty));
            if(terrain<0 || size_t(terrain)>=terrains.size()) continue;
            const auto c=assets_.palette()[terrains[size_t(terrain)].colors[0]];
            const int visibility=tileVisibility(int(tx),int(ty));
            const float light=visibility==2?1.f:visibility==1?144.f/255:0;
            rect(x,y,std::min(2.f,586+halfWidth-x),1,uint8_t(c.r*light),uint8_t(c.g*light),uint8_t(c.b*light));
        }
    }
    auto marker=[&](const Object &o,bool remembered) {
        if(!o.active || o.hidden || o.player<=0) return;
        const auto p=layout.project(o.x,o.y,mapSize_); const auto c=assets_.palette()[uint8_t(playerColorBase(o.player)+4)];
        const float light=remembered?144.f/255:1.f;
        r.fillRect(p[0]*unit,p[1]*unit,3*layout.scale*unit,3*layout.scale*unit,uint8_t(c.r*light),uint8_t(c.g*light),uint8_t(c.b*light),255);
    };
    for(const auto &o:objects_) if(objectVisible(o)) marker(o,false);
    for(const auto &entry:rememberedObjects_) if(!objectVisible(entry.second) && tileVisibility(int(entry.second.x),int(std::ceil(entry.second.y))-1)>0) marker(entry.second,true);
    std::array<std::array<float,2>,4> corners;
    for(int i=0;i<4;++i) {
        float tx,ty; screenToWorld((i==1 || i==2)?float(width):0,(i>=2)?layout.top:0,width,height,tx,ty);
        corners[size_t(i)]=layout.project(std::clamp(tx,0.f,float(mapSize_)),std::clamp(ty,0.f,float(mapSize_)),mapSize_);
    }
    for(int i=0;i<4;++i) r.drawLine(corners[i][0]*unit,corners[i][1]*unit,corners[(i+1)%4][0]*unit,corners[(i+1)%4][1]*unit,unit,255,255,255,255);
    rect(743,37,50,19,4,20,30); text(assets_.localizedString(10020),751,41);
    const auto buttons=classicHudButtons();
    for(size_t i=0;i<buttons.size();++i) if(buttons[i].kind!=HudButton::Empty) {
        const float x=6+float(i%4)*41,y=3+float(i/4)*41;
        if(buttons[i].icon>=0) icon(buttons[i].slp,buttons[i].icon,x+2,y+2,35);
        else text(buttons[i].label,x+2,y+13,9);
    }
    Object *subject=findObject(classicHudSubject_);
    if(!subject || !isInspectable(*subject)) return;
    const int selectedCiv=civilizationForPlayer(subject->player);
    const int selectedIcons=std::max(1,int(assets_.dat().civs[size_t(selectedCiv)].iconSet));
    const auto selected=selectedObjectsInOrder(false);
    if(selected.size()==1) {
    icon((subject->unit->type==dat::UT_Building?kBuildingIconSlpBase:kUnitIconSlpBase)+selectedIcons-1,subject->unit->iconId,180,47,35);
    text(unitDisplayName(*subject->unit),180,32,10);
    text(std::to_string(int(subject->hitPoints))+"/"+std::to_string(int(subject->maxHitPoints)),223,84,10);
    rect(180,89,40,4,75,10,10);
    rect(180,89,40*std::clamp(subject->hitPoints/std::max(1.f,subject->maxHitPoints),0.f,1.f),4,20,210,40);
    if(subject->maxShieldPoints>0) {
        rect(180,99,40,3,75,45,0);
        rect(180,99,40*std::clamp(subject->shieldPoints/subject->maxShieldPoints,0.f,1.f),3,240,185,40);
        text(std::to_string(int(subject->shieldPoints))+"/"+std::to_string(int(subject->maxShieldPoints)),223,94,10);
    }
    }
    if(selected.size()>1) {
        for(size_t i=0;i<std::min(size_t(12),selected.size());++i) {
            const auto *o=selected[i];
            const int set=std::max(1,int(assets_.dat().civs[size_t(civilizationForPlayer(o->player))].iconSet));
            const float x=185+(i%6)*40,y=82+(i/6)*40;
            icon((o->unit->type==dat::UT_Building?kBuildingIconSlpBase:kUnitIconSlpBase)+set-1,o->unit->iconId,x,y,34);
            rect(x,y+35,34*std::clamp(o->hitPoints/std::max(1.f,o->maxHitPoints),0.f,1.f),2,20,210,40);
        }
    }
    else if(subject->resourceAmount>0) text(std::to_string(int(subject->resourceAmount)),185,82);
    if(subject->underConstruction) text(std::to_string(int(100*(1-subject->constructionRemaining/std::max(1.f,subject->constructionTotal))))+"%",300,82);
    if(selected.size()==1) {
        const auto members=classicGarrisonMembers();
        const ClassicGarrisonLayout garrison(members.size(),!subject->productionQueue.empty());
        if(!members.empty()) {
            for(size_t i=garrison.first;i<garrison.first+garrison.count;++i) {
                const auto *o=members[i];
                const auto p=garrison.position(i); const float x=p[0],y=p[1];
                const int memberIcons=std::max(1,int(assets_.dat().civs[size_t(civilizationForPlayer(o->player))].iconSet));
                const int pressed=classicGarrisonPressed_==o->spawnId?1:0;
                // Original btngame frame is54px; two cropped draws preserve its40px button corners.
                croppedIcon(50751,36+pressed,x-14,y-14,x,y,40,40);
                croppedIcon(50751,36+pressed,x,y,x,y,38,38);
                croppedIcon(kUnitIconSlpBase+memberIcons-1,o->unit->iconId,x+1+pressed,y+1+pressed,x+2+pressed,y+2+pressed,36-pressed,36-pressed);
                rect(x+2,y+36,36,2,75,10,10);
                rect(x+2,y+36,36*std::clamp(o->hitPoints/std::max(1.f,o->maxHitPoints),0.f,1.f),2,20,210,40);
                if(o->maxShieldPoints>0) {
                    rect(x+2,y+38,36,2,75,45,0);
                    rect(x+2,y+38,36*std::clamp(o->shieldPoints/o->maxShieldPoints,0.f,1.f),2,240,185,40);
                }
            }
        }
    }
    if(selected.size()==1 && subject->player==localPlayer_) for(size_t i=0;i<std::min(size_t(5),subject->productionQueue.size());++i) {
        const auto &item=subject->productionQueue[i];
        rect(185+i*48,123,43,39,5,12,20);
        if(item.unit) icon(kUnitIconSlpBase+selectedIcons-1,item.unit->iconId,187+i*48,124,34);
        else if(item.technologyId>=0) icon(kTechnologyIconSlpBase+selectedIcons-1,assets_.dat().techs[size_t(item.technologyId)].iconId,187+i*48,124,34);
        if(i==0) rect(185,163,43*std::clamp(1-subject->productionRemaining/std::max(.1f,item.duration),0.f,1.f),3,50,190,240);
    }
    const int hover=layout.slot(cursorX_,cursorY_);
    std::string hoverLabel;
    if(hover>=0 && buttons[size_t(hover)].kind!=HudButton::Empty) hoverLabel=buttons[size_t(hover)].label;
    if(selected.size()==1) {
        const auto members=classicGarrisonMembers();
        const ClassicGarrisonLayout garrison(members.size(),!subject->productionQueue.empty());
        const float x=cursorX_/layout.scale,y=(cursorY_-layout.top)/layout.scale;
        const int index=garrison.hit(x,y);
        if(index>=0) {
                const auto *member=members[index];
                hoverLabel=unitDisplayName(*member->unit)+"  "+std::to_string(int(member->hitPoints))+"/"+std::to_string(int(member->maxHitPoints));
                if(member->maxShieldPoints>0) hoverLabel+="  Shield: "+std::to_string(int(member->shieldPoints))+"/"+std::to_string(int(member->maxShieldPoints));
                hoverLabel+="  — Click to eject";
        }
    }
    if(hover>=0 && buttons[size_t(hover)].kind==HudButton::Action &&
       buttons[size_t(hover)].tab==ActionMenuTab::Commands && subject->unit->type!=dat::UT_Building) {
        const auto commands=unitCommands(*subject);
        const auto option=buttons[size_t(hover)].option;
        if(option<commands.size() && commands[option]==UnitCommand::Heal) {
            std::string help=unitCommandHelp(UnitCommand::Heal);
            for(const char *tag:{"<b>","</b>","<B>","<i>","<I>"})
                for(size_t at=help.find(tag);at!=std::string::npos;at=help.find(tag)) help.erase(at,std::char_traits<char>::length(tag));
            r.fillRect(layout.x(4)*unit,layout.y(-130)*unit,540*layout.scale*unit,114*layout.scale*unit,0,0,0,170);
            text(unitCommandTitle(UnitCommand::Heal),5,-128);
            float y=-108;
            for(const auto &line:wrapText(help+" (Y)",75)) {text(line,5,y); y+=14;}
            return;
        }
    }
    if(!hoverLabel.empty()) {
        r.fillRect(4*unit,(layout.top-24)*unit,(width-8)*unit,22*unit,0,0,0,230);
        drawBitmapText(r,{hoverLabel},10*unit,(layout.top-20)*unit,1.1f*unit);
    }
}
