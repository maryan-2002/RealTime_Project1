#ifndef PLAYER_H
#define PLAYER_H

#include <unistd.h>

#define INITIAL_ENERGY 100

extern int pipes[3][2];  // Declare pipes for communication

void player_process(int team_id, int player_id);
void stabilize(int team_id, int player_id);
void pull_up(int team_id, int player_id);

#endif // PLAYER_H
