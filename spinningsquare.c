#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>

#define MAX_PLAYERS 9  // Max number of players
#define NUM_COLUMNS 6  // Number of columns in the table

// Define the structure to hold player data
typedef struct {
    int rank;
    char* team;
    char* name;
    char* competitor_name;
    int  score;
    float time;
} Player;

// Declare the players array to hold data from the file
Player players[MAX_PLAYERS];
int numPlayers = 0; // Track the number of valid players loaded
time_t last_modified_time = 0; // File modification timestamp

// Comparison function to sort players based on time in ascending order
int compareByTime(const void* a, const void* b) {
    Player* playerA = (Player*)a;
    Player* playerB = (Player*)b;
    if (playerA->time < playerB->time) return -1;
    if (playerA->time > playerB->time) return 1;
    return 0;
}

// Function to load data from file
void loadDataFromFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open file %s for reading\n", filename);
        return;
    }

    struct stat file_stat;
    if (stat(filename, &file_stat) == 0) {
        if (file_stat.st_mtime != last_modified_time) {
            last_modified_time = file_stat.st_mtime;  // Update last modified time
        } else {
            fclose(file);
            return;  // No changes, exit early
        }
    }

    numPlayers = 0; // Reset player count
    for (int i = 0; i < MAX_PLAYERS; i++) {
        players[i].team = (char*)malloc(50 * sizeof(char));
        players[i].name = (char*)malloc(50 * sizeof(char));
        players[i].competitor_name = (char*)malloc(50 * sizeof(char));

        if (fscanf(file, "%d %49s %49s %49s %d %f\n", 
            &players[numPlayers].rank, players[numPlayers].team, players[numPlayers].name, 
            players[numPlayers].competitor_name, &players[numPlayers].score, &players[numPlayers].time) == 6) {
            numPlayers++;
        } else {
            printf("Error reading line %d\n", i + 1);
            free(players[i].team);
            free(players[i].name);
            free(players[i].competitor_name);
        }
    }

    fclose(file);
    
    // Sort the players by time
    qsort(players, numPlayers, sizeof(Player), compareByTime);
    
    // Update ranks based on sorted order
    for (int i = 0; i < numPlayers; i++) {
        players[i].rank = i + 1;  // Assign ranks based on sorted order
    }
}

// Function to free dynamically allocated memory
void freePlayerData() {
    for (int i = 0; i < numPlayers; i++) {
        free(players[i].team);
        free(players[i].name);
        free(players[i].competitor_name);
    }
}

// Function to draw a rounded rectangle
void drawRoundedRect(float x, float y, float width, float height, float radius) {
    glBegin(GL_POLYGON);
    for (int i = 0; i <= 360; i++) {
        float angle = i * 3.14159265358979323846 / 180.0;
        glVertex2f(x + width - radius + cos(angle) * radius, y + height - radius + sin(angle) * radius);
        glVertex2f(x + radius + cos(angle) * radius, y + height - radius + sin(angle) * radius);
        glVertex2f(x + radius + cos(angle) * radius, y + radius + sin(angle) * radius);
        glVertex2f(x + width - radius + cos(angle) * radius, y + radius + sin(angle) * radius);
    }
    glEnd();
}

// Function to calculate the width of a string for proper spacing
int getTextWidth(const char* text) {
    int width = 0;
    while (*text) {
        width += glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, *text);
        text++;
    }
    return width;
}

