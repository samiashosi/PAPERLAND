// ============================================================
// chapter.h - the interface every chapter plugs into.
//
// A chapter owns its own world, characters, puzzles and story.
// It never knows about other chapters: core/game.cpp keeps the
// list of chapters and forwards input / update / draw to the
// active one.  Leave a hook as nullptr if a chapter does not
// need it.
// ============================================================
#ifndef CORE_CHAPTER_H
#define CORE_CHAPTER_H

struct Chapter
{
    const char* title;                          // shown in menus
    void (*start)();                            // (re)start from the beginning
    void (*update)();                           // once per frame
    void (*draw)();                             // world + HUD + overlays
    bool (*mouseClick)(float x, float y);       // logical 1000x650 coords; true = used
    void (*mouseMove)(float x, float y);
    void (*key)(unsigned char key);
    void (*specialKey)(int key);
    bool (*backButtonHit)(float x, float y);    // "BACK TO BOOK" clickable now?
    bool (*isComplete)();                       // chapter finished?
};

#endif
