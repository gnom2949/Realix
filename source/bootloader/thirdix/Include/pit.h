/* pit.h the PIT Driver Header for Realix   
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    The Realix Bootloader Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/

#ifndef __realix_pit__
#define __realix_pit__

#include "stdint.h"

void pit_init(uint32_t frequency_hz);
uint32_t pit_elapsed_ms(void);

#endif