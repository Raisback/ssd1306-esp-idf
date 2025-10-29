#include "ssd1306_i2c.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// ESP-IDF headers for I2C communication
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


// --- Private Definitions ---
#define TAG "SSD1306_DRIVER"
#define SSD1306_CONTROL_BYTE_CMD 0x00
#define SSD1306_CONTROL_BYTE_DATA 0x40
#define SSD1306_ADDR_MODE_HORIZONTAL 0x00
#define OLED_PAGE_COUNT (OLED_HEIGHT / 8)

// Local Frame Buffer (128 columns * 8 pages = 1024 bytes)
static uint8_t oled_buffer[OLED_WIDTH * OLED_PAGE_COUNT];

// --- 5x8 PIXEL FONT TABLE (Shortened for brevity) ---
static const uint8_t font_table[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, {0x00, 0x00, 0x5F, 0x00, 0x00}, {0x00, 0x07, 0x00, 0x07, 0x00}, {0x14, 0x7F, 0x14, 0x7F, 0x14}, {0x24, 0x2A, 0x7F, 0x2A, 0x12}, {0x23, 0x13, 0x08, 0x64, 0x62}, {0x36, 0x49, 0x56, 0x20, 0x50}, {0x00, 0x05, 0x03, 0x00, 0x00}, {0x00, 0x1C, 0x22, 0x41, 0x00}, {0x00, 0x41, 0x22, 0x1C, 0x00}, {0x08, 0x2A, 0x1C, 0x2A, 0x08}, {0x08, 0x08, 0x3E, 0x08, 0x08}, {0x00, 0x50, 0x30, 0x00, 0x00}, {0x08, 0x08, 0x08, 0x08, 0x08}, {0x00, 0x60, 0x60, 0x00, 0x00}, {0x20, 0x10, 0x08, 0x04, 0x02},
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, {0x00, 0x42, 0x7F, 0x40, 0x00}, {0x62, 0x51, 0x49, 0x46, 0x40}, {0x22, 0x41, 0x49, 0x49, 0x36}, {0x18, 0x14, 0x12, 0x7F, 0x10}, {0x2F, 0x49, 0x49, 0x49, 0x31}, {0x3C, 0x4A, 0x49, 0x49, 0x30}, {0x01, 0x71, 0x09, 0x05, 0x03}, {0x36, 0x49, 0x49, 0x49, 0x36}, {0x06, 0x49, 0x49, 0x49, 0x3E},
    {0x00, 0x36, 0x36, 0x00, 0x00}, {0x00, 0x56, 0x36, 0x00, 0x00}, {0x08, 0x14, 0x22, 0x41, 0x00}, {0x14, 0x14, 0x14, 0x14, 0x14}, {0x00, 0x41, 0x22, 0x14, 0x08}, {0x02, 0x01, 0x51, 0x09, 0x06}, {0x3E, 0x41, 0x59, 0x49, 0x7E},
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, {0x7F, 0x49, 0x49, 0x49, 0x36}, {0x3E, 0x41, 0x41, 0x41, 0x22}, {0x7F, 0x41, 0x41, 0x41, 0x3E}, {0x7F, 0x49, 0x49, 0x49, 0x41}, {0x7F, 0x09, 0x09, 0x01, 0x01}, {0x3E, 0x41, 0x49, 0x49, 0x7A}, {0x7F, 0x08, 0x08, 0x08, 0x7F}, {0x00, 0x41, 0x7F, 0x41, 0x00}, {0x30, 0x40, 0x40, 0x40, 0x7F}, {0x7F, 0x08, 0x14, 0x22, 0x41}, {0x7F, 0x40, 0x40, 0x40, 0x40}, {0x7F, 0x02, 0x04, 0x02, 0x7F}, {0x7F, 0x04, 0x08, 0x10, 0x7F}, {0x3E, 0x41, 0x41, 0x41, 0x3E}, {0x7F, 0x09, 0x09, 0x09, 0x06}, {0x3E, 0x41, 0x51, 0x21, 0x5E}, {0x7F, 0x09, 0x19, 0x29, 0x46}, {0x46, 0x49, 0x49, 0x49, 0x31}, {0x01, 0x01, 0x7F, 0x01, 0x01}, {0x3F, 0x40, 0x40, 0x40, 0x3F}, {0x1F, 0x20, 0x40, 0x20, 0x1F}, {0x3F, 0x40, 0x30, 0x40, 0x3F},
    {0x41, 0x22, 0x1C, 0x22, 0x41}, {0x07, 0x08, 0x70, 0x08, 0x07}, {0x41, 0x61, 0x51, 0x49, 0x47},
    {0x00, 0x7F, 0x41, 0x41, 0x00}, {0x02, 0x04, 0x08, 0x10, 0x20}, {0x00, 0x41, 0x41, 0x7F, 0x00}, {0x04, 0x02, 0x01, 0x02, 0x04}, {0x40, 0x40, 0x40, 0x40, 0x40}, {0x00, 0x01, 0x02, 0x04, 0x00},
    {0x20, 0x54, 0x54, 0x54, 0x78}, {0x7F, 0x44, 0x44, 0x44, 0x38}, {0x38, 0x44, 0x44, 0x44, 0x20}, {0x38, 0x44, 0x44, 0x44, 0x7F}, {0x38, 0x54, 0x54, 0x54, 0x18}, {0x08, 0x7E, 0x09, 0x01, 0x02}, {0x08, 0x54, 0x54, 0x54, 0x3c}, {0x7F, 0x04, 0x04, 0x04, 0x78}, {0x00, 0x44, 0x7D, 0x40, 0x00}, {0x20, 0x40, 0x44, 0x3D, 0x00}, {0x7F, 0x10, 0x28, 0x44, 0x00}, {0x00, 0x01, 0x7F, 0x40, 0x00}, {0x7C, 0x04, 0x18, 0x04, 0x78}, {0x7C, 0x04, 0x04, 0x04, 0x78}, {0x38, 0x44, 0x44, 0x44, 0x38}, {0x7C, 0x14, 0x14, 0x14, 0x08}, {0x08, 0x14, 0x14, 0x14, 0x7C}, {0x7C, 0x04, 0x04, 0x04, 0x08}, {0x48, 0x54, 0x54, 0x54, 0x20}, {0x04, 0x3E, 0x44, 0x44, 0x24}, {0x3C, 0x40, 0x40, 0x20, 0x7C}, {0x1C, 0x20, 0x40, 0x20, 0x1C}, {0x3C, 0x40, 0x30, 0x40, 0x3C},
    {0x44, 0x28, 0x10, 0x28, 0x44}, {0x0C, 0x50, 0x50, 0x50, 0x3C}, {0x64, 0x54, 0x54, 0x54, 0x4C},
    {0x00, 0x36, 0x49, 0x41, 0x00}, {0x00, 0x00, 0x7F, 0x00, 0x00}, {0x00, 0x41, 0x49, 0x36, 0x00}, {0x00, 0x40, 0x20, 0x40, 0x20}
};

