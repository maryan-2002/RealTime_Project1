#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>
#include "game.h"

#define TEAMS_NUMBER 3               
#define PLAYERS_FOR_EACH_TEAM 3 
#define INITIAL_ENERGY 100 

pid_t players_id[TEAMS_NUMBER][PLAYERS_FOR_EACH_TEAM];
int flag_jump_team[TEAMS_NUMBER];
pid_t referee_pid;  // Variable to hold referee process ID
const char *team_names[] = {"Team 1", "Team 2", "Team 3"};
int scores[TEAMS_NUMBER][PLAYERS_FOR_EACH_TEAM] = {0};  // Score array for players
int player_energy[TEAMS_NUMBER][PLAYERS_FOR_EACH_TEAM]; 

// Pipes for communication between players and referee
int pipes[TEAMS_NUMBER][2]; // Each player will write to a pipe that the referee reads

// Signal handlers
void handle_sigstop(int signum) {}
void handle_sigcont(int signum) {}

void stabilize(int team_id, int player_id) {
    double energy_factor = player_energy[team_id][player_id] / (double)INITIAL_ENERGY;
    double stabilization_time = (rand() % 5 + 2) / energy_factor; // Random time calculation
    printf("%s Player %d will stabilize in %.2f seconds \n", team_names[team_id], player_id + 1, stabilization_time);
    sleep(stabilization_time);
    printf("%s Player %d stabilized after %.2f seconds.\n", team_names[team_id], player_id + 1, stabilization_time);

    // Send stabilization time through pipe to the referee
    write(pipes[team_id][1], &stabilization_time, sizeof(stabilization_time));
}

// Pull up function simulating the time it takes for two players to pull the jumper up
void pull_up(int team_id, int player_id) {
    double energy_factor = (player_energy[team_id][player_id] + player_energy[team_id][2]) / (2.0 * INITIAL_ENERGY);
    double pulling_time = (rand() % 5 + 5) / energy_factor;  // Random time between 10 and 15 divided by energy factor
    sleep(pulling_time);
    write(pipes[team_id][1], &pulling_time, sizeof(pulling_time));
    printf("%s team members pulled Player 1 up in %.2f seconds.\n", team_names[team_id], pulling_time);
}


// Player process function
void player_process(int team_id, int player_id) {
    signal(SIGTSTP, handle_sigstop);
    signal(SIGCONT, handle_sigcont);

    while (1) {
        pause();  // Wait for the referee's signal to jump
        if(!flag_jump_team[team_id]){
            printf("Player %d in %s jumped!\n", player_id + 1, team_names[team_id]);
            stabilize(team_id, player_id);
        }
        else{

           

        }
        kill(getpid(), SIGTSTP);  // Immediately pause after jumping and pull
    }
}

// Referee process function
void referee_process(GameSettings settings) {
    int max_score=settings.max_score;
    int team_index = 0;
    int player_index = 0;
    double stabilization_time;
    double pulling_time;

    while (1) {
        sleep(1);  // Wait a moment before each jump command

        // Referee directs the current player to jump
        printf("\nReferee: %s Player %d, jump!\n", team_names[team_index], player_index + 1);
        kill(players_id[team_index][player_index], SIGCONT);  // Allow current player to jump

        // Read stabilization time from the pipe
        read(pipes[team_index][0], &stabilization_time, sizeof(stabilization_time));
        printf("%s Player %d stabilized in %.2f seconds.\n", team_names[team_index], player_index + 1, stabilization_time);
         // Send SIGCONT to two other players in the team to assist
        for (int i = 1; i <= 2; i++) {
            int puller_index = (player_index + i) % PLAYERS_FOR_EACH_TEAM;
            kill(players_id[team_index][puller_index], SIGCONT);  // Signal teammates to assist
        }
        read(pipes[team_index][0], &stabilization_time, sizeof(stabilization_time));
        printf("%s Player %d stabilized in %.2f seconds.\n", team_names[team_index], player_index + 1, stabilization_time);

        read(pipes[team_index][0], &stabilization_time, sizeof(stabilization_time));
        printf("%s Player %d stabilized in %.2f seconds.\n", team_names[team_index], player_index + 1, stabilization_time);

        // Move to the next player in round-robin fashion
        team_index = (team_index + 1) % TEAMS_NUMBER;
        if (team_index == 0) {
            player_index = (player_index + 1) % PLAYERS_FOR_EACH_TEAM;
        }
    }
}

int main() {
    // int max_score;
    // printf("Enter the maximum score to end the game: ");
    // scanf("%d", &max_score);

    // int game_duration;
    // printf("Enter the maximum game duration in seconds: ");
    // scanf("%d", &game_duration);

    // Create pipes for each team
    for (int team = 0; team < TEAMS_NUMBER; team++) {
        if (pipe(pipes[team]) == -1) {
            perror("pipe failed");
            exit(1);
        }
    }

    // Start player processes for each team
    for (int team = 0; team < TEAMS_NUMBER; team++) {  
        printf("Starting processes for %s\n", team_names[team]);
        for (int player_id = 0; player_id < PLAYERS_FOR_EACH_TEAM; player_id++) {
            pid_t pid = fork();
            players_id[team][player_id] = pid;
            player_energy[team][player_id] = INITIAL_ENERGY;

            if (pid < 0) {
                perror("fork failed");
                exit(1);
            } else if (pid == 0) {
                printf("Player %d in %s started.\n", player_id + 1, team_names[team]);
                player_process(team, player_id);
                exit(0);
            }
        }
    }

    // Create referee process
    referee_pid = fork();
    if (referee_pid == 0) {
        referee_process(GameSettings.max_score);
        exit(0);
    }

    // Allow game to run for the specified duration
    sleep(GameSettings.game_duration);
    printf("Game over! Time's up!\n");

    // Terminate all player processes
    for (int i = 0; i < TEAMS_NUMBER; i++) {
        for (int j = 0; j < PLAYERS_FOR_EACH_TEAM; j++) {
            kill(players_id[i][j], SIGTERM);
        }
    }

    // Terminate the referee process
    kill(referee_pid, SIGTERM);

    // Wait for all processes to finish
    for (int i = 0; i < TEAMS_NUMBER * PLAYERS_FOR_EACH_TEAM + 1; i++) {  // +1 for the referee
        wait(NULL);
    }

    return 0;
}
