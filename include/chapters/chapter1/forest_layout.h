// ============================================================
// layout.h  -  where everything stands inside the open book
// (logical window space 1000 x 650, origin bottom-left)
// ============================================================
#ifndef LAYOUT_H
#define LAYOUT_H

namespace layout
{
    // The pop-up stage inside the open book
    const float L = 62.0f,  R = 938.0f;      // left / right edge
    const float B = 100.0f, T = 586.0f;      // bottom / top edge
    const float HORIZON = 292.0f;            // where hills meet the ground

    // Zone 2 - water
    const float WF_X = 500.0f;               // waterfall x
    const float WF_TOP = 372.0f;             // top of the fall
    const float WF_BASE = 298.0f;            // where it meets the river
    const float POND_X = 500.0f, POND_Y = 146.0f;
    const float POND_RX = 112.0f, POND_RY = 40.0f;
    const float BRIDGE_X = 500.0f, BRIDGE_Y = 251.0f;

    // Zone 4 - magical heart
    const float TREE_X = 790.0f, TREE_Y = 236.0f, TREE_S = 0.86f;
    const float CHEST_X = 712.0f, CHEST_Y = 214.0f;

    // Zone 3 - village
    const float HOUSE1_X = 150.0f, HOUSE1_Y = 268.0f;
    const float HOUSE2_X = 245.0f, HOUSE2_Y = 250.0f;
    const float HOUSE3_X = 345.0f, HOUSE3_Y = 272.0f;
}

#endif
