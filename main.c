#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "Io_SSD1306.h" // Ensure this matches your SSD1306 library

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_BUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT / 8)
#define MAX_ACTIVE_CELLS 500

typedef struct {
    uint8_t x;
    uint8_t y;
} Cell;

Cell activeCells[MAX_ACTIVE_CELLS];
uint16_t activeCellCount = 0;

uint8_t displayBuffer[DISPLAY_BUFFER_SIZE];

void initializeGame(void);
void updateGame(void);
void activateCell(uint8_t x, uint8_t y);
void drawCell(uint8_t x, uint8_t y, bool state);
int countNeighbors(uint8_t x, uint8_t y);
void delayMs(uint16_t ms);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
    __enable_interrupt();
    i2c_init(); // Initialize I2C for SSD1306
    ssd1306_init(); // Initialize the OLED display

    memset(displayBuffer, 0, DISPLAY_BUFFER_SIZE); // Clear display buffer
    initializeGame();
    while (1) {
        updateGame();
        ssd1306_write(displayBuffer); // Update display
//        delayMs(1000); // Slow down updates for visibility
    }
}


void initializeGame(void) {
    memset(displayBuffer, 0, DISPLAY_BUFFER_SIZE); // Clear the display buffer
    activeCellCount = 0; // Reset active cell count

    // Center of the display
    uint8_t centerX = DISPLAY_WIDTH / 2;
    uint8_t centerY = DISPLAY_HEIGHT / 2;

    // Simple pattern - adjust the coordinates for your pattern
    // This example uses a "+" pattern for simplicity
    activateCell(centerX, centerY); // Center dot
    activateCell(centerX - 1, centerY); // Left
    activateCell(centerX + 1, centerY); // Right
    activateCell(centerX, centerY - 1); // Top
    activateCell(centerX, centerY + 1); // Bottom
}

void updateGame(void) {
    Cell nextActiveCells[MAX_ACTIVE_CELLS];
    uint16_t nextActiveCellCount = 0;

    int i, dx, dy, x, y, nx, ny, count, index;
    bool isActive, nextState;

    for (i = 0; i < activeCellCount; i++) {
        x = activeCells[i].x;
        y = activeCells[i].y;

        for (dx = -1; dx <= 1; dx++) {
            for (dy = -1; dy <= 1; dy++) {
                nx = (x + dx + DISPLAY_WIDTH) % DISPLAY_WIDTH;
                ny = (y + dy + DISPLAY_HEIGHT) % DISPLAY_HEIGHT;

                // Check and update only if the cell is not already processed
                index = ny * DISPLAY_WIDTH + nx;

                if (!(displayBuffer[index / 8] & (1 << (index % 8)))) {
                    count = countNeighbors(nx, ny);
                    isActive = displayBuffer[(y / 8) * DISPLAY_WIDTH + x] & (1 << (y % 8));
                    nextState = (isActive && (count == 2 || count == 3)) || (!isActive && count == 3);

                    if (nextState && nextActiveCellCount < MAX_ACTIVE_CELLS) {
                        nextActiveCells[nextActiveCellCount++] = (Cell){nx, ny};
                    }
                }
            }
        }
    }

    // update display
    for (i = 0; i < activeCellCount; i++) {
        drawCell(activeCells[i].x, activeCells[i].y, false);
    }
    for (i = 0; i < nextActiveCellCount; i++) {
        drawCell(nextActiveCells[i].x, nextActiveCells[i].y, true);
    }
//    memset(displayBuffer, 0, DISPLAY_BUFFER_SIZE); // Prepare for next generation
//    drawCell(nx, ny, true);

    activeCellCount = nextActiveCellCount;
    memcpy(activeCells, nextActiveCells, sizeof(Cell) * nextActiveCellCount);
}

void activateCell(uint8_t x, uint8_t y) {
    if (activeCellCount < MAX_ACTIVE_CELLS) {
        activeCells[activeCellCount++] = (Cell){x, y};
        drawCell(x, y, true);
    }
}

void drawCell(uint8_t x, uint8_t y, bool state) {
    uint16_t byteIndex = (y / 8) * DISPLAY_WIDTH + x;
    if (state) {
        displayBuffer[byteIndex] |= (1 << (y % 8));
    } else {
        displayBuffer[byteIndex] &= ~(1 << (y % 8));
    }
}

int countNeighbors(uint8_t x, uint8_t y) {
    int dx, dy;
    int count = 0;
    for (dx = -1; dx <= 1; dx++) {
        for (dy = -1; dy <= 1; dy++) {
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

void delayMs(uint16_t ms) {
    while (ms--) {
        __delay_cycles(1000); // Adjust this for your clock speed
    }
}
