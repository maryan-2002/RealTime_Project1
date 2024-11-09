#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SIG_STABILIZED (SIGUSR1)
#define TEAMS_NUMBER 3               
#define PLAYERS_FOR_EACH_TEAM 3 
#define INITIAL_ENERGY 100 
#define SIG_PULL (SIGUSR2)      

int pipes[TEAMS_NUMBER][2];  // Pipes for communication between referee and players
int scores[TEAMS_NUMBER] = {0}; 
int MAX_SCORE; 
int player_energy[TEAMS_NUMBER][PLAYERS_FOR_EACH_TEAM]; 
pid_t players_id[TEAMS_NUMBER][PLAYERS_FOR_EACH_TEAM];

const char *team_names[] = {"Team A", "Team B", "Team C"};


void setup_pull_signal_handler() {
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;  // Ignore default behavior for SIG_PULL
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    
    if (sigaction(SIG_PULL, &sa, NULL) == -1) {
        perror("Error setting SIG_PULL handler");
        exit(EXIT_FAILURE);
    }
}

// Pull up function simulating the time it takes for two players to pull the jumper up
double pull_up(int team_id) {
    double energy_factor = (player_energy[team_id][1] + player_energy[team_id][2]) / (2.0 * INITIAL_ENERGY);
    double pulling_time = (rand() % 5 + 30) / energy_factor;  // Random time between 10 and 15 divided by energy factor
    sleep(pulling_time);
    printf("%s team members pulled Player 1 up in %.2f seconds.\n", team_names[team_id], pulling_time);
    return pulling_time;
}



double stabilize(int team_id, int player_id) {
    double energy_factor = player_energy[team_id][player_id - 1] / (double)INITIAL_ENERGY;
    double stabilization_time = (rand() % 5 + 2) / energy_factor; // Random time between 20 and 25 divided by energy factor
    printf("%s Player %d will stabilize in %.2f seconds \n", 
           team_names[team_id], player_id, stabilization_time);
    sleep(stabilization_time);
    printf("%s Player %d stabilized after %.2f seconds.\n", team_names[team_id], player_id, stabilization_time);

    // Send signal through pipe to indicate stabilization
    int stabilized_signal = 1; // Signal indicating stabilization
    write(pipes[team_id][1], &stabilized_signal, sizeof(stabilized_signal));

    return stabilization_time;
}

void player_process(int team_id, int player_id) {
    while (1) {
        pause();  // Wait for a signal to jump
        printf()
        if (player_id == 1) {  // Only the first player jumps
            double jump_time = stabilize(team_id, player_id);
            write(pipes[team_id][1], &jump_time, sizeof(jump_time)); // Send jump time to referee
        } else {
            printf("Team %d Player %d is watching Player 1 jump.\n", team_id + 1, player_id);
            setup_pull_signal_handler();  // Set up handler for pulling signal for Players 2 and 3

            while (1) {
                pause();  // Wait for the pull signal
                if (signal(SIG_PULL, pull_up) != SIG_ERR) {  // Check if this player was asked to pull
                    double pull_time = pull_up(team_id);
                    write(pipes[team_id][1], &pull_time, sizeof(pull_time));  // Send pull time to referee
                    printf("%s Player %d finished pulling.\n", team_names[team_id], player_id);
                }
            }
        }
        
    }
}

void referee_process() {
    int team_index = 0;
    while (1) {
        sleep(2);  // Wait before each jump
        // Send signal to the current team's first player to jump
        printf("Referee: Team %d Player 1 jump!\n", team_index + 1);
        kill(players_id[team_index][0], SIGCONT);  // Signal the first player to jump

         // Variable to hold the stabilized signal
        int stabilized_signal = 0;
        // Wait for the player to stabilize
        while (stabilized_signal == 0) {
            // Read the stabilization signal from the pipe
            read(pipes[team_index][0], &stabilized_signal, sizeof(stabilized_signal));
            
            if (stabilized_signal) {
                printf("%s Player 1 has stabilized. Team members can now start pulling.\n", team_names[team_index]);
                //The referee is the one who decides if a bungee jumper has stabilized hanging down there so that the other 2 team members start pulling him
                kill(players_id[team_index][1], SIG_PULL); 
                kill(players_id[team_index][2], SIG_PULL); 
                break;
                // Here you can add logic for the team members to start pulling the player
            } else {
                printf("%s Player 1 has not stabilized yet. Waiting...\n", team_names[team_index]);
            }
        }

        // Calculate and update score inversely proportional to jump time
        //scores[team_index] += (MAX_SCORE / result_time);
        //printf("Team %d scored %.2f. Total score: %.2f.\n", team_index + 1, (MAX_SCORE / result_time), scores[team_index]);

        // Check if the same sequence described above continues until any of the 3 teams has reached or exceeded a score
        if (scores[team_index] >= MAX_SCORE) {  
            printf("Team %d reached the score limit.\n", team_index + 1);
            break;
        }

        // Rotate to the next team
        team_index = (team_index + 1) % TEAMS_NUMBER;
    }

    // Signal all players to exit once the game ends
    for (int i = 0; i < TEAMS_NUMBER; i++) {
        kill(players_id[i][0], SIGTERM); 
    }
}

int main() {
    // Prompt user for score threshold
    printf("Enter the maximum score to end the game: ");
    scanf("%d", &MAX_SCORE);

    // Prompt for the maximum game duration
    int game_duration;
    printf("Enter the maximum game duration in seconds: ");
    scanf("%d", &game_duration);
    alarm(game_duration);

    // We’ll call the three teams team A, team B and team C consecutively
    for (int team = 0; team < TEAMS_NUMBER; team++) {  
        printf("Starting processes for %s\n", team_names[team]);
        
        // Each team has its own pipe for communication between the referee and players
        if (pipe(pipes[team]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        // Each team is composed of 3 players numbered 1 to 3.
        for (int player_id = 0; player_id < PLAYERS_FOR_EACH_TEAM; player_id++) {
            player_energy[team][player_id] = INITIAL_ENERGY;  // Initially all players have a high level of energy
            pid_t pid = fork();
            players_id[team][player_id] = pid;

            // Check if fork() failed
            if (pid < 0) {
                perror("fork failed");  // Print error message if fork fails
                exit(1);                // Exit the program with an error code
            } 

            // Child process block
            else if (pid == 0) {
                printf("Player %d in %s\n", player_id + 1, team_names[team]);
                // Execute player-specific code
                player_process(team, player_id + 1);   
                // End child process after completing the player's task
                exit(0);
            }
        }
    }

    // Create referee process
    if (fork() == 0) {
        referee_process();
        exit(0);
    }

    // Wait for all processes to finish
    for (int i = 0; i < TEAMS_NUMBER * PLAYERS_FOR_EACH_TEAM + 1; i++) {  // +1 to the referee
        wait(NULL);
    }

    return 0;
}
