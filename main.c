#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "Io_SSD1306.h" // Ensure this matches your SSD1306 library

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_BUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT / 8)


#define CELL_SIZE 4 // Each logical cell represents an 8x8 block of pixels
#define REDUCED_WIDTH (DISPLAY_WIDTH / CELL_SIZE)
#define REDUCED_HEIGHT (DISPLAY_HEIGHT / CELL_SIZE)
#define MAX_ACTIVE_CELLS 512 // Adjust based on your requirements



typedef struct {
    uint8_t x;
    uint8_t y;
} Cell;

Cell activeCells[MAX_ACTIVE_CELLS];
Cell nextActiveCells[MAX_ACTIVE_CELLS];
uint16_t activeCellCount = 0;
uint16_t nextActiveCellCount = 0;

uint8_t displayBuffer[DISPLAY_BUFFER_SIZE];

void initializeGame(void);
void updateGame(void);
void drawCell(uint8_t x, uint8_t y, bool state);
int countNeighbors(uint8_t x, uint8_t y);
void delayMs(uint16_t ms);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
    __enable_interrupt();
    i2c_init(); // Initialize I2C for SSD1306
    ssd1306_init(); // Initialize the OLED display

    ssd1306_write_constant(0xFF);
    delayMs(1000);

    ssd1306_write_constant(0);
    delayMs(1000);

    initializeGame();
    ssd1306_write(displayBuffer); // Update display

    while (1) {
        updateGame();
        ssd1306_write(displayBuffer);
    }
}

void clearDisplayBuffer() {
    memset(displayBuffer, 0, DISPLAY_BUFFER_SIZE); // Clear the display buffer
}

void activateCellAndIncrementCount(uint8_t reducedX, uint8_t reducedY) {
    activeCells[activeCellCount++] = (Cell){reducedX, reducedY};
    drawCell(reducedX, reducedY, true);
}

void initializeGameWithVerticalLine(void) {
    clearDisplayBuffer();
    activeCellCount = 0;

    // Initialize with a pattern that fits the reduced resolution
    // Example: A line of "cells" down the center
    int x = REDUCED_WIDTH / 2;
    int y;
    for (y = 2; y < REDUCED_HEIGHT - 2; y++) {
        activateCellAndIncrementCount(x, y);
    }
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

void initializeGameWithGliders(void) {
    clearDisplayBuffer();  // Ensures the display starts clean
    activeCellCount = 0;   // Reset the count of active cells

    // Place several gliders on the grid
    placeGlider(2, 2);  // Top-left glider
    placeGlider(REDUCED_WIDTH - 5, 2);  // Top-right glider
    placeGlider(2, REDUCED_HEIGHT - 5);  // Bottom-left glider
    placeGlider(REDUCED_WIDTH - 5, REDUCED_HEIGHT - 5);  // Bottom-right glider
}

// checkerboard
void initializeGame(void) {
    clearDisplayBuffer();
    activeCellCount = 0;

    int xLeft = REDUCED_WIDTH / 4 - 4;
    int xRight = 3 * REDUCED_WIDTH / 4 - 4;

    int yLeft = REDUCED_HEIGHT / 4 - 4;
    int yRight = 3 * REDUCED_HEIGHT / 4 - 4;

    placeLWSS(xLeft,  yLeft);
    placeLWSS(xLeft,  yRight);
    placeLWSS(xRight,  yLeft);
    placeLWSS(xRight,  yRight);
}


bool isCellActive(uint8_t reducedX, uint8_t reducedY) {
    int x = reducedX * CELL_SIZE;
    int y = reducedY * CELL_SIZE;

    return displayBuffer[(y / 8) * DISPLAY_WIDTH + x] & (1 << (y % 8));
}

void updateGame(void) {
    nextActiveCellCount = 0;
    int i;

    int x, y;
    for (x = 0; x < REDUCED_WIDTH; x++) {
        for (y = 0; y < REDUCED_HEIGHT; y++) {
            int count = countNeighbors(x, y); // Count live neighbors of the cell
            bool isActive = isCellActive(x, y);
            bool nextState = (isActive && (count == 2 || count == 3)) || (!isActive && count == 3);

            // If the cell should be alive in the next generation, add it to the nextActiveCells array
            if (nextState && nextActiveCellCount < MAX_ACTIVE_CELLS) {
                nextActiveCells[nextActiveCellCount++] = (Cell){x, y};
                // Note: You might want to directly update displayBuffer here if drawing cells immediately
            }
        }
    }

    // update displays
    for (i = 0; i < activeCellCount; i++) {
        drawCell(activeCells[i].x, activeCells[i].y, false);
    }
    for (i = 0; i < nextActiveCellCount; i++) {
        drawCell(nextActiveCells[i].x, nextActiveCells[i].y, true);
    }

    memcpy(activeCells, nextActiveCells, sizeof(Cell) * nextActiveCellCount);
    activeCellCount = nextActiveCellCount;
}

void drawCell(uint8_t reducedX, uint8_t reducedY, bool state) {
    int startX = reducedX * CELL_SIZE;
    int startY = reducedY * CELL_SIZE;

    int x, y;
    for (x = startX; x < startX + CELL_SIZE; x++) {
        for (y = startY; y < startY + CELL_SIZE; y++) {
            uint16_t byteIndex = (y / 8) * DISPLAY_WIDTH + x;
            if (state) {
                displayBuffer[byteIndex] |= (1 << (y % 8));
            } else {
                displayBuffer[byteIndex] &= ~(1 << (y % 8));
            }
        }
    }
}

int countNeighbors(uint8_t x, uint8_t y) {
    int dx, dy;
    int count = 0;
    for (dx = -1; dx <= 1; dx++) {
        for (dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            int nx = (x + dx + REDUCED_WIDTH) % REDUCED_WIDTH;
            int ny = (y + dy + REDUCED_HEIGHT) % REDUCED_HEIGHT;
            if (isCellActive(nx, ny)) {
                count++;
            }
        }
    }
    return count;
}

void delayMs(uint16_t ms) {
    while (ms--) {
        __delay_cycles(1000); // Adjust this for your clock speed
    }
}
