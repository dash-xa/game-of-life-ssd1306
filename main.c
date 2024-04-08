#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#include "icons.h"
#include "Io_SSD1306.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    i2c_init();
    init_WDT();
    init_LED();
    __enable_interrupt();       // Enable global interrupts

    ssd1306_init();             // Initialize the OLED display

    while (1) {
        ssd1306_write_constant(0x0);
        __delay_cycles(200000);

        ssd1306_write_constant(0xFF);
        __delay_cycles(200000);
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
