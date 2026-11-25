#include <GL/freeglut.h>
#include "core/menu.h"
#include <cmath>
#include <string>
#include "shared/book_animation.h"
#include "core/game.h"
#include "shared/audio.h"

// Window dimensions
const float WIDTH = 1000.0f;
const float HEIGHT = 650.0f;

// Colors
void setColor(float r, float g, float b)
{
    glColor3f(r, g, b);
}

// Draw rectangle
void rectangle(float x, float y, float w, float h)
{
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
    glEnd();
}

// Draw circle
void circle(float x, float y, float radius)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);

    for (int i = 0; i <= 60; i++)
    {
        float angle = 2.0f * 3.1415926f * i / 60.0f;

        glVertex2f(
            x + cos(angle) * radius,
            y + sin(angle) * radius
        );
    }

    glEnd();
}

// Draw text
void drawText(
    float x,
    float y,
    const std::string& text,
    void* font = GLUT_BITMAP_HELVETICA_18
)
{
    glRasterPos2f(x, y);

    for (char c : text)
    {
        glutBitmapCharacter(font, c);
    }
}

// Draw a triangle
void triangle(
    float x1, float y1,
    float x2, float y2,
    float x3, float y3
)
{
    glBegin(GL_TRIANGLES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glVertex2f(x3, y3);
    glEnd();
}

// Draw a simple paper tree
void drawTree(float x, float y, float scale)
{
    // Tree trunk
    setColor(0.39f, 0.22f, 0.13f);
    rectangle(
        x - 7 * scale,
        y,
        14 * scale,
        45 * scale
    );

    // Tree layers
    setColor(0.12f, 0.35f, 0.23f);
    triangle(
        x, y + 110 * scale,
        x - 43 * scale, y + 35 * scale,
        x + 43 * scale, y + 35 * scale
    );

    setColor(0.19f, 0.46f, 0.29f);
    triangle(
        x, y + 145 * scale,
        x - 35 * scale, y + 75 * scale,
        x + 35 * scale, y + 75 * scale
    );
}

// Draw a small decorative star
void drawStar(float x, float y, float radius)
{
    setColor(0.88f, 0.66f, 0.27f);

    for (int i = 0; i < 4; i++)
    {
        float angle = i * 3.1415926f / 4.0f;

        float x2 = x + cos(angle) * radius;
        float y2 = y + sin(angle) * radius;

        glLineWidth(2.0f);

        glBegin(GL_LINES);
            glVertex2f(x - cos(angle) * radius, y - sin(angle) * radius);
            glVertex2f(x2, y2);
        glEnd();
    }
}

// Background
void drawBackground()
{
    setColor(0.94f, 0.87f, 0.72f);
    rectangle(0, 0, WIDTH, HEIGHT);

    // Inner parchment area
    setColor(0.98f, 0.94f, 0.83f);
    rectangle(18, 18, 964, 614);

    // Decorative borders
    glLineWidth(2.0f);
    setColor(0.72f, 0.53f, 0.29f);

    glBegin(GL_LINE_LOOP);
        glVertex2f(30, 30);
        glVertex2f(970, 30);
        glVertex2f(970, 620);
        glVertex2f(30, 620);
    glEnd();

    drawStar(65, 580, 9);
    drawStar(935, 580, 9);
    drawStar(65, 70, 9);
    drawStar(935, 70, 9);
}

// Draw the book cover
void drawBook()
{
    // Shadow
    setColor(0.42f, 0.32f, 0.25f);
    rectangle(118, 76, 455, 510);

    // Pages
    setColor(0.88f, 0.81f, 0.65f);
    rectangle(103, 87, 455, 510);

    // Book cover
    setColor(0.32f, 0.12f, 0.14f);
    rectangle(90, 95, 455, 510);

    // Spine
    setColor(0.22f, 0.07f, 0.09f);
    rectangle(90, 95, 55, 510);

    // Golden cover frame
    glLineWidth(3.0f);
    setColor(0.85f, 0.66f, 0.32f);

    glBegin(GL_LINE_LOOP);
        glVertex2f(165, 125);
        glVertex2f(520, 125);
        glVertex2f(520, 575);
        glVertex2f(165, 575);
    glEnd();

    // Inner frame
    glLineWidth(1.5f);

    glBegin(GL_LINE_LOOP);
        glVertex2f(177, 137);
        glVertex2f(508, 137);
        glVertex2f(508, 563);
        glVertex2f(177, 563);
    glEnd();

    // Cover title
    setColor(0.98f, 0.84f, 0.48f);

    drawText(
        218, 510,
        "PAPERLAND",
        GLUT_BITMAP_TIMES_ROMAN_24
    );

    drawText(
        240, 478,
        "THE LIVING BOOK",
        GLUT_BITMAP_HELVETICA_12
    );

    // Forest illustration
    // Mountains
    setColor(0.27f, 0.37f, 0.29f);

    triangle(195, 270, 335, 440, 475, 270);

    setColor(0.37f, 0.47f, 0.32f);

    triangle(300, 270, 420, 420, 510, 270);

    // Forest ground
    setColor(0.16f, 0.32f, 0.19f);
    rectangle(180, 185, 325, 90);

    // Trees
    drawTree(235, 190, 0.65f);
    drawTree(320, 190, 0.85f);
    drawTree(405, 190, 0.72f);
    drawTree(485, 190, 0.58f);

    // Golden stars
    drawStar(230, 365, 8);
    drawStar(465, 390, 7);
    drawStar(350, 345, 6);

    // Small decorative flowers
    setColor(0.94f, 0.68f, 0.42f);
    circle(250, 175, 6);
    circle(370, 165, 6);
    circle(455, 178, 6);

    setColor(0.96f, 0.89f, 0.68f);
    circle(250, 175, 2);
    circle(370, 165, 2);
    circle(455, 178, 2);

    // Author label
    setColor(0.91f, 0.77f, 0.48f);
    drawText(
        240, 150,
        "A WORLD BETWEEN PAGES",
        GLUT_BITMAP_HELVETICA_12
    );

    // Spine decoration
    setColor(0.85f, 0.66f, 0.32f);
    glLineWidth(2.0f);

    glBegin(GL_LINES);
        glVertex2f(117, 115);
        glVertex2f(117, 585);
    glEnd();
}

// Draw one menu button
void drawButton(
    float x,
    float y,
    float w,
    float h,
    const std::string& label
)
{
    // Button shadow
    setColor(0.67f, 0.54f, 0.37f);
    rectangle(x + 4, y - 4, w, h);

    // Main button
    setColor(0.37f, 0.18f, 0.16f);
    rectangle(x, y, w, h);

    // Golden outline
    setColor(0.86f, 0.69f, 0.38f);
    glLineWidth(2.0f);

    glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
    glEnd();

    // Button label
    setColor(0.99f, 0.91f, 0.73f);

    drawText(
        x + w / 2 - label.length() * 5.0f,
        y + h / 2 - 5,
        label
    );
}

// Main menu screen
void drawMainMenu()
{
    drawBackground();
    drawBook();

    // Heading
    setColor(0.32f, 0.17f, 0.14f);

    drawText(
        640, 525,
        "WELCOME TO",
        GLUT_BITMAP_HELVETICA_18
    );

    setColor(0.52f, 0.25f, 0.17f);

    drawText(
        640, 485,
        "PAPERLAND",
        GLUT_BITMAP_TIMES_ROMAN_24
    );

    setColor(0.47f, 0.39f, 0.29f);

    drawText(
        640, 450,
        "Every page holds a new adventure.",
        GLUT_BITMAP_HELVETICA_12
    );

    // Buttons
    drawButton(650, 345, 275, 55, "START");
    drawButton(650, 265, 275, 55, "CHAPTERS");
    drawButton(650, 185, 275, 55, "ABOUT");
    drawButton(650, 105, 275, 55, "EXIT");

    setColor(0.57f, 0.46f, 0.33f);

    drawText(
        665, 55,
        "Click a button or use your keyboard.",
        GLUT_BITMAP_HELVETICA_12
    );
}

// Secondary screen
void drawSecondaryScreen(
    const std::string& heading,
    const std::string& line1,
    const std::string& line2
)
{
    drawBackground();

    setColor(0.36f, 0.17f, 0.14f);

    drawText(
        365, 490,
        heading,
        GLUT_BITMAP_TIMES_ROMAN_24
    );

    setColor(0.43f, 0.35f, 0.26f);

    drawText(280, 410, line1);
    drawText(280, 370, line2);

    drawButton(350, 190, 300, 60, "BACK TO MENU");

    setColor(0.52f, 0.40f, 0.29f);

    drawText(
        390, 130,
        "Press ESC to return to the main menu.",
        GLUT_BITMAP_HELVETICA_12
    );
}

// Render selected screen
void drawMenu(AppScreen screen)
{
    glClear(GL_COLOR_BUFFER_BIT);

    switch (screen)
    {
        case AppScreen::MENU:
            drawMainMenu();
            break;

        case AppScreen::CHAPTERS:
            drawSecondaryScreen(
                "OUR CHAPTERS",
                "Chapter 1: The Enchanted Forest",
                "More magical adventures are coming soon."
            );
            break;

        case AppScreen::ABOUT:
            drawSecondaryScreen(
                "ABOUT PAPERLAND",
                "PAPERLAND is an interactive animated storybook.",
                "Explore magical worlds through paper and animation."
            );
            break;

       case AppScreen::STORY:
    drawBookAnimation();
    break;
    }

    glutSwapBuffers();
}

// Mouse interaction
bool handleMenuMouse(
    int button,
    int state,
    int x,
    int y,
    AppScreen& screen
)
{
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN)
        return false;

    // Convert mouse coordinates
    float mouseX = static_cast<float>(x);
    float mouseY = HEIGHT - static_cast<float>(y);

    // Animated book screen: back button or forest interaction
    if (screen == AppScreen::STORY)
    {
        if (game::backButtonHit(mouseX, mouseY))
        {
            startBookClose();      // the book closes, then the menu returns
            return true;
        }

        return game::mouseClick(mouseX, mouseY);
    }

// Secondary screens: back button
if (screen != AppScreen::MENU)
{
    if (
        mouseX >= 350 && mouseX <= 650 &&
        mouseY >= 190 && mouseY <= 250
    )
    {
        screen = AppScreen::MENU;
        return true;
    }

    return false;
}

    // START
    // START
if (
    mouseX >= 650 && mouseX <= 925 &&
    mouseY >= 345 && mouseY <= 400
)
{
    game::selectChapter(1);
    startBookAnimation();
    screen = AppScreen::STORY;
    return true;
}

    // CHAPTERS
    if (
        mouseX >= 650 && mouseX <= 925 &&
        mouseY >= 265 && mouseY <= 320
    )
    {
        screen = AppScreen::CHAPTERS;
        return true;
    }

    // ABOUT
    if (
        mouseX >= 650 && mouseX <= 925 &&
        mouseY >= 185 && mouseY <= 240
    )
    {
        screen = AppScreen::ABOUT;
        return true;
    }

    // EXIT
   // EXIT
if (
    mouseX >= 650 && mouseX <= 925 &&
    mouseY >= 105 && mouseY <= 160
)
{
    screen = AppScreen::EXIT;
    return true;
}

    return false;
}

// Keyboard interaction
bool handleMenuKeyboard(
    unsigned char key,
    AppScreen& screen
)
{
    if (key == 27)
    {
        if (screen != AppScreen::MENU)
        {
            screen = AppScreen::MENU;
            return true;
        }

        return false;
    }

    if ((key == '1' || key == 13) && screen == AppScreen::MENU)
    {
        game::selectChapter(1);
        startBookAnimation();
        screen = AppScreen::STORY;
        return true;
    }

    if (key == '2' && screen == AppScreen::MENU)
    {
        screen = AppScreen::CHAPTERS;
        return true;
    }

    if (key == '3' && screen == AppScreen::MENU)
    {
        screen = AppScreen::ABOUT;
        return true;
    }

    return false;
}
