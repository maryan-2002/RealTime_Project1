#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include <time.h> // Add this header to fix implicit declaration of 'time'
#include "referee.h"
#include "player.h"
#include "game.h"

pid_t durationfork;
float timeSpendeachRound[NumberOfRound][TEAMS_NUMBER];
float Score[TEAMS_NUMBER];
int pipesreftopare[2];
void referee_process(GameSettings *settings)
{
      // Create a thread to run threaded_duration_end
    pthread_t duration_thread;

    // Allocate memory for the game duration to pass to the thread
    int *duration = malloc(sizeof(int));
    if (!duration) {
        perror("Failed to allocate memory");
        return;
    }
    *duration = settings->game_duration;

    // Start the thread
    if (pthread_create(&duration_thread, NULL, threaded_duration_end, duration) != 0) {
        perror("Failed to create thread");
        free(duration);
        return;
    }

    printf("Max Score: %d\n", settings->max_score);
    int team_index = 0;
    int player_index = 0;
    double stabilization_time;
    referee_message msg_ref;
    ssize_t bytesRead;
    float score;

    if (pipe(pipesreftopare) == -1)
    {
        perror("pipe failed");
        return 1;
    }

    for (team_index = 0; team_index < TEAMS_NUMBER; team_index++)
    {
        // printf("Referee: Team %d Player %d, jump!\n", team_index, player_index + 1);
        pid_t pid = fork();
        ref[team_index] = pid;
        if (pid < 0)
        {
            exit(1);
        }
        else if (pid == 0)
        {
            Score[team_index] = 0;
            referee_process_judge(team_index);
            exit(0);
        }
    }

    while (1)
    {
        bytesRead = read(pipesreftopare[0], &msg_ref, sizeof(msg_ref));
        score = 100 / msg_ref.time;
        timeSpendeachRound[msg_ref.round][msg_ref.team_index] = msg_ref.time;
        Score[msg_ref.team_index] += score;
        if (bytesRead > 0)
        {
            printf("Received message -point :%f Team Index: %d, Time: %.2f, Round: %d\n", Score[msg_ref.team_index], msg_ref.team_index, msg_ref.time, msg_ref.round);
            writeToFile(Score[msg_ref.team_index], msg_ref.team_index, msg_ref.time, msg_ref.round);
        }
        if (Score[msg_ref.team_index] >= settings->max_score)
        {

            bytesRead = read(pipesreftopare[0], &msg_ref, sizeof(msg_ref));
            score = 100 / msg_ref.time;
            timeSpendeachRound[msg_ref.round][msg_ref.team_index] = msg_ref.time;
            Score[msg_ref.team_index] += score;
            writeToFile(Score[msg_ref.team_index], msg_ref.team_index, msg_ref.time, msg_ref.round);

            bytesRead = read(pipesreftopare[0], &msg_ref, sizeof(msg_ref));
            score = 100 / msg_ref.time;
            timeSpendeachRound[msg_ref.round][msg_ref.team_index] = msg_ref.time;
            Score[msg_ref.team_index] += score;
            writeToFile(Score[msg_ref.team_index], msg_ref.team_index, msg_ref.time, msg_ref.round);
            writeStopToFile();
            end_game();
            break;
        }
    }
}

void referee_process_judge(int team)
{
    int roundnumber = 0;
    int puller_index = 0;
    float pull1, pull2, finalpull, stabilize, final;
    player_message msg;
    referee_message msg_ref;
    int data;
    while (1)
    {
        roundnumber++;
        // printf("new new %d\n", team);
        kill(players_id[team][puller_index], SIGCONT);
        data = 1;
        ssize_t bytesWritten = write(pipestoref[team][1], &data, sizeof(data));
        ssize_t bytesRead = read(pipes[team][0], &msg, sizeof(msg));

        writeToAnimationFileTeam(msg.team_index, msg.player_index, msg.time, msg.massageType);
        stabilize = msg.time;
        sleep(msg.time);

        // printf("\nREFEREE JUMP  :Received message - Team: %d, Player: %d, Message Type: %d, Time: %.2f\n",
        //        msg.team_index, msg.player_index + 1, msg.massageType, msg.time);
        player_energy[msg.team_index][msg.player_index] -= 0.4 * msg.time;

        // now for the pull
        for (int i = 1; i <= 2; i++)
        {
            puller_index = (msg.player_index + i) % PLAYERS_FOR_EACH_TEAM;
            kill(players_id[team][puller_index], SIGCONT);
            data = 0; // the integer value you want to write
            bytesWritten = write(pipestoref[team][1], &data, sizeof(data));
            //  printf(" new pull create \n");
        }
        // read the pull time

        bytesRead = read(pipes[team][0], &msg, sizeof(msg));
        writeToAnimationFileTeam(msg.team_index, msg.player_index, msg.time, msg.massageType);
        pull1 = msg.time;
        bytesRead = read(pipes[team][0], &msg, sizeof(msg));

        writeToAnimationFileTeam(msg.team_index, msg.player_index, msg.time, msg.massageType);

        pull2 = msg.time;
        finalpull = (pull1 + pull2) / 2;
        // printf(" the full time is  = %f",finalpull);
        sleep(finalpull);
        for (int player_index = 0; player_index < PLAYERS_FOR_EACH_TEAM; player_index++)
        {
            player_energy[team][player_index] -= 0.5 * msg.time;
            //    printf("Energy after update - Team: %d, Player: %d, Energy: %.2f\n", team, player_index, player_energy[team][player_index]);
        }
        final = finalpull + stabilize;
        msg_ref.round = roundnumber;
        msg_ref.team_index = team;
        msg_ref.time = final;
        write(pipesreftopare[1], &msg_ref, sizeof(msg_ref));
        // now to the next player ;
        int attempts = 0;
        puller_index = (msg.player_index + 1) % PLAYERS_FOR_EACH_TEAM;
        while (attempts < 3)
        {
            if (player_energy[team][puller_index] > 0)
            {
                break;
            }
            else
            {
                attempts++;
                puller_index = (puller_index + 1) % PLAYERS_FOR_EACH_TEAM;
            }
            if (attempts >= 3)
            {
                printf("No players with energy available in team %d after 3 attempts.\n", team);
                terminate_players_team(players_id, team);
            }
        }

        srand(time(0));
        sleep(rand() % 3 + 1); // the next team player will do the bungee jumping as quickly as possible to gain time
        kill(players_id[team][puller_index], SIGCONT);
    }
}

