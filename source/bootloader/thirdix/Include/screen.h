/* screen.h the Screen Driver header for Realix   
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    The Realix Bootloader Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/

#ifndef __realix_screen__
#define __realix_screen__

#include "pcinfo.h"

void screen_new(pcinfo_t *pcinfo);
void screen_clear(void);
void screen_print(const char *str);
void screen_print_hex32(uint32_t value);

#endif 