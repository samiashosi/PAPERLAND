
#include <GL/freeglut.h>
#include <iostream>

#include "../include/menu.h"
#include "../include/animation.h"

// Current application screen
AppScreen currentScreen = AppScreen::MENU;

// Display callback
void display()
{
    drawMenu(currentScreen);
}

// Animation timer
void animationTimer(int value)
{
    if (currentScreen == AppScreen::STORY)
    {
        updateBookAnimation();

        glutPostRedisplay();

        // Continue updating the interactive forest
        glutTimerFunc(16, animationTimer, 0);
    }
}

// Window resizing
void reshape(int width, int height)
{
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
    if (handleMenuMouse(
        button,
        state,
        x,
        y,
        currentScreen
    ))
    {
        if (currentScreen == AppScreen::EXIT)
        {
            glutLeaveMainLoop();
            return;
        }
if (currentScreen == AppScreen::STORY)
{
    glutTimerFunc(16, animationTimer, 0);
}
        glutPostRedisplay();
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

    if (handleMenuKeyboard(key, currentScreen))
    {
        glutPostRedisplay();
    }
}

// Application entry point
int main(int argc, char** argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(
        GLUT_DOUBLE | GLUT_RGB
    );

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

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);

    std::cout
        << "================================\n"
        << "       PAPERLAND PROJECT\n"
        << "================================\n"
        << "Application started successfully!\n"
        << "Press Q to exit.\n";

    glutMainLoop();

    return 0;
}
