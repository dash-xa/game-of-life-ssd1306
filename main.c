#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "Io_SSD1306.h" // Include your SSD1306 library

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_BUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT / 8)

uint8_t displayBuffer[DISPLAY_BUFFER_SIZE];


void initializeGame(void) {
    memset(displayBuffer, 0, DISPLAY_BUFFER_SIZE); // Clear the display buffer

    // Coordinates for a glider pattern
    int positions[][2] = {{1, 0}, {2, 1}, {0, 2}, {1, 2}, {2, 2}};
    int i;
    for (i = 0; i < 5; i++) {
        // Adjust x and y positions to center the glider on the display
        int x = positions[i][0] + (DISPLAY_WIDTH / 2 - 1); // Center horizontally, adjust by -1 to align
        int y = positions[i][1] + (DISPLAY_HEIGHT / 2 - 1); // Center vertically, adjust by -1 to align

        // Calculate byte index and bit position for each cell of the glider
        int byteIndex = (y / 8) * DISPLAY_WIDTH + x;
        int bitPosition = y % 8;

        // Set the corresponding bit to make the cell "alive"
        displayBuffer[byteIndex] |= (1 << bitPosition);
    }
}

void updateGame(void) {
    uint8_t nextBuffer[DISPLAY_BUFFER_SIZE];
    memset(nextBuffer, 0, DISPLAY_BUFFER_SIZE);

    int x, y, dx, dy, nx, ny, byteIndex, neighbors;
    bool isAlive, nextState;
    for (x = 0; x < DISPLAY_WIDTH; x++) {
        for (y = 0; y < DISPLAY_HEIGHT; y++) {
            neighbors = countNeighbors(x, y);
            byteIndex = (y / 8) * DISPLAY_WIDTH + x;
            isAlive = (displayBuffer[byteIndex] & (1 << (y % 8))) != 0;
            nextState = (isAlive && (neighbors == 2 || neighbors == 3)) || (!isAlive && neighbors == 3);

            if (nextState) {
                nextBuffer[byteIndex] |= (1 << (y % 8));
            }
        }
    }
    memcpy(displayBuffer, nextBuffer, DISPLAY_BUFFER_SIZE);
}

int countNeighbors(int x, int y) {
    int count = 0;
    int dx, dy, nx, ny, byteIndex;
    for (dx = -1; dx <= 1; dx++) {
        for (dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            nx = (x + dx + DISPLAY_WIDTH) % DISPLAY_WIDTH;
            ny = (y + dy + DISPLAY_HEIGHT) % DISPLAY_HEIGHT;
            byteIndex = (ny / 8) * DISPLAY_WIDTH + nx;
            if (displayBuffer[byteIndex] & (1 << (ny % 8))) {
                count++;
            }
        }
    }
    return count;
}

void drawHorizontalLine(void) {
    memset(displayBuffer, 0, DISPLAY_BUFFER_SIZE);
    int yMiddle = DISPLAY_HEIGHT / 2;
    int x, byteIndex;
    for (x = 0; x < DISPLAY_WIDTH; x++) {
        byteIndex = (yMiddle / 8) * DISPLAY_WIDTH + x;
        displayBuffer[byteIndex] |= (1 << (yMiddle % 8));
    }
}

void delaySeconds(int seconds) {
    int i;
    for (i = 0; i < seconds * 1000; i++) {
        __delay_cycles(1000); // Assumes a 1MHz clock, adjust as necessary.
    }
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    __enable_interrupt();
    i2c_init();
    ssd1306_init();

    ssd1306_write_constant(0);
    drawHorizontalLine();
    ssd1306_write(displayBuffer);
    delaySeconds(2);


    ssd1306_write_constant(0);
    delaySeconds(2);

    initializeGame();
    while (1) {
        ssd1306_write_constant(0);
        delaySeconds(1);

        updateGame();
        ssd1306_write(displayBuffer);
//        delaySeconds(1);
    }
}
