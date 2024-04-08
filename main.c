#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#include "icons.h"

#define SSD1306_I2C_ADDRESS 0x3C // OLED I2C address
#define SSD1306_HEIGHT 64
#define SSD1306_WIDTH 128
#define SSD1306_BYTES (SSD1306_HEIGHT * SSD1306_WIDTH / 8)

#define min(a, b) ((a) < (b) ? (a) : (b))
#define I2C_DATA_CHUNK SSD1306_BYTES



// Prototypes
void init_WDT(void);

void init_LED(void);

void toggle_LED(void);

void i2c_init(void);

void i2c_transmit(uint8_t *buf, int len);

void ssd1306_init(void);

void ssd1306_write(int x);

typedef enum {
    SSD1306_SETLOWCOLUMN = 0x00,
    SSD1306_SETHIGHCOLUMN = 0x10,
    SSD1306_MEMORYMODE = 0x20,
    SSD1306_COLUMNADDR = 0x21,
    SSD1306_PAGEADDR = 0x22,
    SSD1306_SETSTARTLINE = 0x40,
    SSD1306_DEFAULT_ADDRESS = 0x78,
    SSD1306_SETCONTRAST = 0x81,
    SSD1306_CHARGEPUMP = 0x8D,
    SSD1306_SEGREMAP = 0xA0,
    SSD1306_DISPLAYALLON_RESUME = 0xA4,
    SSD1306_DISPLAYALLON = 0xA5,
    SSD1306_NORMALDISPLAY = 0xA6,
    SSD1306_INVERTDISPLAY = 0xA7,
    SSD1306_SETMULTIPLEX = 0xA8,
    SSD1306_DISPLAYOFF = 0xAE,
    SSD1306_DISPLAYON = 0xAF,
    SSD1306_SETPAGE = 0xB0,
    SSD1306_COMSCANINC = 0xC0,
    SSD1306_COMSCANDEC = 0xC8,
    SSD1306_SETDISPLAYOFFSET = 0xD3,
    SSD1306_SETDISPLAYCLOCKDIV = 0xD5,
    SSD1306_SETPRECHARGE = 0xD9,
    SSD1306_SETCOMPINS = 0xDA,
    SSD1306_SETVCOMDETECT = 0xDB,

    SSD1306_SWITCHCAPVCC = 0x02,
    SSD1306_NOP = 0xE3,
} ssd1306_cmd_t;

