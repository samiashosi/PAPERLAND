// ============================================================
// chapter1.cpp - Chapter 1: The Enchanted Forest (director)
//
// Timeline (seconds after the book cover has opened):
//   Stage 1  landscape rises            0.2 -  2.6
//   Stage 2  trees unfold               2.2 -  4.6
//   Stage 3  water begins to flow       4.4 -  6.6
//   Stage 4  magical village appears    6.4 -  8.6
//   Stage 5  inhabitants arrive         8.4 - 10.2
//   Stage 6  the magic awakens         10.0 - 12.2
//   Stage 7  the forest is alive       12.4 -> explore
// ============================================================
#include <GL/freeglut.h>
#include <cmath>

#include "chapters/chapter1/chapter1.h"
#include "shared/book_animation.h"
#include "shared/paper.h"
#include "chapters/chapter1/forest_layout.h"
#include "chapters/chapter1/forest_environment.h"
#include "chapters/chapter1/forest_objects.h"
#include "chapters/chapter1/forest_characters.h"
#include "chapters/chapter1/forest_effects.h"
#include "chapters/chapter1/forest_interactions.h"
#include "chapters/chapter1/forest_story.h"
#include "shared/audio.h"

using namespace pl;

// ------------------------------------------------------------
// state
// ------------------------------------------------------------
static float g_t = 0.0f;                    // forest timeline
static ForestPhase g_phase = FP_UNFOLD;
static float g_exploreT = 0.0f;
static float g_allFoundT = 0.0f;
static const float T_READY = 12.4f;
static const float EXPLORE_LIMIT = 55.0f;   // the story starts by itself after this

struct Stages { float land, trees, water, village, inhab, magic; };

static Stages stages()
{
    Stages s;
    s.land = window(g_t, 0.2f, 2.6f);
    s.trees = window(g_t, 2.2f, 4.6f);
    s.water = window(g_t, 4.4f, 6.6f);
    s.village = window(g_t, 6.4f, 8.6f);
    s.inhab = window(g_t, 8.4f, 10.2f);
    s.magic = window(g_t, 10.0f, 12.2f);
    return s;
}

void startChapter1()
{
    seedRandom(2024);
    g_t = 0.0f;
    g_phase = FP_UNFOLD;
    g_exploreT = 0.0f;
    g_allFoundT = 0.0f;
    fx::initEffects();
    env::initEnvironment();
    obj::initObjects();
    chara::initCharacters();
    ui::initInteractions();
    story::initStory();
    ui::resetCursor();
    audio::startMusic();
    audio::play(audio::SFX_PAGE);
}

bool isChapter1Ready() { return g_phase != FP_UNFOLD; }
ForestPhase chapter1Phase() { return g_phase; }
bool isChapter1Complete() { return g_phase == FP_AFTER; }
bool chapter1BackButtonHit(float x, float y)
{
    return g_phase != FP_STORY && ui::backButtonHit(x, y);
}

// ------------------------------------------------------------
// update
// ------------------------------------------------------------
void updateChapter1()
{
    float dt = pl::dt();
    float bookP = getBookAnimationProgress();
    bool closing = isBookClosing();

    if (bookP >= 0.72f && !closing) g_t += dt;
    Stages s = stages();

    fx::setLeafAmount(s.trees);
    fx::setWaterfallAmount(s.water);
    fx::setFireflyAmount(s.magic);
    fx::updateEffects(dt);
    env::updateEnvironment(dt);
    obj::updateObjects(dt);
    chara::updateCharacters(dt);
    ui::updateInteractions(dt);
    story::updateStory(dt);

    if (g_phase == FP_UNFOLD && g_t >= T_READY)
    {
        g_phase = FP_EXPLORE;
        g_exploreT = 0.0f;
        ui::toast("The forest is alive!", "Click the creatures and objects, and find 5 hidden treasures.", 6.0f, 1);
        audio::play(audio::SFX_SPARKLE);
    }

    if (g_phase == FP_EXPLORE)
    {
        g_exploreT += dt;
        float remaining = EXPLORE_LIMIT - g_exploreT;
        ui::setStoryCountdown(remaining <= 15.0f ? remaining : -1.0f);

        // all five found: give the player a moment, then the story begins
        if (ui::allDiscovered() && !ui::discoveryBusy())
        {
            g_allFoundT += dt;
            ui::setStoryCountdown(-1.0f);
            if (g_allFoundT > 4.5f) { g_phase = FP_STORY; story::startStory(); }
        }
        else if (remaining <= 0.0f)
        {
            g_phase = FP_STORY;
            story::startStory();
        }
        if (g_phase == FP_STORY) { ui::setStoryCountdown(-1.0f); ui::resetCursor(); }
    }

    if (g_phase == FP_STORY && story::storyJustFinished())
    {
        g_phase = FP_AFTER;
        fx::forestParticles().sparkles(layout::TREE_X, layout::TREE_Y + 260, 50, Col(1.0f, 0.9f, 0.45f), 170, 10);
        ui::toast("The light has returned!", "Explore freely, or press BACK TO BOOK. (T replays the story)", 7.0f, 1);
        audio::play(audio::SFX_ALLFOUND);
    }
}

