// SPDX-License-Identifier: GPL-3.0-or-later
// Minimal 2D renderer interface. The engine only ever draws textured,
// alpha-blended quads in screen space, which maps well to both vitaGL and a
// CPU rasterizer (used for headless tests on PC).
#pragma once

#include <cstdint>
#include <string>

namespace swgb {

struct Texture {
    int width = 0, height = 0;
    bool alphaOnly = false;
    virtual ~Texture() = default;
};

struct Quad {
    float x, y, w, h;      // destination rectangle in screen pixels
    float u0, v0, u1, v1;  // source rectangle in texels (u1 < u0 means flipped)
};

class Renderer {
public:
    virtual ~Renderer() = default;

    // rgba: width*height*4 bytes, tightly packed, straight alpha.
    virtual Texture *createTexture(int width, int height, const uint8_t *rgba) = 0;
    // alpha: width*height bytes. Sampling yields white RGB and the stored alpha.
    virtual Texture *createMaskTexture(int width, int height, const uint8_t *alpha) = 0;
    virtual void destroyTexture(Texture *t) = 0;

    virtual void beginFrame(int screenW, int screenH, float scale, uint8_t r, uint8_t g, uint8_t b) = 0;
    virtual void draw(Texture *tex, const Quad &q) = 0;
    virtual void drawMasked(Texture *tex, const Quad &q, Texture *mask, const Quad &maskQ) = 0;
    virtual void drawMaskedTinted(Texture *tex, const Quad &q, Texture *mask,
                                  const Quad &maskQ, uint8_t r, uint8_t g,
                                  uint8_t b, uint8_t a) = 0;
    // Texture modulated by a colour (single texture unit: cheap on vitaGL,
    // used for UI text).
    virtual void drawTinted(Texture *tex, const Quad &q, uint8_t r, uint8_t g, uint8_t b, uint8_t a) = 0;
    // Original fog alternates untouched and palette-zero destination pixels.
    // shortcut: GPU backends use average opacity until their shader implements screen-pixel parity.
    virtual void drawFog(Texture *mask,const Quad &q,uint8_t r,uint8_t g,uint8_t b) {drawTinted(mask,q,r,g,b,128);}
    // Solid line segment of the given thickness (one rotated quad).
    virtual void drawLine(float x0, float y0, float x1, float y1, float thickness,
                          uint8_t r, uint8_t g, uint8_t b, uint8_t a) = 0;
    virtual void fillRect(float x, float y, float w, float h, uint8_t r, uint8_t g, uint8_t b, uint8_t a) = 0;
    // Optional platform typography; negative width selects the baked-font fallback.
    virtual float measureInterfaceText(const std::string &, int) { return -1; }
    virtual void drawInterfaceText(const std::string &, float, float, int, uint8_t, uint8_t, uint8_t) {}
    virtual void endFrame() = 0;
};

} // namespace swgb
