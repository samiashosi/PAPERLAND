// ============================================================
// interactions.cpp - clicks, discoveries, messages and HUD
// ============================================================
#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>
#include <vector>

#include "../include/interactions.h"
#include "../include/characters.h"
#include "../include/objects.h"
#include "../include/environment.h"
#include "../include/effects.h"
#include "../include/layout.h"
#include "../include/audio.h"

using namespace pl;

namespace ui
{
// ------------------------------------------------------------
// state
// ------------------------------------------------------------
struct Hidden
{
    float x, y, s;
    bool found;
    float flyT;          // 0 = not flying, >0 while flying to the counter
    float flyFromX, flyFromY;
    const char* title;
    const char* body;
};
static Hidden g_hidden[5];
static float g_hintT = 0.0f;
static float g_visible[5];                // how visible each object currently is

struct Toast { std::string title, body; float t, dur; int kind; bool active; };
static Toast g_toast = { "", "", 0, 0, 0, false };
static float g_countdown = -1.0f;
static int g_cursor = -1;

static const float SLOT_X0 = 770.0f, SLOT_DX = 38.0f, SLOT_Y = 621.0f;

void initInteractions()
{
    g_hidden[0] = { 96, 212, 0.62f, false, 0, 0, 0, "A glowing crystal!", "It hums softly, warm as a tiny sunrise." };
    g_hidden[1] = { 0, 0, 0.62f, false, 0, 0, 0, "An ancient leaf!", "Golden veins trace the story of every season." };
    g_hidden[2] = { 405, 114, 0.62f, false, 0, 0, 0, "A magical feather!", "It floats upward whenever you let go." };
    g_hidden[3] = { 872, 122, 0.60f, false, 0, 0, 0, "A tiny golden key!", "It looks just the right size for the old chest." };
    g_hidden[4] = { 372, 516, 0.70f, false, 0, 0, 0, "A forgotten star fragment!", "It sparkles as if it still remembers the sky." };
    env::oakCanopyPoint(g_hidden[1].x, g_hidden[1].y);
    for (int i = 0; i < 5; i++) g_visible[i] = 0.0f;
    g_toast.active = false;
    g_hintT = 0.0f;
    g_countdown = -1.0f;
}

void resetCursor() { g_cursor = -1; glutSetCursor(GLUT_CURSOR_LEFT_ARROW); }

void updateInteractions(float dt)
{
    for (int i = 0; i < 5; i++)
        if (g_hidden[i].flyT > 0.0f)
        {
            g_hidden[i].flyT += dt / 1.1f;
            if (g_hidden[i].flyT >= 1.0f)
            {
                g_hidden[i].flyT = 0.0f;
                fx::forestParticles().sparkles(SLOT_X0 + SLOT_DX * i, SLOT_Y, 10, Col(1.0f, 0.9f, 0.4f), 60, 6);
            }
        }
    if (g_hintT > 0.0f) g_hintT -= dt;
    if (g_toast.active)
    {
        g_toast.t += dt;
        if (g_toast.t >= g_toast.dur) g_toast.active = false;
    }
}

int discoveredCount()
{
    int n = 0;
    for (int i = 0; i < 5; i++) if (g_hidden[i].found) n++;
    return n;
}
bool allDiscovered() { return discoveredCount() == 5; }
bool discovered(int i) { return i >= 0 && i < 5 && g_hidden[i].found; }
bool discoveryBusy()
{
    for (int i = 0; i < 5; i++) if (g_hidden[i].flyT > 0.0f) return true;
    return false;
}

void toast(const std::string& title, const std::string& body, float seconds, int kind)
{
    g_toast.title = title; g_toast.body = body; g_toast.t = 0.0f;
    g_toast.dur = seconds; g_toast.kind = kind; g_toast.active = true;
}

void setStoryCountdown(float s) { g_countdown = s; }

void giveHint()
{
    g_hintT = 3.0f;
    toast("Psst... a hint", "Shimmering rings mark the places where treasures hide.", 3.0f, 0);
    audio::play(audio::SFX_SPARKLE);
}

// ------------------------------------------------------------
// hidden objects
// ------------------------------------------------------------
void drawHidden(int i, float amount)
{
    g_visible[i] = amount;
    if (amount <= 0.0f) return;
    const Hidden& h = g_hidden[i];
    if (h.found) return;
    float t = now();
    float k = easeOutBack(amount);
    // objects glint only now and then so they have to be spotted
    float cycle = std::fmod(t * 0.45f + i * 0.37f, 1.0f);
    float glint = cycle > 0.8f ? std::sin((cycle - 0.8f) / 0.2f * PI) : 0.0f;
    obj::drawDiscoveryIcon(i, h.x, h.y + std::sin(t * 1.6f + i) * 1.5f, h.s * k, 0.25f + 0.4f * glint);
    if (glint > 0.05f)
        sparkle4(h.x + 8, h.y + 9, 5.0f + 5.0f * glint, t * 2.0f, Col(1, 1, 0.85f, glint));
}

void drawHintRings()
{
    if (g_hintT <= 0.0f) return;
    float t = now();
    for (int i = 0; i < 5; i++)
    {
        if (g_hidden[i].found || g_visible[i] < 0.9f) continue;
        for (int k = 0; k < 2; k++)
        {
            float u = std::fmod(t * 0.9f + k * 0.5f, 1.0f);
            float a = std::sin(clamp01(g_hintT) * PI * 0.5f);
            ring(g_hidden[i].x, g_hidden[i].y, 8 + 30 * u, 10 + 30 * u, Col(1.0f, 0.95f, 0.5f, 0.8f * (1 - u) * a));
        }
    }
}

// ------------------------------------------------------------
// clicks
// ------------------------------------------------------------
static void discover(int i)
{
    Hidden& h = g_hidden[i];
    h.found = true;
    h.flyT = 0.001f;
    h.flyFromX = h.x; h.flyFromY = h.y;
    fx::forestParticles().sparkles(h.x, h.y, 24, Col(1.0f, 0.92f, 0.45f), 120, 8);
    audio::play(audio::SFX_FOUND);
    char buf[64];
    int n = discoveredCount();
    std::snprintf(buf, sizeof(buf), "  (%d / 5)", n);
    toast(std::string(h.title) + buf, h.body, 4.0f, 1);
    if (n == 5)
    {
        toast("ALL FIVE TREASURES FOUND!", "The Ancient Tree stirs... a story is about to begin.", 6.0f, 1);
        audio::play(audio::SFX_ALLFOUND);
    }
}

bool handleClick(float x, float y)
{
    // 1) hidden treasures have priority - they are tiny
    for (int i = 0; i < 5; i++)
    {
        const Hidden& h = g_hidden[i];
        if (h.found || g_visible[i] < 0.9f) continue;
        float dx = x - h.x, dy = y - h.y;
        if (dx * dx + dy * dy < 20.0f * 20.0f) { discover(i); return true; }
    }
    // 2) creatures
    if (chara::click(x, y)) return true;
    // 3) objects (houses, tree, chest, flowers, mushrooms)
    obj::Hit hit = obj::click(x, y);
    if (hit == obj::H_ANCIENT_TREE)
    {
        toast("The Ancient Tree glows", "\"Five treasures lie hidden in the forest. Find them for me...\"", 4.5f, 1);
        return true;
    }
    if (hit == obj::H_CHEST)
    {
        toast(obj::chestOpen() ? "The treasure chest opens!" : "The chest closes with a soft thud",
              obj::chestOpen() ? "Golden coins and a warm light spill out." : "Its magic keeps sleeping inside.", 3.0f, 0);
        return true;
    }
    if (hit == obj::H_HOUSE)
    {
        toast("A mushroom house", "The little door creaks open. Someone is home!", 2.6f, 0);
        return true;
    }
    if (hit != obj::H_NONE) return true;
    // 4) water
    if (env::hitWaterfall(x, y)) { env::sparkleWaterfall(); audio::play(audio::SFX_SPARKLE); return true; }
    if (env::hitPond(x, y))
    {
        fx::ripple(x, y, 46); fx::ripple(x + 6, y - 2, 70);
        audio::play(audio::SFX_SPLASH);
        fx::forestParticles().drops(x, y, 8);
        return true;
    }
    // 5) paper trees fold and unfold again
    if (env::clickTree(x, y)) { audio::play(audio::SFX_PAGE); return true; }
    return false;
}

void handleHover(float x, float y)
{
    bool hot = chara::hover(x, y) || obj::hover(x, y) || env::hitWaterfall(x, y) ||
               env::hitPond(x, y) || env::hoverTree(x, y);
    for (int i = 0; i < 5 && !hot; i++)
    {
        const Hidden& h = g_hidden[i];
        if (!h.found && g_visible[i] > 0.9f)
        {
            float dx = x - h.x, dy = y - h.y;
            if (dx * dx + dy * dy < 20.0f * 20.0f) hot = true;
        }
    }
    int want = hot ? 1 : 0;
    if (want != g_cursor)
    {
        g_cursor = want;
        glutSetCursor(hot ? GLUT_CURSOR_INFO : GLUT_CURSOR_LEFT_ARROW);
    }
}

// ------------------------------------------------------------
// HUD
// ------------------------------------------------------------
void drawFlyingIcons()
{
    for (int i = 0; i < 5; i++)
    {
        const Hidden& h = g_hidden[i];
        if (h.flyT <= 0.0f) continue;
        float u = easeInOutCubic(h.flyT);
        float tx = SLOT_X0 + SLOT_DX * i, ty = SLOT_Y;
        float x = lerp(h.flyFromX, tx, u);
        float y = lerp(h.flyFromY, ty, u) + std::sin(u * PI) * 50.0f;
        float s = lerp(h.s * 1.6f, 0.62f, u);
        obj::drawDiscoveryIcon(i, x, y, s, 1.0f);
        glow(x, y, 26, Col(1.0f, 0.92f, 0.5f, 0.5f * (1 - u)));
    }
}

void drawHUD(float titleAlpha, float counterAlpha)
{
    float t = now();
    if (titleAlpha > 0.0f)
    {
        textCentered(340, 620, "THE ENCHANTED FOREST", GLUT_BITMAP_TIMES_ROMAN_24, Col(0.30f, 0.17f, 0.10f, titleAlpha));
        text(70, 620, "Chapter 1", GLUT_BITMAP_HELVETICA_12, Col(0.50f, 0.36f, 0.24f, titleAlpha));
    }
    if (counterAlpha > 0.0f)
    {
        float a = counterAlpha;
        panel(640, 603, 944, 645, Col(0.99f, 0.93f, 0.78f, a), Col(0.62f, 0.42f, 0.22f, a));
        text(652, 633, "Forest discoveries", GLUT_BITMAP_HELVETICA_12, Col(0.40f, 0.26f, 0.14f, a));
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%d / 5", discoveredCount());
        text(652, 608, buf, GLUT_BITMAP_TIMES_ROMAN_24, Col(0.55f, 0.28f, 0.14f, a));
        for (int i = 0; i < 5; i++)
        {
            float sx = SLOT_X0 + SLOT_DX * i;
            circle(sx, SLOT_Y, 15, Col(0.50f, 0.34f, 0.20f, 0.9f * a));
            bool lit = g_hidden[i].found && g_hidden[i].flyT <= 0.0f;
            circle(sx, SLOT_Y, 13, lit ? Col(0.30f, 0.24f, 0.36f, a) : Col(0.84f, 0.74f, 0.56f, a));
            if (lit) obj::drawDiscoveryIcon(i, sx, SLOT_Y, 0.62f, 0.6f);
            else text(sx - 3, SLOT_Y - 5, "?", GLUT_BITMAP_HELVETICA_12, Col(0.55f, 0.42f, 0.28f, 0.8f * a));
        }
    }

    // toast / hint line in the bottom strip
    if (g_toast.active)
    {
        float in = easeOutCubic(g_toast.t / 0.3f);
        float out = clamp01((g_toast.dur - g_toast.t) / 0.5f);
        float a = in * out;
        float yoff = (1.0f - in) * -14.0f;
        Col fill = g_toast.kind == 1 ? Col(1.0f, 0.93f, 0.62f, a) : Col(0.99f, 0.95f, 0.84f, a);
        Col bord = g_toast.kind == 1 ? Col(0.85f, 0.60f, 0.15f, a) : Col(0.62f, 0.42f, 0.22f, a);
        panel(250, 24 + yoff, 750, 84 + yoff, fill, bord);
        textCentered(500, 62 + yoff, g_toast.title, GLUT_BITMAP_HELVETICA_18, Col(0.40f, 0.20f, 0.10f, a));
        std::vector<std::string> lines = wrapText(g_toast.body, GLUT_BITMAP_HELVETICA_12, 460);
        float ly = 42 + yoff;
        for (size_t i = 0; i < lines.size() && i < 2; i++, ly -= 14)
            textCentered(500, ly, lines[i], GLUT_BITMAP_HELVETICA_12, Col(0.42f, 0.30f, 0.20f, a));
    }
    else
    {
        textCentered(500, 52, "Click characters and objects  -  find 5 hidden treasures", GLUT_BITMAP_HELVETICA_12, Col(0.45f, 0.32f, 0.20f, 0.85f));
        textCentered(500, 34, "H hint   SPACE pause   S skip   M sound   R restart   ESC menu", GLUT_BITMAP_HELVETICA_10, Col(0.52f, 0.40f, 0.28f, 0.8f));
    }

    if (g_countdown >= 0.0f)
    {
        char buf[48];
        std::snprintf(buf, sizeof(buf), "Story begins in %ds", (int)std::ceil(g_countdown));
        panel(762, 30, 950, 76, Col(0.30f, 0.16f, 0.14f, 0.92f), Col(0.86f, 0.69f, 0.38f));
        textCentered(856, 58, buf, GLUT_BITMAP_HELVETICA_12, Col(1.0f, 0.90f, 0.65f));
        textCentered(856, 40, "press ENTER to start now", GLUT_BITMAP_HELVETICA_10, Col(0.95f, 0.80f, 0.55f));
    }
    (void)t;
}

// ------------------------------------------------------------
// back button (bottom-left)
// ------------------------------------------------------------
static const float BX1 = 40, BY1 = 28, BX2 = 228, BY2 = 76;

bool backButtonHit(float x, float y) { return inRect(x, y, BX1, BY1, BX2, BY2); }

void drawBackButton(bool afterStory)
{
    float pulse = afterStory ? 0.5f + 0.5f * std::sin(now() * 4.0f) : 0.0f;
    bool hover = backButtonHit(mouseX(), mouseY());
    if (afterStory) glow(134, 52, 120, Col(1.0f, 0.85f, 0.4f, 0.35f * pulse));
    paperShadow(4, -4, 0.3f);
    Col fill = afterStory ? Col(0.50f + 0.1f * pulse, 0.18f, 0.16f) : Col(0.35f, 0.12f, 0.12f);
    if (hover) fill = shade(fill, 1.2f);
    rect(BX1, BY1, BX2, BY2, fill);
    noShadow();
    glColor4f(0.86f, 0.69f, 0.38f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(BX1, BY1); glVertex2f(BX2, BY1); glVertex2f(BX2, BY2); glVertex2f(BX1, BY2);
    glEnd();
    textCentered(134, 47, afterStory ? "BACK TO BOOK" : "BACK TO MENU", GLUT_BITMAP_HELVETICA_18, Col(1.0f, 0.88f, 0.65f));
}

} // namespace ui