// --- Private I2C Functions ---

/**
 * @brief Sends a command byte to the SSD1306 via I2C.
 * @param command The command byte to send.
 * @return esp_err_t ESP_OK on success.
 */
static esp_err_t ssd1306_command(uint8_t command)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SSD1306_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, SSD1306_CONTROL_BYTE_CMD, true);
    i2c_master_write_byte(cmd, command, true);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// --- Public Screen Interface Implementations ---

void screen_init()
{
    // 1. Initialize I2C Master
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {
            .clk_speed = I2C_MASTER_FREQ_HZ, 
        },
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    // FIX: Replaced I2C_MASTER_RX_BUF_DISABLE and I2C_MASTER_TX_BUF_DISABLE with 0
    // This is required for newer ESP-IDF versions (like v5.x)
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0));
    ESP_LOGI(TAG, "I2C Master initialized and driver installed.");

    // 2. Initialize SSD1306
    ssd1306_command(0xAE); // Display OFF
    ssd1306_command(0x20); ssd1306_command(SSD1306_ADDR_MODE_HORIZONTAL); // Horizontal Addressing Mode
    ssd1306_command(0xA8); ssd1306_command(OLED_HEIGHT - 1); // Set Multiplex Ratio (63 for 64-height)
    ssd1306_command(0xD3); ssd1306_command(0x00); // Set Display Offset
    ssd1306_command(0x40); // Set Display Start Line 
    ssd1306_command(0x8D); ssd1306_command(0x14); // Charge Pump Setting
    ssd1306_command(0xA1); // Segment Remap (0xA1 reverses columns)
    ssd1306_command(0xC8); // COM Output Scan Direction (0xC8 reverses rows)
    ssd1306_command(0xDA); ssd1306_command(0x12); // COM Pins Hardware Configuration
    ssd1306_command(0x81); ssd1306_command(0xCF); // Set Contrast
    ssd1306_command(0xD9); ssd1306_command(0xF1); // Set Pre-charge Period
    ssd1306_command(0xDB); ssd1306_command(0x40); // Set VCOM Deselect Level
    ssd1306_command(0xA4); // Entire Display ON/OFF
    ssd1306_command(0xA6); // Set Normal/Inverse Display
    ssd1306_command(0xAF); // Display ON
    
    screen_fill_buffer(0x00); // Clear the buffer
    screen_update_display();  // Send the clear buffer to screen
    ESP_LOGI(TAG, "SSD1306 initialized successfully.");
}

