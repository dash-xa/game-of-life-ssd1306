#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "Io_SSD1306.h" // Include your SSD1306 library

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_BUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT / 8)

uint8_t displayBuffer[DISPLAY_BUFFER_SIZE];

void initializeGame(void);
void updateGame(void);
void drawHorizontalLine(void);
int countNeighbors(int x, int y);
void delaySeconds(int seconds);
extern void ssd1306_write(const uint8_t img[]); // Make sure this matches your SSD1306 library

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    __enable_interrupt();       // Enable global interrupts
    i2c_init();                 // Initialize I2C for SSD1306
    ssd1306_init();             // Initialize the OLED display

    drawHorizontalLine();       // Draw a horizontal line
    ssd1306_write(displayBuffer);
    delaySeconds(2);            // Hold the line for 2 seconds

    initializeGame();           // Set initial state for Game of Life
    while (1) {
        updateGame();
        ssd1306_write(displayBuffer);
        delaySeconds(1); // Game speed, adjust as needed
    }
}

void initializeGame(void) {
    memset(displayBuffer, 0, DISPLAY_BUFFER_SIZE); // Clear the display buffer
    // Example: Initialize a glider
    int positions[][2] = {{1, 0}, {2, 1}, {0, 2}, {1, 2}, {2, 2}}; // Coordinates for a glider
    for (int i = 0; i < 5; i++) {
        int x = positions[i][0] + DISPLAY_WIDTH / 2; // Center the glider
        int y = positions[i][1] + DISPLAY_HEIGHT / 2;
        displayBuffer[(y / 8) * DISPLAY_WIDTH + x] |= (1 << (y % 8));
    }
}

void updateGame(void) {
    uint8_t nextBuffer[DISPLAY_BUFFER_SIZE];
    memset(nextBuffer, 0, DISPLAY_BUFFER_SIZE);

    for (int x = 0; x < DISPLAY_WIDTH; x++) {
        for (int y = 0; y < DISPLAY_HEIGHT; y++) {
            int neighbors = countNeighbors(x, y);
            bool isAlive = (displayBuffer[(y / 8) * DISPLAY_WIDTH + x] & (1 << (y % 8))) != 0;
            bool nextState = (isAlive && (neighbors == 2 || neighbors == 3)) || (!isAlive && neighbors == 3);

            if (nextState) {
                nextBuffer[(y / 8) * DISPLAY_WIDTH + x] |= (1 << (y % 8));
            }
        }
    }
    memcpy(displayBuffer, nextBuffer, DISPLAY_BUFFER_SIZE);
}

int countNeighbors(int x, int y) {
    int count = 0;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            int nx = (x + dx + DISPLAY_WIDTH) % DISPLAY_WIDTH;
            int ny = (y + dy + DISPLAY_HEIGHT) % DISPLAY_HEIGHT;
            if (displayBuffer[(ny / 8) * DISPLAY_WIDTH + nx] & (1 << (ny % 8))) {
                count++;
            }
        }
    }
    return count;
}

void drawHorizontalLine(void) {
    memset(displayBuffer, 0, DISPLAY_BUFFER_SIZE); // Clear display buffer
    int yMiddle = DISPLAY_HEIGHT / 2;
    for (int x = 0; x < DISPLAY_WIDTH; x++) {
        displayBuffer[(yMiddle / 8) * DISPLAY_WIDTH + x] |= (1 << (yMiddle % 8));
    }
}

void delaySeconds(int seconds) {
    for (int i = 0; i < seconds * 1000; i++) {
        __delay_cycles(1000); // Assumes a 1MHz clock. Adjust __delay_cycles accordingly.
    }
}
