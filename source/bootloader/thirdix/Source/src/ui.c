/* ui.c the Thirdix UserInterface for Realix    
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    The Realix Bootloader Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/
#include "../../Include/ui.h"
#include "../../Include/graphics.h"
#include "../../Include/screen.h"
#include "../../Include/bootcfg.h"
#include "../../Include/thrfs.h"
#include "../../Include/keyboard.h"
#include "../../Include/pit.h"
#include "../../Include/string.h"
#include "../../Include/com.h"

#define UI_TIMER_HZ 100

#define UI_TIMEOUT_CHOICE UI_CHOICE_32BIT

typedef struct {
    const char *label;
    ui_choice_t choice;
} ui_menu_item;

static const ui_menu_item MENU_ITEMS[] = {
    {"[1] 16-bit Real Mode", UI_CHOICE_16BIT },
    {"[2] 32-bit Protected Mode", UI_CHOICE_32BIT},
};

#define MENU_ITEM_COUNT (sizeof(MENU_ITEMS) / sizeof(MENU_ITEMS[0]))

static void ui_draw_background(const bootcfg_t *cfg)
{
    if (cfg->background[0] == '\0') {
        vbe_clear_black();
        return;
    }

    size_t bg_size = 0;
    const void *bg_data = thrfs_find(cfg->background, &bg_size);
    if (bg_data == NULL)
    {
        serial_print("Background not found\n");
        vbe_clear_black();
        return;
    }

    vbe_draw_tga((const uint8_t*)bg_data, 0, 0);
}

static uint16_t ui_draw_menu(void)
{
    uint16_t ln_height = FONT_HEIGHT + 6;
    uint16_t block_height = (uint16_t)(MENU_ITEM_COUNT * ln_height);
    uint16_t top_y = (vbe_get_height() > block_height) ? (vbe_get_height() - block_height) / 2 : 0;

    for (size_t i = 0; i < MENU_ITEM_COUNT; i++)
    {
        size_t len = StrLen(MENU_ITEMS[i].label);
        uint16_t text_width = (uint16_t)(len * FONT_WIDTH);
        uint16_t x = (vbe_get_width() > text_width) ? (vbe_get_width() - text_width) / 2 : 0;
        uint16_t y = (uint16_t)(top_y + i * ln_height);

        vbe_draw_string(x, y, MENU_ITEMS[i].label, VGA_WHITE, VGA_BLACK, 1);
    }

    return top_y;
}

static void ui_draw_title(const bootcfg_t *cfg, uint16_t menu_top_y)
{
    size_t len = StrLen(cfg->title);
    uint16_t text_width = (uint16_t)(len * FONT_WIDTH);
    uint16_t x = (vbe_get_width() > text_width) ? (vbe_get_width() - text_width) / 2 : 0;
    uint16_t y = (menu_top_y > FONT_HEIGHT * 2) ? (menu_top_y - FONT_HEIGHT * 2) : 0;
    vbe_draw_string(x, y, cfg->title, VGA_WHITE, VGA_BLACK, 1);
}

static void ui_draw_timer(uint32_t seconds_remaining)
{
    char buf[4];

    buf[0] = (char)('0' + (seconds_remaining / 10) % 10);
    buf[1] = (char)('0' + seconds_remaining % 10);
    buf[2] = 's';
    buf[3] = '\0';

    uint16_t x = vbe_get_width() - (4 * FONT_WIDTH) - 8;
    uint16_t y = vbe_get_height() - FONT_HEIGHT - 8;

    vbe_fill_rect(x, y, 4 * FONT_WIDTH, FONT_HEIGHT, VGA_BLACK);
    vbe_draw_string(x, y, buf, VGA_YELLOW, VGA_BLACK, 1);
}

ui_choice_t ui_run(pcinfo_t *pcinfo)
{
    (void)pcinfo; 

    bootcfg_t cfg;
    bootcfg_load(&cfg);

    if (!vbe_is_initialized()) return -1;

    ui_draw_background(&cfg);
    uint16_t menu_top_y = ui_draw_menu();
    ui_draw_title(&cfg, menu_top_y);

    pit_init(UI_TIMER_HZ);
    uint32_t timeout_ms = (uint32_t)cfg.timer_seconds * 1000;
    uint32_t last_drawn_seconds = 0xFFFFFFFF;

    for (;;)
    {
        int digit = keyboard_poll_digit();
        for (size_t i = 0; i < MENU_ITEM_COUNT; i++)
            if (digit == (int)(i + 1))
                return MENU_ITEMS[i].choice;

        uint32_t elapsed_ms = pit_elapsed_ms();
        uint32_t remaining_ms = (elapsed_ms >= timeout_ms) ? 0 : (timeout_ms - elapsed_ms);

        uint32_t remaining_s = (remaining_ms + 999) / 1000;

        if (remaining_s != last_drawn_seconds)
        {
            ui_draw_timer(remaining_s);
            last_drawn_seconds = remaining_s;
        }

        if (elapsed_ms >= timeout_ms) 
            return UI_TIMEOUT_CHOICE;
    }
}