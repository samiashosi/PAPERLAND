// ============================================================
// objects.h - mushroom houses, Ancient Tree, treasure chest,
//             flowers, mushrooms, signs, discovery icons
// (Zone 3 village + Zone 4 magical heart)
// ============================================================
#ifndef OBJECTS_H
#define OBJECTS_H

#include "paper.h"

namespace obj
{
    enum Hit { H_NONE, H_HOUSE, H_ANCIENT_TREE, H_CHEST, H_FLOWER, H_MUSHROOM };

    void initObjects();
    void updateObjects(float dt);

    // ---- scene drawing, each takes its unfold amount 0..1 ----
    int   houseCount();
    float houseBaseY(int i);
    void  drawHouse(int i, float a, float windowGlow);
    void  drawSign(float a);
    void  drawAncientTree(float a, float magic);
    void  drawChest(float a);
    void  drawFlowers(float a);            // decorative flowers (village/fore)
    void  drawMagicalFlowers(float a);     // around the Ancient Tree
    void  drawMushrooms(float a);
    void  setChestVisible(float a);

    // ---- interaction ----
    Hit   click(float x, float y);         // returns what was clicked
    bool  hover(float x, float y);
    float treeGlow();                      // current glow of the Ancient Tree
    bool  chestOpen();

    // ---- reusable drawing (story + UI use these too) ----
    void drawMushroomHouseAt(float x, float y, float s, const pl::Col& cap,
                             float doorOpen, float windowGlow);
    void drawAncientTreeAt(float x, float y, float s, float glow, float sway);
    void drawChestAt(float x, float y, float s, float open, float glow);
    void drawFlowerAt(float x, float y, float s, float bloom,
                      const pl::Col& petal, bool magical);
    // discovery icons: 0 crystal, 1 leaf, 2 feather, 3 key, 4 star fragment
    void drawDiscoveryIcon(int kind, float x, float y, float s, float glowAmt);
    const char* discoveryName(int kind);
    void setTreeDim(float d);              // 0 = bright, 1 = dark (cinematic)
}

#endif
