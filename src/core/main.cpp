
#include <GL/freeglut.h>
#include <iostream>
#include <cstdlib>

#include "core/menu.h"
#include "shared/book_animation.h"
#include "core/game.h"
#include "shared/paper.h"
#include "shared/audio.h"

// Current application screen
AppScreen currentScreen = AppScreen::MENU;

// Display callback
void display()
{
    drawMenu(currentScreen);
}

// Convert window pixels into the logical 1000 x 650 space
static void toLogical(int x, int y, int& lx, int& ly)
{
    lx = (int)(x * 1000.0f / pl::winW());
    ly = (int)(y * 650.0f / pl::winH());
}

// Frame timer: runs the whole time, updates only the animated screen
void animationTimer(int value)
{
    pl::tick();

    if (currentScreen == AppScreen::STORY)
    {
        updateBookAnimation();

        if (isBookClosed())
        {
            // "BACK TO BOOK" finished: return to the closed-book menu
            currentScreen = AppScreen::MENU;
            glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
        }

        glutPostRedisplay();
    }

    glutTimerFunc(16, animationTimer, 0);
}

// Window resizing
void reshape(int width, int height)
{
    pl::setWindowSize(width, height);
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, 1000, 0, 650, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Mouse callback
void mouse(int button, int state, int x, int y)
{
    int lx, ly;
    toLogical(x, y, lx, ly);

    if (handleMenuMouse(
        button,
        state,
        lx,
        ly,
        currentScreen
    ))
    {
        if (currentScreen == AppScreen::EXIT)
        {
            glutLeaveMainLoop();
            return;
        }
        glutPostRedisplay();
    }
}

// Mouse movement (hover cursor + button highlights)
void mouseMotion(int x, int y)
{
    int lx, ly;
    toLogical(x, y, lx, ly);

    if (currentScreen == AppScreen::STORY)
    {
        game::mouseMove((float)lx, (float)(650 - ly));
    }
}

// Keyboard callback
void keyboard(unsigned char key, int x, int y)
{
    if (key == 'q' || key == 'Q')
    {
        glutLeaveMainLoop();
        return;
    }

    if (currentScreen == AppScreen::STORY)
    {
        if (key == 27)                       // ESC: back to the menu at once
        {
            audio::stopMusic();
            glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
            pl::setPaused(false);
            currentScreen = AppScreen::MENU;
            glutPostRedisplay();
            return;
        }

        if (key == 'r' || key == 'R')        // restart Chapter 1
        {
            startBookAnimation();
            return;
        }

        game::key(key);
        return;
    }

    if (handleMenuKeyboard(key, currentScreen))
    {
        glutPostRedisplay();
    }
}

// Arrow keys
void special(int key, int x, int y)
{
    if (currentScreen == AppScreen::STORY)
    {
        game::specialKey(key);
    }
}

// Application entry point
int main(int argc, char** argv)
{
    glutInit(&argc, argv);

    // Ask for anti-aliasing when the driver offers it (smoother paper edges)
    glutSetOption(GLUT_MULTISAMPLE, 4);
    glutInitDisplayMode(
        GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE
    );

    if (!glutGet(GLUT_DISPLAY_MODE_POSSIBLE))
    {
        glutInitDisplayMode(
            GLUT_DOUBLE | GLUT_RGB
        );
    }

    glutInitWindowSize(1000, 650);
    glutInitWindowPosition(150, 80);

    glutCreateWindow(
        "PAPERLAND - The Living Book"
    );

    glutSetOption(
        GLUT_ACTION_ON_WINDOW_CLOSE,
        GLUT_ACTION_GLUTMAINLOOP_RETURNS
    );

    glClearColor(
        0.98f, 0.94f, 0.83f, 1.0f
    );

    // Alpha blending is used by shadows, glows and particles
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(mouseMotion);
    glutMotionFunc(mouseMotion);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);

    game::init();
    audio::init();
    std::atexit(audio::shutdown);

    std::cout
        << "================================\n"
        << "       PAPERLAND PROJECT\n"
        << "================================\n"
        << "Application started successfully!\n"
        << "Chapter 1 keys: SPACE pause | H hint | S skip | M sound | R restart | ESC menu | Q quit\n";

    glutTimerFunc(16, animationTimer, 0);
    glutMainLoop();

    return 0;
}
