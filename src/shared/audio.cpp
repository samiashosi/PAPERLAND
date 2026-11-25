// ============================================================
// audio.cpp - procedural sound effects + background lullaby
//
// All sounds are synthesized in code at start-up (no asset files).
// Windows: a small software mixer feeds winmm's waveOut API.  winmm.dll
//          is loaded with LoadLibrary, so no extra linker flag is needed.
// Other platforms: every function is a silent no-op.
// ============================================================
#include "shared/audio.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>
#include <vector>
#include <cmath>
#include <cstdlib>

namespace audio
{
static const int RATE = 22050;
typedef std::vector<short> Wave;

static Wave g_sfx[SFX_COUNT];
static Wave g_music;
static bool g_muted = false;
static bool g_ready = false;
static bool g_musicOn = false;

// ---------------- synthesis helpers ----------------
static float frand() { return (std::rand() / (float)RAND_MAX) * 2.0f - 1.0f; }

static void addTone(Wave& w, float f0, float f1, float dur, float vol,
                    int shape, float attack = 0.01f, float decayPow = 2.0f,
                    float start = 0.0f)
{
    size_t s0 = (size_t)(start * RATE);
    size_t n = (size_t)(dur * RATE);
    if (w.size() < s0 + n) w.resize(s0 + n, 0);
    float ph = 0.0f;
    for (size_t i = 0; i < n; i++)
    {
        float t = (float)i / RATE, u = (float)i / n;
        float f = f0 + (f1 - f0) * u;
        ph += 2.0f * 3.14159265f * f / RATE;
        float v;
        switch (shape)
        {
        case 1:  v = (std::fmod(ph, 6.2831853f) / 3.14159265f) - 1.0f; break;        // saw
        case 2:  v = frand(); break;                                                   // noise
        case 3:  v = 2.0f * std::fabs(std::fmod(ph / 6.2831853f, 1.0f) * 2.0f - 1.0f) - 1.0f; break; // triangle
        default: v = std::sin(ph);
        }
        float env = (t < attack) ? t / attack : std::pow(1.0f - u, decayPow);
        int idx = (int)(s0 + i);
        int mixed = w[idx] + (int)(v * env * vol * 12000.0f);
        if (mixed > 32000) mixed = 32000;
        if (mixed < -32000) mixed = -32000;
        w[idx] = (short)mixed;
    }
}

static float note(int semis) { return 261.63f * std::pow(2.0f, semis / 12.0f); }   // C4 based

static void buildSounds()
{
    std::srand(7);
    addTone(g_sfx[SFX_CLICK], 880, 700, 0.07f, 0.5f, 0);
    addTone(g_sfx[SFX_HOP], 300, 620, 0.13f, 0.6f, 0);
    addTone(g_sfx[SFX_BOING], 180, 420, 0.12f, 0.7f, 0);
    addTone(g_sfx[SFX_BOING], 420, 170, 0.22f, 0.7f, 0, 0.01f, 2.0f, 0.12f);
    addTone(g_sfx[SFX_DOOR], 190, 110, 0.35f, 0.35f, 1, 0.02f, 1.5f);
    addTone(g_sfx[SFX_DOOR], 700, 500, 0.08f, 0.5f, 0, 0.005f, 2.0f, 0.36f);
    for (int i = 0; i < 3; i++)
        addTone(g_sfx[SFX_BLOOM], note(12 + i * 4), note(12 + i * 4), 0.3f, 0.45f, 0, 0.01f, 2.0f, i * 0.09f);
    addTone(g_sfx[SFX_TREE], note(12), note(12), 1.3f, 0.35f, 3, 0.05f, 1.5f);
    addTone(g_sfx[SFX_TREE], note(19), note(19), 1.3f, 0.30f, 0, 0.1f, 1.5f, 0.1f);
    addTone(g_sfx[SFX_TREE], note(28), note(28), 1.1f, 0.22f, 0, 0.2f, 1.5f, 0.2f);
    addTone(g_sfx[SFX_CHEST], 90, 60, 0.25f, 0.8f, 0);
    for (int i = 0; i < 6; i++)
        addTone(g_sfx[SFX_CHEST], note(12 + i * 2 + (i > 2 ? 1 : 0)), note(12 + i * 2 + (i > 2 ? 1 : 0)), 0.25f, 0.35f, 0, 0.005f, 2.0f, 0.12f + i * 0.07f);
    addTone(g_sfx[SFX_SPLASH], 0, 0, 0.35f, 0.5f, 2, 0.005f, 3.0f);
    addTone(g_sfx[SFX_SPARKLE], note(24), note(24), 0.25f, 0.3f, 0, 0.003f, 2.5f);
    addTone(g_sfx[SFX_SPARKLE], note(31), note(31), 0.3f, 0.3f, 0, 0.003f, 2.5f, 0.07f);
    for (int i = 0; i < 4; i++)
        addTone(g_sfx[SFX_FOUND], note(19 + i * 4 - (i == 3 ? 1 : 0)), note(19 + i * 4 - (i == 3 ? 1 : 0)), 0.4f, 0.4f, 0, 0.005f, 2.0f, i * 0.08f);
    for (int i = 0; i < 7; i++)
        addTone(g_sfx[SFX_ALLFOUND], note(12 + (i % 4) * 4 + (i / 4) * 12), note(12 + (i % 4) * 4 + (i / 4) * 12), 0.5f, 0.4f, 3, 0.005f, 1.8f, i * 0.12f);
    for (int i = 0; i < 4; i++)
        addTone(g_sfx[SFX_FLAP], 0, 0, 0.07f, 0.35f, 2, 0.003f, 2.0f, i * 0.09f);
    for (int i = 0; i < 6; i++)
        addTone(g_sfx[SFX_FLUTTER], 1200 + (i % 2) * 300, 1400, 0.05f, 0.25f, 0, 0.003f, 2.0f, i * 0.05f);
    addTone(g_sfx[SFX_PAGE], 0, 0, 0.5f, 0.3f, 2, 0.12f, 1.2f);
    addTone(g_sfx[SFX_STORY], note(0), note(0), 1.6f, 0.3f, 3, 0.4f, 1.0f);
    addTone(g_sfx[SFX_STORY], note(7), note(7), 1.6f, 0.25f, 3, 0.5f, 1.0f);
    addTone(g_sfx[SFX_STORY], note(16), note(16), 1.6f, 0.2f, 0, 0.6f, 1.0f);
    addTone(g_sfx[SFX_NEXT], 660, 760, 0.08f, 0.35f, 0);
    addTone(g_sfx[SFX_YIP], 520, 940, 0.09f, 0.5f, 0);
    addTone(g_sfx[SFX_YIP], 940, 620, 0.12f, 0.5f, 0, 0.005f, 2.0f, 0.09f);

    // lullaby: 4 chords x 4 beats, 16 seconds, arpeggios + a gentle melody
    const int chordRoots[4] = { 0, -3, -7, -5 };           // C, Am, F, G
    const float step = 0.333f;
    for (int c = 0; c < 8; c++)
    {
        int root = chordRoots[c % 4];
        for (int i = 0; i < 6; i++)
        {
            static const int arp[6] = { 0, 7, 12, 16, 12, 7 };
            addTone(g_music, note(root + arp[i]), note(root + arp[i]), 0.9f, 0.20f, 3, 0.02f, 2.2f,
                    (c * 6 + i) * step);
        }
        addTone(g_music, note(root - 12), note(root - 12), 1.9f, 0.20f, 0, 0.1f, 1.2f, c * 6 * step);
    }
    static const int mel[16] = { 16, 19, 21, 19, 16, 14, 12, 14, 17, 16, 14, 12, 14, 16, 12, 12 };
    for (int i = 0; i < 16; i++)
        addTone(g_music, note(mel[i] + 12), note(mel[i] + 12), 0.8f, 0.12f, 0, 0.05f, 1.6f, i * step * 3.0f);
}

// ---------------- mixer ----------------
struct Voice { const Wave* w; size_t pos; float vol; bool loop; bool active; };
static Voice g_voices[10];
static CRITICAL_SECTION g_cs;

typedef MMRESULT (WINAPI *FnOpen)(LPHWAVEOUT, UINT, LPCWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD);
typedef MMRESULT (WINAPI *FnPrep)(HWAVEOUT, LPWAVEHDR, UINT);
typedef MMRESULT (WINAPI *FnWrite)(HWAVEOUT, LPWAVEHDR, UINT);
typedef MMRESULT (WINAPI *FnClose)(HWAVEOUT);
typedef MMRESULT (WINAPI *FnReset)(HWAVEOUT);

static HMODULE g_lib = 0;
static FnOpen pOpen; static FnPrep pPrep, pUnprep; static FnWrite pWrite; static FnClose pClose; static FnReset pReset;
static HWAVEOUT g_dev = 0;
static HANDLE g_thread = 0;
static volatile bool g_quit = false;

static const int BUFN = 4, BUFS = 1024;
static WAVEHDR g_hdr[BUFN];
static short g_buf[BUFN][BUFS];

static void mixInto(short* out, int n)
{
    int acc[BUFS];
    for (int i = 0; i < n; i++) acc[i] = 0;
    EnterCriticalSection(&g_cs);
    for (int v = 0; v < 10; v++)
    {
        Voice& vc = g_voices[v];
        if (!vc.active || !vc.w || vc.w->empty()) continue;
        for (int i = 0; i < n; i++)
        {
            if (vc.pos >= vc.w->size())
            {
                if (vc.loop) vc.pos = 0;
                else { vc.active = false; break; }
            }
            acc[i] += (int)((*vc.w)[vc.pos++] * vc.vol);
        }
    }
    LeaveCriticalSection(&g_cs);
    for (int i = 0; i < n; i++)
    {
        int s = g_muted ? 0 : acc[i];
        if (s > 32000) s = 32000;
        if (s < -32000) s = -32000;
        out[i] = (short)s;
    }
}

static DWORD WINAPI mixThread(LPVOID)
{
    while (!g_quit)
    {
        for (int b = 0; b < BUFN; b++)
        {
            if (g_hdr[b].dwFlags & WHDR_DONE)
            {
                mixInto(g_buf[b], BUFS);
                pWrite(g_dev, &g_hdr[b], sizeof(WAVEHDR));
            }
        }
        Sleep(8);
    }
    return 0;
}

void init()
{
    if (g_ready) return;
    InitializeCriticalSection(&g_cs);
    for (int i = 0; i < 10; i++) g_voices[i].active = false;
    buildSounds();

    g_lib = LoadLibraryA("winmm.dll");
    if (!g_lib) return;
    pOpen = (FnOpen)GetProcAddress(g_lib, "waveOutOpen");
    pPrep = (FnPrep)GetProcAddress(g_lib, "waveOutPrepareHeader");
    pUnprep = (FnPrep)GetProcAddress(g_lib, "waveOutUnprepareHeader");
    pWrite = (FnWrite)GetProcAddress(g_lib, "waveOutWrite");
    pClose = (FnClose)GetProcAddress(g_lib, "waveOutClose");
    pReset = (FnReset)GetProcAddress(g_lib, "waveOutReset");
    if (!pOpen || !pPrep || !pUnprep || !pWrite || !pClose || !pReset) return;

    WAVEFORMATEX fmt;
    fmt.wFormatTag = WAVE_FORMAT_PCM;
    fmt.nChannels = 1;
    fmt.nSamplesPerSec = RATE;
    fmt.wBitsPerSample = 16;
    fmt.nBlockAlign = 2;
    fmt.nAvgBytesPerSec = RATE * 2;
    fmt.cbSize = 0;
    if (pOpen(&g_dev, WAVE_MAPPER, &fmt, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) { g_dev = 0; return; }

    for (int b = 0; b < BUFN; b++)
    {
        ZeroMemory(&g_hdr[b], sizeof(WAVEHDR));
        g_hdr[b].lpData = (LPSTR)g_buf[b];
        g_hdr[b].dwBufferLength = BUFS * sizeof(short);
        pPrep(g_dev, &g_hdr[b], sizeof(WAVEHDR));
        g_hdr[b].dwFlags |= WHDR_DONE;          // lets the thread fill + queue it
    }
    g_quit = false;
    g_thread = CreateThread(0, 0, mixThread, 0, 0, 0);
    g_ready = (g_thread != 0);
}

void shutdown()
{
    if (!g_ready) return;
    g_quit = true;
    WaitForSingleObject(g_thread, 500);
    pReset(g_dev);
    for (int b = 0; b < BUFN; b++) pUnprep(g_dev, &g_hdr[b], sizeof(WAVEHDR));
    pClose(g_dev);
    g_ready = false;
}

void play(Sfx s)
{
    if (!g_ready || s < 0 || s >= SFX_COUNT) return;
    EnterCriticalSection(&g_cs);
    for (int v = 1; v < 10; v++)
        if (!g_voices[v].active)
        {
            g_voices[v].w = &g_sfx[s]; g_voices[v].pos = 0;
            g_voices[v].vol = 0.9f; g_voices[v].loop = false; g_voices[v].active = true;
            break;
        }
    LeaveCriticalSection(&g_cs);
}

void startMusic()
{
    if (!g_ready || g_musicOn) return;
    EnterCriticalSection(&g_cs);
    g_voices[0].w = &g_music; g_voices[0].pos = 0; g_voices[0].vol = 0.55f;
    g_voices[0].loop = true; g_voices[0].active = true;
    LeaveCriticalSection(&g_cs);
    g_musicOn = true;
}

void stopMusic()
{
    if (!g_ready) return;
    EnterCriticalSection(&g_cs);
    g_voices[0].active = false;
    LeaveCriticalSection(&g_cs);
    g_musicOn = false;
}

void toggleMute() { g_muted = !g_muted; }
bool muted() { return g_muted; }
}

#else  // ---------------- non-Windows: silent stubs ----------------
namespace audio
{
static bool g_m = false;
void init() {}
void shutdown() {}
void play(Sfx) {}
void startMusic() {}
void stopMusic() {}
void toggleMute() { g_m = !g_m; }
bool muted() { return g_m; }
}
#endif
