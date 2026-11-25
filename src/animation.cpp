
#include <GL/freeglut.h>

#include "../include/animation.h"
#include "../include/forest.h"

// ============================================
// BOOK ANIMATION STATE
// ============================================

static float animationProgress = 0.0f;
static bool animationRunning = false;


// ============================================
// SMOOTH ANIMATION
// ============================================

static float smooth(float t)
{
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    return t * t * (3.0f - 2.0f * t);
}


// ============================================
// BASIC RECTANGLE
// ============================================

static void rectangle(
    float x1, float y1,
    float x2, float y2,
    float r, float g, float b)
{
    glColor3f(r, g, b);

    glBegin(GL_QUADS);

    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);

    glEnd();
}


// ============================================
// START BOOK ANIMATION
// ============================================

void startBookAnimation()
{
    animationProgress = 0.0f;
    animationRunning = true;

    // Reset the forest animation too
    startForest();
}


// ============================================
// UPDATE BOOK ANIMATION
// ============================================

bool updateBookAnimation()
{
    if (animationRunning)
    {
        animationProgress += 0.008f;

        if (animationProgress >= 1.0f)
        {
            animationProgress = 1.0f;
            animationRunning = false;
        }
    }

    // Update forest animations
    updateForest();

    return !animationRunning;
}


// ============================================
// GET ANIMATION PROGRESS
// ============================================

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

    // Book shadow
    rectangle(
        180, 135, 820, 165,
        0.55f, 0.42f, 0.28f
    );

    // Left page
    rectangle(
        200, 170, 500, 470,
        0.96f, 0.88f, 0.68f
    );

    // Right page
    rectangle(
        500, 170, 800, 470,
        0.99f, 0.93f, 0.79f
    );

    // Center fold
    rectangle(
        496, 170, 504, 470,
        0.86f, 0.77f, 0.59f
    );

    // Draw forest from forest.cpp
    drawForest();

    // Book cover disappears as the book opens
    float eased = smooth(animationProgress);
    float coverWidth = 300.0f * (1.0f - eased);

    if (coverWidth > 1.0f)
    {
        rectangle(
            500 - coverWidth, 160,
            500, 480,
            0.35f, 0.08f, 0.12f
        );
    }

    // Chapter title
    if (animationProgress >= 1.0f)
    {
        glColor3f(0.25f, 0.15f, 0.10f);
        glRasterPos2f(345, 80);

        const char* title = "THE ENCHANTED FOREST";

        for (int i = 0; title[i] != '\0'; i++)
        {
            glutBitmapCharacter(
                GLUT_BITMAP_TIMES_ROMAN_24,
                title[i]
            );
        }
    }

    // Back button
    rectangle(
        50, 35, 230, 85,
        0.35f, 0.12f, 0.12f
    );

    glColor3f(1.0f, 0.88f, 0.65f);
    glRasterPos2f(78, 53);

    const char* backText = "BACK TO MENU";

    for (int i = 0; backText[i] != '\0'; i++)
    {
        glutBitmapCharacter(
            GLUT_BITMAP_HELVETICA_18,
            backText[i]
        );
    }
}
