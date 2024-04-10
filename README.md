# Conway's Game Of Life on an SSD1306

## Hardware
- MCU: MSP430F5529
- Display: "[OLED Display Module, 128 x 64 0.96 inches OLED Display 12864 LCD Module for 51 Series MSP430 STM32 electronical Accessories OLED Display](https://www.amazon.ca/dp/B07YNP2L95?psc=1&ref=ppx_yo2ov_dt_b_product_details)"

## Software
No drivers were used. Everything was written from scratch, including I2C communication and display buffer transmission logic. The core application logic is in `main.c`, while `Io_SSD1306.c` contains the display IO logic.

## Demo
![demo](https://github.com/dash-xa/game-of-life-ssd1306/assets/27713668/c9b59a82-02f6-477d-b606-77b99a68b965)
