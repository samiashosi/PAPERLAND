
#ifndef FOREST_H
#define FOREST_H

// Start and reset the forest animation
void startForest();

// Update forest animation
void updateForest();

// Draw the complete forest
void drawForest();

// Handle clicks inside the forest
bool handleForestClick(float x, float y);

// Check whether the forest has finished unfolding
bool isForestReady();

#endif
