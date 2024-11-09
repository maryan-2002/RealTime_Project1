#ifndef REFEREE_H
#define REFEREE_H

void referee_process(int max_score);
void terminate_players(pid_t players_id[3][3]);
void referee_process_judge(int team);
#endif // REFEREE_H
