// ============================================================
// forest_environment.cpp - Chapter 1 Zone 1: sky, hills, ground (init + update)
// ============================================================
#include <GL/freeglut.h>
#include <cmath>
#include <vector>

#include "chapters/chapter1/forest_environment.h"
#include "chapters/chapter1/forest_environment_internal.h"
#include "chapters/chapter1/forest_effects.h"
#include "chapters/chapter1/forest_layout.h"

using namespace pl;

namespace env
{
// ------------------------------------------------------------
// palette
// ------------------------------------------------------------
const Col HAZE(0.88f, 0.94f, 0.88f, 1.0f);

Col hazeMix(const Col& c, float lightness)     // 0 = full color, 1 = hazy
{
    return mix(c, HAZE, clamp01(lightness));
}

// ------------------------------------------------------------
// state
// ------------------------------------------------------------
std::vector<TreeDef> g_trees;
float g_wfBoost = 0.0f;

struct Tuft { float x, y, s; };
static std::vector<Tuft> g_tufts;

void initEnvironment()
{
    g_trees.clear();
    // back to front (higher baseY = further away)
    TreeDef t[] = {
        { 585, 292, 0.78f, 0, 1.0f, false },    // pine right of river (far)
        { 400, 290, 0.80f, 1, 1.0f, false },    // oak by the bridge (far)
        {  92, 282, 0.95f, 0, 1.0f, false },    // pine left edge
        { 662, 270, 1.00f, 1, 1.0f, false },    // oak hiding the ancient leaf
    };
    for (size_t i = 0; i < sizeof(t) / sizeof(t[0]); i++) g_trees.push_back(t[i]);

    g_tufts.clear();
    seedRandom(77);
    for (int i = 0; i < 70; i++)
    {
        Tuft u;
        u.x = rnd(layout::L + 10, layout::R - 10);
        u.y = rnd(layout::B + 12, layout::HORIZON - 14);
        u.s = 0.6f + (1.0f - (u.y - layout::B) / (layout::HORIZON - layout::B)) * 0.8f;
        g_tufts.push_back(u);
    }
    g_wfBoost = 0.0f;
}

void updateEnvironment(float dt)
{
    if (g_wfBoost > 0.0f) g_wfBoost = g_wfBoost - dt < 0.0f ? 0.0f : g_wfBoost - dt;
    for (size_t i = 0; i < g_trees.size(); i++)
    {
        TreeDef& t = g_trees[i];
        if (t.folding)
        {
            t.fold += dt / 1.7f;
            if (t.fold >= 1.0f) { t.fold = 1.0f; t.folding = false; }
        }
    }
}

// ------------------------------------------------------------
// helpers
// ------------------------------------------------------------
// Terrain: region between a flat baseline and a wavy crest
static void terrain(const std::vector<float>& crest, float x0, float step,
                    float baseY, const Col& cBottom, const Col& cTop,
                    float shadowA)
{
    int n = (int)crest.size();
    if (shadowA > 0.0f)
    {
        glColor4f(0.08f, 0.05f, 0.02f, shadowA);
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i < n; i++)
        {
            glVertex2f(x0 + i * step + 2.5f, crest[i] - 3.0f);
            glVertex2f(x0 + i * step + 2.5f, baseY);
        }
        glEnd();
    }
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i < n; i++)
    {
        glColor4f(cTop.r, cTop.g, cTop.b, cTop.a);
        glVertex2f(x0 + i * step, crest[i]);
        glColor4f(cBottom.r, cBottom.g, cBottom.b, cBottom.a);
        glVertex2f(x0 + i * step, baseY);
    }
    glEnd();
}

static std::vector<float> makeCrest(float base, float a1, float f1, float p1,
                                    float a2, float f2, float p2)
{
    std::vector<float> c;
    for (float x = layout::L; x <= layout::R + 0.1f; x += 8.0f)
        c.push_back(base + a1 * std::sin(x * f1 + p1) + a2 * std::sin(x * f2 + p2));
    return c;
}

