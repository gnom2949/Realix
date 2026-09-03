/* screen.c the Screen Driver for Realix   
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    The Realix Bootloader Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/
#include "../../Include/screen.h"
#include "../../Include/graphics.h"

#define SCREEN_FALLBACK_WIDTH 1024
#define SCREEN_FALLBACK_HEIGHT 768
#define SCREEN_FALLBACK_BPP 32

static uint16_t cursor_x = 0;
static uint16_t cursor_y = 0;

void screen_new(pcinfo_t *pcinfo)
{
    vbe_init_from_pcinfo(pcinfo);
    if (vbe_is_initialized()) return;

    vbe_init_fallback(SCREEN_FALLBACK_WIDTH, SCREEN_FALLBACK_HEIGHT, SCREEN_FALLBACK_BPP);
}

void screen_clear(void)
{
    cursor_x = 0;
    cursor_y = 0;

    if (vbe_is_initialized()) vbe_clear_black();
}

static void screen_putc(char c)
{
    if (c == '\n')
    {
        cursor_x = 0;
        cursor_y += FONT_HEIGHT;
    } else {
        vbe_draw_char(cursor_x, cursor_y, c, VGA_WHITE, VGA_BLACK, 0);
        cursor_x += FONT_WIDTH;
        if (cursor_x + FONT_WIDTH > vbe_get_width())
        {
            cursor_x = 0;
            cursor_y += FONT_HEIGHT;
        }
    }

    if (cursor_y + FONT_HEIGHT > vbe_get_height())
    {
        vbe_clear_black();
        cursor_x = 0;
        cursor_y = 0;
    }
}

void screen_print(const char *str)
{
    while(*str)
        screen_putc(*str++);
}

void screen_print_hex32(uint32_t value)
{
    static const char hexadecimal[] = "0123456789ABCDEF";
    screen_print("0x");
    for (int shift = 28; shift >= 0; shift -= 4)
        screen_putc(hexadecimal[(value >> shift) & 0xF]);
}