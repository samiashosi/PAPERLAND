// ============================================================
// game.h - central controller.  Registers the chapters and routes
// main.cpp / menu / book animation to whichever chapter is active.
// To add a chapter: write chapterN.h/.cpp exposing a Chapter table
// (see chapters/chapter1) and register it in game::init().
// ============================================================
#ifndef CORE_GAME_H
#define CORE_GAME_H

namespace game
{
    const int MAX_CHAPTERS = 4;

    void init();                        // register chapters, hook the book animation
    bool selectChapter(int number);     // 1..4; false if that chapter is not built yet
    int  activeChapter();               // 1..4

    // forwarded to the active chapter
    void start();
    void update();
    void draw();
    bool mouseClick(float x, float y);
    void mouseMove(float x, float y);
    void key(unsigned char key);
    void specialKey(int key);
    bool backButtonHit(float x, float y);
    bool isComplete();
}

#endif
