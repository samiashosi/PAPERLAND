// ============================================================
// audio.h - procedurally synthesized sound effects + music.
// No sound files needed: everything is generated at start-up.
// On Windows it plays through a tiny waveOut mixer (winmm is loaded
// dynamically, so no extra linker flag is required). On other
// platforms every call is a harmless no-op.
// ============================================================
#ifndef AUDIO_H
#define AUDIO_H

namespace audio
{
    enum Sfx
    {
        SFX_CLICK, SFX_HOP, SFX_BOING, SFX_DOOR, SFX_BLOOM, SFX_TREE,
        SFX_CHEST, SFX_SPLASH, SFX_SPARKLE, SFX_FOUND, SFX_ALLFOUND,
        SFX_FLAP, SFX_FLUTTER, SFX_PAGE, SFX_STORY, SFX_NEXT, SFX_YIP,
        SFX_COUNT
    };

    void init();                 // build sounds + open the device
    void shutdown();
    void play(Sfx s);
    void startMusic();           // soft looping lullaby
    void stopMusic();
    void toggleMute();           // key M
    bool muted();
}

#endif
