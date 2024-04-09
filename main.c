#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#include "icons.h"
#include "Io_SSD1306.h"
#define SNAKE_LENGTH        64

#define min(a, b) ((a) < (b) ? (a) : (b))

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define GRID_SIZE 8
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_BUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT / 8)
#define BLOCK_SIZE (DISPLAY_WIDTH / GRID_SIZE)

uint8_t displayBuffer[DISPLAY_BUFFER_SIZE];
bool grid[GRID_SIZE][GRID_SIZE] = {0};
bool nextGrid[GRID_SIZE][GRID_SIZE] = {0};

// Forward declarations
void initializeGame(void);
void updateGame(void);
void drawGridToDisplayBuffer(void);
void toggleRandomCell(void);
int countNeighbors(int x, int y);
extern void ssd1306_write(const uint8_t img[]); // Assuming this function is defined elsewhere

void initializeGame(void) {
    // Clear the grid
    memset(grid, 0, sizeof(grid));

    // Initialize with a "Beacon" pattern
    grid[1][1] = true;
    grid[1][2] = true;
    grid[2][1] = true;
    grid[2][2] = true;

    grid[3][3] = true;
    grid[3][4] = true;
    grid[4][3] = true;
    grid[4][4] = true;
}

void updateGame(void) {
    int x, y;
    for (x = 0; x < GRID_SIZE; x++) {
        for (y = 0; y < GRID_SIZE; y++) {
            int neighbors = countNeighbors(x, y);
            nextGrid[x][y] = (grid[x][y] && (neighbors == 2 || neighbors == 3)) || (!grid[x][y] && neighbors == 3);
        }
    }
    memcpy(grid, nextGrid, sizeof(grid));
}

void drawGridToDisplayBuffer(void) {
    memset(displayBuffer, 0, DISPLAY_BUFFER_SIZE); // Clear buffer for fresh draw
    int x, y, bx, by;
    for (x = 0; x < GRID_SIZE; x++) {
        for (y = 0; y < GRID_SIZE; y++) {
            if (grid[x][y]) {
                for (bx = 0; bx < BLOCK_SIZE; bx++) {
                    for (by = 0; by < BLOCK_SIZE; by++) {
                        int pixelIndex = ((y * BLOCK_SIZE + by) * DISPLAY_WIDTH + (x * BLOCK_SIZE + bx)) / 8;
                        int bitPosition = 7 - ((y * BLOCK_SIZE + by) % 8);
                        displayBuffer[pixelIndex] |= (1 << bitPosition);
                    }
                }
            }
        }
    }
}

void toggleRandomCell(void) {
    // This would ideally use a better random generator suited for your needs
    int x = rand() % GRID_SIZE;
    int y = rand() % GRID_SIZE;
    grid[x][y] = !grid[x][y];
}

int countNeighbors(int x, int y) {
    int count = 0, dx, dy;
    for (dx = -1; dx <= 1; dx++) {
        for (dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue; // Skip the cell itself
            int nx = (x + dx + GRID_SIZE) % GRID_SIZE;
            int ny = (y + dy + GRID_SIZE) % GRID_SIZE;
            count += grid[nx][ny] ? 1 : 0;
        }
    }
    return count;
}




void clear_buffer(uint8_t* buffer) {
    memset(buffer, 0, SSD1306_BYTES);
}

void draw_snake(uint8_t* buffer, uint16_t headPosition) {
    uint16_t segmentLength = SNAKE_LENGTH;
    uint16_t position = headPosition;

    while (segmentLength--) {
        uint16_t x = 0, y = 0;

        // Determine the segment's position (x, y) based on the current position around the perimeter
        if (position < SSD1306_WIDTH) {
            // Top edge
            x = position;
            y = 0;
        } else if (position < (SSD1306_WIDTH + SSD1306_HEIGHT - 2)) {
            // Right edge
            x = SSD1306_WIDTH - 1;
            y = position - SSD1306_WIDTH + 1;
        } else if (position < (2 * SSD1306_WIDTH + SSD1306_HEIGHT - 3)) {
            // Bottom edge
            x = 2 * SSD1306_WIDTH + SSD1306_HEIGHT - 4 - position;
            y = SSD1306_HEIGHT - 1;
        } else {
            // Left edge
            x = 0;
            y = 2 * (SSD1306_WIDTH + SSD1306_HEIGHT - 2) - position - 1;
        }

        // Convert (x, y) to buffer index and bit position
        uint16_t index = (y / 8) * SSD1306_WIDTH + x;
        uint8_t bit_position = y % 8;
        buffer[index] |= (1 << bit_position);

        // Move to the next position
        position = (position + 1) % (2 * (SSD1306_WIDTH + SSD1306_HEIGHT - 2));
    }
}


int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    i2c_init();
    init_WDT();
    init_LED();
    __enable_interrupt();       // Enable global interrupts

    ssd1306_init();             // Initialize the OLED display

    initializeGame();
    drawGridToDisplayBuffer();
    ssd1306_write(displayBuffer);

    // Main loop
    while(1) {
        updateGame();
        drawGridToDisplayBuffer();
        ssd1306_write(displayBuffer);
        __delay_cycles(500000); // Simple delay; adjust as needed
//        toggleRandomCell(); // For demonstration, toggle a random cell each cycle
    }


    while (1) {
        // Main loop can perform other tasks or be put to sleep to save power
        __bis_SR_register(LPM3_bits + GIE); // Enter LPM3 with interrupts
    }
}

void init_WDT(void) {
    WDTCTL = WDT_ADLY_1000;  // WDT interval timer mode, ACLK, 1000ms
    SFRIE1 |= WDTIE;         // Enable WDT interrupt
}

void init_LED(void) {
    P1DIR |= BIT0;           // Set P1.0 to output direction (LED)
    P1OUT &= ~BIT0;          // Start with LED off
}

// WDT Interrupt Service Routine
#pragma vector=WDT_VECTOR

__interrupt void WDT_ISR(void) {
    toggle_LED();           // Toggle LED every WDT interrupt (1 second)
}

void toggle_LED(void) {
    P1OUT ^= BIT0;          // Toggle P1.0 using exclusive-OR
}

