#include "Io_SSD1306.h"
#include "GameOfLifeInitializations.h"

void drawVerticalLine(int x) {
    int y;
    for (y = 2; y < REDUCED_HEIGHT - 2; y++) {
        activateCellAndIncrementCount(x, y);
    }
}

void initializeGameWithVerticalLine(void) {
    drawVerticalLine(REDUCED_WIDTH / 4);
    drawVerticalLine(3 * REDUCED_WIDTH / 4);
}

void placeLWSS(uint8_t startX, uint8_t startY) {
    /*
     *  # . . # .
        . . . . #
        # . . . #
        . # # # #
     */

    activateCellAndIncrementCount(startX, startY);
    activateCellAndIncrementCount(startX + 3, startY);

    activateCellAndIncrementCount(startX + 4, startY + 1);

    activateCellAndIncrementCount(startX, startY + 2);
    activateCellAndIncrementCount(startX + 4, startY + 2);

    activateCellAndIncrementCount(startX + 1, startY + 3);
    activateCellAndIncrementCount(startX + 2, startY + 3);
    activateCellAndIncrementCount(startX + 3, startY + 3);
    activateCellAndIncrementCount(startX + 4, startY + 3);
}



// Helper function to place a glider at a specific starting position in the reduced grid
void placeGlider(uint8_t startX, uint8_t startY) {
    activateCellAndIncrementCount(startX, startY + 2);
    activateCellAndIncrementCount(startX + 1, startY + 2);
    activateCellAndIncrementCount(startX + 2, startY + 2);
    activateCellAndIncrementCount(startX + 2, startY + 1);
    activateCellAndIncrementCount(startX + 1, startY);
}

/*void initializeGameWithGliders(void) {
    // Place several gliders on the grid
    placeGlider(2, 2);  // Top-left glider
    placeGlider(REDUCED_WIDTH - 5, 2);  // Top-right glider
    placeGlider(2, REDUCED_HEIGHT - 5);  // Bottom-left glider
    placeGlider(REDUCED_WIDTH - 5, REDUCED_HEIGHT - 5);  // Bottom-right glider

    placeGlider(REDUCED_WIDTH/2, REDUCED_HEIGHT/2);  // Bottom-right glider
}*/

#define GLIDER_SIZE 3 // Since a glider takes up a 3x3 area
#define NUMBER_OF_GLIDERS_PER_COLUMN 3
#define NUMBER_OF_GLIDERS_PER_ROW 6

void initializeGameWithGliders(void) {
    // Calculate spacing based on the size of the glider and the display dimensions
    int horizontalSpacing = (REDUCED_WIDTH - GLIDER_SIZE) / (NUMBER_OF_GLIDERS_PER_ROW - 1);
    int verticalSpacing = (REDUCED_HEIGHT - GLIDER_SIZE) / (NUMBER_OF_GLIDERS_PER_COLUMN - 1);

    // Place gliders in a grid pattern, avoiding the edges
    int x, y;
    for (x = GLIDER_SIZE / 2; x < REDUCED_WIDTH - GLIDER_SIZE / 2; x += horizontalSpacing) {
        for (y = GLIDER_SIZE / 2; y < REDUCED_HEIGHT - GLIDER_SIZE / 2; y += verticalSpacing) {
            placeGlider(x, y);
        }
    }
}


// checkerboard
void initializeGameLWSS(void) {
    int xLeft = REDUCED_WIDTH / 4 - 4;
    int xRight = 3 * REDUCED_WIDTH / 4 - 2;

    int yLeft = REDUCED_HEIGHT / 4 - 4;
    int yRight = 3 * REDUCED_HEIGHT / 4 - 2;

    placeLWSS(xLeft,  yLeft);
    placeLWSS(xLeft,  yRight);
    placeLWSS(xRight,  yLeft);
    placeLWSS(xRight,  yRight);
}


void drawDoubleTriple(int x, int y) {
    activateCellAndIncrementCount(x + 2, y);
    activateCellAndIncrementCount(x + 3, y);
    activateCellAndIncrementCount(x + 4, y);
    activateCellAndIncrementCount(x + 8, y);
    activateCellAndIncrementCount(x + 9, y);
    activateCellAndIncrementCount(x + 10, y);
}

void drawQuadrupleSingle(int x, int y) {
    activateCellAndIncrementCount(x, y);
    activateCellAndIncrementCount(x + 5, y);
    activateCellAndIncrementCount(x + 7, y);
    activateCellAndIncrementCount(x + 12, y);
}

void drawPulsar(int x, int y) {
    drawDoubleTriple(x, y);

    drawQuadrupleSingle(x, y+2);
    drawQuadrupleSingle(x, y+3);
    drawQuadrupleSingle(x, y+4);

    drawDoubleTriple(x, y+5);
    drawDoubleTriple(x, y+7);

    drawQuadrupleSingle(x, y+8);
    drawQuadrupleSingle(x, y+9);
    drawQuadrupleSingle(x, y+10);

    drawDoubleTriple(x, y+12);
}

void initializeGameWith2Pulsars(void) {
    drawPulsar(2, 2);
    drawPulsar(REDUCED_WIDTH / 2 + 2, 2);
}

void initializeGameWithPulsar(void) {
    drawPulsar(REDUCED_WIDTH / 4 + 2, 2);
}