// From: https://github.com/lexus2k/ssd1306/blob/master/src/lcd/ssd1306_commands.h#L44
static const ssd1306_cmd_t ssd1306_init_cmds[] = {
        SSD1306_DISPLAYOFF, // display off
        SSD1306_MEMORYMODE | 0x0, // Page Addressing mode
        SSD1306_COMSCANDEC,             // Scan from 127 to 0 (Reverse scan)
        SSD1306_SETSTARTLINE | 0x00,    // First line to start scanning from
        SSD1306_SETCONTRAST, 0x7F,      // contast value to 0x7F according to datasheet
        SSD1306_SEGREMAP | 0x01,        // Use reverse mapping. 0x00 - is normal mapping
        SSD1306_NORMALDISPLAY,
        SSD1306_SETMULTIPLEX, 63,       // Reset to default MUX. See datasheet
        SSD1306_SETDISPLAYOFFSET, 0x00, // no offset
        SSD1306_SETDISPLAYCLOCKDIV, 0x80,// set to default ratio/osc frequency
        SSD1306_SETPRECHARGE, 0x22,     // switch precharge to 0x22 // 0xF1
        SSD1306_SETCOMPINS, 0x12,       // set divide ratio
        SSD1306_SETVCOMDETECT, 0x20,    // vcom deselect to 0x20 // 0x40
        SSD1306_CHARGEPUMP, 0x14,       // Enable charge pump
        SSD1306_DISPLAYALLON_RESUME,
        SSD1306_DISPLAYON,
};

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    i2c_init();
    init_WDT();
    init_LED();
    __enable_interrupt();       // Enable global interrupts

    ssd1306_init();             // Initialize the OLED display

    while (1) {
        ssd1306_write(0);
        __delay_cycles(1000000);

//        ssd1306_write(0xFF);
//        __delay_cycles(1000000);

//        ssd1306_write(0x0F);
//        __delay_cycles(1000000);
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


void i2c_init(void) {
    P3SEL |= BIT0 + BIT1;        // Assign P3.0 and P3.1 to I2C function

    UCB0CTL1 |= UCSWRST;         // Enable SW reset

    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC; // I2C Master, sync mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST; // Use SMCLK, keep SW reset

    UCB0BR0 = 10;                // fSCL = SMCLK/10 ~ 100kHz
    UCB0BR1 = 0;

    UCB0I2CSA = SSD1306_I2C_ADDRESS; // Set I2C address

    UCB0CTL1 &= ~UCSWRST;        // Clear SW reset, resume operation
}

void i2c_transmit(uint8_t *buf, int length) {
    UCB0CTL1 |= UCTR; // Transmitter mode
    UCB0CTL1 |= UCTXSTT; // Generate start condition

    int i;
    for (i = 0; i < length; i++) {
        UCB0TXBUF = buf[i]; // Write next data byte to register
        while (!(UCB0IFG & UCTXIFG)); // Wait for flag to confirm data was sent
    }

    UCB0CTL1 |= UCTXSTP; // Send stop condition
    while (!(UCB0CTL1 & UCTXSTP)); // Wait to confirm stop

    __delay_cycles(100); // This shouldn't be necessary but this micro is bad
}

void ssd1306_init(void) {
    int i = 0;
    for (i = 0; i < sizeof(ssd1306_init_cmds); i++) {
        uint8_t cmd_buf[2] = {0x00, ssd1306_init_cmds[i]};
        i2c_transmit(cmd_buf, 2);
    }
}

// Use the provided font array to write characters to the screen buffer
//void writeCharToBuffer(uint8_t* screen, char character, uint8_t x, uint8_t y) {
//    if(character < ' ' || character > '~') return;
//
//    int offset = (character - ' ') * FONTWIDTH;
//    for(int col = 0; col < FONTWIDTH; col++) {
//        uint8_t columnData = FONT6x8[offset + col];
//        for(int bit = 0; bit < FONTHEIGHT; bit++) {
//            if(columnData & (1 << bit)) {
//                int pixelIndex = x + col + (y + bit) * SSD1306_WIDTH;
//                screen[pixelIndex / 8] |= (1 << (pixelIndex % 8));
//            }
//        }
//    }
//}


uint8_t screen[7] = {0x40, 0x28, 0xFE, 0x28, 0xFE, 0x28, 0x00};

// Reset page and column addresses.
void reset_addresses() {
    const int8_t page_buf[4] = {0x00, SSD1306_PAGEADDR, 0, 7};
    i2c_transmit(page_buf, sizeof(page_buf));

    const int8_t col_buf[4] = {0x00, SSD1306_COLUMNADDR, 0, 127};
    i2c_transmit(col_buf, sizeof(col_buf));
}

void copy_image(const char img[], int length) {
    int i;
    for(i = 0; i < length; i += I2C_DATA_CHUNK) {
        uint8_t data_buf[I2C_DATA_CHUNK + 1];
        data_buf[0] = 0x40;

        const int chunk_size = min(I2C_DATA_CHUNK, length - i);
        memcpy(&data_buf[1], &img[i], chunk_size);

        i2c_transmit(data_buf, chunk_size + 1);
    }
}

void solid_background(const uint8_t val) {
    uint8_t img[SSD1306_BYTES];
    memset(img, val, sizeof(img) / sizeof(img[0]));
    copy_image(img, SSD1306_BYTES);
}

void image_draw(const char image[], char height, char width, char row, char column) {
    reset_addresses();
    int z;
    for (z = 0; z < 10; z++) {
        solid_background(0x0);
        __delay_cycles(2000000);

        solid_background(0xFF);
        __delay_cycles(2000000);
    }
}

void ssd1306_write(int x) {

    image_draw(brain, brain[1], brain[0], 0, 0);

    __delay_cycles(10000); // This shouldn't be necessary but this micro is bad
//    }
}
