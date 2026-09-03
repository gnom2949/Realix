/* bootcfg.h the Thirdix Boot Config header for Realix   
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    The Realix Bootloader Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/
#ifndef __realix_bootcfg__
#define __realix_bootcfg__

#include "stdint.h"
#include "stdbool.h"

#define BOOTCFG_MAGIC   "TCFG"
#define BOOTCFG_VERSI   1
#define BOOTCFG_FILEN   "boot.cfg"

typedef struct __attribute__((packed)) {
    char magic[4];
    uint8_t version;
    char background[32];
    char title[32];
    uint16_t timer_seconds;
    uint8_t reserved[16];
} bootcfg_t;

bool bootcfg_load(bootcfg_t *cfg);

#endif 