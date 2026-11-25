// ============================================================
// particles.h - shared particle pool used by all chapters
// ============================================================
#ifndef SHARED_PARTICLES_H
#define SHARED_PARTICLES_H

#include <vector>
#include "shared/paper.h"

namespace fx
{
    enum ParticleKind { PK_SPARK, PK_DOT, PK_LEAF, PK_DROP, PK_MIST, PK_COIN };

    struct Particle
    {
        float x, y, vx, vy;
        float gravity;      // downward acceleration (px/s^2)
        float life, maxLife;
        float size, rot, vr, phase;
        pl::Col c;
        int   kind;
        float groundY;      // leaves stop falling here (<0 = never)
    };

    // A self-contained particle pool.  The forest owns one, the cinematic
    // story owns another so they never interfere.
    class ParticleSystem
    {
    public:
        std::vector<Particle> p;

        void clear();
        void update(float dt);
        void draw() const;

        void add(const Particle& q);
        void sparkles(float x, float y, int n, const pl::Col& c,
                      float speed = 90.0f, float size = 6.0f);
        void motes(float x, float y, int n, const pl::Col& c, float spread);
        void leaf(float x, float y, float groundY);
        void drops(float x, float y, int n);
        void mist(float x, float y, int n);
        void coins(float x, float y, int n);
        size_t count() const { return p.size(); }
    };
}

#endif
