/* bump.h the bump allocator header for Realix.
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    The Realix Bootloader Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/
#ifndef __REALIX_BOOTLOADER_BUMP__
#define __REALIX_BOOTLOADER_BUMP__

#include "stddef.h"

void binit(void *base, size_t size);
void *balloc(size_t size);
void *ballocaligned(size_t size, size_t align);
void bump_reset(void);

size_t bump_used(void);
size_t bump_remaining(void);
#endif /*__REALIX_BOOTLOADER_BUMP__*/