// rise a layer from its baseline
static void riseBegin(float baseY, float a)
{
    glPushMatrix();
    glTranslatef(0, baseY, 0);
    glScalef(1.0f, easeOutBack(clamp01(a)), 1.0f);
    glTranslatef(0, -baseY, 0);
}
static void riseEnd() { glPopMatrix(); }

// ------------------------------------------------------------
// ZONE 1 - sky, sun, clouds
// ------------------------------------------------------------
void drawSky(float a)
{
    if (a <= 0.0f) return;
    float top = layout::HORIZON - 40.0f;
    float h = (layout::T - top) * smooth(a);
    // paper shadow under the backdrop panel
    rect(layout::L + 3, top - 3, layout::R + 3, top + h - 3, Col(0.1f, 0.06f, 0.02f, 0.25f * a));
    rectGrad(layout::L, top, layout::R, top + h, Col(1.0f, 0.97f, 0.80f), Col(0.55f, 0.80f, 0.95f));
}

void drawSun(float a)
{
    if (a <= 0.0f) return;
    float t = now();
    float cx = 178, cy = 518;
    float s = easeOutBack(a);
    glow(cx, cy, 120.0f * s, Col(1.0f, 0.90f, 0.45f, 0.55f * a));
    glow(cx, cy, 70.0f * s, Col(1.0f, 0.95f, 0.6f, 0.6f * a));
    // rotating paper rays
    for (int i = 0; i < 14; i++)
    {
        float an = t * 0.25f + i * 2.0f * PI / 14.0f;
        float r1 = 38.0f * s, r2 = (60.0f + 6.0f * std::sin(t * 2.0f + i)) * s;
        tri(cx + std::cos(an - 0.09f) * r1, cy + std::sin(an - 0.09f) * r1,
            cx + std::cos(an + 0.09f) * r1, cy + std::sin(an + 0.09f) * r1,
            cx + std::cos(an) * r2, cy + std::sin(an) * r2,
            Col(1.0f, 0.82f, 0.30f, 0.9f * a));
    }
    paperShadow(2, -3, 0.18f);
    circle(cx, cy, 34.0f * s, Col(1.0f, 0.80f, 0.28f));
    noShadow();
    circle(cx, cy, 26.0f * s, Col(1.0f, 0.90f, 0.48f));
}

void drawClouds(float a)
{
    if (a <= 0.0f) return;
    float t = now();
    struct C { float y, s, speed, off; };
    static const C cl[] = {
        { 505, 1.00f, 9.0f, 0.00f }, { 548, 0.75f, 6.0f, 0.35f },
        { 470, 0.85f, 7.5f, 0.62f }, { 535, 1.10f, 5.0f, 0.86f } };
    float span = (layout::R - layout::L) + 220.0f;
    for (int i = 0; i < 4; i++)
    {
        float x = layout::L - 110.0f + std::fmod(t * cl[i].speed + cl[i].off * span, span);
        float s = cl[i].s * (0.6f + 0.4f * easeOutCubic(a));
        float y = cl[i].y;
        paperShadow(3, -4, 0.14f);
        Col w(1.0f, 1.0f, 1.0f, 0.96f * a);
        ellipse(x, y, 44 * s, 15 * s, w);
        circle(x - 22 * s, y + 10 * s, 20 * s, w);
        circle(x + 8 * s, y + 18 * s, 26 * s, w);
        circle(x + 32 * s, y + 8 * s, 18 * s, w);
        noShadow();
        ellipse(x, y - 6 * s, 38 * s, 7 * s, Col(0.85f, 0.92f, 0.98f, 0.8f * a));
    }
}

