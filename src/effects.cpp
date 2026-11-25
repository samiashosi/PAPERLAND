// ============================================================
// effects.cpp  -  fireflies, sparkles, ripples, falling leaves
// ============================================================
#include <GL/freeglut.h>
#include <cmath>

#include "../include/effects.h"
#include "../include/layout.h"

using namespace pl;

namespace fx
{
// ------------------------------------------------------------
// ParticleSystem
// ------------------------------------------------------------
void ParticleSystem::clear() { p.clear(); }

void ParticleSystem::add(const Particle& q)
{
    if (p.size() > 1400) p.erase(p.begin(), p.begin() + 100);
    p.push_back(q);
}

static Particle base(float x, float y, int kind)
{
    Particle q;
    q.x = x; q.y = y; q.vx = 0; q.vy = 0; q.gravity = 0;
    q.life = 0; q.maxLife = 1; q.size = 4; q.rot = 0; q.vr = 0;
    q.phase = rnd(0, 6.28f); q.c = Col(1, 1, 1, 1); q.kind = kind;
    q.groundY = -1.0f;
    return q;
}

void ParticleSystem::sparkles(float x, float y, int n, const Col& c,
                              float speed, float size)
{
    for (int i = 0; i < n; i++)
    {
        Particle q = base(x, y, PK_SPARK);
        float a = rnd(0, 2 * PI), s = rnd(0.3f, 1.0f) * speed;
        q.vx = std::cos(a) * s; q.vy = std::sin(a) * s;
        q.gravity = -speed * 0.25f;
        q.maxLife = rnd(0.7f, 1.4f);
        q.size = rnd(0.6f, 1.0f) * size;
        q.rot = rnd(0, PI); q.vr = rnd(-3, 3);
        q.c = mix(c, Col(1, 1, 1, 1), rnd(0, 0.5f));
        add(q);
    }
}

void ParticleSystem::motes(float x, float y, int n, const Col& c, float spread)
{
    for (int i = 0; i < n; i++)
    {
        Particle q = base(x + rnd(-spread, spread), y + rnd(-spread * 0.4f, spread * 0.4f), PK_DOT);
        q.vx = rnd(-8, 8); q.vy = rnd(10, 40);
        q.maxLife = rnd(1.5f, 3.0f);
        q.size = rnd(2.0f, 5.0f);
        q.c = c;
        add(q);
    }
}

void ParticleSystem::leaf(float x, float y, float groundY)
{
    Particle q = base(x, y, PK_LEAF);
    static const Col cols[4] = {
        Col(0.55f, 0.78f, 0.32f), Col(0.95f, 0.78f, 0.25f),
        Col(0.93f, 0.55f, 0.22f), Col(0.42f, 0.70f, 0.38f) };
    q.c = cols[rndi(4)];
    q.vy = -rnd(18, 34);
    q.gravity = 0;
    q.size = rnd(7, 11);
    q.maxLife = 14.0f;
    q.vr = rnd(-70, 70);
    q.rot = rnd(0, 360);
    q.groundY = groundY;
    add(q);
}

void ParticleSystem::drops(float x, float y, int n)
{
    for (int i = 0; i < n; i++)
    {
        Particle q = base(x + rnd(-14, 14), y, PK_DROP);
        q.vx = rnd(-40, 40); q.vy = rnd(30, 120);
        q.gravity = -380.0f;
        q.maxLife = rnd(0.5f, 0.9f);
        q.size = rnd(1.6f, 3.0f);
        q.c = Col(0.85f, 0.95f, 1.0f, 0.9f);
        add(q);
    }
}

void ParticleSystem::mist(float x, float y, int n)
{
    for (int i = 0; i < n; i++)
    {
        Particle q = base(x + rnd(-22, 22), y + rnd(-4, 6), PK_MIST);
        q.vx = rnd(-10, 10); q.vy = rnd(8, 22);
        q.maxLife = rnd(1.4f, 2.4f);
        q.size = rnd(10, 18);
        q.c = Col(0.95f, 0.98f, 1.0f, 0.30f);
        add(q);
    }
}

void ParticleSystem::coins(float x, float y, int n)
{
    for (int i = 0; i < n; i++)
    {
        Particle q = base(x + rnd(-10, 10), y, PK_COIN);
        q.vx = rnd(-70, 70); q.vy = rnd(120, 230);
        q.gravity = -420.0f;
        q.maxLife = rnd(1.0f, 1.5f);
        q.size = rnd(4.5f, 6.5f);
        q.vr = rnd(8, 16);
        q.c = Col(1.0f, 0.82f, 0.25f, 1);
        add(q);
    }
}

void ParticleSystem::update(float dt)
{
    for (size_t i = 0; i < p.size();)
    {
        Particle& q = p[i];
        q.life += dt;
        bool dead = q.life >= q.maxLife;

        if (q.kind == PK_LEAF)
        {
            // gentle swaying fall
            q.vx = std::sin(q.phase + q.life * 2.3f) * 26.0f;
            q.x += q.vx * dt;
            if (q.y > q.groundY)
            {
                q.y += q.vy * dt;
                q.rot += q.vr * dt;
            }
            else
            {
                q.y = q.groundY;                 // rest on the page
                q.vr = 0;
                if (q.life < q.maxLife - 3.0f) q.maxLife = q.life + 3.0f;
            }
        }
        else
        {
            q.vy += q.gravity * dt;
            q.x += q.vx * dt;
            q.y += q.vy * dt;
            q.rot += q.vr * dt;
            if (q.kind == PK_MIST) q.size += 9.0f * dt;
        }

        if (dead) { p[i] = p.back(); p.pop_back(); }
        else i++;
    }
}

void ParticleSystem::draw() const
{
    for (size_t i = 0; i < p.size(); i++)
    {
        const Particle& q = p[i];
        float t = q.life / q.maxLife;
        float fade = 1.0f - t;
        switch (q.kind)
        {
        case PK_SPARK:
            sparkle4(q.x, q.y, q.size * (0.5f + fade), q.rot, withA(q.c, fade));
            break;
        case PK_DOT:
        {
            float a = std::sin(t * PI);
            glow(q.x, q.y, q.size * 3.0f, withA(q.c, 0.7f * a));
            break;
        }
        case PK_LEAF:
        {
            float a = (t > 0.8f) ? (1.0f - t) / 0.2f : 1.0f;
            paperShadow(1.5f, -1.5f, 0.2f);
            leafShape(q.x, q.y, q.size * 1.3f, q.size * 0.8f, q.rot, withA(q.c, a));
            noShadow();
            break;
        }
        case PK_DROP:
            circle(q.x, q.y, q.size, withA(q.c, fade));
            break;
        case PK_MIST:
            glow(q.x, q.y, q.size, withA(q.c, 0.30f * fade), false);
            break;
        case PK_COIN:
        {
            float sx = std::fabs(std::cos(q.rot));
            ellipse(q.x, q.y, q.size * (0.25f + 0.75f * sx), q.size, withA(q.c, fade > 0.2f ? 1.0f : fade * 5));
            break;
        }
        }
    }
}

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