// ------------------------------------------------------------
// draw
// ------------------------------------------------------------
void drawChapter1()
{
    Stages s = stages();
    float scaleOpen = isBookClosing() ? smooth(getBookAnimationProgress() * 1.6f) : 1.0f;

    glPushMatrix();
    // when the book closes the pop-up scene folds flat
    glTranslatef(0, layout::B, 0);
    glScalef(1.0f, 0.04f + 0.96f * scaleOpen, 1.0f);
    glTranslatef(0, -layout::B, 0);

    scissor(layout::L, layout::B, layout::R, layout::T);

    // ---- Zone 1: background landscape ----
    env::drawSky(s.land);
    env::drawSun(window(s.land, 0.15f, 1.0f));
    env::drawClouds(window(s.land, 0.1f, 0.9f));
    env::drawMountains(s.land);
    ui::drawHidden(4, s.village);                    // star fragment glimmering in the sky
    chara::drawBirds(s.inhab);
    env::drawHills(s.land);
    env::drawDistantTrees(window(s.land, 0.3f, 1.0f));
    env::drawGround(s.land);
    env::drawGrassTufts(window(s.land, 0.5f, 1.0f));

    // ---- Zone 2: water (waterfall cliff, river, pond) ----
    env::drawWaterfallCliff(window(s.water, 0.0f, 0.4f));
    env::drawRiver(window(s.water, 0.1f, 0.8f));
    env::drawWaterfall(window(s.water, 0.1f, 0.6f), env::waterfallBoost());
    env::drawPond(window(s.water, 0.6f, 1.0f));
    fx::drawEffectsBack();                           // ripples sit on the water
    env::drawPathStones(s.village);

    // ---- standing paper objects, far to near ----
    env::drawTreeIndex(0, stagger(s.trees, 0, 4, 0.5f));
    env::drawTreeIndex(1, stagger(s.trees, 1, 4, 0.5f));
    env::drawTreeIndex(2, stagger(s.trees, 2, 4, 0.5f));
    float glowAmt = s.village;
    obj::drawHouse(0, s.village, glowAmt);           // yellow house (far)
    obj::drawHouse(1, s.village, glowAmt);           // red house
    env::drawTreeIndex(3, stagger(s.trees, 3, 4, 0.5f));
    ui::drawHidden(1, s.village);                    // ancient leaf hiding in the oak
    env::drawBridge(s.water);
    obj::drawHouse(2, s.village, glowAmt);           // lavender house (near)
    obj::drawSign(window(s.village, 0.4f, 1.0f));
    obj::drawAncientTree(window(s.trees, 0.2f, 1.0f), s.magic);
    obj::drawChest(window(s.magic, 0.3f, 0.8f));

    // ---- foreground: flowers, hidden objects, characters ----
    ui::drawHidden(0, s.village);                    // crystal in the flowers
    obj::drawFlowers(s.village);
    obj::drawMagicalFlowers(window(s.magic, 0.0f, 1.0f));
    ui::drawHidden(2, s.village);                    // feather by the pond
    ui::drawHidden(3, s.village);                    // golden key under the mushrooms
    obj::drawMushrooms(s.village);
    chara::drawGroundCharacters(s.inhab);
    chara::drawButterflies(s.inhab);

    // ---- light and particles ----
    fx::drawEffectsFront();
    ui::drawHintRings();

    scissorOff();
    glPopMatrix();

    // stage frame (little paper border around the pop-up scene)
    glColor4f(0.55f, 0.38f, 0.22f, 0.9f * s.land);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(layout::L, layout::B); glVertex2f(layout::R, layout::B);
    glVertex2f(layout::R, layout::T); glVertex2f(layout::L, layout::T);
    glEnd();
    glLineWidth(1.0f);

    // ---- HUD ----
    bool storyOn = (g_phase == FP_STORY);
    ui::drawHUD(window(g_t, 0.0f, 1.0f), window(g_t, 8.4f, 9.4f));
    if (!storyOn) ui::drawBackButton(g_phase == FP_AFTER);
    ui::drawFlyingIcons();

    // ---- cinematic overlay ----
    story::drawStory();

    if (pl::paused())
    {
        overlay(Col(0.1f, 0.06f, 0.04f, 0.35f));
        panel(390, 300, 610, 360, Col(0.99f, 0.93f, 0.78f), Col(0.62f, 0.42f, 0.22f));
        textCentered(500, 322, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24, Col(0.40f, 0.20f, 0.10f));
        textCentered(500, 306, "press SPACE to continue", GLUT_BITMAP_HELVETICA_12, Col(0.5f, 0.36f, 0.24f));
    }
}

