// ============================================================
// characters.h - Flick the fox, Pip the rabbit, the deer,
//                butterflies and birds   (Zone 5 - living forest)
// ============================================================
#ifndef CHARACTERS_H
#define CHARACTERS_H

#include "shared/paper.h"

namespace chara
{
    void initCharacters();
    void updateCharacters(float dt);

    // forest drawing (a = how far the inhabitants have "arrived", 0..1)
    void drawBirds(float a);
    void drawButterflies(float a);
    void drawGroundCharacters(float a);       // fox, deer, rabbit by depth

    bool click(float x, float y);             // true when a creature was hit
    bool hover(float x, float y);

    // Reusable character art (the cinematic reuses these at larger scale).
    //   dir  : +1 faces right, -1 faces left
    //   walk : walk-cycle phase (radians), 0 = standing
    //   jump : height above the ground in px
    //   talk : 0..1 mouth/head animation while speaking
    void drawFlick(float x, float y, float s, int dir, float walk, float jump, float talk);
    void drawPip(float x, float y, float s, int dir, float hop, float jump, float talk);
    void drawDeer(float x, float y, float s, int dir, float walk, float headDown,
                  float lookDeg, float talk);
    void drawButterflyShape(float x, float y, float s, float flap, const pl::Col& wing);
    void drawBirdShape(float x, float y, float s, int dir, float flap);

    // positions used by the cinematic to match the forest
    float flickHomeX();
}

#endif
