/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <sys/cdefs.h>
#include "sdkconfig.h"
#if CONFIG_LCD_ENABLE_DEBUG_LOG
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
#endif
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_commands.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "lcd_panel";

static esp_err_t panel_lcd_del(esp_lcd_panel_t *panel);
static esp_err_t panel_lcd_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_lcd_init(esp_lcd_panel_t *panel);
static esp_err_t panel_lcd_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data);
static esp_err_t panel_lcd_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
static esp_err_t panel_lcd_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
static esp_err_t panel_lcd_swap_xy(esp_lcd_panel_t *panel, bool swap_axes);
static esp_err_t panel_lcd_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap);
static esp_err_t panel_lcd_disp_on_off(esp_lcd_panel_t *panel, bool off);

typedef struct
{
    esp_lcd_panel_t base;
    esp_lcd_panel_io_handle_t io;
    int reset_gpio_num;
    bool reset_level;
    int x_gap;
    int y_gap;
    unsigned int bits_per_pixel;
    uint8_t madctl_val; // save current value of LCD_CMD_MADCTL register
    uint8_t colmod_cal; // save surrent value of LCD_CMD_COLMOD register
} lcd_panel_t;

esp_err_t esp_lcd_new_panel_lcd(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel)
{
#if CONFIG_LCD_ENABLE_DEBUG_LOG
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
#endif
    esp_err_t ret = ESP_OK;
    lcd_panel_t *lcd = NULL;
    ESP_GOTO_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, err, TAG, "invalid argument");
    lcd = calloc(1, sizeof(lcd_panel_t));
    ESP_GOTO_ON_FALSE(lcd, ESP_ERR_NO_MEM, err, TAG, "no mem for lcd panel");

    if (panel_dev_config->reset_gpio_num >= 0)
    {
        gpio_config_t io_conf = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num,
        };
        ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for RST line failed");
    }

    switch (panel_dev_config->color_space)
    {
    case ESP_LCD_COLOR_SPACE_RGB:
        lcd->madctl_val = 0;
        break;
    case ESP_LCD_COLOR_SPACE_BGR:
        lcd->madctl_val |= LCD_CMD_BGR_BIT;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported color space");
        break;
    }

    switch (panel_dev_config->bits_per_pixel)
    {
    case 16:
        lcd->colmod_cal = 0x55;
        break;
    case 18:
        lcd->colmod_cal = 0x66;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported pixel width");
        break;
    }

    lcd->io = io;
    lcd->bits_per_pixel = panel_dev_config->bits_per_pixel;
    lcd->reset_gpio_num = panel_dev_config->reset_gpio_num;
    lcd->reset_level = panel_dev_config->flags.reset_active_high;
    lcd->base.del = panel_lcd_del;
    lcd->base.reset = panel_lcd_reset;
    lcd->base.init = panel_lcd_init;
    lcd->base.draw_bitmap = panel_lcd_draw_bitmap;
    lcd->base.invert_color = panel_lcd_invert_color;
    lcd->base.set_gap = panel_lcd_set_gap;
    lcd->base.mirror = panel_lcd_mirror;
    lcd->base.swap_xy = panel_lcd_swap_xy;
    lcd->base.disp_on_off = panel_lcd_disp_on_off;
    *ret_panel = &(lcd->base);
    ESP_LOGD(TAG, "new lcd panel @%p", lcd);

    return ESP_OK;

err:
    if (lcd)
    {
        if (panel_dev_config->reset_gpio_num >= 0)
        {
            gpio_reset_pin(panel_dev_config->reset_gpio_num);
        }
        free(lcd);
    }
    return ret;
}

static esp_err_t panel_lcd_del(esp_lcd_panel_t *panel)
{
    lcd_panel_t *lcd = __containerof(panel, lcd_panel_t, base);

    if (lcd->reset_gpio_num >= 0)
    {
        gpio_reset_pin(lcd->reset_gpio_num);
    }
    ESP_LOGD(TAG, "del lcd panel @%p", lcd);
    free(lcd);
    return ESP_OK;
}

