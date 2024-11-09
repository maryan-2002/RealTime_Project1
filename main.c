#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <GL/glut.h>
#include "player.h"
#include "referee.h"
#include "game.h"
#include "animation.h"

pid_t players_id[TEAMS_NUMBER][PLAYERS_FOR_EACH_TEAM];
int flag_jump_team[TEAMS_NUMBER];
pid_t referee_pid; // Variable to hold referee process ID
const char *team_names[];
int scores[TEAMS_NUMBER][PLAYERS_FOR_EACH_TEAM]; // Score array for players
int player_energy[TEAMS_NUMBER][PLAYERS_FOR_EACH_TEAM];
int pipes[TEAMS_NUMBER][2];
int pipes_animation[TEAMS_NUMBER][2];
int pipes_animationte[2];

pid_t openGL_pid; // Global variable
pid_t ref[TEAMS_NUMBER];
void initGame(int argc, char **argv)
{
    // Game setup
    int max_score, game_duration;
    printf("Enter the maximum score to end the game: ");
    scanf("%d", &max_score);
    printf("Enter the maximum game duration in seconds: ");
    scanf("%d", &game_duration);
    if (pipe(pipes_animationte) == -1)
    {
        perror("pipe failed");
        exit(1);
    }
    // Create pipes for each team
    for (int team = 0; team < TEAMS_NUMBER; team++)
    {
        if (pipe(pipes[team]) == -1)
        {
            perror("pipe failed");
            exit(1);
        }
        if (pipe(pipes_animation[team]) == -1)
        {
            perror("pipe failed");
            exit(1);
        }
    }

    // Start player processes for each team
    for (int team = 0; team < TEAMS_NUMBER; team++)
    {
        for (int player_id = 0; player_id < PLAYERS_FOR_EACH_TEAM; player_id++)
        {
            pid_t pid = fork();
            players_id[team][player_id] = pid;

            if (pid < 0)
            {
                perror("fork failed");
                exit(1);
            }
            else if (pid == 0)
            {
                player_process(team, player_id);
                exit(0);
            }
        }
    }

    // Create referee process
    referee_pid = fork();
    if (referee_pid == 0)
    {
        referee_process(max_score);
        exit(0);
    }

    openGL_pid = fork(); // Global variable
    if (openGL_pid == 0)
    {
        initializePositions();

        // Create a pipe
        if (pipe(pipeFd) == -1)
        {
            perror("pipe failed");
            exit(1);
        }
        readMessageFromPipe();
        // Initialize OpenGL and run the simulation
        initGL(argc, argv);
        glutDisplayFunc(display);
        glutTimerFunc(1000, timerFunc, 0);

        // Animation loop
        while (1)
        {
            // Poll for messages and update display
            animationLoop();
        }
    }

    sleep(game_duration);
    printf("Game over! Time's up!\n");

    // Terminate all player processes
    terminate_players(players_id);

    // Terminate the referee process
    kill(referee_pid, SIGTERM);

    // Wait for all processes to finish
    for (int i = 0; i < TEAMS_NUMBER * PLAYERS_FOR_EACH_TEAM + 1; i++)
    {
        wait(NULL);
    }
}

int main(int argc, char **argv)
{
    initGame(argc, argv); // Initialize and start the game
    return 0;
}
