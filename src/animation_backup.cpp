
#include <GL/freeglut.h>
#include <cmath>
#include "../include/animation.h"

// ------------------------------------
// ANIMATION STATE
// ------------------------------------

static float animationProgress = 0.0f;
static bool animationRunning = false;
static float butterflyPhase = 0.0f;

static bool flowerBloomed[3] = { false, false, false };

const float PI = 3.1415926f;

// Smooth animation curve
static float smooth(float t)
{
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    return t * t * (3.0f - 2.0f * t);
}

// Give an animation element its own time window
static float stage(float progress, float start, float end)
{
    return smooth((progress - start) / (end - start));
}

// ------------------------------------
// BASIC SHAPES
// ------------------------------------

static void rectangle(
    float x1, float y1, float x2, float y2,
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

static void triangle(
    float x1, float y1,
    float x2, float y2,
    float x3, float y3,
    float r, float g, float b)
{
    glColor3f(r, g, b);

    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glVertex2f(x3, y3);
    glEnd();
}

static void circle(
    float x, float y, float radius,
    float r, float g, float b)
{
    glColor3f(r, g, b);

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);

    for (int i = 0; i <= 40; i++)
    {
        float angle = i * 2.0f * PI / 40.0f;

        glVertex2f(
            x + cos(angle) * radius,
            y + sin(angle) * radius
        );
    }

    glEnd();
}

// ------------------------------------
// POP-UP GROUND
// ------------------------------------

static void drawGround(float amount)
{
    if (amount <= 0.0f)
        return;

    glPushMatrix();

    // The paper rises from the center fold
    glTranslatef(500, 255, 0);
    glScalef(1.0f, amount, 1.0f);
    glTranslatef(-500, -255, 0);

    rectangle(
        255, 255, 745, 285,
        0.20f, 0.43f, 0.27f
    );

    // Front paper edge
    rectangle(
        255, 255, 745, 262,
        0.13f, 0.32f, 0.19f
    );

    glPopMatrix();
}

// ------------------------------------
// POP-UP MOUNTAINS
// ------------------------------------

static void drawMountain(
    float x, float baseY,
    float width, float height,
    float amount,
    float r, float g, float b)
{
    if (amount <= 0.0f)
        return;

    glPushMatrix();

    // Scale from the bottom edge
    glTranslatef(x, baseY, 0);
    glScalef(1.0f, amount, 1.0f);

    triangle(
        -width / 2, 0,
        width / 2, 0,
        0, height,
        r, g, b
    );

    glPopMatrix();
}

// ------------------------------------
// POP-UP TREES
// ------------------------------------

static void drawTree(
    float x, float baseY,
    float scale, float amount)
{
    if (amount <= 0.0f)
        return;

    float trunkHeight = 55.0f * scale;
    float canopyHeight = 85.0f * scale;

    glPushMatrix();

    glTranslatef(x, baseY, 0);

    // Trunk rises vertically from the page
    glPushMatrix();

    glScalef(1.0f, amount, 1.0f);

    rectangle(
        -7 * scale, 0,
        7 * scale, trunkHeight,
        0.40f, 0.22f, 0.12f
    );

    glPopMatrix();

    // Canopy unfolds after the trunk rises
    float canopy = stage(amount, 0.30f, 1.0f);

    if (canopy > 0.0f)
    {
        glPushMatrix();

        // Rotate around the top of the trunk
        glTranslatef(0, trunkHeight, 0);

        glRotatef(
            -90.0f * (1.0f - canopy),
            0, 0, 1
        );

        triangle(
            -38 * scale, 0,
            38 * scale, 0,
            0, canopyHeight,
            0.13f, 0.39f, 0.24f
        );

        glTranslatef(0, 25 * scale, 0);

        triangle(
            -29 * scale, 0,
            29 * scale, 0,
            0, canopyHeight * 0.75f,
            0.21f, 0.49f, 0.30f
        );

        glPopMatrix();
    }

    glPopMatrix();
}

// ------------------------------------
// FLOWERS
// ------------------------------------

static void drawFlower(float x, float y, int index)
{
    float bloom = stage(animationProgress, 0.76f, 0.90f);

    if (bloom <= 0.0f)
        return;

    float radius = flowerBloomed[index] ? 13.0f : 7.0f;
    radius *= bloom;

    circle(x, y, radius, 1.0f, 0.55f, 0.35f);
    circle(x, y, radius * 0.35f, 1.0f, 0.85f, 0.25f);
}

// ------------------------------------
// BUTTERFLIES
// ------------------------------------

