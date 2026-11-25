
#ifndef MENU_H
#define MENU_H

// Application screen states
enum class AppScreen
{
    MENU,
    CHAPTERS,
    ABOUT,
    STORY,
    EXIT
};

// Draw the selected screen
void drawMenu(AppScreen screen);

// Handle mouse interaction
bool handleMenuMouse(
    int button,
    int state,
    int x,
    int y,
    AppScreen& screen
);

// Handle keyboard interaction
bool handleMenuKeyboard(
    unsigned char key,
    AppScreen& screen
);

#endif
