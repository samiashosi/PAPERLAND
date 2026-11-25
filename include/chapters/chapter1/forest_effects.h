// ============================================================
// forest_effects.h - Chapter 1 effects: ripples, fireflies,
//                    falling leaves, waterfall spray
// ============================================================
#ifndef FOREST_EFFECTS_H
#define FOREST_EFFECTS_H

#include "shared/particles.h"

namespace fx
{
    // ---------- forest-wide effects ----------
    void initEffects();                       // reset everything
    void updateEffects(float dt);             // one call per frame
    void drawEffectsBack();                   // ripples (on the water)
    void drawEffectsFront();                  // fireflies, sparkles, leaves

    ParticleSystem& forestParticles();

    void ripple(float x, float y, float maxRadius);
    void waterSparkle();                      // waterfall click
    void setFireflyAmount(float a);           // 0..1 (stage 6 fades them in)
    void setLeafAmount(float a);              // 0..1 ambient falling leaves
    void setWaterfallAmount(float a);         // spray + mist
    void drawFireflies(float alpha);          // used by the story too
}

#endif
