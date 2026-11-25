// ============================================================
// environment.h - sky, mountains, hills, trees, river, waterfall,
//                 bridge, pond  (Zone 1 + Zone 2 of the forest)
// Every draw function takes an "amount" 0..1 describing how far
// that layer has unfolded from the page.
// ============================================================
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "paper.h"

namespace env
{
    void initEnvironment();
    void updateEnvironment(float dt);

    // ---- Zone 1: background landscape ----
    void drawSky(float a);
    void drawSun(float a);
    void drawClouds(float a);
    void drawMountains(float a);
    void drawHills(float a);
    void drawDistantTrees(float a);
    void drawGround(float a);
    void drawGrassTufts(float a);

    // ---- trees (stage 2) ----
    int  treeCount();
    void drawTreeIndex(int i, float a);   // mid-ground trees, back to front
    float treeBaseY(int i);
    bool clickTree(float x, float y);     // re-fold / unfold + falling leaves
    bool hoverTree(float x, float y);
    void oakCanopyPoint(float& x, float& y);   // where the ancient leaf hides

    // ---- Zone 2: water (stage 3) ----
    void drawWaterfallCliff(float a);
    void drawWaterfall(float a, float boost);
    void drawRiver(float a);
    void drawBridge(float a);
    void drawPond(float a);
    void drawPathStones(float a);         // stage 4 (village paths)
    void sparkleWaterfall();              // click response (boost timer)
    float waterfallBoost();
    bool hitWaterfall(float x, float y);
    bool hitPond(float x, float y);

    // generic helpers other modules reuse
    void paperPine(float x, float baseY, float s, float lightness);
    void paperOak(float x, float baseY, float s, float lightness);
}

#endif
