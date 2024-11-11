#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>  // Add this header to fix implicit declaration of 'time'

#include "player.h"
#include "game.h"



void handle_sigstop(int signum) {}
void handle_sigcont(int signum) {}

void stabilize(int team_id, int player_id) {
    srand(time(0));
    float stabilization_time = ((rand() % 3 + 4) + (team_id+1 * player_id+1 % 2))*(player_energy[team_id][player_id]/INITIAL_ENERGY); 
    //printf("Team %d Player %d stabilizes in %.2f seconds\n", team_id, player_id + 1, stabilization_time);
    
    player_message msg;
    msg.team_index = team_id;
    msg.player_index = player_id;
    
    msg.time = stabilization_time;
    msg.massageType = 0;  // 0 means jump

    //printf("Sending message  jummppp - Team: %d, Player: %d, Message Type: %d, Time: %.2f\n",    msg.team_index, msg.player_index, msg.massageType, msg.time);

    // Write message to pipe
    ssize_t bytesWritten = write(pipes[team_id][1], &msg, sizeof(msg));
   // ssize_t bytesWritten2 = write(pipeFd[1], &msg, sizeof(msg));
}

// Function to pull up another player
void pull_up(int team_id, int player_id) {
    double pulling_time = ((rand() % 3 + 10) + (team_id+1 * player_id+1 % 2))*(player_energy[team_id][player_id]/INITIAL_ENERGY);
    //printf("Team %d Player %d pulled up another player in %.2f seconds\n", team_id, player_id + 1, pulling_time);
    player_message msg;
    msg.team_index = team_id;
    msg.player_index = player_id;
    msg.time = pulling_time;
    msg.massageType = 1;  // 1 means pull

    // printf("Sending message - Team: %d, Player: %d, Message Type: %d, Time: %.2f\n",
    //    msg.team_index, msg.player_index, msg.massageType, msg.time);

    // Write message to pipe
    ssize_t bytesWritten = write(pipes[team_id][1], &msg, sizeof(msg));
}

// Function representing player process
void player_process(int team_id, int player_id) {
    signal(SIGTSTP, handle_sigstop);
    signal(SIGCONT, handle_sigcont);
    int type;
    while (1) {

        pause();  // Wait for the referee's signal to jump
        ssize_t bytesRead = read(pipestoref[team_id][0], &type, sizeof(int));
        //printf(" the stutes =%d %d %d\n",type ,team_id,player_id);
        if(type==1){
            stabilize(team_id, player_id);
        }
        else  if(type==0){
            pull_up(team_id, player_id);
        }

        kill(getpid(), SIGTSTP);  // Pause after stabilizing
    }
}
