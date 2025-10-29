#ifndef SSD1306_I2C_H
#define SSD1306_I2C_H

#include <stdint.h>
#include "driver/i2c.h" // For I2C_MASTER_NUM type

// --- Display Configuration ---
// Change these values to match your specific setup (e.g., if using a 128x32 screen)
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// --- I2C Configuration ---
// WARNING: Change these based on your ESP32/ESP32-S2/C3 board wiring
#define I2C_MASTER_SCL_IO 10        // GPIO number for I2C master clock (SCL)
#define I2C_MASTER_SDA_IO 9         // GPIO number for I2C master data (SDA)
#define I2C_MASTER_NUM I2C_NUM_0    // I2C port number
#define I2C_MASTER_FREQ_HZ 400000   // I2C master clock frequency (400kHz is typical)
#define SSD1306_I2C_ADDRESS 0x3C    // 128x64 display I2C address (0x3C or 0x3D)


// --- Function Prototypes ---

/**
 * @brief Initializes the I2C driver and sends the SSD1306 configuration commands.
 */
void screen_init();

/**
 * @brief Sends the local framebuffer content to the physical display.
 */
void screen_update_display();

/**
 * @brief Fills the entire local framebuffer with a given pattern (0x00 for clear, 0xFF for full).
 * @param pattern The byte value to fill the buffer with.
 */
void screen_fill_buffer(uint8_t pattern);

/**
 * @brief Sets or clears a single pixel in the local framebuffer.
 * @param x X coordinate (0 to OLED_WIDTH - 1).
 * @param y Y coordinate (0 to OLED_HEIGHT - 1).
 * @param set 1 to set (draw), 0 to clear (erase).
 */
void screen_set_pixel(int16_t x, int16_t y, uint8_t set);


// --- Graphics Primitives ---

/**
 * @brief Draws a line between two points (x1, y1) and (x2, y2) using Bresenham's algorithm.
 */
void screen_draw_line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t set);

/**
 * @brief Draws the outline of a rectangle.
 */
void screen_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t set);

/**
 * @brief Draws a filled rectangle.
 */
void screen_draw_frect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t set);

/**
 * @brief Draws the outline of a rectangle with rounded corners.
 */
void screen_draw_rrect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint8_t set);

/**
 * @brief Draws a filled rectangle with rounded corners.
 */
void screen_draw_rfrect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint8_t set);


// --- Text & Bitmap Functions ---

/**
 * @brief Draws a single 5x8 character.
 * @note This function ignores the 'set' parameter, it always draws using the 5x8 font table.
 */
void screen_draw_char(uint8_t x, uint8_t y, char c, uint8_t set);

/**
 * @brief Draws a null-terminated string. Assumes a 5x8 font size + 1 pixel spacer.
 */
void screen_draw_str(uint8_t x, uint8_t y, const char *str, uint8_t set);

/**
 * @brief Draws a bitmap from column-major data at a specified location.
 * @note This is the core bitmap function with the fixed vertical orientation logic.
 * @param x_start Starting X coordinate.
 * @param y_start Starting Y coordinate.
 * @param w Width of the bitmap in pixels.
 * @param h Height of the bitmap in pixels.
 * @param bitmap Pointer to the column-major bitmap data array.
 * @param set 1 to draw foreground, 0 to draw background.
 */
void screen_draw_bitmap(uint8_t x_start, uint8_t y_start, uint8_t w, uint8_t h, const uint8_t *bitmap, uint8_t set);


#endif // SSD1306_I2C_H

