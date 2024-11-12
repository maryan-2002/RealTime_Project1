#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <GL/glut.h>
#include <unistd.h>  // for sleep()

#define NUM_TEAMS 3
#define PLAYERS_PER_TEAM 3
#define PLAYER_RADIUS 0.05  // Radius for each player circle
#define JUMP_STOP_HEIGHT -0.8f

// Team colors to differentiate between teams
GLfloat teamColors[NUM_TEAMS][3] = {
    {1.0f, 0.0f, 0.0f},  // Team 1 - Red
    {0.0f, 1.0f, 0.0f},  // Team 2 - Green
    {0.0f, 0.0f, 1.0f}   // Team 3 - Blue
};

// Player's positions (starting positions for players)
float PlayerPositions[NUM_TEAMS][PLAYERS_PER_TEAM][2] = {
    {{-0.4f, 0.9f}, {-0.3f, 0.9f}, {-0.2f, 0.9f}},  // Team 1
    {{0.0f, 0.9f}, {0.1f, 0.9f}, {0.2f, 0.9f}},    // Team 2
    {{0.4f, 0.9f}, {0.5f, 0.9f}, {0.6f, 0.9f}}     // Team 3
};
// Track current player for jumping in each team
int currentPlayer[NUM_TEAMS] = {0};  // All teams start with the first player

// Store jump status for each player (whether they have finished jumping)
bool playerJumped[NUM_TEAMS][PLAYERS_PER_TEAM] = {{false, false, false}, {false, false, false}, {false, false, false}};


long lastPos = 0;  // Keep track of the last read position in the file

// Structure to store events for each player
typedef struct {
    int teamIndex;
    int playerIndex;
    float time;
    int messageType;  // 0 for jump, 1 for pull
} PlayerEvent;

// Global variables to store the events
static PlayerEvent events[100];
static int eventCount = 0;

void readFromAnimationFileTeam(PlayerEvent *events, int *eventCount) {
    FILE *file = fopen("playerAnimationA.txt", "r");
    int teamIndex, playerIndex, messageType;
    float time;
    *eventCount = 0;

    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    while (fscanf(file, "%d %d %f %d", &teamIndex, &playerIndex, &time, &messageType) == 4) {
        events[*eventCount].teamIndex = teamIndex;
        events[*eventCount].playerIndex = playerIndex;
        events[*eventCount].time = time;
        events[*eventCount].messageType = messageType;
        (*eventCount)++;
        printf("Read event: %d %d %.2f %d\n", teamIndex, playerIndex, time, messageType);  // Debugging
    }

    printf("Total events read: %d\n", *eventCount);
    fclose(file);
}

// Display callback function for rendering the scene
void Display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the bridge and teams with the updated player positions
    readFromAnimationFileTeam(events, &eventCount);  // Read events from the file
    processEvents(events, eventCount);  // Process events (jump & pull)

    drawBridge();
    drawTeams(events, eventCount);  // Draw teams with updated positions

    glutSwapBuffers();
}
float pullTimes[NUM_TEAMS][PLAYERS_PER_TEAM] = {0};  // Store pull times for each player

void processEvents(PlayerEvent *events, int eventCount) {
    float jumpTimes[NUM_TEAMS][PLAYERS_PER_TEAM] = {0};  // Store jump times for each player
    float pullTimes[NUM_TEAMS][PLAYERS_PER_TEAM] = {0};  // Store pull times for each player
    int pullCounts[NUM_TEAMS] = {0};  // Store the number of pull events for each team

    // First, handle jump events and store jump times
    for (int i = 0; i < eventCount; i++) {
        if (events[i].messageType == 0) {  // Jump event
            int team = events[i].teamIndex;
            int player = events[i].playerIndex;

            if (player == currentPlayer[team]) {  // Only allow the current player to jump
                jumpTimes[team][player] = events[i].time;
                playerJumped[team][player] = true;  // Mark this player as having jumped
                printf("Player %d from Team %d jumped at time %.2f\n", player, team, events[i].time);

                // After the player jumps, move to the next player in the team
                currentPlayer[team]++;
                if (currentPlayer[team] >= PLAYERS_PER_TEAM) {
                    printf("All players in Team %d have jumped!\n", team);
                }
            }
        }
    }

    // Now handle pull events and accumulate total pull times for each player
    for (int i = 0; i < eventCount; i++) {
        if (events[i].messageType == 1) {  // Pull event
            int team = events[i].teamIndex;
            int player = events[i].playerIndex;
            pullTimes[team][player] = events[i].time;  // Store pull time for the player
            pullCounts[team]++;  // Increment the number of pull events for the team
            printf("Player %d from Team %d pulled at time %.2f\n", player, team, events[i].time);
        }
    }

    // Calculate average pull times for each player
    for (int team = 0; team < NUM_TEAMS; team++) {
        for (int player = 0; player < PLAYERS_PER_TEAM; player++) {
            if (playerJumped[team][player] && player > 0) {
                // Calculate average pull time for the player (average of previous two pull events)
                int previousPlayer1 = player - 1;
                int previousPlayer2 = player - 2;
                if (previousPlayer1 >= 0 && previousPlayer2 >= 0) {
                    // Average the pull times of the previous two players
                    float avgPullTime = (pullTimes[team][previousPlayer1] + pullTimes[team][previousPlayer2]) / 2.0f;
                    printf("Average pull time for Player %d from Team %d: %.2f\n", player, team, avgPullTime);
                    // Store the average pull time for the player (to be used later in the drawTeams function)
                    pullTimes[team][player] = avgPullTime;
                }
            }
        }
    }
}