static esp_err_t panel_lcd_reset(esp_lcd_panel_t *panel)
{
    lcd_panel_t *lcd = __containerof(panel, lcd_panel_t, base);
    esp_lcd_panel_io_handle_t io = lcd->io;

    // perform hardware reset
    if (lcd->reset_gpio_num >= 0)
    {
        gpio_set_level(lcd->reset_gpio_num, lcd->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(lcd->reset_gpio_num, !lcd->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    else
    { // perform software reset
        esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0);
        vTaskDelay(pdMS_TO_TICKS(100)); // spec, wait at least 5m before sending new command
    }

    return ESP_OK;
}

static esp_err_t panel_lcd_init(esp_lcd_panel_t *panel)
{
    lcd_panel_t *lcd = __containerof(panel, lcd_panel_t, base);
    esp_lcd_panel_io_handle_t io = lcd->io;
    // Init LCD

    // Set_EXTC
    esp_lcd_panel_io_tx_param(io, 0xB9, (uint8_t[]){0xFF, 0x83, 0x69}, 3);
   // Set Power
    esp_lcd_panel_io_tx_param(io, 0xB1, (uint8_t[]){0x01, 0x00, 0x34, 0x06, 0x00, 0x11, 0x11, 0x2a, 0x32, 0x3f, //
    0x3f, 0x07, 0x23, 0x01, 0xe6, 0xe6, 0xe6, 0xe6, 0xe6},
    19);

    // SET Display 480x800
    // 0x2b;0x20-MCU;0x29-DPI;RM,DM; RM=0:DPI IF;  RM=1:RGB IF;
    esp_lcd_panel_io_tx_param(io, 0xB2, (uint8_t[]){0x00, 0x20, 0x03, 0x03, 0x70, 0x00, 0xff, 0x00, 0x00, 0x00, //
                                                    0x00, 0x03, 0x03, 0x00, 0x01},
                              15);
    // SET Display CYC
    esp_lcd_panel_io_tx_param(io, 0xb4, (uint8_t[]){0x00, 0x0C, 0xA0, 0x0E, 0x06},
                              5);
    // SET VCOM
    esp_lcd_panel_io_tx_param(io, 0xb6, (uint8_t[]){0x2C, 0x2C},
                              2);

    // SET GIP
    esp_lcd_panel_io_tx_param(io, 0xD5, (uint8_t[]){0x00, 0x05, 0x03, 0x00, 0x01, 0x09, 0x10, 0x80, 0x37, 0x37, 0x20, 0x31, 0x46, //
                                                    0x8a, 0x57, 0x9b, 0x20, 0x31, 0x46, 0x8a, 0x57, 0x9b, 0x07, 0x0f, 0x02, 0x00},
                              26);

    //  SET GAMMA
    esp_lcd_panel_io_tx_param(io, 0xE0, (uint8_t[]){0x00, 0x08, 0x0d, 0x2d, 0x34, 0x3f, 0x19, 0x38, 0x09, 0x0e, 0x0e, 0x12, 0x14, 0x12, 0x14, 0x13, 0x19, //
                                                    0x00, 0x08, 0x0d, 0x2d, 0x34, 0x3f, 0x19, 0x38, 0x09, 0x0e, 0x0e, 0x12, 0x14, 0x12, 0x14, 0x13, 0x19},
                              34);

    // set DGC
    esp_lcd_panel_io_tx_param(io, 0xC1, (uint8_t[]){0x01, 0x02, 0x08, 0x12, 0x1a, 0x22, 0x2a, 0x31, 0x36, 0x3f, 0x48, 0x51, 0x58, 0x60, 0x68, 0x70, //
                                                    0x78, 0x80, 0x88, 0x90, 0x98, 0xa0, 0xa7, 0xaf, 0xb6, 0xbe, 0xc7, 0xce, 0xd6, 0xde, 0xe6, 0xef, //
                                                    0xf5, 0xfb, 0xfc, 0xfe, 0x8c, 0xa4, 0x19, 0xec, 0x1b, 0x4c, 0x40, 0x02, 0x08, 0x12, 0x1a, 0x22, //
                                                    0x2a, 0x31, 0x36, 0x3f, 0x48, 0x51, 0x58, 0x60, 0x68, 0x70, 0x78, 0x80, 0x88, 0x90, 0x98, 0xa0, //
                                                    0xa7, 0xaf, 0xb6, 0xbe, 0xc7, 0xce, 0xd6, 0xde, 0xe6, 0xef, 0xf5, 0xfb, 0xfc, 0xfe, 0x8c, 0xa4, //
                                                    0x19, 0xec, 0x1b, 0x4c, 0x40, 0x02, 0x08, 0x12, 0x1a, 0x22, 0x2a, 0x31, 0x36, 0x3f, 0x48, 0x51, //
                                                    0x58, 0x60, 0x68, 0x70, 0x78, 0x80, 0x88, 0x90, 0x98, 0xa0, 0xa7, 0xaf, 0xb6, 0xbe, 0xc7, 0xce, //
                                                    0xd6, 0xde, 0xe6, 0xef, 0xf5, 0xfb, 0xfc, 0xfe, 0x8c, 0xa4, 0x19, 0xec, 0x1b, 0x4c, 0x40},
                              127);

    //  Colour Set
    uint8_t cmd_192[192];
    for (size_t i = 0; i <= 63; i++)
    {
        cmd_192[i] = i * 8;
    }
    for (size_t i = 64; i <= 127; i++)
    {
        cmd_192[i] = i * 4;
    }
    for (size_t i = 128; i <= 191; i++)
    {
        cmd_192[i] = i * 8;
    }
    esp_lcd_panel_io_tx_param(io, 0x2D, cmd_192,
                              192);

    // LCD goes into sleep mode and display will be turned off after power on reset, exit sleep mode first
    esp_lcd_panel_io_tx_param(io, LCD_CMD_SLPOUT, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]){
                                                      lcd->madctl_val,
                                                  },
                              1);
    esp_lcd_panel_io_tx_param(io, LCD_CMD_COLMOD, (uint8_t[]){
                                                      lcd->colmod_cal,
                                                  },
                              1);

    return ESP_OK;
}

