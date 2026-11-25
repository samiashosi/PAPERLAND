// ============================================================
// forest_effects.cpp - Chapter 1 effects: ripples, fireflies,
//                      falling leaves, waterfall spray
// ============================================================
#include <GL/freeglut.h>
#include <cmath>

#include "chapters/chapter1/forest_effects.h"
#include "chapters/chapter1/forest_layout.h"

using namespace pl;

namespace fx
{
// ------------------------------------------------------------
// forest-wide state
// ------------------------------------------------------------
static ParticleSystem g_forest;
ParticleSystem& forestParticles() { return g_forest; }

struct Ripple { float x, y, age, maxR, speed; };
static std::vector<Ripple> g_ripples;

struct Firefly { float ax, ay, phase, speed, rx, ry, blink; };
static std::vector<Firefly> g_fireflies;
static float g_fireflyAmount = 0.0f;
static float g_leafAmount = 0.0f;
static float g_waterAmount = 0.0f;
static float g_leafTimer = 0.0f, g_rippleTimer = 0.0f, g_sprayTimer = 0.0f;

void initEffects()
{
    g_forest.clear();
    g_ripples.clear();
    g_fireflies.clear();
    g_fireflyAmount = g_leafAmount = g_waterAmount = 0.0f;
    g_leafTimer = g_rippleTimer = g_sprayTimer = 0.0f;

    // fireflies gather around the Ancient Tree, the pond and the village
    for (int i = 0; i < 26; i++)
    {
        Firefly f;
        if (i < 12)      { f.ax = rnd(690, 920); f.ay = rnd(200, 470); }
        else if (i < 19) { f.ax = rnd(110, 380); f.ay = rnd(200, 340); }
        else             { f.ax = rnd(380, 640); f.ay = rnd(120, 230); }
        f.phase = rnd(0, 6.28f);
        f.speed = rnd(0.5f, 1.2f);
        f.rx = rnd(14, 42); f.ry = rnd(8, 26);
        f.blink = rnd(1.2f, 2.6f);
        g_fireflies.push_back(f);
    }
}

void ripple(float x, float y, float maxRadius)
{
    Ripple r; r.x = x; r.y = y; r.age = 0; r.maxR = maxRadius; r.speed = 1.0f;
    g_ripples.push_back(r);
}

void waterSparkle()
{
    for (int i = 0; i < 18; i++)
    {
        float y = rnd(layout::WF_BASE, layout::WF_TOP);
        g_forest.sparkles(layout::WF_X + rnd(-20, 20), y, 1,
                          Col(0.7f, 0.92f, 1.0f), 40.0f, 9.0f);
    }
    g_forest.drops(layout::WF_X, layout::WF_BASE + 4, 14);
}

void setFireflyAmount(float a) { g_fireflyAmount = clamp01(a); }
void setLeafAmount(float a) { g_leafAmount = clamp01(a); }
void setWaterfallAmount(float a) { g_waterAmount = clamp01(a); }

void updateEffects(float dt)
{
    // ambient falling leaves from the big canopies
    if (g_leafAmount > 0.05f)
    {
        g_leafTimer -= dt;
        if (g_leafTimer <= 0.0f)
        {
            g_leafTimer = rnd(0.9f, 1.8f) / g_leafAmount;
            float x, y;
            if (rnd() < 0.55f) { x = rnd(700, 900); y = rnd(390, 460); }
            else               { x = rnd(110, 690); y = rnd(330, 400); }
            g_forest.leaf(x, y, rnd(120, 240));
        }
    }

    // waterfall spray + mist
    if (g_waterAmount > 0.1f)
    {
        g_sprayTimer -= dt;
        if (g_sprayTimer <= 0.0f)
        {
            g_sprayTimer = 0.06f;
            g_forest.drops(layout::WF_X, layout::WF_BASE + 2, 1);
            if (rnd() < 0.35f) g_forest.mist(layout::WF_X, layout::WF_BASE + 4, 1);
        }
        // little raindrop ripples in the pond
        g_rippleTimer -= dt;
        if (g_rippleTimer <= 0.0f)
        {
            g_rippleTimer = rnd(1.2f, 2.6f);
            ripple(layout::POND_X + rnd(-70, 70), layout::POND_Y + rnd(-18, 18), rnd(16, 30));
        }
    }

    g_forest.update(dt);

    for (size_t i = 0; i < g_ripples.size();)
    {
        g_ripples[i].age += dt * 0.8f;
        if (g_ripples[i].age >= 1.0f) { g_ripples.erase(g_ripples.begin() + i); }
        else i++;
    }
}

void drawEffectsBack()
{
    for (size_t i = 0; i < g_ripples.size(); i++)
    {
        const Ripple& r = g_ripples[i];
        for (int k = 0; k < 3; k++)
        {
            float t = r.age - k * 0.14f;
            if (t <= 0.0f || t >= 1.0f) continue;
            float rad = r.maxR * easeOutCubic(t);
            float a = (1.0f - t) * 0.75f;
            // elliptical ring (perspective squash)
            glColor4f(1.0f, 1.0f, 1.0f, a);
            glLineWidth(2.0f);
            glBegin(GL_LINE_LOOP);
            for (int s = 0; s < 36; s++)
            {
                float an = 2 * PI * s / 36;
                glVertex2f(r.x + std::cos(an) * rad, r.y + std::sin(an) * rad * 0.34f);
            }
            glEnd();
        }
    }
}

void drawFireflies(float alpha)
{
    if (alpha <= 0.01f) return;
    float t = now();
    for (size_t i = 0; i < g_fireflies.size(); i++)
    {
        const Firefly& f = g_fireflies[i];
        // each firefly fades in at its own moment
        float appear = clamp01(alpha * 1.6f - (float)i / g_fireflies.size() * 0.6f);
        if (appear <= 0.0f) continue;
        float x = f.ax + std::sin(t * f.speed + f.phase) * f.rx;
        float y = f.ay + std::cos(t * f.speed * 1.3f + f.phase * 1.7f) * f.ry;
        float b = 0.5f + 0.5f * std::sin(t * f.blink + f.phase * 3.0f);
        b = b * b;
        float a = appear * (0.25f + 0.75f * b);
        glow(x, y, 15.0f, Col(1.0f, 0.95f, 0.45f, 0.75f * a));
        circle(x, y, 2.0f, Col(1.0f, 1.0f, 0.8f, a));
    }
}

void drawEffectsFront()
{
    g_forest.draw();
    drawFireflies(g_fireflyAmount);
}

} // namespace fx
