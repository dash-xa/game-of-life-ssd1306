// game_logic.h
#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdint.h>
#include "Io_SSD1306.h"

#define CELL_SIZE 4 // Each logical cell represents an 8x8 block of pixels
#define REDUCED_WIDTH (DISPLAY_WIDTH / CELL_SIZE)
#define REDUCED_HEIGHT (DISPLAY_HEIGHT / CELL_SIZE)
#define MAX_ACTIVE_CELLS 512 // Adjust based on your requirements

typedef struct {
    uint8_t x;
    uint8_t y;
} Cell;

void initializeGameWithVerticalLine(void);
void placeLWSS(uint8_t startX, uint8_t startY);
void placeGlider(uint8_t startX, uint8_t startY);
void initializeGameWithGliders(void);
void initializeGameLWSS(void);
void initializeGameWithPulsar(void);
void initializeGameWith2Pulsars(void);
#endif // GAME_LOGIC_H
