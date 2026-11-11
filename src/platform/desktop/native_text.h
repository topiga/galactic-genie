// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#ifdef __APPLE__
#include <CoreText/CoreText.h>
#include <map>
#include <array>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include "../../render/renderer.h"

namespace desktop {
// Cache complete UTF-8 labels so CoreText keeps kerning and accented glyphs.
class NativeText {
    struct Label { swgb::Texture *texture; int width, height; };
    swgb::Renderer &renderer_;
    std::map<std::string,Label> labels_;
    std::string family_;
public:
    explicit NativeText(swgb::Renderer &renderer) : renderer_(renderer) {}
    ~NativeText() { clear(); }
    void clear() { for(auto &entry:labels_) renderer_.destroyTexture(entry.second.texture); labels_.clear(); }
    void family(const std::string &name) { clear(); family_=name; }
    const Label &label(const std::string &value,int pixels,bool bold) {
        const std::string key=std::to_string(pixels)+(bold?"B:":"N:")+value;
        auto found=labels_.find(key); if(found!=labels_.end()) return found->second;
        if(labels_.size()>=256) clear();
        auto name=CFStringCreateWithCString(nullptr,family_.c_str(),kCFStringEncodingUTF8);
        auto base=CTFontCreateWithName(name,pixels,nullptr);
        auto resolved=CTFontCopyFamilyName(base);
        const bool matches=CFStringCompare(name,resolved,kCFCompareCaseInsensitive)==kCFCompareEqualTo;
        CFRelease(resolved); CFRelease(name);
        if(!matches) { CFRelease(base); throw std::runtime_error("Original interface font unavailable: "+family_); }
        auto font=bold?CTFontCreateCopyWithSymbolicTraits(base,0,nullptr,kCTFontBoldTrait,kCTFontBoldTrait):nullptr;
        if(!font) font=static_cast<CTFontRef>(CFRetain(base));
        CFRelease(base);
        auto string=CFStringCreateWithCString(nullptr,value.c_str(),kCFStringEncodingUTF8);
        if(!string) { CFRelease(font); throw std::runtime_error("Invalid UTF-8 interface label"); }
        const void *keys[]={kCTFontAttributeName,kCTForegroundColorFromContextAttributeName};
        const void *values[]={font,kCFBooleanTrue};
        auto attributes=CFDictionaryCreate(nullptr,keys,values,2,&kCFTypeDictionaryKeyCallBacks,&kCFTypeDictionaryValueCallBacks);
        auto attributed=CFAttributedStringCreate(nullptr,string,attributes);
        auto line=CTLineCreateWithAttributedString(attributed);
        CGFloat ascent=0,descent=0;
        const int width=std::max(1,int(std::ceil(CTLineGetTypographicBounds(line,&ascent,&descent,nullptr)))+4);
        const int height=std::max(1,int(std::ceil(ascent+descent))+4);
        std::vector<uint8_t> rgba(size_t(width)*height*4,0);
        auto space=CGColorSpaceCreateDeviceRGB();
        auto context=CGBitmapContextCreate(rgba.data(),width,height,8,width*4,space,kCGImageAlphaPremultipliedLast|kCGBitmapByteOrder32Big);
        CGColorSpaceRelease(space);
        if(context) {
            CGContextSetRGBFillColor(context,1,1,1,1);
            CGContextSetTextPosition(context,2,2+descent);
            CTLineDraw(line,context);
            CGContextRelease(context);
        }
        CFRelease(line); CFRelease(attributed); CFRelease(attributes); CFRelease(string); CFRelease(font);
        if(!context) throw std::runtime_error("Cannot rasterize interface label");
        std::vector<uint8_t> alpha(size_t(width)*height);
        for(int y=0;y<height;++y) for(int x=0;x<width;++x)
            alpha[size_t(y)*width+x]=rgba[(size_t(y)*width+x)*4+3];
        return labels_.emplace(key,Label{renderer_.createMaskTexture(width,height,alpha.data()),width,height}).first->second;
    }
    int height(const std::string &value,int pixels=11,bool bold=true) { return label(value,pixels,bold).height-4; }
    int width(const std::string &value,int pixels=11,bool bold=true) { return label(value,pixels,bold).width-4; }
    void draw(const std::string &value,float x,float y,bool bright,int pixels=11,bool bold=true,bool focus=false,const std::array<uint8_t,3> &focusColor={102,152,203}) {
        drawColored(value,x,y,focus?focusColor[0]:bright?255:140,
                    focus?focusColor[1]:bright?255:150,focus?focusColor[2]:bright?255:160,pixels,bold);
    }
    void drawColored(const std::string &value,float x,float y,uint8_t red,uint8_t green,uint8_t blue,
                     int pixels,bool bold,float scale=1,bool drawShadow=true,float shadowX=1) {
        const auto &l=label(value,pixels,bold);
        swgb::Quad q{x-2/scale,y-2/scale,l.width/scale,l.height/scale,0,0,float(l.width),float(l.height)};
        auto shadow=q; shadow.x+=shadowX/scale; shadow.y+=1/scale;
        if(drawShadow) renderer_.drawTinted(l.texture,shadow,0,0,0,255);
        renderer_.drawTinted(l.texture,q,red,green,blue,255);
    }
};
}
#endif