static esp_err_t panel_lcd_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    lcd_panel_t *lcd = __containerof(panel, lcd_panel_t, base);
    assert((x_start < x_end) && (y_start < y_end) && "start position must be smaller than end position");
    esp_lcd_panel_io_handle_t io = lcd->io;

    x_start += lcd->x_gap;
    x_end += lcd->x_gap;
    y_start += lcd->y_gap;
    y_end += lcd->y_gap;

    // define an area of frame memory where MCU can access
    esp_lcd_panel_io_tx_param(io, LCD_CMD_CASET, (uint8_t[]){
                                                     (x_start >> 8) & 0xFF,
                                                     x_start & 0xFF,
                                                     ((x_end - 1) >> 8) & 0xFF,
                                                     (x_end - 1) & 0xFF,
                                                 },
                              4);
    esp_lcd_panel_io_tx_param(io, LCD_CMD_RASET, (uint8_t[]){
                                                     (y_start >> 8) & 0xFF,
                                                     y_start & 0xFF,
                                                     ((y_end - 1) >> 8) & 0xFF,
                                                     (y_end - 1) & 0xFF,
                                                 },
                              4);
    // transfer frame buffer
    size_t len = (x_end - x_start) * (y_end - y_start) * lcd->bits_per_pixel / 8;
    esp_lcd_panel_io_tx_color(io, LCD_CMD_RAMWR, color_data, len);

    return ESP_OK;
}

static esp_err_t panel_lcd_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
{
    lcd_panel_t *lcd = __containerof(panel, lcd_panel_t, base);
    esp_lcd_panel_io_handle_t io = lcd->io;
    int command = 0;
    if (invert_color_data)
    {
        command = LCD_CMD_INVON;
    }
    else
    {
        command = LCD_CMD_INVOFF;
    }
    esp_lcd_panel_io_tx_param(io, command, NULL, 0);
    return ESP_OK;
}

static esp_err_t panel_lcd_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    lcd_panel_t *lcd = __containerof(panel, lcd_panel_t, base);
    esp_lcd_panel_io_handle_t io = lcd->io;
    if (mirror_x)
    {
        lcd->madctl_val |= LCD_CMD_MX_BIT;
    }
    else
    {
        lcd->madctl_val &= ~LCD_CMD_MX_BIT;
    }
    if (mirror_y)
    {
        lcd->madctl_val |= LCD_CMD_MY_BIT;
    }
    else
    {
        lcd->madctl_val &= ~LCD_CMD_MY_BIT;
    }
    esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]){lcd->madctl_val}, 1);
    return ESP_OK;
}

static esp_err_t panel_lcd_swap_xy(esp_lcd_panel_t *panel, bool swap_axes)
{
    lcd_panel_t *lcd = __containerof(panel, lcd_panel_t, base);
    esp_lcd_panel_io_handle_t io = lcd->io;
    if (swap_axes)
    {
        lcd->madctl_val |= LCD_CMD_MV_BIT;
    }
    else
    {
        lcd->madctl_val &= ~LCD_CMD_MV_BIT;
    }
    esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]){lcd->madctl_val}, 1);
    return ESP_OK;
}

static esp_err_t panel_lcd_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
    lcd_panel_t *lcd = __containerof(panel, lcd_panel_t, base);
    lcd->x_gap = x_gap;
    lcd->y_gap = y_gap;
    return ESP_OK;
}

static esp_err_t panel_lcd_disp_on_off(esp_lcd_panel_t *panel, bool on_off)
{
    lcd_panel_t *lcd = __containerof(panel, lcd_panel_t, base);
    esp_lcd_panel_io_handle_t io = lcd->io;
    int command = 0;
    if (on_off)
    {
        command = LCD_CMD_DISPON;
    }
    else
    {
        command = LCD_CMD_DISPOFF;
    }
    esp_lcd_panel_io_tx_param(io, command, NULL, 0);
    return ESP_OK;
}
