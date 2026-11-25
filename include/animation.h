
#ifndef ANIMATION_H
#define ANIMATION_H

// Start the book-opening animation (also restarts the whole chapter)
void startBookAnimation();

// Update animation progress (call once per frame after pl::tick())
// Returns true when the book is completely open
bool updateBookAnimation();

// Get current animation progress (0.0 closed .. 1.0 open)
float getBookAnimationProgress();

// Draw the animated book (pages, forest, cover, HUD)
void drawBookAnimation();

// Closing the book again ("BACK TO BOOK")
void startBookClose();
bool isBookClosing();
bool isBookClosed();      // true once the closing animation has finished

// Handle clicks inside the forest
bool handleForestClick(float x, float y);

#endif