static void drawButterfly(float x, float y)
{
    float flap = sin(butterflyPhase * 5.0f) * 5.0f;

    circle(
        x - 8, y + flap,
        8, 0.95f, 0.35f, 0.55f
    );

    circle(
        x + 8, y - flap,
        8, 0.65f, 0.35f, 0.85f
    );

    rectangle(
        x - 2, y - 7,
        x + 2, y + 7,
        0.25f, 0.15f, 0.12f
    );
}

// ------------------------------------
// COMPLETE FOREST
// ------------------------------------

static void drawForest()
{
    float ground = stage(animationProgress, 0.12f, 0.32f);
    float mountain1 = stage(animationProgress, 0.27f, 0.49f);
    float mountain2 = stage(animationProgress, 0.38f, 0.60f);

    float tree1 = stage(animationProgress, 0.45f, 0.70f);
    float tree2 = stage(animationProgress, 0.52f, 0.76f);
    float tree3 = stage(animationProgress, 0.59f, 0.83f);
    float tree4 = stage(animationProgress, 0.66f, 0.90f);

    // Sun appears gradually
    float sun = stage(animationProgress, 0.35f, 0.60f);

    if (sun > 0.0f)
    {
        circle(
            680, 415, 27 * sun,
            1.0f, 0.75f, 0.29f
        );
    }

    // Ground unfolds first
    drawGround(ground);

    // Mountains rise in separate layers
    drawMountain(
        390, 280, 340, 190,
        mountain1,
        0.43f, 0.55f, 0.42f
    );

    drawMountain(
        625, 280, 310, 160,
        mountain2,
        0.35f, 0.49f, 0.34f
    );

    // Trees pop up one after another
    drawTree(330, 280, 0.90f, tree1);
    drawTree(430, 280, 0.85f, tree2);
    drawTree(550, 280, 0.90f, tree3);
    drawTree(665, 280, 0.95f, tree4);

    // Flowers appear near the end
    drawFlower(375, 270, 0);
    drawFlower(510, 270, 1);
    drawFlower(710, 270, 2);

    // Butterflies start flying after the forest unfolds
    if (animationProgress > 0.88f)
    {
        float x1 = 420 + sin(butterflyPhase) * 65;
        float y1 = 390 + cos(butterflyPhase * 1.5f) * 25;

        float x2 = 620 + cos(butterflyPhase) * 40;
        float y2 = 350 + sin(butterflyPhase) * 20;

        drawButterfly(x1, y1);
        drawButterfly(x2, y2);
    }
}

// ------------------------------------
// ANIMATION CONTROLS
// ------------------------------------

void startBookAnimation()
{
    animationProgress = 0.0f;
    animationRunning = true;
    butterflyPhase = 0.0f;

    for (int i = 0; i < 3; i++)
        flowerBloomed[i] = false;
}

bool updateBookAnimation()
{
    if (animationRunning)
    {
        // Slower animation for a more natural unfolding
        animationProgress += 0.008f;

        if (animationProgress >= 1.0f)
        {
            animationProgress = 1.0f;
            animationRunning = false;
        }
    }

    // Butterflies keep moving after the forest is ready
    if (animationProgress > 0.88f)
        butterflyPhase += 0.035f;

    return !animationRunning;
}

float getBookAnimationProgress()
{
    return animationProgress;
}

// ------------------------------------
// DRAW THE OPEN BOOK
// ------------------------------------

void drawBookAnimation()
{
    glClearColor(0.96f, 0.90f, 0.76f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Book shadow
    rectangle(
        180, 135, 820, 165,
        0.55f, 0.42f, 0.28f
    );

    // Left and right pages
    rectangle(
        200, 170, 500, 470,
        0.96f, 0.88f, 0.68f
    );

    rectangle(
        500, 170, 800, 470,
        0.99f, 0.93f, 0.79f
    );

    // Center fold
    rectangle(
        496, 170, 504, 470,
        0.86f, 0.77f, 0.59f
    );

    // Forest unfolds progressively
    drawForest();

    // Cover disappears as the book opens
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

// ------------------------------------
// FLOWER INTERACTION
// ------------------------------------

bool handleForestClick(float x, float y)
{
    if (animationProgress < 0.90f)
        return false;

    if (y < 250 || y > 290)
        return false;

    if (x > 350 && x < 400)
    {
        flowerBloomed[0] = true;
        return true;
    }

    if (x > 485 && x < 535)
    {
        flowerBloomed[1] = true;
        return true;
    }

    if (x > 685 && x < 735)
    {
        flowerBloomed[2] = true;
        return true;
    }

    return false;
}
