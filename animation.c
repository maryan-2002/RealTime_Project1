#include <GL/glut.h>
#include "animation.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Data structure to hold player positions
typedef struct
{
    float posX;
    float posY;
} PlayerPosition;

PlayerPosition playerPositions[TEAMS_NUMBER][PLAYERS_FOR_EACH_TEAM];

// Pipe variables
int pipeFd[2];

// Function to initialize OpenGL
void initGL(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Bungee Jumping Simulation");
    initOpenGL();
}

// OpenGL initialization
void initOpenGL()
{
    glClearColor(1, 1, 1, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0); // Set coordinate system
}

// Function to initialize player positions
void initializePositions()
{
    float spacing = 0.6f;
    float playerSpacing = 0.1f;
    for (int teamIdx = 0; teamIdx < TEAMS_NUMBER; teamIdx++)
    {
        float baseX = -0.7f + teamIdx * spacing;
        for (int playerIdx = 0; playerIdx < PLAYERS_FOR_EACH_TEAM; playerIdx++)
        {
            playerPositions[teamIdx][playerIdx].posX = baseX + playerIdx * playerSpacing;
            playerPositions[teamIdx][playerIdx].posY = 0.8f;
        }
    }
}

// Display function for animation
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Render teams' players
    for (int teamIdx = 0; teamIdx < TEAMS_NUMBER; teamIdx++)
    {
        for (int playerIdx = 0; playerIdx < PLAYERS_FOR_EACH_TEAM; playerIdx++)
        {
            float playerX = playerPositions[teamIdx][playerIdx].posX;
            float playerY = playerPositions[teamIdx][playerIdx].posY;

            glColor3f(0.0f, 1.0f, 0.0f);
            glBegin(GL_POLYGON);
            for (int i = 0; i < 20; i++)
            {
                float theta = 2.0f * 3.1415926f * i / 20;
                float dx = 0.02f * cosf(theta);
                float dy = 0.02f * sinf(theta);
                glVertex2f(playerX + dx, playerY + dy);
            }
            glEnd();
        }
    }

    glutSwapBuffers();
}

// Timer function to update display at intervals
void timerFunc(int value)
{
    glutPostRedisplay();
    glutTimerFunc(1000, timerFunc, 0);
}

// Function to handle the jumping animation
void startJumping(player_message msg)
{
    playerPositions[msg.team_index][msg.player_index].posY = 1.2f;
    glutPostRedisplay();
    usleep(10000); // Simulate time delay
}

// Pipe reading function (non-blocking)
void readMessageFromPipe()
{
    player_message msg;

    // Make pipe non-blocking
    fcntl(pipeFd[0], F_SETFL, O_NONBLOCK);

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(pipeFd[0], &readfds);

    // Set timeout for select (e.g., 1 second)
    struct timeval timeout = {1, 0};

    int ret = select(pipeFd[0] + 1, &readfds, NULL, NULL, &timeout);
    if (ret > 0)
    {
        if (FD_ISSET(pipeFd[0], &readfds))
        {
            printf("HIIIIIIIIIIIIIII");
            ssize_t bytesRead = read(pipeFd[0], &msg, sizeof(player_message));
            if (bytesRead > 0)
            {
                printf("Received message - Team: %d, Player: %d, Type: %d, Time: %f\n",
                       msg.team_index, msg.player_index, msg.massageType, msg.time);

                if (msg.massageType == 0)
                {
                    startJumping(msg);
                }
            }
            else
            {
                printf("Error reading from pipe\n");
            }
        }
    }
    else if (ret == 0)
    {
        // Timeout, no data, continue
    }
    else
    {
        perror("Error with select");
    }
}

// Animation loop function to handle pipe checks and OpenGL rendering
void animationLoop()
{
    // Poll the pipe for messages
    readMessageFromPipe();

    // Call glutMainLoopEvent to process OpenGL events
    glutMainLoopEvent();
}