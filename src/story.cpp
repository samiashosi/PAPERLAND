// ============================================================
// story.cpp - the cinematic: scenes, camera, dialogue, transitions
// ============================================================
#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "../include/story.h"
#include "../include/paper.h"
#include "../include/effects.h"
#include "../include/characters.h"
#include "../include/objects.h"
#include "../include/interactions.h"
#include "../include/audio.h"

using namespace pl;

namespace story
{
// ------------------------------------------------------------
// data
// ------------------------------------------------------------
enum Who { W_FLICK = 0, W_PIP = 1, W_DEER = 2 };
struct Line { int who; const char* text; };
struct Cam { float cx, cy, z; };
struct SceneDef { const char* title; int kind; int nLines; Line lines[3]; Cam c0, c1; };

static const SceneDef SCENES[5] = {
    { "THE ANCIENT TREE", 0, 3,
      { { W_FLICK, "Something is wrong. The Ancient Tree has lost its light!" },
        { W_PIP,   "It has glowed every night since the book was first opened. Why is it so dark?" },
        { W_DEER,  "Listen... the whole forest is holding its breath." } },
      { 450, 215, 1.0f }, { 470, 228, 1.18f } },
    { "THE FALLEN STAR", 1, 3,
      { { W_DEER,  "Long ago, five treasures kept the tree alive: a crystal, a leaf, a feather, a key and a star." },
        { W_FLICK, "And last night I saw a star fall from the sky. Its glow went with it!" },
        { W_PIP,   "" } },
      { 560, 250, 1.25f }, { 400, 235, 1.05f } },
    { "ACROSS THE RIVER", 2, 3,
      { { W_FLICK, "The old chest lies at the roots, past the bridge. Follow me!" },
        { W_PIP,   "Wait for me! Hop, hop, hop!" },
        { W_DEER,  "Careful on the planks. The waterfall mist makes them slippery." } },
      { 300, 190, 1.12f }, { 560, 190, 1.12f } },
    { "THE TREASURE CHEST", 3, 3,
      { { W_PIP,   "Here it is, the old treasure chest!" },
        { W_FLICK, "Quick, turn the golden key gently..." },
        { W_DEER,  "The lid is opening... a light! A tiny, golden light!" } },
      { 440, 215, 1.0f }, { 440, 160, 1.6f } },
    { "THE LIGHT RETURNS", 4, 3,
      { { W_FLICK, "It's working! The Ancient Tree is glowing again!" },
        { W_PIP,   "" },
        { W_DEER,  "Thank you, dear reader. Every page you turn keeps our forest alive." } },
      { 450, 190, 1.25f }, { 450, 225, 1.0f } }
};

static std::string lineText(int sc, int ln)
{
    const Line& L = SCENES[sc].lines[ln];
    if (sc == 1 && ln == 2)
    {
        int n = ui::discoveredCount();
        char b[96];
        if (n == 0) return "We have found none yet... but we will search every corner of the forest!";
        if (n == 5) return "Look at them shining! You found all five treasures!";
        std::snprintf(b, sizeof(b), "Look! %d of the five treasures are already shining!", n);
        return b;
    }
    if (sc == 4 && ln == 1)
        return ui::allDiscovered() ? "All five treasures together, just like the legend said!"
                                   : "Even with some treasures still hidden, its light is returning!";
    return L.text;
}

// ------------------------------------------------------------
// state
// ------------------------------------------------------------
enum Phase { SP_IDLE, SP_INTRO, SP_PLAY, SP_OUTRO };
static Phase g_phase = SP_IDLE;
static float g_pt = 0.0f;           // time in the current phase
static int   g_scene = 0, g_line = 0;
static float g_sceneT = 0.0f;
static float g_typeT = 0.0f;        // seconds since this line started typing
static float g_holdT = 0.0f;
static std::string g_text;
static float g_trans = -1.0f;       // scene transition 0..1 (-1 = none)
static float g_dim = 0.0f;
static float g_camP = 0.0f;
static bool g_finishedOnce = false, g_justFinished = false;
static float g_frameOpen = 0.0f;
static fx::ParticleSystem g_fx;          // inside the cinema frame
static fx::ParticleSystem g_fxScreen;    // full-screen intro magic
static float g_spawn = 0.0f;
static float g_chestOpen = 0.0f;

static const float FX1 = 60, FY1 = 165, FX2 = 940, FY2 = 595;      // cinema frame
static const float FW = 880, FH = 430;
static const float GY = 96.0f;                                      // ground line inside the frame

void initStory()
{
    g_phase = SP_IDLE; g_dim = 0; g_frameOpen = 0; g_fx.clear(); g_fxScreen.clear();
    g_finishedOnce = false; g_justFinished = false; g_trans = -1;
}

bool storyActive() { return g_phase != SP_IDLE; }
bool storyFinishedOnce() { return g_finishedOnce; }
bool storyJustFinished() { bool j = g_justFinished; g_justFinished = false; return j; }
float storyDim() { return g_dim; }

static void beginLine()
{
    g_text = lineText(g_scene, g_line);
    g_typeT = (g_line == 0 ? -0.9f : -0.15f);
    g_holdT = 0.0f;
    audio::play(audio::SFX_NEXT);
}

static void beginScene(int sc)
{
    g_scene = sc; g_line = 0; g_sceneT = 0.0f; g_camP = 0.0f; g_chestOpen = 0.0f;
    g_fx.clear();
    beginLine();
}

void startStory()
{
    g_phase = SP_INTRO; g_pt = 0; g_trans = -1;
    g_scene = 0; g_line = 0; g_dim = 0; g_frameOpen = 0;
    g_fx.clear(); g_fxScreen.clear();
    audio::play(audio::SFX_STORY);
}

static void endStory()
{
    g_phase = SP_IDLE;
    g_finishedOnce = true;
    g_justFinished = true;
    g_dim = 0;
    g_fx.clear(); g_fxScreen.clear();
}

void skipStory()
{
    if (g_phase == SP_IDLE) return;
    if (g_phase == SP_OUTRO) return;
    g_phase = SP_OUTRO; g_pt = 2.6f;           // jump to the fade-out part
}

void storyAdvance()
{
    if (g_phase == SP_INTRO) { g_pt = 3.6f; return; }
    if (g_phase == SP_OUTRO) { if (g_pt < 2.6f) g_pt = 2.6f; return; }
    if (g_phase != SP_PLAY || g_trans >= 0.0f) return;
    if (g_typeT < 0.0f) { g_typeT = 0.0f; return; }                 // start typing right now
    if (g_typeT * 42.0f < (float)g_text.size())
    {
        g_typeT = (float)g_text.size() / 42.0f + 0.01f;     // finish typing first
        return;
    }
    // next line / scene
    if (g_line < SCENES[g_scene].nLines - 1)
    {
        g_line++;
        beginLine();
    }
    else if (g_scene < 4) g_trans = 0.0f;
    else { g_phase = SP_OUTRO; g_pt = 0.0f; }
}

bool storyClick(float x, float y)
{
    if (g_phase == SP_IDLE) return false;
    if (inRect(x, y, 790, 34, 912, 66)) { skipStory(); return true; }
    storyAdvance();
    return true;
}

// ------------------------------------------------------------
// update
// ------------------------------------------------------------
static float speakerTalk()
{
    if (g_phase != SP_PLAY || g_trans >= 0.0f) return 0.0f;
    bool typing = g_typeT >= 0.0f && g_typeT * 42.0f < (float)g_text.size();
    return typing ? 1.0f : 0.0f;
}

void updateStory(float dt)
{
    g_fx.update(dt); g_fxScreen.update(dt);
    if (g_phase == SP_IDLE) return;
    g_pt += dt;

    if (g_phase == SP_INTRO)
    {
        g_dim = smooth(g_pt / 1.4f) * 0.84f;
        g_frameOpen = smooth((g_pt - 2.2f) / 1.2f);
        // magical particles spiral inward
        g_spawn -= dt;
        if (g_spawn <= 0.0f && g_pt < 3.2f)
        {
            g_spawn = 0.02f;
            float an = rnd(0, 2 * PI), r = rnd(260, 420);
            fx::Particle q;
            q.x = 500 + std::cos(an) * r; q.y = 380 + std::sin(an) * r * 0.6f;
            q.vx = -std::cos(an) * 220; q.vy = -std::sin(an) * 130;
            q.gravity = 0; q.life = 0; q.maxLife = r / 240.0f; q.size = rnd(3, 6);
            q.rot = 0; q.vr = rnd(-4, 4); q.phase = 0; q.kind = fx::PK_SPARK;
            q.c = Col(1.0f, 0.9f, 0.5f); q.groundY = -1;
            g_fxScreen.add(q);
        }
        if (g_pt >= 3.6f) { g_phase = SP_PLAY; g_frameOpen = 1.0f; beginScene(0); }
        return;
    }

    if (g_phase == SP_PLAY)
    {
        g_dim = 0.84f;
        g_sceneT += dt;
        // scene transition: dip to black, swap, dip back in
        if (g_trans >= 0.0f)
        {
            float before = g_trans;
            g_trans += dt / 1.0f;
            if (before < 0.5f && g_trans >= 0.5f) beginScene(g_scene + 1);
            if (g_trans >= 1.0f) g_trans = -1.0f;
        }
        else
        {
            g_typeT += dt;
            bool done = g_typeT * 42.0f >= (float)g_text.size();
            if (done) g_holdT += dt;
            if (done && g_holdT > 2.4f + g_text.size() * 0.02f) storyAdvance();
        }
        // camera eases toward its target (time based, never goes backwards)
        float nL = (float)SCENES[g_scene].nLines;
        float target = clamp01((g_sceneT / (nL * 6.5f) > (float)g_line / nL ? g_sceneT / (nL * 6.5f) : (float)g_line / nL));
        g_camP += (target - g_camP) * clamp01(dt * 2.5f);

        // scene specific particles
        g_spawn -= dt;
        if (g_spawn <= 0.0f)
        {
            int k = SCENES[g_scene].kind;
            g_spawn = 0.25f;
            if (k == 0) g_fx.leaf(rnd(300, 640), rnd(260, 330), rnd(70, 96));
            if (k == 1) g_fx.motes(rnd(100, 780), rnd(120, 300), 1, Col(0.7f, 0.8f, 1.0f), 10);
            if (k == 2) g_fx.mist(760, 100, 1);
            if (k == 3 && g_chestOpen > 0.5f) g_fx.motes(440, 160, 2, Col(1.0f, 0.9f, 0.5f), 30);
            if (k == 4)
            {
                float glowv = smooth(g_sceneT / 5.0f);
                if (glowv > 0.2f) { g_fx.motes(rnd(200, 700), rnd(110, 330), 2, Col(1.0f, 0.95f, 0.5f), 8); g_spawn = 0.12f; }
            }
        }
        if (SCENES[g_scene].kind == 3)
        {
            float prev = g_chestOpen;
            float target2 = g_sceneT > 2.6f ? 1.0f : 0.0f;
            g_chestOpen += (target2 - g_chestOpen) * clamp01(dt * 3.0f);
            if (prev < 0.5f && g_chestOpen >= 0.5f)
            {
                g_fx.coins(440, 140, 14);
                g_fx.sparkles(440, 160, 30, Col(1.0f, 0.9f, 0.45f), 160, 9);
                audio::play(audio::SFX_CHEST);
            }
        }
        if (SCENES[g_scene].kind == 4 && g_sceneT > 1.5f && g_sceneT - dt <= 1.5f)
            audio::play(audio::SFX_TREE);
        return;
    }

    if (g_phase == SP_OUTRO)
    {
        g_sceneT += dt;
        if (g_pt < 2.6f) { g_dim = 0.84f; }
        else
        {
            float u = (g_pt - 2.6f) / 1.6f;
            g_dim = 0.84f * (1.0f - smooth(u));
            g_frameOpen = 1.0f - smooth(u * 1.5f);
            if (u >= 1.0f) endStory();
        }
    }
}

// ------------------------------------------------------------
// scene painting helpers (frame-local coordinates 880 x 430)
// ------------------------------------------------------------
static void skyGradient(const Col& top, const Col& bottom)
{
    rectGrad(-500, GY - 2, 1380, 760, bottom, top);
}

static void hillBand(float baseY, float amp, float freq, float ph, const Col& c, const Col& cb)
{
    glBegin(GL_QUAD_STRIP);
    for (float x = -500; x <= 1380; x += 16)
    {
        float y = baseY + amp * std::sin(x * freq + ph) + amp * 0.5f * std::sin(x * freq * 2.3f + ph * 1.7f);
        glColor4f(c.r, c.g, c.b, 1); glVertex2f(x, y);
        glColor4f(cb.r, cb.g, cb.b, 1); glVertex2f(x, GY - 220);
    }
    glEnd();
}

static void mountainRow(const Col& c, float h, float spacing, float off, float base)
{
    for (float x = -500 + off; x < 1400; x += spacing)
    {
        float hh = h * (0.75f + 0.25f * std::sin(x * 0.07f));
        tri(x - spacing * 0.8f, base, x + spacing * 0.8f, base, x, base + hh, c);
    }
}

static void starField(float alpha)
{
    if (alpha <= 0.01f) return;
    float t = now();
    unsigned seed = 91;
    for (int i = 0; i < 80; i++)
    {
        seed = seed * 1664525u + 1013904223u;
        float x = -150 + (seed >> 8 & 0xFFFF) / 65535.0f * 1180.0f;
        seed = seed * 1664525u + 1013904223u;
        float y = 190 + (seed >> 8 & 0xFFFF) / 65535.0f * 440.0f;
        float tw = 0.5f + 0.5f * std::sin(t * 2.0f + i * 1.3f);
        sparkle4(x, y, 2.5f + 3.0f * tw, 0.0f, Col(1, 1, 0.9f, alpha * (0.4f + 0.6f * tw)));
    }
}

static void faceEmblem(int who, float x, float y, float r)
{
    circle(x, y, r + 4, Col(0.86f, 0.69f, 0.38f));
    if (who == W_FLICK)
    {
        circle(x, y, r, Col(0.98f, 0.82f, 0.62f));
        tri(x - r * 0.85f, y + r * 0.2f, x - r * 0.35f, y + r * 1.05f, x - r * 0.1f, y + r * 0.55f, Col(0.95f, 0.52f, 0.18f));
        tri(x + r * 0.85f, y + r * 0.2f, x + r * 0.35f, y + r * 1.05f, x + r * 0.1f, y + r * 0.55f, Col(0.95f, 0.52f, 0.18f));
        ellipse(x, y - r * 0.05f, r * 0.82f, r * 0.7f, Col(0.95f, 0.52f, 0.18f));
        tri(x - r * 0.7f, y - r * 0.15f, x + r * 0.7f, y - r * 0.15f, x, y - r * 0.75f, Col(1, 0.96f, 0.86f));
        circle(x, y - r * 0.45f, r * 0.1f, Col(0.2f, 0.1f, 0.06f));
        circle(x - r * 0.32f, y + r * 0.1f, r * 0.09f, Col(0.2f, 0.1f, 0.06f));
        circle(x + r * 0.32f, y + r * 0.1f, r * 0.09f, Col(0.2f, 0.1f, 0.06f));
    }
    else if (who == W_PIP)
    {
        circle(x, y, r, Col(0.86f, 0.80f, 0.95f));
        ellipse(x - r * 0.35f, y + r * 0.95f, r * 0.18f, r * 0.6f, Col(0.97f, 0.95f, 0.92f));
        ellipse(x + r * 0.35f, y + r * 0.95f, r * 0.18f, r * 0.6f, Col(0.97f, 0.95f, 0.92f));
        circle(x, y - r * 0.05f, r * 0.7f, Col(0.97f, 0.95f, 0.92f));
        circle(x - r * 0.25f, y + r * 0.08f, r * 0.08f, Col(0.2f, 0.1f, 0.1f));
        circle(x + r * 0.25f, y + r * 0.08f, r * 0.08f, Col(0.2f, 0.1f, 0.1f));
        tri(x - r * 0.1f, y - r * 0.18f, x + r * 0.1f, y - r * 0.18f, x, y - r * 0.32f, Col(1.0f, 0.65f, 0.75f));
    }
    else
    {
        circle(x, y, r, Col(0.78f, 0.90f, 0.78f));
        float a1[] = { x - r * 0.3f, y + r * 0.5f, x - r * 0.6f, y + r * 1.1f, x - r * 0.35f, y + r * 1.35f };
        float a2[] = { x + r * 0.3f, y + r * 0.5f, x + r * 0.6f, y + r * 1.1f, x + r * 0.35f, y + r * 1.35f };
        ribbon(a1, 3, 2.4f, Col(0.52f, 0.36f, 0.20f)); ribbon(a2, 3, 2.4f, Col(0.52f, 0.36f, 0.20f));
        ellipse(x, y - r * 0.05f, r * 0.62f, r * 0.75f, Col(0.82f, 0.60f, 0.36f));
        ellipse(x, y - r * 0.42f, r * 0.34f, r * 0.26f, Col(0.92f, 0.76f, 0.54f));
        circle(x, y - r * 0.46f, r * 0.09f, Col(0.2f, 0.1f, 0.06f));
        circle(x - r * 0.25f, y + r * 0.12f, r * 0.08f, Col(0.2f, 0.1f, 0.06f));
        circle(x + r * 0.25f, y + r * 0.12f, r * 0.08f, Col(0.2f, 0.1f, 0.06f));
    }
}

static const char* whoName(int w)
{
    return w == W_FLICK ? "Flick the Fox" : (w == W_PIP ? "Pip the Rabbit" : "The Friendly Deer");
}
static Col whoColor(int w)
{
    return w == W_FLICK ? Col(0.78f, 0.36f, 0.08f) : (w == W_PIP ? Col(0.52f, 0.30f, 0.72f) : Col(0.45f, 0.32f, 0.12f));
}

// ------------------------------------------------------------
// the five scene backdrops
// ------------------------------------------------------------
static void paintScene(int kind, float sceneT, float camP)
{
    float t = now();
    int speaker = (g_phase == SP_PLAY) ? SCENES[g_scene].lines[g_line].who : -1;
    float talkF = speakerTalk(), talkP = talkF, talkD = talkF;
    talkF = speaker == W_FLICK ? talkF : 0;
    talkP = speaker == W_PIP ? talkP : 0;
    talkD = speaker == W_DEER ? talkD : 0;

    if (kind == 0)          // ---- dusk: the dark Ancient Tree ----
    {
        skyGradient(Col(0.20f, 0.16f, 0.40f), Col(0.98f, 0.58f, 0.42f));
        glow(120, 110, 200, Col(1.0f, 0.7f, 0.4f, 0.45f), false);
        starField(0.25f);
        mountainRow(Col(0.42f, 0.32f, 0.56f), 120, 150, 0, GY);
        hillBand(GY + 22, 14, 0.010f, 1.0f, Col(0.26f, 0.32f, 0.42f), Col(0.20f, 0.26f, 0.34f));
        hillBand(GY + 4, 10, 0.014f, 3.0f, Col(0.17f, 0.28f, 0.28f), Col(0.12f, 0.22f, 0.22f));
        rectGrad(-500, -200, 1380, GY - 2, Col(0.09f, 0.20f, 0.18f), Col(0.15f, 0.28f, 0.24f));
        float flick = 0.04f + 0.04f * std::sin(t * 7.0f) * (std::sin(t * 1.3f) > 0.6f ? 1.0f : 0.0f);
        obj::setTreeDim(0.85f);
        obj::drawAncientTreeAt(450, GY - 4, 0.95f, flick, 0);
        chara::drawFlick(250, GY - 8, 1.6f, 1, 0, 0, talkF);
        chara::drawPip(640, GY - 6, 1.55f, -1, 0, 0, talkP);
        chara::drawDeer(770, GY - 10, 1.4f, -1, 0, 0, std::sin(t * 0.8f) * 6, talkD);
    }
    else if (kind == 1)     // ---- night: the fallen star ----
    {
        skyGradient(Col(0.03f, 0.04f, 0.15f), Col(0.20f, 0.16f, 0.42f));
        starField(1.0f);
        circle(720, 360, 46, Col(1.0f, 0.97f, 0.82f));
        glow(720, 360, 120, Col(1.0f, 0.97f, 0.85f, 0.35f));
        circle(735, 372, 40, Col(0.96f, 0.93f, 0.78f, 0.0f));
        // shooting star
        float u = std::fmod(t * 0.3f, 1.0f);
        float sx = 880 - u * 700, sy = 560 - u * 330;
        line(sx, sy, sx + 70, sy + 42, 3, Col(1, 1, 0.8f, 0.7f * (1 - u)));
        sparkle4(sx, sy, 10, t * 3, Col(1, 1, 0.8f, 1 - u));
        mountainRow(Col(0.14f, 0.14f, 0.34f), 130, 170, 40, GY);
        hillBand(GY + 8, 12, 0.012f, 2.0f, Col(0.10f, 0.16f, 0.28f), Col(0.07f, 0.12f, 0.22f));
        rectGrad(-500, -200, 1380, GY - 2, Col(0.05f, 0.11f, 0.15f), Col(0.09f, 0.18f, 0.20f));
        // the dim tree and its light drifting away
        obj::setTreeDim(0.7f);
        obj::drawAncientTreeAt(110, GY - 2, 0.8f, 0.0f, 0);
        float lu = std::fmod(t * 0.2f, 1.0f);
        glow(130 + lu * 300, 260 + lu * 260, 26 + 10 * std::sin(t * 6), Col(1.0f, 0.9f, 0.5f, 0.9f * (1 - lu)));
        // lantern light on the ground
        ellipse(480, GY - 12, 190, 30, Col(1.0f, 0.82f, 0.45f, 0.22f));
        glow(480, GY - 6, 150, Col(1.0f, 0.85f, 0.5f, 0.3f));
        // ring of treasures: shining when found, ghostly when not
        for (int i = 0; i < 5; i++)
        {
            float an = t * 0.4f + i * 2.0f * PI / 5.0f;
            float ix = 480 + std::cos(an) * 130, iy = 300 + std::sin(an) * 40;
            bool found = ui::discovered(i);
            obj::drawDiscoveryIcon(i, ix, iy, 1.25f, found ? 1.0f : 0.0f);
            if (!found) circle(ix, iy, 24, Col(0.06f, 0.07f, 0.18f, 0.72f));
        }
        chara::drawFlick(330, GY - 8, 1.55f, 1, 0, 0, talkF);
        chara::drawPip(480, GY - 24, 1.5f, -1, 0, 0, talkP);
        chara::drawDeer(650, GY - 10, 1.4f, -1, 0, 0, 0, talkD);
    }
    else if (kind == 2)     // ---- dawn: the journey across the bridge ----
    {
        skyGradient(Col(0.50f, 0.70f, 0.92f), Col(1.0f, 0.84f, 0.70f));
        glow(300, 140, 220, Col(1.0f, 0.88f, 0.55f, 0.5f), false);
        circle(300, 140, 36, Col(1.0f, 0.88f, 0.5f));
        mountainRow(Col(0.62f, 0.72f, 0.86f), 140, 190, 10, GY);
        hillBand(GY + 12, 12, 0.010f, 0.5f, Col(0.60f, 0.80f, 0.52f), Col(0.50f, 0.72f, 0.44f));
        // cliff + waterfall on the right
        float cl[] = { 700, GY, 716, GY + 120, 760, GY + 170, 840, GY + 170, 880, GY + 110, 890, GY };
        poly(cl, 6, Col(0.62f, 0.57f, 0.52f));
        rectGradH(748, GY - 6, 796, GY + 168, Col(0.6f, 0.84f, 0.97f), Col(0.78f, 0.94f, 1.0f));
        for (int k = 0; k < 8; k++)
        {
            float yy = GY + 168 - std::fmod(t * 120 + k * 22, 170);
            line(752 + k * 5.5f, yy, 752 + k * 5.5f, yy - 20, 2, Col(1, 1, 1, 0.8f));
        }
        ellipse(772, GY - 2, 46, 10, Col(1, 1, 1, 0.9f));
        rectGrad(-500, -200, 1380, GY, Col(0.34f, 0.64f, 0.34f), Col(0.52f, 0.78f, 0.42f));
        // river
        float rb = 30, rt = 70;
        glBegin(GL_QUAD_STRIP);
        for (float x = -500; x <= 1380; x += 20)
        {
            float w = 3 * std::sin(x * 0.05f + t);
            glColor4f(0.40f, 0.72f, 0.92f, 1); glVertex2f(x, rt + w);
            glColor4f(0.30f, 0.60f, 0.86f, 1); glVertex2f(x, rb - w);
        }
        glEnd();
        for (int k = 0; k < 16; k++)
        {
            float x = std::fmod(t * 40 + k * 90, 1100) - 100;
            line(x, 40 + (k % 4) * 8, x + 26, 40 + (k % 4) * 8, 2, Col(1, 1, 1, 0.5f));
        }
        // bridge: arch deck
        std::vector<float> deck;
        for (int i = 0; i <= 20; i++)
        {
            float u = i / 20.0f;
            deck.push_back(290 + u * 300);
            deck.push_back(74 + std::sin(u * PI) * 36);
        }
        paperShadow(4, -5, 0.3f);
        ribbon(deck.data(), 21, 16, Col(0.68f, 0.46f, 0.24f));
        noShadow();
        for (int i = 0; i <= 20; i++)
            line(deck[2 * i], deck[2 * i + 1] - 8, deck[2 * i], deck[2 * i + 1] + 8, 1.5f, Col(0.45f, 0.28f, 0.14f, 0.7f));
        std::vector<float> rail;
        for (int i = 0; i <= 20; i++) { rail.push_back(deck[2 * i]); rail.push_back(deck[2 * i + 1] + 34); }
        ribbon(rail.data(), 21, 4, Col(0.60f, 0.40f, 0.20f));
        for (int i = 0; i <= 20; i += 4) rect(deck[2 * i] - 2.5f, deck[2 * i + 1] + 8, deck[2 * i] + 2.5f, deck[2 * i + 1] + 34, Col(0.52f, 0.34f, 0.17f));
        // the three friends walk right across the bridge
        float p = smooth(camP);
        float fxp = lerp(200, 700, p);
        float walkPh = t * 9.0f;
        auto yOn = [&](float x) { float u = clamp01((x - 290) / 300.0f); return 82 + std::sin(u * PI) * 36; };
        float dxs = 0;
        (void)dxs;
        chara::drawDeer(fxp - 310, yOn(fxp - 310) - 6, 1.1f, 1, walkPh * 0.8f, 0, 0, talkD);
        chara::drawPip(fxp - 155, yOn(fxp - 155) - 4, 1.25f, 1, 0, std::fabs(std::sin(t * 7)) * 14, talkP);
        chara::drawFlick(fxp, yOn(fxp) - 6, 1.3f, 1, walkPh, 0, talkF);
    }
    else if (kind == 3)     // ---- the treasure chest at the roots ----
    {
        skyGradient(Col(0.24f, 0.36f, 0.50f), Col(0.90f, 0.76f, 0.56f));
        starField(0.15f);
        hillBand(GY + 16, 12, 0.011f, 4.0f, Col(0.30f, 0.44f, 0.38f), Col(0.22f, 0.36f, 0.32f));
        rectGrad(-500, -200, 1380, GY - 2, Col(0.12f, 0.26f, 0.20f), Col(0.22f, 0.38f, 0.28f));
        // giant roots of the tree on both sides
        obj::setTreeDim(0.5f);
        obj::drawAncientTreeAt(-40, GY - 30, 1.8f, 0.08f, 0);
        obj::drawAncientTreeAt(930, GY - 30, 1.7f, 0.08f, 1.5f);
        // light pool
        ellipse(440, GY - 10, 160 + 80 * g_chestOpen, 26, Col(1.0f, 0.85f, 0.4f, 0.12f + 0.2f * g_chestOpen));
        obj::drawChestAt(440, GY - 16, 1.9f, g_chestOpen, 0.2f);
        // key + star float out of the chest
        if (g_chestOpen > 0.5f)
        {
            float u = clamp01((sceneT - 3.0f) / 2.0f);
            float e = easeOutCubic(u);
            glow(440, 200 + 120 * e, 80, Col(1.0f, 0.9f, 0.5f, 0.5f * e));
            obj::drawDiscoveryIcon(3, 395, 150 + 120 * e + std::sin(t * 2) * 5, 1.6f, 1.0f);
            obj::drawDiscoveryIcon(4, 490, 150 + 150 * e + std::sin(t * 2.4f) * 5, 1.8f, 1.0f);
        }
        chara::drawFlick(270, GY - 10, 1.55f, 1, 0, 0, talkF);
        chara::drawPip(620, GY - 12, 1.5f, -1, 0, 0, talkP);
        chara::drawDeer(760, GY - 14, 1.3f, -1, 0, 0, std::sin(t) * 10, talkD);
    }
    else                    // ---- dawn: the light returns ----
    {
        float g = smooth(sceneT / 5.0f);
        skyGradient(mix(Col(0.04f, 0.05f, 0.20f), Col(0.50f, 0.72f, 1.0f), g),
                    mix(Col(0.16f, 0.14f, 0.36f), Col(1.0f, 0.86f, 0.55f), g));
        starField(1.0f - g);
        glow(450, 250, 380 * g, Col(1.0f, 0.9f, 0.5f, 0.5f * g));
        mountainRow(mix(Col(0.14f, 0.14f, 0.34f), Col(0.62f, 0.74f, 0.88f), g), 130, 170, 40, GY);
        hillBand(GY + 14, 12, 0.011f, 1.5f, mix(Col(0.10f, 0.18f, 0.28f), Col(0.58f, 0.80f, 0.50f), g),
                 mix(Col(0.07f, 0.12f, 0.22f), Col(0.46f, 0.70f, 0.42f), g));
        rectGrad(-500, -200, 1380, GY - 2, mix(Col(0.05f, 0.11f, 0.15f), Col(0.34f, 0.64f, 0.34f), g),
                 mix(Col(0.09f, 0.18f, 0.20f), Col(0.56f, 0.80f, 0.42f), g));
        obj::setTreeDim(0.8f * (1.0f - g));
        obj::drawAncientTreeAt(450, GY - 4, 1.25f, g * 1.2f, 0);
        // flowers bloom as the light spreads
        static const float fxs[] = { 60, 160, 250, 340, 560, 650, 740, 830 };
        for (int i = 0; i < 8; i++)
        {
            float k = clamp01(g * 1.4f - i * 0.08f);
            static const Col fc[4] = { Col(1.0f, 0.62f, 0.78f), Col(0.55f, 0.85f, 1.0f), Col(1.0f, 0.85f, 0.4f), Col(0.75f, 0.62f, 1.0f) };
            obj::drawFlowerAt(fxs[i], GY - 30 - (i % 3) * 8, 1.5f, k * 1.1f, fc[i % 4], true);
        }
        float cheer = g > 0.5f ? std::fabs(std::sin(t * 5.0f)) * 24 : 0.0f;
        float cheer2 = g > 0.5f ? std::fabs(std::sin(t * 5.0f + 1.0f)) * 22 : 0.0f;
        chara::drawFlick(250, GY - 10, 1.55f, 1, 0, cheer, talkF);
        chara::drawPip(660, GY - 14, 1.5f, -1, 0, cheer2, talkP);
        chara::drawDeer(780, GY - 14, 1.35f, -1, 0, 0, std::sin(t * 0.8f) * 5, talkD);
        // expanding rings of light
        for (int k = 0; k < 3; k++)
        {
            float u = std::fmod(t * 0.35f + k / 3.0f, 1.0f);
            ring(450, GY + 200, 30 + 420 * u, 34 + 420 * u, Col(1.0f, 0.92f, 0.55f, 0.35f * g * (1 - u)));
        }
    }
    obj::setTreeDim(0.0f);
}

// ------------------------------------------------------------
// drawing
// ------------------------------------------------------------
static void drawVignette(float cx, float cy, float rx, float ry, float alpha)
{
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0, 0, 0, 0);
    glVertex2f(cx, cy);
    glColor4f(0.02f, 0.02f, 0.08f, alpha);
    for (int i = 0; i <= 40; i++)
    {
        float a = 2 * PI * i / 40;
        glVertex2f(cx + std::cos(a) * rx, cy + std::sin(a) * ry);
    }
    glEnd();
}

