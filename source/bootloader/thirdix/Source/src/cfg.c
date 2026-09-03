/* cfg.c the Config parser for Realix   
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    The Realix Bootloader Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/
#include "../../Include/bootcfg.h"
#include "../../Include/thrfs.h"
#include "../../Include/string.h"

static void bootcfg_defaults(bootcfg_t *cfg)
{
    MemSet(cfg, 0, sizeof(*cfg));
    StrBoundedCopy(cfg->title, "RealixOS", sizeof(cfg->title) - 1);
    cfg->timer_seconds = 20;
}

bool bootcfg_load(bootcfg_t *cfg)
{
    bootcfg_defaults(cfg);

    size_t file_size = 0;
    const void *data = thrfs_find(BOOTCFG_FILEN, &file_size);
    if (data == NULL || file_size < sizeof(bootcfg_t))
        return false;

    const bootcfg_t *loaded = (const bootcfg_t*)data;
    if (MemCompare(loaded->magic, BOOTCFG_MAGIC, 4) != 0)
        return false;

    if (loaded->version != BOOTCFG_VERSI)
        return false;

    MemCopy(cfg, loaded, sizeof(bootcfg_t));

    cfg->background[sizeof(cfg->background) - 1] = '\0';
    cfg->title[sizeof(cfg->title) - 1] = '\0';

    if (cfg->title[0] == '\0')
        StrBoundedCopy(cfg->title, "RealixOS", sizeof(cfg->title) - 1);

    if (cfg->timer_seconds == 0) cfg->timer_seconds = 20;

    return true;
}