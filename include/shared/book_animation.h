
#ifndef SHARED_BOOK_ANIMATION_H
#define SHARED_BOOK_ANIMATION_H

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

// The book is chapter-agnostic: core/game registers the active chapter's
// start / update / draw functions here (the page content inside the book).
void setBookContent(void (*start)(), void (*update)(), void (*draw)());

#endif