static void drawDialoguePanel()
{
    const SceneDef& S = SCENES[g_scene];
    int who = S.lines[g_line].who;
    Col nc = whoColor(who);
    panel(70, 18, 930, 152, Col(0.99f, 0.94f, 0.80f, 0.97f), Col(0.62f, 0.42f, 0.22f));
    faceEmblem(who, 128, 88, 34);
    text(185, 126, whoName(who), GLUT_BITMAP_TIMES_ROMAN_24, nc);

    size_t n = (size_t)clampf(g_typeT * 42.0f, 0.0f, (float)g_text.size());
    std::string shown = g_text.substr(0, n);
    std::vector<std::string> lines = wrapText(g_text, GLUT_BITMAP_HELVETICA_18, 540);
    size_t consumed = 0;
    float ly = 98;
    for (size_t i = 0; i < lines.size(); i++, ly -= 23)
    {
        size_t len = lines[i].size();
        size_t take = consumed >= n ? 0 : (n - consumed < len ? n - consumed : len);
        text(185, ly, lines[i].substr(0, take), GLUT_BITMAP_HELVETICA_18, Col(0.28f, 0.16f, 0.10f));
        consumed += len + 1;
    }
    // blinking cursor when typing is done
    if (n >= g_text.size())
        text(176, 36, (std::fmod(now(), 1.0f) < 0.6f) ? "..." : "", GLUT_BITMAP_HELVETICA_12, Col(0.5f, 0.4f, 0.3f));

    // right column: scene counter + buttons
    char buf[32];
    std::snprintf(buf, sizeof(buf), "Scene %d of 5", g_scene + 1);
    textCentered(852, 134, buf, GLUT_BITMAP_HELVETICA_12, Col(0.50f, 0.36f, 0.24f));
    bool hv = inRect(mouseX(), mouseY(), 760, 78, 912, 118);
    panel(760, 78, 912, 118, hv ? Col(0.60f, 0.22f, 0.18f) : Col(0.45f, 0.17f, 0.15f), Col(0.86f, 0.69f, 0.38f));
    textCentered(836, 94, "Next dialogue >", GLUT_BITMAP_HELVETICA_12, Col(1.0f, 0.9f, 0.68f));
    bool hs = inRect(mouseX(), mouseY(), 790, 34, 912, 66);
    panel(790, 34, 912, 66, hs ? Col(0.80f, 0.72f, 0.58f) : Col(0.88f, 0.80f, 0.66f), Col(0.62f, 0.42f, 0.22f));
    textCentered(851, 46, "SKIP STORY", GLUT_BITMAP_HELVETICA_12, Col(0.38f, 0.24f, 0.14f));

    // auto-advance timer bar
    if (n >= g_text.size())
    {
        float need = 2.4f + g_text.size() * 0.02f;
        rect(80, 22, 80 + 620 * clamp01(g_holdT / need), 25, Col(0.86f, 0.69f, 0.38f, 0.8f));
    }
}

