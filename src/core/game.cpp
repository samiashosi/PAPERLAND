// ============================================================
// game.cpp - chapter registry and routing
// ============================================================
#include "core/game.h"
#include "core/chapter.h"
#include "shared/book_animation.h"
#include "chapters/chapter1/chapter1.h"

namespace game
{
static const Chapter* g_chapters[MAX_CHAPTERS] = { 0, 0, 0, 0 };
static int g_active = 1;

static const Chapter* current() { return g_chapters[g_active - 1]; }

void init()
{
    g_chapters[0] = &chapter1();         // Chapter 1: The Enchanted Forest
    // g_chapters[1] = &chapter2();      // Chapter 2: (not built yet)
    // g_chapters[2] = &chapter3();
    // g_chapters[3] = &chapter4();

    // the book shows whichever chapter is active
    setBookContent(start, update, draw);
}

bool selectChapter(int number)
{
    if (number < 1 || number > MAX_CHAPTERS || !g_chapters[number - 1]) return false;
    g_active = number;
    return true;
}

int activeChapter() { return g_active; }

void start()  { if (current() && current()->start)  current()->start(); }
void update() { if (current() && current()->update) current()->update(); }
void draw()   { if (current() && current()->draw)   current()->draw(); }

bool mouseClick(float x, float y)
{
    return current() && current()->mouseClick && current()->mouseClick(x, y);
}
void mouseMove(float x, float y)
{
    if (current() && current()->mouseMove) current()->mouseMove(x, y);
}
void key(unsigned char k)
{
    if (current() && current()->key) current()->key(k);
}
void specialKey(int k)
{
    if (current() && current()->specialKey) current()->specialKey(k);
}
bool backButtonHit(float x, float y)
{
    return current() && current()->backButtonHit && current()->backButtonHit(x, y);
}
bool isComplete()
{
    return current() && current()->isComplete && current()->isComplete();
}
} // namespace game