// ------------------------------------------------------------
// ZONE 1 - mountains, hills, distant trees
// ------------------------------------------------------------
void drawMountains(float a)
{
    if (a <= 0.0f) return;
    struct M { float x, w, h; };
    static const M far_[] = { { 190, 340, 190 }, { 450, 400, 232 }, { 710, 350, 205 }, { 900, 270, 150 } };
    static const M mid_[] = { { 110, 260, 135 }, { 330, 310, 168 }, { 590, 300, 138 }, { 810, 340, 172 } };
    float base = layout::HORIZON - 4.0f;

    for (int i = 0; i < 4; i++)
    {
        float k = stagger(window(a, 0.0f, 0.7f), i, 4, 0.6f);
        if (k <= 0.0f) continue;
        float h = far_[i].h * easeOutBack(k);
        paperShadow(4, -3, 0.18f);
        tri(far_[i].x - far_[i].w / 2, base, far_[i].x + far_[i].w / 2, base,
            far_[i].x, base + h, Col(0.66f, 0.78f, 0.88f));
        noShadow();
        // snowy paper cap + facet
        float cap = 0.22f;
        tri(far_[i].x - far_[i].w * cap / 2, base + h * (1 - cap), far_[i].x + far_[i].w * cap / 2,
            base + h * (1 - cap), far_[i].x, base + h, Col(0.96f, 0.98f, 1.0f));
        tri(far_[i].x, base, far_[i].x + far_[i].w / 2, base, far_[i].x, base + h,
            Col(0.58f, 0.70f, 0.82f, 0.55f));
    }
    for (int i = 0; i < 4; i++)
    {
        float k = stagger(window(a, 0.25f, 1.0f), i, 4, 0.6f);
        if (k <= 0.0f) continue;
        float h = mid_[i].h * easeOutBack(k);
        paperShadow(4, -3, 0.2f);
        tri(mid_[i].x - mid_[i].w / 2, base - 6, mid_[i].x + mid_[i].w / 2, base - 6,
            mid_[i].x, base - 6 + h, Col(0.50f, 0.72f, 0.60f));
        noShadow();
        tri(mid_[i].x, base - 6, mid_[i].x + mid_[i].w / 2, base - 6, mid_[i].x, base - 6 + h,
            Col(0.40f, 0.62f, 0.52f, 0.55f));
    }
}

void drawHills(float a)
{
    if (a <= 0.0f) return;
    float k1 = stage(a, 0.0f, 0.8f), k2 = stage(a, 0.25f, 1.0f);
    static std::vector<float> c1 = makeCrest(layout::HORIZON + 18, 14, 0.011f, 0.6f, 7, 0.027f, 2.0f);
    static std::vector<float> c2 = makeCrest(layout::HORIZON - 2, 11, 0.014f, 2.4f, 6, 0.033f, 0.3f);
    if (k1 > 0.0f)
    {
        riseBegin(layout::HORIZON - 30, k1);
        terrain(c1, layout::L, 8.0f, layout::HORIZON - 30, Col(0.58f, 0.80f, 0.50f), Col(0.70f, 0.87f, 0.55f), 0.22f);
        riseEnd();
    }
    if (k2 > 0.0f)
    {
        riseBegin(layout::HORIZON - 40, k2);
        terrain(c2, layout::L, 8.0f, layout::HORIZON - 40, Col(0.50f, 0.75f, 0.42f), Col(0.60f, 0.82f, 0.46f), 0.25f);
        riseEnd();
    }
}

void paperPine(float x, float baseY, float s, float lightness)
{
    Col trunk = hazeMix(Col(0.42f, 0.26f, 0.14f), lightness);
    Col g1 = hazeMix(Col(0.18f, 0.48f, 0.32f), lightness);
    Col g2 = hazeMix(Col(0.26f, 0.60f, 0.38f), lightness);
    Col g3 = hazeMix(Col(0.38f, 0.72f, 0.44f), lightness);
    paperShadow(3 * s, -3 * s, 0.22f);
    rect(x - 7 * s, baseY, x + 7 * s, baseY + 36 * s, trunk);
    tri(x - 46 * s, baseY + 24 * s, x + 46 * s, baseY + 24 * s, x, baseY + 98 * s, g1);
    tri(x - 38 * s, baseY + 62 * s, x + 38 * s, baseY + 62 * s, x, baseY + 130 * s, g2);
    tri(x - 28 * s, baseY + 98 * s, x + 28 * s, baseY + 98 * s, x, baseY + 160 * s, g3);
    noShadow();
    // lighter facet on the right half (paper fold)
    tri(x, baseY + 24 * s, x + 46 * s, baseY + 24 * s, x, baseY + 98 * s, Col(1, 1, 1, 0.10f));
    tri(x, baseY + 62 * s, x + 38 * s, baseY + 62 * s, x, baseY + 130 * s, Col(1, 1, 1, 0.10f));
}

