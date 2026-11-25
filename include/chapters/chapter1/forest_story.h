// ============================================================
// story.h - cinematic story: dim, transition, 5 illustrated scenes
//           with camera movement and character dialogue
// ============================================================
#ifndef STORY_H
#define STORY_H

namespace story
{
    void initStory();
    void startStory();                // begin the cinematic sequence
    void updateStory(float dt);
    void drawStory();                 // overlay drawn above the forest
    bool storyActive();               // true while the cinematic owns input
    bool storyFinishedOnce();         // set when a playthrough has ended
    bool storyJustFinished();         // true for exactly one poll after ending
    bool storyClick(float x, float y);
    void storyAdvance();              // next dialogue (key / click)
    void skipStory();
    float storyDim();                 // 0..1 how dark the forest is now
}

#endif
