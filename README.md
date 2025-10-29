# Lightweight SSD1306 (128x64) I2C Driver for ESP-IDF (ESP32 & ESP32-S3)

This is a portable and efficient I2C driver for the standard 128x64 pixel SSD1306 OLED display, developed for the Espressif IoT Development Framework (ESP-IDF).
The driver is designed for efficiency and ease of use, utilizing a local 1KB framebuffer in SRAM for fast, flicker-free drawing operations before pushing the complete image to the display.

**Key Features**

 Chip Compatibility: Verified working on  ESP32-S3.

API: Uses the stable, legacy ESP-IDF I2C master driver API (driver/i2c.h) for maximum compatibility across different ESP-IDF versions.
Drawing Primitives: Full set of functions for drawing individual pixels, lines (screen_draw_line), rectangles (screen_draw_rect), and rounded rectangles (screen_draw_rrect).
Text & Graphics: Built-in 5x8 pixel font and support for drawing external column-major bitmaps (screen_draw_bitmap).

 Getting Started

## 1. Component Setup

Clone this repository into the components folder of your main ESP-IDF project:

```cd YOUR_ESP_IDF_PROJECT_ROOT```

```mkdir -p components/ssd1306_i2c```

```git clone [https://github.com/Raisback/ssd1306_esp_idf.git](https://github.com/Raisback/ssd1306_esp_idf.git) components/ssd1306_i2c```


## 2. Update Configuration (Crucial Step)

The most important step for portability is setting the correct I2C pins for your target chip. The driver defaults to pins common on some ESP32-S3 boards.
You must open components/ssd1306_i2c/ssd1306_i2c.h and update the GPIO pins to match your wiring.


**Default (As Configured)**

Recommended ESP32 Pinout

**2C_MASTER_SCL_IO --> 10**

**I2C_MASTER_SDA_IO -->  9**


**The full list of configurable items in the header file:**
```
#define I2C_MASTER_SCL_IO 10        // GPIO number for I2C master clock (SCL)
#define I2C_MASTER_SDA_IO 9         // GPIO number for I2C master data (SDA)
#define I2C_MASTER_NUM I2C_NUM_0    // I2C port number
#define I2C_MASTER_FREQ_HZ 400000   // I2C master clock frequency (400kHz is typical)
#define SSD1306_I2C_ADDRESS 0x3C    // 128x64 display I2C address (0x3C or 0x3D)
```

## 3. Usage Example

The following code demonstrates a simple initialization sequence, drawing primitives, and displaying text:
```
#include "ssd1306_i2c.h"

void app_main(void) {
    // 1. Initialize the I2C driver and SSD1306 controller
    screen_init();

    // 2. Clear the entire local framebuffer
    screen_fill(0x00); // 0x00 for black/off

    // 3. Draw content into the buffer
    screen_draw_rect(0, 0, OLED_WIDTH, OLED_HEIGHT, 1); // Border around the screen
    screen_draw_frect(5, 5, 20, 20, 1); // Filled square
    
    // Draw text (x, y, string, set/unset)
    screen_draw_str(30, 10, "Driver Ready", 1);
    screen_draw_str(30, 25, "ESP32 & S3", 1);
    
    // Draw a line
    screen_draw_line(10, 40, 120, 40, 1);

    // 4. Send the content of the buffer to the physical display
    screen_update_display();

    // The display will now show the content
    // ... rest of your application code ...
}
```

📜 License
This project is licensed under the MIT License. See the LICENSE file for details.
