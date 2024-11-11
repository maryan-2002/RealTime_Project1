#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <GL/glut.h>
#include "player.h"

#include "game.h"
#include "referee.h"
#include "animation.h"
#include "spinningsquare.h"  // Include spinningsquare.h to use spin_square function


pid_t openGL_pid; // Global variable

pid_t players_id[TEAMS_NUMBER][PLAYERS_FOR_EACH_TEAM];
int flag_jump_team[TEAMS_NUMBER];
pid_t referee_pid; // Variable to hold referee process ID
const char *team_names[];
int scores[TEAMS_NUMBER][PLAYERS_FOR_EACH_TEAM]; // Score array for players
float player_energy[TEAMS_NUMBER][PLAYERS_FOR_EACH_TEAM];
int pipes[TEAMS_NUMBER][2];
int pipestoref[TEAMS_NUMBER][2];
int pipes_animation[TEAMS_NUMBER][2];
int pipes_animationte[2];
int pipe_myanimation[2];
//int max_score, game_duration;


pid_t openGL_pid; // Global variable
pid_t ref[TEAMS_NUMBER];


void initGraphics(int argc, char *argv[]); // Add the function prototype

void initGame(int argc, char *argv[], GameSettings *settings)
{
    // Game setup
    // printf("Enter the maximum score to end the game: ");
    // scanf("%d", &max_score);
    // printf("Enter the maximum game duration in seconds: ");
    // scanf("%d", &game_duration);
    if (pipe(pipes_animationte) == -1)
    {
        perror("pipe failed");
        exit(1);
    }
    
        // Create the Myanimation pipe
    if (pipe(pipe_myanimation) == -1) {
            perror("Failed to create pipe_myanimation");
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
        if (pipe(pipestoref[team]) == -1)
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
            player_energy[team][player_id]= INITIAL_ENERGY;
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
        referee_process(&settings);
        exit(0);
    }

        openGL_pid = fork();
    if (openGL_pid == 0)
    {
        initGraphics(argc, argv); // Initialize and start the OpenGL graphics
        exit(0);
    }

    sleep(&settings->game_duration);
    printf("Game over! Time's up!\n");
    hii();
    end_game();

    sleep(100000000);    
}
void hii(){

    printf("hiiiiii");
}

void loadSettings(const char* filename, GameSettings *settings) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Process each line as needed, e.g., parse key-value pairs
        char key[50];
        int value;
        if (strcmp(key, "gameDuration") == 0) {
                settings->game_duration = value;
            } else if (strcmp(key, "gameScore") == 0) {
                settings->max_score = value;
            }
    }
    fclose(file);
}

int main(int argc, char *argv[])
{
    if (argc < 2){
        printf("Usage: %s <settings_file>\n", argv[0]);
        return 0;
    }
    GameSettings settings = {0};
    loadSettings(argv[1], &settings);
    printf("Game Settings:\n");
    printf("Game Duration: %d\n", settings.game_duration);
    printf("Game Score: %d\n", settings.max_score);
    initGame(argc, argv, &settings); // Initialize and start the game
    return 0;
}


