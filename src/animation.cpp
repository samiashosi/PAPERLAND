
// ============================================================
// animation.cpp - the open book: cover flip, pages, closing
// ============================================================
#include <GL/freeglut.h>
#include <cmath>

#include "../include/animation.h"
#include "../include/forest.h"
#include "../include/paper.h"
#include "../include/audio.h"
#include "../include/interactions.h"

using namespace pl;

// ============================================
// BOOK ANIMATION STATE
// ============================================

static float animationProgress = 0.0f;
static bool animationRunning = false;
static bool closing = false;
static bool closed = false;

void startBookAnimation()
{
    animationProgress = 0.0f;
    animationRunning = true;
    closing = false;
    closed = false;
    pl::setPaused(false);

    // Reset the forest animation too
    startForest();
}

void startBookClose()
{
    if (closing) return;
    closing = true;
    closed = false;
    animationRunning = false;
    audio::play(audio::SFX_PAGE);
}

bool isBookClosing() { return closing; }
bool isBookClosed() { return closed; }

bool updateBookAnimation()
{
    float dt = pl::dt();

    if (closing)
    {
        animationProgress -= dt / 1.1f;
        if (animationProgress <= 0.0f)
        {
            animationProgress = 0.0f;
            closed = true;
            audio::stopMusic();
        }
    }
    else if (animationRunning)
    {
        animationProgress += dt / 2.0f;

        if (animationProgress >= 1.0f)
        {
            animationProgress = 1.0f;
            animationRunning = false;
        }
    }

    // Update forest animations
    updateForest();

    return !animationRunning && !closing;
}

float getBookAnimationProgress()
{
    return animationProgress;
}

// ============================================
// DRAW THE OPEN BOOK
// ============================================

void drawBookAnimation()
{
    glClearColor(0.96f, 0.90f, 0.76f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // desk: soft wooden stripes
    for (int i = 0; i < 9; i++)
        rect(0, i * 75.0f, 1000, i * 75.0f + 1.5f, Col(0.80f, 0.68f, 0.50f, 0.25f));

    // book board, shadow and page stack
    rect(46, 66, 972, 596, Col(0.30f, 0.22f, 0.12f, 0.35f));
    rect(38, 78, 962, 606, Col(0.35f, 0.08f, 0.12f));
    rect(38, 78, 962, 86, Col(0.27f, 0.05f, 0.09f));
    for (int i = 0; i < 4; i++)
        rect(52, 90 - i * 1.5f, 948, 91 - i * 1.5f, Col(0.88f, 0.82f, 0.66f));

    // left and right pages
    rect(50, 94, 500, 594, Col(0.99f, 0.95f, 0.82f));
    rect(500, 94, 950, 594, Col(0.99f, 0.96f, 0.86f));

    // gutter shading
    rectGradH(450, 94, 500, 594, Col(0, 0, 0, 0), Col(0.35f, 0.25f, 0.1f, 0.18f));
    rectGradH(500, 94, 550, 594, Col(0.35f, 0.25f, 0.1f, 0.2f), Col(0, 0, 0, 0));
    rect(498, 94, 502, 594, Col(0.78f, 0.68f, 0.5f));

    // the pop-up forest, its HUD and the cinematic overlay
    drawForest();

    // ---- the front cover flips over to the left ----
    float p = animationProgress;
    float flip = smooth(clamp01(p / 0.7f));
    if (flip < 1.0f)
    {
        float sx = std::cos(flip * PI);
        float edge = std::sin(flip * PI) * 9.0f;
        float xe = 500 + 450 * sx;

        // cast shadow on the page beneath the turning cover
        if (sx > 0.0f)
            rectGradH(xe, 94, xe + 70 * std::sin(flip * PI), 594, Col(0, 0, 0, 0.28f), Col(0, 0, 0, 0));
        else
            rectGradH(xe - 70 * std::sin(flip * PI), 94, xe, 594, Col(0, 0, 0, 0), Col(0, 0, 0, 0.22f));

        float quad[] = { 500, 94, xe, 94 - edge, xe, 594 + edge, 500, 594 };
        if (sx > 0.0f)
        {
            poly(quad, 4, Col(0.35f, 0.08f, 0.12f));
            if (sx > 0.5f)
            {
                float a = (sx - 0.5f) * 2.0f;
                // gold frame and title on the cover
                glColor4f(0.85f, 0.66f, 0.32f, a);
                glLineWidth(3.0f);
                glBegin(GL_LINE_LOOP);
                glVertex2f(500 + 450 * sx * 0.06f, 130); glVertex2f(500 + 450 * sx * 0.94f, 130);
                glVertex2f(500 + 450 * sx * 0.94f, 560); glVertex2f(500 + 450 * sx * 0.06f, 560);
                glEnd();
                glLineWidth(1.0f);
                textCentered(500 + 225 * sx, 400, "PAPERLAND", GLUT_BITMAP_TIMES_ROMAN_24, Col(0.98f, 0.84f, 0.48f, a));
                textCentered(500 + 225 * sx, 370, "THE LIVING BOOK", GLUT_BITMAP_HELVETICA_12, Col(0.95f, 0.80f, 0.50f, a));
            }
        }
        else
        {
            poly(quad, 4, Col(0.95f, 0.90f, 0.76f));   // inside of the cover
        }
    }
}
