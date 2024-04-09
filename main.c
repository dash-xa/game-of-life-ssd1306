#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "Io_SSD1306.h" // Ensure this contains prototypes for ssd1306_write() and i2c_transmit()

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_BUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT / 8)

uint8_t displayBuffer[DISPLAY_BUFFER_SIZE];

void initializeGame(void);
void updateGame(void);
void drawPixel(int x, int y, bool state);
int countNeighbors(int x, int y);
extern void ssd1306_write(const uint8_t img[]);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    __enable_interrupt();       // Enable global interrupts
    i2c_init();                 // Initialize I2C for SSD1306, make sure this is set up for your MSP430
    ssd1306_init();             // Initialize the OLED display

    initializeGame();
    while(1) {
        updateGame();
        ssd1306_write(displayBuffer);
        __delay_cycles(500000); // Adjust delay for game speed
    }
}

void initializeGame(void) {
    memset(displayBuffer, 0, DISPLAY_BUFFER_SIZE); // Clear display buffer
    // Example initialization: Glider pattern
    drawPixel(2, 3, true);
    drawPixel(3, 4, true);
    drawPixel(4, 2, true);
    drawPixel(4, 3, true);
    drawPixel(4, 4, true);
}

void drawPixel(int x, int y, bool state) {
    int byteIndex = x + (y / 8) * DISPLAY_WIDTH;
    if (state) {
        displayBuffer[byteIndex] |= (1 << (y % 8));
    } else {
        displayBuffer[byteIndex] &= ~(1 << (y % 8));
    }
}

void updateGame(void) {
    uint8_t nextBuffer[DISPLAY_BUFFER_SIZE];
    memset(nextBuffer, 0, DISPLAY_BUFFER_SIZE); // Prepare the next state buffer

    int x, y;
    for (x = 0; x < DISPLAY_WIDTH; x++) {
        for (y = 0; y < DISPLAY_HEIGHT; y++) {
            int count = countNeighbors(x, y);
            int byteIndex = x + (y / 8) * DISPLAY_WIDTH;
            bool isAlive = displayBuffer[byteIndex] & (1 << (y % 8));
            bool nextState = (isAlive && (count == 2 || count == 3)) || (!isAlive && count == 3);

            if (nextState) {
                nextBuffer[byteIndex] |= (1 << (y % 8));
            }
        }
    }
    memcpy(displayBuffer, nextBuffer, DISPLAY_BUFFER_SIZE); // Update current state
}

int countNeighbors(int x, int y) {
    int count = 0, dx, dy, nx, ny;
    for (dx = -1; dx <= 1; dx++) {
        for (dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue; // Skip the cell itself

            nx = (x + dx + DISPLAY_WIDTH) % DISPLAY_WIDTH;
            ny = (y + dy + DISPLAY_HEIGHT) % DISPLAY_HEIGHT;
            int byteIndex = nx + (ny / 8) * DISPLAY_WIDTH;

            if (displayBuffer[byteIndex] & (1 << (ny % 8))) {
                count++;
            }
        }
    }
    return count;
}
