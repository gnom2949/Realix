/* keyboard.c the Keyboard Driver for Realix    
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    The Realix Bootloader Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/

#include "../../Include/keyboard.h"
#include "../../Include/io.h"

int keyboard_poll_digit(void)
{
    if (!(inb(0x64) & 0x01)) return 0;

    uint8_t scancode = inb(0x60);

    switch (scancode) {
        case 0x02: return 1;
        case 0x03: return 2;
        default: return -1;
    }
}