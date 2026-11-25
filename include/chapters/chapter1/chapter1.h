// ============================================================
// chapter1.h - Chapter 1: The Enchanted Forest (public interface)
//
// Everything outside chapters/chapter1 talks to the chapter only
// through these functions (or the Chapter table below).  The
// storyline, characters, puzzles and visuals live in the
// forest_*.cpp files next to this one.
// ============================================================
#ifndef CHAPTER1_H
#define CHAPTER1_H

#include "core/chapter.h"

enum ForestPhase { FP_UNFOLD, FP_EXPLORE, FP_STORY, FP_AFTER };

void startChapter1();                           // start / reset
void updateChapter1();                          // once per frame
void drawChapter1();                            // world + HUD + cinematic overlay
bool handleChapter1Mouse(float x, float y);     // click, logical 1000x650; true if used
void handleChapter1MouseMove(float x, float y);
void handleChapter1Keyboard(unsigned char key);
void handleChapter1SpecialKey(int key);
bool isChapter1Complete();                      // story finished ("BACK TO BOOK" state)

bool chapter1BackButtonHit(float x, float y);   // back button clickable right now
bool isChapter1Ready();                         // unfolding finished
ForestPhase chapter1Phase();

const Chapter& chapter1();                      // table for core/game.cpp

#endif
