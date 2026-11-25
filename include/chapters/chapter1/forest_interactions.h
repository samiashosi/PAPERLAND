// ============================================================
// interactions.h - clicking, discoveries (0/5), messages, HUD
// ============================================================
#ifndef INTERACTIONS_H
#define INTERACTIONS_H

#include <string>

namespace ui
{
    void initInteractions();
    void updateInteractions(float dt);

    // world input (logical 1000 x 650 coordinates)
    bool handleClick(float x, float y);
    void handleHover(float x, float y);

    // hidden discoveries
    void drawHidden(int index, float amount);   // draw one hidden object
    void drawHintRings();
    int  discoveredCount();
    bool allDiscovered();
    bool discovered(int index);
    bool discoveryBusy();                       // an icon is still flying
    void giveHint();

    // HUD
    void toast(const std::string& title, const std::string& body,
               float seconds = 4.0f, int kind = 0);   // kind 0 info, 1 gold
    void drawHUD(float titleAlpha, float counterAlpha);
    void drawBackButton(bool afterStory);
    bool backButtonHit(float x, float y);
    void setStoryCountdown(float seconds);      // <0 hides it
    void drawFlyingIcons();
    void resetCursor();
}

#endif
