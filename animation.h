#ifndef ANIMATION_H
#define ANIMATION_H

#include "game.h"  // Include game.h to get access to the Team and Player structures

void initGL(int argc, char** argv);  // Function to initialize OpenGL context
void initOpenGL();  // Function for OpenGL setup
void display();  // Display function for OpenGL rendering
void timerFunc(int value);  // Timer function for animation updates
void readMessageFromPipe() ;
//void readMessageFromTeamPipe (int team_index);
void readMessageFromPipe();
void startJumping(player_message msg);
#endif // ANIMATION_H
