// ssd1306.h

#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <stdbool.h>

// OLED I2C address
#define SSD1306_I2C_ADDRESS 0x3C

// OLED screen dimensions
#define SSD1306_HEIGHT 64
#define SSD1306_WIDTH 128

// Number of bytes needed to store the screen data
#define SSD1306_BYTES (SSD1306_HEIGHT * SSD1306_WIDTH / 8)

// Chunk size for I2C data transmission
#define I2C_DATA_CHUNK SSD1306_BYTES

// Enumeration for SSD1306 command codes
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

// Array of initialization commands for the SSD1306 display
extern const ssd1306_cmd_t ssd1306_init_cmds[];

// Function prototypes
void i2c_init(void);

void i2c_transmit(uint8_t *buf, int length);
void i2c_transmit_pre(uint8_t pre, uint8_t *buf, int len);

void ssd1306_init(void);

void ssd1306_write(const uint8_t img[]);

void ssd1306_write_constant(const uint8_t val);

#endif // SSD1306_H
