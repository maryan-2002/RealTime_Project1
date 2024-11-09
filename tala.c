#include <GL/glut.h>
#include "animation.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

typedef struct
{
    float posX;
    float posY;
} PlayerPosition;

PlayerPosition playerPositions[TEAMS_NUMBER][PLAYERS_FOR_EACH_TEAM];

// Pipe variables
int pipeFd[2];

void initGL(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Bungee Jumping Simulation");
    initOpenGL();
}

void initOpenGL()
{
    glClearColor(1, 1, 1, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0); // Set coordinate system
}
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

            printf(" the   %d %d %f  %f \n", playerIdx, teamIdx, playerPositions[teamIdx][playerIdx].posX, playerPositions[teamIdx][playerIdx].posY);
        }
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.6f, 0.3f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(-0.8f, 0.8f);
    glVertex2f(0.8f, 0.8f);
    glVertex2f(0.8f, 0.75f);
    glVertex2f(-0.8f, 0.75f);
    glEnd();

    for (int teamIdx = 0; teamIdx < TEAMS_NUMBER; teamIdx++)
    {
        for (int playerIdx = 0; playerIdx < PLAYERS_FOR_EACH_TEAM; playerIdx++)
        {
            printf(" the   %d %d %f  %f \n", playerIdx, teamIdx, playerPositions[teamIdx][playerIdx].posX, playerPositions[teamIdx][playerIdx].posY);

            float playerX = playerPositions[teamIdx][playerIdx].posX;
            float playerY = playerPositions[teamIdx][playerIdx].posY;

            glBegin(GL_POLYGON);
            glColor3f(0.0f, 1.0f, 0.0f);
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

void timerFunc(int value)
{
    glutPostRedisplay();
    glutTimerFunc(1000, timerFunc, 0);
}

void startJumping(player_message msg)
{

    float targetPosY = 0.8f;                     // Start at 0.8
    float maxHeight = 1.2f;                      // The maximum height reached
    float velocity = sqrt(2 * 9.8f * maxHeight); // Initial velocity based on max height
    float gravity = -9.8f;                       // Gravity constant (meters per second squared)
    float timeStep = 0.01f;                      // Time step for the simulation
    float timeElapsed = 0.0f;                    // Time since jump started
    float jumpTime = msg.time;                   // Time for the entire jump (received in the message)

    // Ensure we are jumping upwards until we reach the peak
    playerPositions[msg.team_index][msg.player_index].posY = 1.2f;
    printf(" the   %d %d %f  %f \n", msg.team_index, msg.player_index, playerPositions[msg.team_index][msg.player_index].posX, playerPositions[msg.team_index][msg.player_index].posY);
    printf("HHHHHHIII \n");
    // Position calculation based on velocity and gravity
    // float height = targetPosY + velocity * timeElapsed + 0.5f * gravity * timeElapsed * timeElapsed;

    // if (height > maxHeight)
    // {
    //     velocity = -velocity; // Reverse direction when reaching maximum height
    //     height = maxHeight;
    // }

    // playerPositions[msg.team_index][msg.player_index].posY = height;

    // timeElapsed += timeStep;
    // usleep(10000);       // Sleep to simulate the passage of time
    glutPostRedisplay();

    // Simulate stabilization time after the jump
    usleep((int)((msg.time - timeElapsed) * 1000000)); // Sleep for the remaining stabilization time
}

void readMessageFromPipe()
{
    player_message msg;
    while (1)
    {
        ssize_t bytesRead = read(pipes_animationte[0], &msg, sizeof(player_message));

        if (bytesRead > 0)
        {
            printf("Animation :Received message - Team: %d, Player: %d, Massage Type: %d, Time: %f\n",
                   msg.team_index, msg.player_index, msg.massageType, msg.time);

            if (msg.massageType == 0)
            {
                startJumping(msg);
            }
        }
        else if (bytesRead == 0)
        {
            printf("No data in the pipe\n");
        }
        else
        {
            perror("Error reading from pipe");
        }
    }
    // for (int team_index = 0; team_index < TEAMS_NUMBER; team_index++)
    // {
    //     pid_t pid = fork();
    //     if (pid < 0)
    //     {
    //         perror("fork failed");
    //         exit(1);
    //     }
    //     else if (pid == 0)
    //     {
    //         readMessageFromTeamPipe(team_index);
    //         exit(0);
    //     }
    // }
}

void readMessageFromTeamPipe(int team_index)
{

    player_message msg;

    while (1)
    {
        ssize_t bytesRead = read(pipes_animation[team_index][0], &msg, sizeof(player_message));

        if (bytesRead > 0)
        {
            printf("Animation :Received message - Team: %d, Player: %d, Massage Type: %d, Time: %f\n",
                   msg.team_index, msg.player_index, msg.massageType, msg.time);

            if (msg.massageType == 0)
            {
                startJumping(msg);
            }
        }
        else if (bytesRead == 0)
        {
            printf("No data in the pipe\n");
        }
        else
        {
            perror("Error reading from pipe");
        }
    }
}


