#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "Io_SSD1306.h"
#include "GameOfLifeInitializations.h"

#define DELAY_TIME_US 250000


Cell activeCells[MAX_ACTIVE_CELLS];
Cell nextActiveCells[MAX_ACTIVE_CELLS];
uint16_t activeCellCount = 0;
uint16_t nextActiveCellCount = 0;

void (*games[4])(void) = {
    initializeGameWithVerticalLine,
    initializeGameWithGliders,
    initializeGameLWSS,
//    initializeGameWithPulsar,
    initializeGameWith2Pulsars
};
uint8_t gameIndex = 0;

uint8_t displayBuffer[DISPLAY_BUFFER_SIZE];

void initializeGame(void);
void updateGame(void);
void drawCell(uint8_t x, uint8_t y, bool state);
int countNeighbors(uint8_t x, uint8_t y);
void delayMs(uint16_t ms);

void startGame(uint8_t i) {
    clearDisplayBuffer();
    activeCellCount = 0;
    games[i]();
}

void initInterruptLogic() {
    P1DIR = 0b00000001;             // Set  P1.0 pin for output rest including  P1.1 are outputs
    P1OUT = 0b00000010;             // Set Pin P1.0  to low and P1.1 to pullup
    P1REN = 0b00000010;             // Enable pull up/down resistor on P1.1
    P1IE =  0b00000010;             // Enable input at P1.1 as an interrupt
    _BIS_SR(GIE);                  // Turn on interrupts
}

int main(void) {
    WDTCTL = WDTPW + WDTHOLD;       // Stop watchdog timer
    initInterruptLogic();

    // Set up game
    i2c_init(); // Initialize I2C for SSD1306
    ssd1306_init(); // Initialize the OLED display

    ssd1306_write_constant(0xFF);
    delayMs(1000);

    ssd1306_write_constant(0);
    delayMs(1000);

    while (1) {
        uint8_t runningGame = gameIndex;

        startGame(runningGame);
        ssd1306_write(displayBuffer); // Update display
//        delayMs(1000);

        while (runningGame == gameIndex) {
            updateGame();
            ssd1306_write(displayBuffer);
        }
    }
}

void __attribute__ ((interrupt(PORT1_VECTOR))) PORT1_ISR(void) {
    gameIndex = (gameIndex + 1) % (sizeof(games) / sizeof(games[0]));
//    P1OUT ^= 0b00000001;
    P1IFG &= ~0b00000010; // Clear P1.1 IFG. If you don't, it just happens again.
}

void initializeGame(void) {
    initializeGameLWSS();
}

void clearDisplayBuffer() {
    memset(displayBuffer, 0, DISPLAY_BUFFER_SIZE); // Clear the display buffer
}

void activateCellAndIncrementCount(uint8_t reducedX, uint8_t reducedY) {
    activeCells[activeCellCount++] = (Cell){reducedX, reducedY};
    drawCell(reducedX, reducedY, true);
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
        __delay_cycles(1000);
    }
}
