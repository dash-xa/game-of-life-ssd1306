#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#include "icons.h"
#include "Io_SSD1306.h"
#define SNAKE_LENGTH        5


#define min(a, b) ((a) < (b) ? (a) : (b))

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

    ssd1306_write_constant(0x00);

    uint8_t displayBuffer[SSD1306_BYTES];
    uint16_t headPosition = 0;
    const uint16_t maxPosition = 2 * (SSD1306_WIDTH + SSD1306_HEIGHT - 2); // Total perimeter length

    while (1) {
        clear_buffer(displayBuffer);
        draw_snake(displayBuffer, headPosition);

        ssd1306_write(displayBuffer);
//        __delay_cycles(10);  // Adjust for your system's timing


        headPosition = (headPosition + 1) % maxPosition;
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