// Draw the table with player data
void drawTable() {
    float startX = 0.1f;  // Move the table slightly leftward
    float startY = 0.9f;
    float rowHeight = 0.08f;  // Row height
    float cellWidth = 0.14f;  // Reduced column width for a smaller table
    float radius = 0.02f;
    float rowSpacing = 0.04f;  // Row spacing

    // Title
    glColor3f(0.0f, 0.0f, 0.0f);  // Black text
    glRasterPos2f(startX + cellWidth * 2, startY);
    const char *title = "Player Rankings";
    for (const char *c = title; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    startY -= rowHeight;  // Move down after title

    // Draw header
    glColor3f(0.2f, 0.3f, 0.5f);  // Blue header background
    drawRoundedRect(startX - 0.05f, startY - 0.05f, cellWidth * NUM_COLUMNS + 0.1f, rowHeight + 0.1f, radius);

    // Header text
    glColor3f(1.0f, 1.0f, 1.0f);  // White text
    const char *header[] = {"Rank", "Team", "Name", "Competitor", "Score", "Time"};
    for (int i = 0; i < NUM_COLUMNS; ++i) {
        glRasterPos2f(startX, startY + rowHeight / 2);
        for (const char *c = header[i]; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
        startX += cellWidth;
    }

    // Draw player data rows
    for (int i = 0; i < numPlayers; i++) {
        startX = 0.1f;  // Reset the start position
        startY -= (rowHeight + rowSpacing);  // Move down by row height + spacing

        // Draw row background
        if (i % 2 == 0) glColor3f(0.9f, 0.9f, 0.9f);  // Light gray for even rows
        else glColor3f(1.0f, 1.0f, 1.0f);  // White for odd rows
        drawRoundedRect(startX - 0.05f, startY - 0.05f, cellWidth * NUM_COLUMNS + 0.1f, rowHeight + 0.1f, radius);

        // Draw player data
        glColor3f(0.0f, 0.0f, 0.0f);  // Black text
        char buffer[50];

        // Rank
        snprintf(buffer, sizeof(buffer), "%d", players[i].rank);
        glRasterPos2f(startX, startY + rowHeight / 2);
        for (const char *c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }

        startX += cellWidth;

        // Team
        glRasterPos2f(startX, startY + rowHeight / 2);
        for (const char *c = players[i].team; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }

        startX += cellWidth;

        // Name
        glRasterPos2f(startX, startY + rowHeight / 2);
        for (const char *c = players[i].name; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }

        startX += cellWidth;

        // Competitor Name
        glRasterPos2f(startX, startY + rowHeight / 2);
        for (const char *c = players[i].competitor_name; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }

        startX += cellWidth;

        // Score
        snprintf(buffer, sizeof(buffer), "%d", players[i].score);
        glRasterPos2f(startX, startY + rowHeight / 2);
        for (const char *c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }

        startX += cellWidth;

        // Time
        snprintf(buffer, sizeof(buffer), "%.2f", players[i].time);
        glRasterPos2f(startX, startY + rowHeight / 2);
        for (const char *c = buffer; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    }
}

// Timer function to check for file changes and reload data
void timer(int value) {
    loadDataFromFile("players.txt");
    glutPostRedisplay();  // Redraw the window
    glutTimerFunc(10000, timer, 0);  // Check again after 10 seconds
}

// Function to draw a circle
void drawCircle(float x, float y, float radius) {
    glBegin(GL_POLYGON);
    for (int i = 0; i <= 360; i++) {
        float angle = i * 3.14159265358979323846 / 180.0;
        glVertex2f(x + cos(angle) * radius, y + sin(angle) * radius);
    }
    glEnd();
}

// Modify the display function to draw the table and the circle, ensuring the entire table is visible
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Draw the circle on the left side of the screen
    glColor3f(1.0f, 0.0f, 0.0f);  // Red circle
    drawCircle(-0.7f, 0.0f, 0.2f);  // Position the circle on the left

    // Draw the table on the right side of the screen
    drawTable();  // Draw the table with updated data

    glutSwapBuffers();  // Swap buffers for double buffering
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // Increase the window size (larger screen)
    glutInitWindowSize(1800, 800);  // Increased window size from 800x600 to 1200x800
    glutCreateWindow("Player Rankings");

    // Adjust the orthographic projection to fit the larger window
    glOrtho(-2.0f, 2.0f, -1.5f, 1.5f, -1.0f, 1.0f);  // Further extend the left-right and up-down limits

    // Load the initial data
    loadDataFromFile("players.txt");

    // Register the display function
    glutDisplayFunc(display);

    // Start the timer to periodically check for file changes
    glutTimerFunc(10000, timer, 0);  // Check every 10 seconds

    // Enter the GLUT main loop
    glutMainLoop();

    // Clean up memory before exiting
    freePlayerData();
    
    return 0;
}
