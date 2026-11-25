
#ifndef ANIMATION_H
#define ANIMATION_H

// Start the book-opening animation
void startBookAnimation();

// Update animation progress
// Returns true when the animation is complete
bool updateBookAnimation();

// Get current animation progress (0.0 to 1.0)
float getBookAnimationProgress();

// Draw the animated book
void drawBookAnimation();

// Handle clicks inside the forest
bool handleForestClick(float x, float y);

#endif
