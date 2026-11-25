// ============================================================
// forest.h - Chapter 1 director: unfolding timeline, scene
//            composition, exploration and the cinematic hand-over
// ============================================================
#ifndef FOREST_H
#define FOREST_H

enum ForestPhase { FP_UNFOLD, FP_EXPLORE, FP_STORY, FP_AFTER };

// Start and reset the forest animation
void startForest();

// Update forest animation (one call per frame)
void updateForest();

// Draw the complete forest (world + HUD + cinematic overlay)
void drawForest();

// Mouse: click in logical 1000x650 coordinates, true if it was used
bool handleForestClick(float x, float y);
void forestMouseMove(float x, float y);

// Keyboard
void forestKey(unsigned char key);
void forestSpecialKey(int key);

// Check whether the forest has finished unfolding
bool isForestReady();
ForestPhase forestPhase();
bool forestBackHit(float x, float y);   // back button is clickable right now
bool forestStoryFinished();             // "BACK TO BOOK" state

#endif
