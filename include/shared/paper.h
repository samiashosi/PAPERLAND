// ============================================================
// paper.h  -  shared "paper-cut" drawing toolkit for PAPERLAND
//
// Every module (environment, characters, objects, effects, story)
// draws through these helpers so the whole book has one look:
//   * flat colored paper shapes
//   * a soft offset shadow under each layer (layered-paper depth)
//   * a thin darker edge (cut-paper outline)
// ============================================================
#ifndef PAPER_H
#define PAPER_H

#include <string>
#include <vector>

namespace pl
{
    const float PI = 3.14159265f;

    // ---------- color ----------
    struct Col
    {
        float r, g, b, a;
        Col() : r(0), g(0), b(0), a(1) {}
        Col(float R, float G, float B, float A = 1.0f)
            : r(R), g(G), b(B), a(A) {}
    };

    Col mix(const Col& a, const Col& b, float t);
    Col shade(const Col& c, float f);          // multiply rgb
    Col withA(const Col& c, float a);          // replace alpha

    // ---------- math / easing ----------
    float clamp01(float t);
    float clampf(float v, float lo, float hi);
    float lerp(float a, float b, float t);
    float smooth(float t);                     // smoothstep
    float easeOutCubic(float t);
    float easeInOutCubic(float t);
    float easeOutBack(float t);                // springy overshoot
    float stage(float p, float start, float end);   // smooth window
    float window(float p, float start, float end);  // linear window
    // staggered progress for item i of n inside one stage
    float stagger(float p, int i, int n, float spread = 0.55f);

    // ---------- deterministic random ----------
    void  seedRandom(unsigned s);
    float rnd();                               // 0..1
    float rnd(float a, float b);
    int   rndi(int n);                         // 0..n-1

    // ---------- time ----------
    void  tick();                              // call once per frame
    float dt();                                // seconds this frame
    float now();                               // scene clock (stops on pause)
    void  setPaused(bool p);
    bool  paused();
    void  setFixedDt(float d);                 // test hook (<=0 disables)

    // ---------- window / mouse (logical 1000 x 650 space) ----------
    void  setWindowSize(int w, int h);
    int   winW();
    int   winH();
    void  setMouse(float x, float y);
    float mouseX();
    float mouseY();
    void  scissor(float x1, float y1, float x2, float y2);
    void  scissorOff();

    // ---------- paper style ----------
    void  paperShadow(float dx, float dy, float alpha);
    void  noShadow();
    void  paperEdge(bool on);

    // ---------- shapes ----------
    void rect(float x1, float y1, float x2, float y2, const Col& c);
    void rectGrad(float x1, float y1, float x2, float y2,
                  const Col& bottom, const Col& top);
    void rectGradH(float x1, float y1, float x2, float y2,
                   const Col& left, const Col& right);
    void roundRect(float x1, float y1, float x2, float y2, float rad,
                   const Col& c);
    void tri(float x1, float y1, float x2, float y2, float x3, float y3,
             const Col& c);
    void circle(float x, float y, float r, const Col& c);
    void ellipse(float x, float y, float rx, float ry, const Col& c);
    // half-ellipse dome standing on y (upper half)
    void dome(float x, float y, float rx, float ry, const Col& c);
    void poly(const float* xy, int n, const Col& c);   // star-shaped polygon
    void ring(float x, float y, float r1, float r2, const Col& c);
    void line(float x1, float y1, float x2, float y2, float w, const Col& c);
    void ribbon(const float* xy, int n, float w, const Col& c);  // thick polyline
    void ribbonTaper(const float* xy, int n, float w0, float w1, const Col& c);
    void star(float x, float y, float r1, float r2, int points, float rot,
              const Col& c);
    void leafShape(float x, float y, float len, float wid, float angleDeg,
                   const Col& c);
    void glow(float x, float y, float r, const Col& c, bool additive = true);
    void sparkle4(float x, float y, float r, float rot, const Col& c);
    void overlay(const Col& c);               // full-window tint

    // ---------- text ----------
    void text(float x, float y, const std::string& s, void* font,
              const Col& c);
    void textCentered(float cx, float y, const std::string& s, void* font,
                      const Col& c);
    int  textWidth(const std::string& s, void* font);
    std::vector<std::string> wrapText(const std::string& s, void* font,
                                      int maxWidth);

    // ---------- UI ----------
    void panel(float x1, float y1, float x2, float y2, const Col& fill,
               const Col& border);
    bool inRect(float px, float py, float x1, float y1, float x2, float y2);
    bool inEllipse(float px, float py, float cx, float cy, float rx, float ry);

    // ---------- pop-up helper ----------
    // Paper that rises from the page: scales up from its base with a little
    // overshoot and a hinge-like tilt.  Returns false if nothing to draw.
    bool beginPopUp(float x, float baseY, float amount, float tiltDeg = 0.0f);
    void endPopUp();
    void creaseShadow(float x, float baseY, float halfWidth, float amount);
}

#endif
