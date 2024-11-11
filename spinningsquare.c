#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "player.h"
#include "game.h"
#include "referee.h"

// Declare the global variables
#define INITIAL_ROWS 100  // Initial allocation size
#define MAX_COLS 4
float **tableDataFloat;  // For storing score and time
int **tableDataInt;      // For storing team index and round

int rowCount = 0;  // Track the number of rows already read from the file
int prevRowCount = 0; // Store the previous row count

// Global scroll offset variables
float scrollOffset = 0.0f; // Vertical scroll offset

// Adjust row height (you can change the value based on how much space each row takes)
float rowHeight = 0.1f;

// Function to dynamically allocate memory for table arrays
void allocateMemory(int rows) {
    tableDataFloat = (float **)realloc(tableDataFloat, rows * sizeof(float *));
    tableDataInt = (int **)realloc(tableDataInt, rows * sizeof(int *));
    
    for (int i = rowCount; i < rows; i++) {
        tableDataFloat[i] = (float *)malloc(2 * sizeof(float));
        tableDataInt[i] = (int *)malloc(2 * sizeof(int));
    }
}

// Function to read the new data from the file and append it to tableData
void readNewDataFromFile() {
    FILE *file = fopen("playerScore.txt", "r");

    if (file == NULL) {
        // Handle file error
        printf("File could not be opened.\n");
        return;
    }

    // Skip previously read rows
    rewind(file);
    for (int i = 0; i < prevRowCount; i++) {
        fscanf(file, " %*f %*d %*f %*d");
    }

    // Ensure space for rows
    allocateMemory(rowCount + INITIAL_ROWS);

    // Read new data
    while (fscanf(file, " %f %d %f %d", &tableDataFloat[rowCount][0], &tableDataInt[rowCount][0],
                  &tableDataFloat[rowCount][1], &tableDataInt[rowCount][1]) != EOF) {

        if (tableDataFloat[rowCount][0] == -1.0f || tableDataInt[rowCount][0] == -1 ||
            tableDataFloat[rowCount][1] == -1.0f || tableDataInt[rowCount][1] == -1) {
            break;
        }

        printf("New data read: %.2f %d %.2f %d\n", tableDataFloat[rowCount][0], tableDataInt[rowCount][0], 
               tableDataFloat[rowCount][1], tableDataInt[rowCount][1]);
        rowCount++;
        
        if (rowCount >= 1000) { // Maximum allowed rows
            break;
        }
    }

    prevRowCount = rowCount;
    fclose(file);
}

// Function to render text on the screen
void renderText(float x, float y, char *text) {
    glRasterPos2f(x, y);
    for (int i = 0; i < strlen(text); i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}

// Function to compare scores for sorting
int compareScores(const void *a, const void *b) {
    float scoreA = ((float **)a)[0][0];
    float scoreB = ((float **)b)[0][0];
    return (scoreB - scoreA); // Sort in descending order
}

// Function to render a single row with color based on score
void renderRow(float xPos, float yPos, float score, int teamIndex, float time, int round, float color[3]) {
    glColor3f(color[0], color[1], color[2]);

    char buffer[100];
    
    // Score
    sprintf(buffer, "%.2f", score);
    renderText(xPos, yPos, buffer);

    // Team Index
    sprintf(buffer, "%d", teamIndex);
    renderText(xPos + 0.3f, yPos, buffer);

    // Time
    sprintf(buffer, "%.2f", time);
    renderText(xPos + 0.6f, yPos, buffer);

    // Round
    sprintf(buffer, "%d", round);
    renderText(xPos + 0.9f, yPos, buffer);
}

// Function to display the table in OpenGL window
void displayTable() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Set up text rendering
    glColor3f(0.0, 0.0, 0.0);  // Black text on white background
    float xPos = -0.8f; // Starting x position for the table
    float yPos = 0.8f + scrollOffset;  // Adjust yPos based on the scroll offset

    // Render table headers
    renderText(xPos, yPos, "Score");
    renderText(xPos + 0.3f, yPos, "Team Index");
    renderText(xPos + 0.6f, yPos, "Time");
    renderText(xPos + 0.9f, yPos, "Round");

    yPos -= 0.1f;  // Move down for the data rows

    // Sort and render rows round-by-round
    int currentRound = -1; // Track current round to detect changes
    int startRowIndex = 0;

    for (int i = 0; i < rowCount; i++) {
        if (tableDataInt[i][1] != currentRound) {
            // We have entered a new round; sort previous round’s data
            if (i > startRowIndex) {
                qsort(&tableDataFloat[startRowIndex], i - startRowIndex, sizeof(float*), compareScores);
            }
            currentRound = tableDataInt[i][1];
            startRowIndex = i;
        }

        // Render row with color
        float color[3];
        if (i - startRowIndex == 0) {
            color[0] = 0.0f; color[1] = 1.0f; color[2] = 0.0f;  // Green for highest score
        } else if (i - startRowIndex == 1) {
            color[0] = 1.0f; color[1] = 1.0f; color[2] = 0.0f;  // Yellow for second highest
        } else {
            color[0] = 1.0f; color[1] = 0.0f; color[2] = 0.0f;  // Red for others
        }

        renderRow(xPos, yPos, tableDataFloat[i][0], tableDataInt[i][0], tableDataFloat[i][1], tableDataInt[i][1], color);
        yPos -= 0.1f; // Move down for next row
    }

    glutSwapBuffers();
}

// Function to handle window resizing
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0); // Set orthographic projection
    glMatrixMode(GL_MODELVIEW);
}

// Function to periodically read the new data and update the window
void update(int value) {
    readNewDataFromFile();  // Read only new data
    glutPostRedisplay();
    glutTimerFunc(1000, update, 0); // Update every 1000 ms
}

// Function to handle keyboard input for scrolling
void keyboard(unsigned char key, int x, int y) {
    // Maximum scroll offset based on rows
    float maxScrollOffset = (rowCount * rowHeight) - 1.0f;

    if (key == 'w' || key == 'W') {  // Scroll up (W key)
        scrollOffset += 0.1f;
        if (scrollOffset > 0.8f) {  // Limit scrolling upwards (keep some margin from top)
            scrollOffset = 0.8f;
        }
    }
    if (key == 's' || key == 'S') {  // Scroll down (S key)
        scrollOffset -= 0.1f;
        if (scrollOffset < -maxScrollOffset) {  // Limit scrolling down (keep the table visible)
            scrollOffset = -maxScrollOffset;
        }
    }

    glutPostRedisplay();
}

// The function that initializes the graphics
void initGraphics(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Dynamic Table Display");

    glClearColor(1.0, 1.0, 1.0, 1.0);  // White background

    glutDisplayFunc(displayTable);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    glutTimerFunc(1000, update, 0); // Call update every second to read new data

    glutMainLoop();
}