void screen_update_display()
{
    // 1. Set column and page start/end addresses for Horizontal Addressing Mode
    i2c_cmd_handle_t cmd_set_addr = i2c_cmd_link_create();
    i2c_master_start(cmd_set_addr);
    i2c_master_write_byte(cmd_set_addr, (SSD1306_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_set_addr, SSD1306_CONTROL_BYTE_CMD, true);

    // Set Column Address Start/End (0 to 127)
    i2c_master_write_byte(cmd_set_addr, 0x21, true); 
    i2c_master_write_byte(cmd_set_addr, 0x00, true); 
    i2c_master_write_byte(cmd_set_addr, OLED_WIDTH - 1, true); 

    // Set Page Address Start/End (0 to 7)
    i2c_master_write_byte(cmd_set_addr, 0x22, true); 
    i2c_master_write_byte(cmd_set_addr, 0x00, true); 
    i2c_master_write_byte(cmd_set_addr, OLED_PAGE_COUNT - 1, true); 

    i2c_master_stop(cmd_set_addr);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd_set_addr, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_set_addr);

    // 2. Send the entire data buffer (1024 bytes)
    i2c_cmd_handle_t cmd_send_data = i2c_cmd_link_create();
    i2c_master_start(cmd_send_data);
    i2c_master_write_byte(cmd_send_data, (SSD1306_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_send_data, SSD1306_CONTROL_BYTE_DATA, true); 

    i2c_master_write(cmd_send_data, oled_buffer, OLED_WIDTH * OLED_PAGE_COUNT, true);

    i2c_master_stop(cmd_send_data);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd_send_data, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_send_data);
}

void screen_fill_buffer(uint8_t pattern)
{
    memset(oled_buffer, pattern, OLED_WIDTH * OLED_PAGE_COUNT);
}

void screen_set_pixel(int16_t x, int16_t y, uint8_t set)
{
    // Boundary check
    if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT) {
        return; 
    }

    // Calculate index: x + (page * width)
    uint16_t byte_index = x + (y / 8) * OLED_WIDTH;
    // Calculate bit mask: bit position within the byte (0-7)
    uint8_t bit_mask = 1 << (y % 8);

    if (set) {
        oled_buffer[byte_index] |= bit_mask;    // Set bit (Turn pixel ON)
    } else {
        oled_buffer[byte_index] &= ~bit_mask;   // Clear bit (Turn pixel OFF)
    }
}

// --- Drawing Primitives Implementation ---

