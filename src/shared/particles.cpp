// ============================================================
// particles.cpp - shared particle pool (sparkles, motes, leaves,
//                 drops, mist, coins). Used by every chapter.
// ============================================================
#include <GL/freeglut.h>
#include <cmath>

#include "shared/particles.h"

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

} // namespace fx
