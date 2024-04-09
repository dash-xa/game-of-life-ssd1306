#include "Io_SSD1306.h"
#include <msp430.h>
#include "icons.h"  // Assuming this header is provided by you and contains image data for the display
#include <string.h> // For memcpy and memset

// Define the initialization command sequence for SSD1306
const ssd1306_cmd_t ssd1306_init_cmds[] = {
        SSD1306_DISPLAYOFF,
        SSD1306_MEMORYMODE | 0x0,
        SSD1306_COMSCANDEC,
        SSD1306_SETSTARTLINE | 0x00,
        SSD1306_SETCONTRAST, 0x7F,
        SSD1306_SEGREMAP | 0x01,
        SSD1306_NORMALDISPLAY,
        SSD1306_SETMULTIPLEX, 63,
        SSD1306_SETDISPLAYOFFSET, 0x00,
        SSD1306_SETDISPLAYCLOCKDIV, 0x80,
        SSD1306_SETPRECHARGE, 0x22,
        SSD1306_SETCOMPINS, 0x12,
        SSD1306_SETVCOMDETECT, 0x20,
        SSD1306_CHARGEPUMP, 0x14,
        SSD1306_DISPLAYALLON_RESUME,
        SSD1306_DISPLAYON,
};

// Initialize the I2C for SSD1306 communication
void i2c_init(void) {
    P3SEL |= BIT0 + BIT1; // Assign I2C pins

    UCB0CTL1 |= UCSWRST; // Enable SW reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC; // I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST; // Use SMCLK, keep SW reset
    UCB0BR0 = 2; // Set baud rate
    UCB0BR1 = 0;
    UCB0I2CSA = SSD1306_I2C_ADDRESS; // Set slave address
    UCB0CTL1 &= ~UCSWRST; // Clear SW reset, resume operation
}

// Transmit data over I2C
void i2c_transmit_pre(uint8_t pre, uint8_t *buf, int len) {
    int i;
    UCB0CTL1 |= UCTR + UCTXSTT; // Transmitter mode + start condition

    while (!(UCB0IFG & UCTXIFG)); // Wait until buffer is ready
    UCB0TXBUF = pre; // Send byte

    for (i = 0; i < len; i++) {
        while (!(UCB0IFG & UCTXIFG)); // Wait until buffer is ready
        UCB0TXBUF = buf[i]; // Send byte
    }

    while (!(UCB0IFG & UCTXIFG)); // Ensure the last byte is transmitted
    UCB0CTL1 |= UCTXSTP; // Send stop condition
    while (UCB0CTL1 & UCTXSTP); // Wait for stop condition to be sent
}

// Transmit data over I2C
void i2c_transmit(uint8_t *buf, int len) {
    int i;
    UCB0CTL1 |= UCTR + UCTXSTT; // Transmitter mode + start condition

    for (i = 0; i < len; i++) {
        while (!(UCB0IFG & UCTXIFG)); // Wait until buffer is ready
        UCB0TXBUF = buf[i]; // Send byte
    }

    while (!(UCB0IFG & UCTXIFG)); // Ensure the last byte is transmitted
    UCB0CTL1 |= UCTXSTP; // Send stop condition
    while (UCB0CTL1 & UCTXSTP); // Wait for stop condition to be sent
}

// Initialize the SSD1306 display
void ssd1306_init(void) {
    int i;
    for (i = 0; i < sizeof(ssd1306_init_cmds) / sizeof(ssd1306_cmd_t); i++) {
        uint8_t cmd[2] = {0x00, ssd1306_init_cmds[i]};
        i2c_transmit(cmd, 2);
    }
}

// Write an image to the SSD1306 display
void ssd1306_write(const uint8_t img[]) {
    uint8_t page_buf[] = {0x00, SSD1306_PAGEADDR, 0, 7};
    i2c_transmit(page_buf, sizeof(page_buf));

    uint8_t col_buf[] = {0x00, SSD1306_COLUMNADDR, 0, 127};
    i2c_transmit(col_buf, sizeof(col_buf));

    i2c_transmit_pre(0x40, img, SSD1306_BYTES + 1);
}

// Fill the display with a constant value
void ssd1306_write_constant(const uint8_t val) {
    uint8_t img[SSD1306_BYTES];
    memset(img, val, SSD1306_BYTES);
    ssd1306_write(img);
}