void screen_draw_line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t set)
{
    int16_t dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int16_t dy = abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int16_t err = (dx > dy ? dx : -dy) / 2;
    int16_t e2;

    for(;;) {
        screen_set_pixel(x1, y1, set);
        if (x1 == x2 && y1 == y2) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x1 += sx; }
        if (e2 < dy) { err += dx; y1 += sy; }
    }
}

void screen_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t set)
{
    // Top, Bottom, Left, Right lines
    screen_draw_line(x, y, x + w - 1, y, set);
    screen_draw_line(x, y + h - 1, x + w - 1, y + h - 1, set);
    screen_draw_line(x, y, x, y + h - 1, set);
    screen_draw_line(x + w - 1, y, x + w - 1, y + h - 1, set);
}

void screen_draw_frect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t set)
{
    // Simple vertical line fill method
    for (int16_t i = x; i < x + w; i++) {
        screen_draw_line(i, y, i, y + h - 1, set);
    }
}

static void screen_draw_circle_quadrant(int16_t x0, int16_t y0, int16_t r, uint8_t corner_mask, uint8_t set) {
    int16_t x = 0;
    int16_t y = r;
    int16_t d = 1 - r;

    while (x <= y) {
        // Upper Right (Quadrant 4, mask 0x08)
        if (corner_mask & 0x08) { screen_set_pixel(x0 + x, y0 - y, set); screen_set_pixel(x0 + y, y0 - x, set); }
        // Upper Left (Quadrant 3, mask 0x04)
        if (corner_mask & 0x04) { screen_set_pixel(x0 - x, y0 - y, set); screen_set_pixel(x0 - y, y0 - x, set); }
        // Lower Left (Quadrant 2, mask 0x02)
        if (corner_mask & 0x02) { screen_set_pixel(x0 - x, y0 + y, set); screen_set_pixel(x0 - y, y0 + x, set); }
        // Lower Right (Quadrant 1, mask 0x01)
        if (corner_mask & 0x01) { screen_set_pixel(x0 + x, y0 + y, set); screen_set_pixel(x0 + y, y0 + x, set); }

        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

void screen_draw_rrect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint8_t set)
{
    if (r <= 0) {
        screen_draw_rect(x, y, w, h, set);
        return;
    }
    // Constrain radius
    if (r > w/2 || r > h/2) {
        r = (w < h ? w : h) / 2;
    }

    // Horizontal segments
    screen_draw_line(x + r, y, x + w - r - 1, y, set);             // Top
    screen_draw_line(x + r, y + h - 1, x + w - r - 1, y + h - 1, set); // Bottom
    // Vertical segments
    screen_draw_line(x, y + r, x, y + h - r - 1, set);                 // Left
    screen_draw_line(x + w - 1, y + r, x + w - 1, y + h - r - 1, set); // Right

    // Draw the four quarter circles (1=LR, 2=LL, 3=UL, 4=UR)
    screen_draw_circle_quadrant(x + w - r - 1, y + h - r - 1, r, 0x01, set); // Lower Right
    screen_draw_circle_quadrant(x + r, y + h - r - 1, r, 0x02, set); // Lower Left
    screen_draw_circle_quadrant(x + r, y + r, r, 0x04, set); // Upper Left
    screen_draw_circle_quadrant(x + w - r - 1, y + r, r, 0x08, set); // Upper Right
}

void screen_draw_rfrect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint8_t set)
{
    if (r <= 0) {
        screen_draw_frect(x, y, w, h, set);
        return;
    }
    if (r > w/2 || r > h/2) {
        r = (w < h ? w : h) / 2;
    }
    
    // 1. Draw the central, large rectangle 
    screen_draw_frect(x, y + r, w, h - 2 * r, set); 

    // 2. Draw the top and bottom rectangular caps 
    screen_draw_frect(x + r, y, w - 2 * r, r, set);
    screen_draw_frect(x + r, y + h - r, w - 2 * r, r, set);

    // 3. Fill the four quarter circles by drawing horizontal lines
    int16_t x_center, y_center;
    int16_t radius_sq = r * r;
    int16_t len;
    
    for (int16_t i = 1; i <= r; i++) {
        // Calculate the horizontal chord length
        len = (int16_t)sqrt(radius_sq - (i * i));
        
        // Upper two corners:
        y_center = y + r;
        x_center = x + r; // UL Center
        screen_draw_line(x_center - len, y_center - i, x_center - 1, y_center - i, set); 

        x_center = x + w - r - 1; // UR Center
        screen_draw_line(x_center + 1, y_center - i, x_center + len, y_center - i, set); 

        // Lower two corners:
        y_center = y + h - r - 1;
        x_center = x + r; // LL Center
        screen_draw_line(x_center - len, y_center + i, x_center - 1, y_center + i, set); 

        x_center = x + w - r - 1; // LR Center
        screen_draw_line(x_center + 1, y_center + i, x_center + len, y_center + i, set); 
    }
}