// Function to periodically check the file for updates
void checkFile(int value) {
    static bool fileRead = false;

    if (!fileRead) {
        readFromAnimationFileTeam(events, &eventCount);  // Read events from file only once
        processEvents(events, eventCount);  // Process events (jump & pull)
        fileRead = true;  // Mark the file as read
    }
    
    glutPostRedisplay();  // Request a redraw of the scene
    glutTimerFunc(1000, checkFile, 0);  // Call again in 1000 milliseconds (1 second)
}

// Draw a circle representing a player at the specified position
void drawPlayer(float x, float y) {
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i <= 20; i++) {
        float angle = 2 * 3.14159f * i / 20;
        glVertex2f(x + cos(angle) * PLAYER_RADIUS, y + sin(angle) * PLAYER_RADIUS);
    }
    glEnd();
}

// Draws the bridge at the top of the window
void drawBridge() {
    glColor3f(0.5f, 0.35f, 0.05f);  // Brown color for the bridge
    glBegin(GL_QUADS);
    glVertex2f(-0.6f, 0.7f);
    glVertex2f(0.8f, 0.7f);
    glVertex2f(0.8f, 0.8f);
    glVertex2f(-0.6f, 0.8f);
    glEnd();
}

void drawTeams(PlayerEvent *events, int eventCount) {
    for (int team = 0; team < NUM_TEAMS; team++) {
        glColor3fv(teamColors[team]);  // Set color for the current team
        for (int player = 0; player < PLAYERS_PER_TEAM; player++) {
            float x = PlayerPositions[team][player][0];
            float y = PlayerPositions[team][player][1];

            // Check for any events related to the player
            for (int i = 0; i < eventCount; i++) {
                if (events[i].teamIndex == team && events[i].playerIndex == player) {
                    if (events[i].messageType == 0) {  // Jump event
                        // Jump logic: If the player is jumping and not yet at stop height, decrease y
                        if (y > JUMP_STOP_HEIGHT) {
                            y -= 0.7f;  // Adjust the value for faster/slower jumps
                            if (y < JUMP_STOP_HEIGHT) {
                                y = JUMP_STOP_HEIGHT;  // Stop at the jump height
                            }
                            printf("Player %d from Team %d is jumping: y = %.2f\n", player, team, y);
                        }
                    } else if (events[i].messageType == 1 && events[i].time == 0) {  // Pull event with timer == 0
                        // Pull logic: Update pullTimes array after pull event
                        pullTimes[team][player] = events[i].time;  // Assuming time is the pull time
                        printf("Pull time for Player %d from Team %d: %.2f\n", player, team, events[i].time);
                    }
                }
            }

            // After the player has jumped and stopped, process pull events and upward movement
            if (y == JUMP_STOP_HEIGHT) {
                // If timer is 0 and pull time has been set, start moving up
                if (pullTimes[team][player] == 0) {
                    // Move player up gradually
                    y += 0.05f;  // Adjust the upward speed factor (0.05f) as needed
                    
                    // Prevent going above the top of the screen (e.g., y = 1.0)
                    if (y > 0.9f) {
                        y = 0.9f;
                    }
                    printf("Player %d from Team %d is moving up: y = %.2f\n", player, team, y);
                }
            }

            // Update player position
            PlayerPositions[team][player][1] = y;

            // Draw the player (circle)
            drawPlayer(x, y);
        }
    }
}





// Function to process and move players after they finish jumping
void movePlayersAfterJump(PlayerEvent *events, int eventCount) {
    for (int team = 0; team < NUM_TEAMS; team++) {
        for (int player = 0; player < PLAYERS_PER_TEAM; player++) {
            float x = PlayerPositions[team][player][0];
            float y = PlayerPositions[team][player][1];
            int pullCount = 0;
            float avgPullTime = 0;

            // Check if the player has jumped and reached the stop height
            if (playerJumped[team][player] && y == JUMP_STOP_HEIGHT) {
                // Look for pull events for this player in the current team
                for (int i = 0; i < eventCount; i++) {
                    if (events[i].teamIndex == team && events[i].playerIndex == player && events[i].messageType == 1) {
                        avgPullTime += events[i].time;  // Sum pull times
                        pullCount++;
                    }
                }

                // If there are pull events, calculate the average pull time
                if (pullCount > 0) {
                    avgPullTime /= pullCount;
                    printf("Average pull time for Player %d from Team %d: %.2f\n", player, team, avgPullTime);

                    // Move the player up based on the average pull time (simplified: one step per frame)
                    y += 0.01f * avgPullTime;  // Adjust the factor (0.01f) to control speed of upward movement
                    if (y > 0.9f) {  // Don't allow players to go above the top of the screen
                        y = 0.9f;
                    }
                }
            }

            // Update player position
            PlayerPositions[team][player][1] = y;
        }
    }
}

void InitOpenGL() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // Set the background color to white
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);  // Set orthogonal projection
}

void initGraphics2(int argc, char **argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Pulling Contest");

    // Initialize OpenGL settings
    InitOpenGL();

    // Set up the display and timer callbacks
    glutDisplayFunc(Display);
    glutTimerFunc(1000, checkFile, 0);  // Check the file every 1000 milliseconds

    // Start the main GLUT loop
    glutMainLoop();

}
