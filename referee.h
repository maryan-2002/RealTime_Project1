#ifndef REFEREE_H
#define REFEREE_H
#include "game.h"
void referee_process(GameSettings *settings);
void terminate_players_team(pid_t players_id[3][3], int team);
void terminate_players(pid_t players_id[3][3]);
void referee_process_judge(int team);
void printTimeSpentTable(float timeSpendeachRound[NumberOfRound][TEAMS_NUMBER]) ;
void terminate_referee(pid_t ref[3]);
void end_game();
int findWinningTeam(float Score[TEAMS_NUMBER]);
void tesstFunction();
void writeStopToFile();
void writeToAnimationFileTeam(int team_index,int player_index ,float time, int type) ;
void writeToScoreFile(int teamIndex, int team_index, float score, float time);
void writeToFile(float score, int teamIndex, float time, int round);
void *threaded_duration_end(void *arg) ;
#endif // REFEREE_H
