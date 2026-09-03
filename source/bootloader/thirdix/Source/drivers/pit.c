/* pit.c the PIT Driver for Realix    
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    The Realix Bootloader Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/
#include "../../Include/pit.h"
#include "../../Include/io.h"

#define PIT_BASE_FREQ 1193182u
#define PIT_CH2_DATA  0x42
#define PIT_COMMAND   0x43
#define PIT_CTRL_PORT 0x61

static uint32_t g_frequency_hz  = 0;
static uint32_t g_toggle_count  = 0;
static uint32_t g_last_state    = 0;

void pit_init(uint32_t frequency_hz)
{
    g_frequency_hz = frequency_hz;
    g_toggle_count = 0;

    uint16_t reload = (uint16_t)(PIT_BASE_FREQ / frequency_hz);

    outb(PIT_COMMAND, 0xB6);
    outb(PIT_CH2_DATA, reload & 0xFF);
    outb(PIT_CH2_DATA, (reload >> 8) & 0xFF);

    uint8_t control = inb(PIT_CTRL_PORT);
    control |= 0x01; // Bit 0: включить gate таймера 2.
    control &= ~0x02; // Bit 1: не подключать к спикеру.
    outb(PIT_CTRL_PORT, control);

    g_last_state = inb(PIT_CTRL_PORT) & 0x20;
}

uint32_t pit_elapsed_ms(void)
{
    uint8_t state = inb(PIT_CTRL_PORT) & 0x20;
    if (state != g_last_state)
    {
        g_toggle_count++;
        g_last_state = state;
    }

    return (uint32_t)(((uint32_t)g_toggle_count * 1000) / (2u * g_frequency_hz));
}