void drawStory()
{
    if (g_phase == SP_IDLE) return;
    float t = now();

    // dim the whole forest
    overlay(Col(0.03f, 0.04f, 0.12f, g_dim));
    drawVignette(500, 380, 760, 520, 0.75f);
    if (g_phase == SP_INTRO || g_dim > 0.0f)
    {
        float k = (g_phase == SP_INTRO) ? smooth(g_pt / 1.4f) : 1.0f;
        glow(500, 380, 300 * k, Col(1.0f, 0.88f, 0.5f, 0.18f * (g_phase == SP_INTRO ? std::sin(clamp01(g_pt / 3.6f) * PI) : 0.0f)));
    }
    g_fxScreen.draw();

    // title card during the intro
    if (g_phase == SP_INTRO)
    {
        float a = std::sin(clamp01((g_pt - 1.0f) / 2.2f) * PI);
        textCentered(500, 360, "ONCE UPON A TIME...", GLUT_BITMAP_TIMES_ROMAN_24, Col(1.0f, 0.92f, 0.65f, a));
        textCentered(500, 330, "deep in the Enchanted Forest", GLUT_BITMAP_HELVETICA_18, Col(0.95f, 0.85f, 0.65f, a * 0.9f));
    }

    // cinema frame
    if (g_frameOpen > 0.01f)
    {
        float fo = g_frameOpen;
        float sc = 0.88f + 0.12f * fo;
        float ccx = (FX1 + FX2) / 2, ccy = (FY1 + FY2) / 2;
        glPushMatrix();
        glTranslatef(ccx, ccy, 0);
        glScalef(sc, sc, 1);
        glTranslatef(-ccx, -ccy, 0);
        // frame shadow + backing
        rect(FX1 - 8, FY1 - 8, FX2 + 8, FY2 + 8, Col(0.04f, 0.03f, 0.05f, fo));
        // scene (clipped to the frame)
        if (fo > 0.99f) scissor(FX1, FY1, FX2, FY2);
        glPushMatrix();
        glTranslatef(FX1, FY1, 0);
        if (g_phase == SP_PLAY || g_phase == SP_OUTRO)
        {
            const SceneDef& S = SCENES[g_scene];
            float p = smooth(g_camP);
            Cam c = { lerp(S.c0.cx, S.c1.cx, p), lerp(S.c0.cy, S.c1.cy, p), lerp(S.c0.z, S.c1.z, p) };
            glPushMatrix();
            glTranslatef(FW / 2, FH / 2, 0);
            glScalef(c.z, c.z, 1);
            glTranslatef(-c.cx, -c.cy, 0);
            paintScene(S.kind, g_sceneT, g_camP);
            g_fx.draw();
            glPopMatrix();
            drawVignette(FW / 2, FH / 2, 640, 420, 0.38f);
            // end card
            if (g_phase == SP_OUTRO)
            {
                float a = std::sin(clamp01(g_pt / 2.4f) * PI);
                float aa = g_pt < 2.6f ? a : 0.0f;
                overlay(Col(0, 0, 0, 0));
                rect(0, 150, FW, 290, Col(0.03f, 0.03f, 0.10f, 0.55f * aa));
                textCentered(FW / 2, 232, "THE END OF CHAPTER 1", GLUT_BITMAP_TIMES_ROMAN_24, Col(1.0f, 0.92f, 0.62f, aa));
                textCentered(FW / 2, 198, "The forest remembers you.", GLUT_BITMAP_HELVETICA_18, Col(1.0f, 0.95f, 0.8f, aa));
            }
            // scene change dip + title card
            float dip = 0.0f;
            if (g_trans >= 0.0f) dip = 1.0f - std::fabs(g_trans * 2.0f - 1.0f);
            if (dip > 0.0f) rect(-10, -10, FW + 10, FH + 10, Col(0.02f, 0.02f, 0.06f, dip));
            if (g_phase == SP_PLAY && g_line == 0 && g_trans < 0.0f)
            {
                float a = std::sin(clamp01(g_sceneT / 2.0f) * PI);
                char b[64];
                std::snprintf(b, sizeof(b), "SCENE %d", g_scene + 1);
                textCentered(FW / 2, 262, b, GLUT_BITMAP_HELVETICA_18, Col(1.0f, 0.9f, 0.6f, a));
                textCentered(FW / 2, 232, S.title, GLUT_BITMAP_TIMES_ROMAN_24, Col(1, 1, 1, a));
            }
        }
        glPopMatrix();
        scissorOff();
        // frame border
        glColor4f(0.86f, 0.69f, 0.38f, fo);
        glLineWidth(3.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(FX1, FY1); glVertex2f(FX2, FY1); glVertex2f(FX2, FY2); glVertex2f(FX1, FY2);
        glEnd();
        glColor4f(0.35f, 0.18f, 0.14f, fo);
        glLineWidth(6.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(FX1 - 5, FY1 - 5); glVertex2f(FX2 + 5, FY1 - 5); glVertex2f(FX2 + 5, FY2 + 5); glVertex2f(FX1 - 5, FY2 + 5);
        glEnd();
        glLineWidth(1.0f);
        glPopMatrix();
    }

    // captions and dialogue
    if (g_phase == SP_PLAY)
    {
        rect(0, 598, 1000, 650, Col(0.03f, 0.04f, 0.12f, 0.97f));
        char b[96];
        std::snprintf(b, sizeof(b), "SCENE %d  -  %s", g_scene + 1, SCENES[g_scene].title);
        textCentered(500, 612, b, GLUT_BITMAP_HELVETICA_18, Col(1.0f, 0.9f, 0.65f));
        drawDialoguePanel();
    }
    (void)t;
}

} // namespace story