void writeToFile(float score, int teamIndex, float time, int round)
{
    FILE *file = fopen("playerScore.txt", "a"); // Open the file in append mode

    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    // Write the data to the file
    fprintf(file, " %.2f %d %.2f %d\n", score, teamIndex, time, round);

    fclose(file); // Close the file after writing
}

// Terminate all player processes
void terminate_players_team(pid_t players_id[3][3], int team)
{
    for (int j = 0; j < 3; j++)
    {
        kill(players_id[team][j], SIGTERM);
    }
}

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

void terminate_referee(pid_t ref[3])
{

    for (int j = 0; j < 3; j++)
    {
        kill(ref[j], SIGTERM);
    }
}
void tesstFunction()
{
    printf("Time Spent Each Round (in seconds):\n");
}
void printTimeSpentTable(float timeSpendeachRound[NumberOfRound][TEAMS_NUMBER])
{
    printf("Time Spent Each Round (in seconds):\n");
    printf("Round |");

    // Print header for team columns
    for (int team = 0; team < TEAMS_NUMBER; team++)
    {
        printf(" Team %d |", team + 1);
    }
    printf("\n");

    // Print each round and team times
    for (int round = 0; round < NumberOfRound; round++)
    {
        printf(" %4d |", round + 1); // Print round number
        for (int team = 0; team < TEAMS_NUMBER; team++)
        {
            printf(" %7.2f |", timeSpendeachRound[round][team]); // Print time for each team
        }
        printf("\n");
    }
}
// Wrapper function for pthread compatibility
void *threaded_duration_end(void *arg) {
    int time = *(int *)arg; // Retrieve the time argument
    //printf("hiiiiiiiiiiiiiiiiiiiiiiiiii iam durationtime %d\n", time);
    sleep(time);
   // printf("hiiiiiiiiiiiiiiiiiiiiiiiiii iam durationtime\n");
    end_game();
    return NULL;
}

void end_game()
{
    terminate_players(players_id);
    terminate_referee(ref);
    printf("\n____________________________ (( End of game ))_________________________ :\n");
    printTimeSpentTable(timeSpendeachRound);
    printf("Score for: TEAM A = %.2f, TEAM B = %.2f, TEAM C = %.2f\n", Score[0], Score[1], Score[2]);
     int winning_team_index = findWinningTeam(Score);
    printf("*************** Team [%d] winnnnnn with Score = %.2f ***************\n", winning_team_index, Score[winning_team_index]);
}

int findWinningTeam(float Score[TEAMS_NUMBER])
{
    int max_index = 0;
    for (int i = 1; i < TEAMS_NUMBER; i++)
    {
        if (Score[i] > Score[max_index])
        {
            max_index = i;
        }
    }
    return max_index;
}

void writeStopToFile()
{
    // Open the file in append mode
    FILE *file = fopen("playerScore.txt", "a");

    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    // Write "stop" to the file
    fprintf(file, "-1\n");

    fclose(file); // Close the file after writing
}

void writeToAnimationFileTeam(int team_index, int player_index, float time, int type)
{
    FILE *file = fopen("playerAnimationA.txt", "a"); // Open the file in append mode

    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    // Write the data to the file
    fprintf(file, "%d %d %.2f %d\n", team_index, player_index, time, type);

    fclose(file); // Close the file after writing
}
void writeToScoreFile(int teamIndex, int team_index, float score, float time)
{
    // Open the file in append mode
    FILE *file = fopen("teamScore.txt", "a");

    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    // Write the data to the file
    fprintf(file, "TeamIndex: %d, Team: %d, Score: %.2f, Time: %.2f\n", teamIndex, team_index, score, time);

    fclose(file); // Close the file after writing
}

void writeToAnimationFileTeamB(int team_index, int player_index, float time, int type)
{
    FILE *file = fopen("playerAnimationB.txt", "a"); // Open the file in append mode

    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    // Write the data to the file
    fprintf(file, "%d %d %.2f %d\n", team_index, player_index, time, type);

    fclose(file); // Close the file after writing
}
void writeToAnimationFileTeamC(int team_index, int player_index, float time, int type)
{
    FILE *file = fopen("playerAnimationC.txt", "a"); // Open the file in append mode

    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    // Write the data to the file
    fprintf(file, "%d %d %.2f %d\n", team_index, player_index, time, type);

    fclose(file); // Close the file after writing
}
