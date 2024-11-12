#ifndef GAME_H
#define GAME_H

#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

#define TEAMS_NUMBER 3
#define PLAYERS_FOR_EACH_TEAM 3
#define INITIAL_ENERGY 100
#define NumberOfRound  40
typedef struct {
    int team_index;
    int player_index;
    int massageType;
    float time;
} player_message;

typedef struct {
    int team_index;
    float time;
    int round;
} referee_message;

typedef struct {
    int team_index;
    int player_index;
    float x;
    float y;
} animation_message;

typedef struct {
    int max_score;
    int game_duration;
}GameSettings;
//extern max_score, game_duration;
// Shared variables (extern declarations)
extern pid_t players_id[TEAMS_NUMBER][PLAYERS_FOR_EACH_TEAM];
extern pid_t ref[TEAMS_NUMBER];

extern int flag_jump_team[TEAMS_NUMBER];
extern pid_t referee_pid;  // Variable to hold referee process ID
extern const char *team_names[];
extern int scores[TEAMS_NUMBER][PLAYERS_FOR_EACH_TEAM];  // Score array for players
extern float player_energy[TEAMS_NUMBER][PLAYERS_FOR_EACH_TEAM];
extern float timeSpendeachRound[NumberOfRound][TEAMS_NUMBER];
// Pipes for communication between players and referee
extern int pipes[TEAMS_NUMBER][2]; // Each player will write to a pipe that the referee reads
extern int pipestoref[TEAMS_NUMBER][2]; // Each player will write to a pipe that the referee reads
extern int pipesreftopare[2]; 
extern int pipes_animationte[2]; // Each player will write to a pipe that the referee reads
extern int pipeFd[2];
extern float score[TEAMS_NUMBER];

extern int pipe_myanimation[2];  // Declaration for the new pipe


// Function prototypes
void referee_process();
void player_process(int team_id, int player_id);
void terminate_players();

#endif // GAME_H