// ------------------------------------------------------------
// input
// ------------------------------------------------------------
bool handleChapter1Mouse(float x, float y)
{
    if (pl::paused()) return false;
    if (g_phase == FP_STORY) return story::storyClick(x, y);
    if (g_phase == FP_UNFOLD) return false;
    return ui::handleClick(x, y);
}

void handleChapter1MouseMove(float x, float y)
{
    pl::setMouse(x, y);
    if (g_phase == FP_EXPLORE || g_phase == FP_AFTER) ui::handleHover(x, y);
}

void handleChapter1Keyboard(unsigned char key)
{
    switch (key)
    {
    case ' ':
        pl::setPaused(!pl::paused());
        break;
    case 'm': case 'M':
        audio::toggleMute();
        break;
    case 'h': case 'H':
        if (g_phase == FP_EXPLORE || g_phase == FP_AFTER) ui::giveHint();
        break;
    case 's': case 'S':
        if (g_phase == FP_UNFOLD) g_t = T_READY;                // skip the unfolding
        else if (g_phase == FP_STORY) story::skipStory();
        break;
    case 13: case 'n': case 'N':
        if (g_phase == FP_STORY) story::storyAdvance();
        else if (g_phase == FP_EXPLORE) { g_phase = FP_STORY; story::startStory(); ui::resetCursor(); }
        break;
    case 't': case 'T':
        if (g_phase == FP_EXPLORE || g_phase == FP_AFTER) { g_phase = FP_STORY; story::startStory(); ui::resetCursor(); }
        break;
    default:
        break;
    }
}

void handleChapter1SpecialKey(int key)
{
    if (g_phase == FP_STORY)
    {
        if (key == GLUT_KEY_RIGHT) story::storyAdvance();
        return;
    }
    if (key == GLUT_KEY_RIGHT)
        ui::toast("Chapter 2 is still folded", "The Paper Village will unfold in a later release.", 3.0f, 0);
    else if (key == GLUT_KEY_LEFT)
        ui::toast("This is the first page", "There is no earlier chapter. Press ESC for the menu.", 3.0f, 0);
}

// ------------------------------------------------------------
// table handed to core/game.cpp
// ------------------------------------------------------------
const Chapter& chapter1()
{
    static const Chapter c = {
        "Chapter 1: The Enchanted Forest",
        startChapter1, updateChapter1, drawChapter1,
        handleChapter1Mouse, handleChapter1MouseMove,
        handleChapter1Keyboard, handleChapter1SpecialKey,
        chapter1BackButtonHit, isChapter1Complete
    };
    return c;
}
