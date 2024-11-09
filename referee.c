#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "referee.h"
#include "player.h"
#include "game.h"

void referee_process(int max_score)
{
    int team_index = 0;
    int player_index = 0;
    double stabilization_time;
    for (team_index = 0; team_index < TEAMS_NUMBER; team_index++)
    {
        printf("Referee: Team %d Player %d, jump!\n", team_index, player_index + 1);
        kill(players_id[team_index][player_index], SIGCONT);
        pid_t pid = fork();
        ref[team_index] = pid;
        if (pid < 0)
        {
            perror("fork failed");
            exit(1);
        }
        else if (pid == 0)
        {
            referee_process_judge(team_index);
            exit(0);
        }
    }
}

// Read from the pipe
// unsigned char buffer[sizeof(player_message)];
// ssize_t bytesRead = read(pipes_pl_ref[0], buffer, sizeof(buffer));
// if (bytesRead > 0) {
//     // Print raw byte data to debug
//     printf("Read %ld bytes from the pipe: ", bytesRead);
//     for (int i = 0; i < bytesRead; i++) {
//         printf("%02x ", buffer[i]);
//     }
//     printf("\n");
//     fflush(stdout);

//     // Copy the raw data back into the msg structure
//     memcpy(&msg, buffer, sizeof(player_message));  // memcpy is now defined

//     // Print the received message
//     printf("Received message - Team: %d, Player: %d, Message Type: %d, Time: %.2f\n",
//            msg.team_index, msg.player_index + 1, msg.massageType, msg.time);
//     fflush(stdout);
// } else {
//     printf("Error reading from pipe or no data available\n");
//     fflush(stdout);
// }

void referee_process_judge(int team)
{
    player_message msg;
    while (1)
    {
        ssize_t bytesRead = read(pipes[team][0], &msg, sizeof(msg));
        // printf("REFEREE :Received message - Team: %d, Player: %d, Message Type: %d, Time: %.2f\n",
        //        msg.team_index, msg.player_index + 1, msg.massageType, msg.time);
    }
 }

// Terminate all player processes
void terminate_players(pid_t players_id[3][3])
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            kill(players_id[i][j], SIGTERM);
        }
    }
}