void paperOak(float x, float baseY, float s, float lightness)
{
    Col trunk = hazeMix(Col(0.45f, 0.28f, 0.15f), lightness);
    Col g1 = hazeMix(Col(0.30f, 0.62f, 0.34f), lightness);
    Col g2 = hazeMix(Col(0.42f, 0.74f, 0.38f), lightness);
    Col g3 = hazeMix(Col(0.58f, 0.82f, 0.42f), lightness);
    paperShadow(3 * s, -3 * s, 0.22f);
    // trunk with a small branch
    float tp[] = { x - 9 * s, baseY, x + 9 * s, baseY, x + 6 * s, baseY + 66 * s, x - 6 * s, baseY + 66 * s };
    poly(tp, 4, trunk);
    float br[] = { x, baseY + 40 * s, x + 28 * s, baseY + 62 * s };
    ribbon(br, 2, 6 * s, trunk);
    circle(x - 30 * s, baseY + 82 * s, 34 * s, g1);
    circle(x + 30 * s, baseY + 82 * s, 34 * s, g1);
    circle(x, baseY + 104 * s, 44 * s, g2);
    noShadow();
    circle(x - 10 * s, baseY + 116 * s, 28 * s, g3);
    circle(x + 24 * s, baseY + 104 * s, 18 * s, g3);
}

void drawDistantTrees(float a)
{
    if (a <= 0.0f) return;
    static const float xs[] = { 90, 125, 160, 215, 300, 345, 380, 430, 470, 545, 575, 640, 690, 730, 770, 880, 915 };
    int n = (int)(sizeof(xs) / sizeof(xs[0]));
    for (int i = 0; i < n; i++)
    {
        float k = stagger(a, i, n, 0.7f);
        if (k <= 0.0f) continue;
        float s = 0.30f + 0.10f * std::sin(i * 1.7f);
        float y = layout::HORIZON + 3 + 10 * std::sin(xs[i] * 0.02f + 1.2f);
        glPushMatrix();
        glTranslatef(xs[i], y, 0);
        glScalef(1, easeOutBack(k), 1);
        paperPine(0, 0, s, 0.45f);
        glPopMatrix();
    }
}

// ------------------------------------------------------------
// ground
// ------------------------------------------------------------
void drawGround(float a)
{
    if (a <= 0.0f) return;
    static std::vector<float> c = makeCrest(layout::HORIZON - 12, 6, 0.010f, 1.0f, 3, 0.031f, 4.0f);
    float k = stage(a, 0.0f, 1.0f);
    riseBegin(layout::B, 0.35f + 0.65f * k);
    terrain(c, layout::L, 8.0f, layout::B, Col(0.34f, 0.64f, 0.34f), Col(0.58f, 0.80f, 0.42f), 0.25f);
    riseEnd();
    // front paper edge of the page-ground
    rectGrad(layout::L, layout::B, layout::R, layout::B + 9, Col(0.20f, 0.42f, 0.24f), Col(0.28f, 0.54f, 0.30f));
}

void drawGrassTufts(float a)
{
    if (a <= 0.0f) return;
    float t = now();
    for (size_t i = 0; i < g_tufts.size(); i++)
    {
        float k = stagger(a, (int)i, (int)g_tufts.size(), 0.7f);
        if (k <= 0.0f) continue;
        const Tuft& u = g_tufts[i];
        float sway = std::sin(t * 1.6f + u.x * 0.05f) * 2.0f * u.s;
        Col c(0.22f, 0.50f, 0.26f, 0.9f);
        float h = 11.0f * u.s * k;
        for (int b = -1; b <= 1; b++)
        {
            tri(u.x + b * 3.0f * u.s - 1.6f * u.s, u.y,
                u.x + b * 3.0f * u.s + 1.6f * u.s, u.y,
                u.x + b * 5.0f * u.s + sway, u.y + h, c);
        }
    }
}
} // namespace env
