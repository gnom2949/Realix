/* thrfs.c the THRFS Driver Header for Realix   
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    The Realix Bootloader Thirdix.
    THRFS or Thirdix Filesystem is POSIX ustar-based Filesystem for Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/
#ifndef __realix_thrfs__
#define __realix_thrfs__

#include "stdint.h"
#include "stddef.h"

void thrfs_new(const void *base, size_t size);
const void *thrfs_find(const char *name, size_t *out_size);
#endif