void screen_draw_char(uint8_t x_start, uint8_t y_start, char c, uint8_t set)
{
    // The existing font implementation only supports writing 5x8 characters directly to the page buffer
    if (!set) return;

    int char_index = c - 32;
    if (char_index < 0 || char_index >= sizeof(font_table) / sizeof(font_table[0])) {
        char_index = 0; // Fallback to space
    }
    const uint8_t *char_data = font_table[char_index];

    uint8_t page = y_start / 8;
    uint16_t buffer_index_start = x_start + page * OLED_WIDTH;
    
    for (int i = 0; i < 5; i++) {
        if ((x_start + i) < OLED_WIDTH) {
            // Font data is column-major, 8 bits high. We write directly to the page.
            oled_buffer[buffer_index_start + i] = char_data[i];
        }
    }
    if ((x_start + 5) < OLED_WIDTH) {
        oled_buffer[buffer_index_start + 5] = 0x00; // Spacer column
    }
}

void screen_draw_str(uint8_t x, uint8_t y, const char *str, uint8_t set)
{
    int current_x = x;
    const int CHAR_WIDTH_WITH_SPACE = 6;
    
    for (size_t i = 0; i < strlen(str); i++) {
        screen_draw_char(current_x, y, str[i], set);
        current_x += CHAR_WIDTH_WITH_SPACE;
        if (current_x + CHAR_WIDTH_WITH_SPACE > OLED_WIDTH) {
            break;
        }
    }
}

void screen_draw_bitmap(uint8_t x_start, uint8_t y_start, uint8_t w, uint8_t h, const uint8_t *bitmap, uint8_t set)
{
    // Adopting the Horizontal Scanline Bit-packed format (8 horizontal pixels per byte) 
    // based on the user's working PCD8544 implementation.
    
    // Calculate the number of 8-pixel horizontal blocks (banks) per row.
    const uint8_t h_banks = (w + 7) / 8; 
    uint16_t data_index = 0;

    for (uint8_t row = 0; row < h; row++) { // Iterate over vertical pixels (rows)
        int16_t global_y = y_start + row;
        
        // Check vertical bounds
        if (global_y >= OLED_HEIGHT) {
             data_index += h_banks; // Skip the rest of this row's data
             continue;
        }

        for (uint8_t h_bank = 0; h_bank < h_banks; h_bank++) { // Iterate over horizontal 8-bit chunks
            
            // Check to avoid reading past the end of the bitmap array
            if (data_index >= (uint16_t)w * h_banks) break;
            
            uint8_t bank_data = bitmap[data_index++]; // Read one byte from bitmap data
            int16_t bank_start_x = x_start + (h_bank * 8);

            for (uint8_t bit_col = 0; bit_col < 8; bit_col++) { // Iterate over the 8 bits in the byte
                int16_t global_x = bank_start_x + bit_col;
                
                // Check horizontal bounds, ensuring we don't draw past the bitmap width 'w'
                if (global_x >= OLED_WIDTH || global_x >= x_start + w) continue;
                
                // If the bit is set (1) in the bitmap data, set the pixel on the display buffer
                if (bank_data & (1 << bit_col)) { 
                    screen_set_pixel(global_x, global_y, set);
                }
            }
        }
    }
}