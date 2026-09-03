/* mbfill.c the Multiboot1 structure filler for Realix   
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    The Realix Bootloader Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/
#include "../../Include/multiboot.h"
#include "../../Include/string.h"
#include "../../Include/graphics.h"
#include "../../Include/pcinfo.h"

static multiboot_info_t g_mb_info;
static multiboot_mmap_entry_t g_mb_mmap[PCINFO_E820_MAX_ENTRIES];

multiboot_info_t *mb_fill_info(pcinfo_t *pcinfo)
{
    MemSet(&g_mb_info, 0, sizeof(g_mb_info));

    uint32_t flags = 0;

    // MULTIBOOT_INFO_MEMORY
    uint32_t total_kb = pcinfo->all_mem_mb * 1024;
    g_mb_info.mem_lower = pcinfo->low_mem_kb;
    g_mb_info.mem_upper = (total_kb > 1024) ? (total_kb - 1024) : 0;
    flags |= MULTIBOOT_INFO_MEMORY;

    // MULTIBOOT_INFO_BOOTDEV
    // Формат: [drive:8][part1:8][part2:8][part3:8], остальное - 0чFF
    g_mb_info.boot_device = ((uint32_t)(pcinfo->drive & 0xFF) << 24) | 0x00FFFFFFu;
    flags |= MULTIBOOT_INFO_BOOTDEV;

    // MULTIBOOT_INFO_MEM_MAP
    // конвертация из сырого e820 (base/len/type/ext, 24 байта - см. PCINFO)
    // в формат multiboot (size/addr/len/type, тоже 24, но другой раскладкой полей).
    uint16_t entry_count = pcinfo->mmap_entries;
    if (entry_count > PCINFO_E820_MAX_ENTRIES)
        entry_count = PCINFO_E820_MAX_ENTRIES;

    for (uint16_t i = 0; i < entry_count; i++)
    {
        g_mb_mmap[i].size = sizeof(multiboot_mmap_entry_t) - sizeof(uint32_t);
        g_mb_mmap[i].addr = pcinfo->mmap[i].base;
        g_mb_mmap[i].len =  pcinfo->mmap[i].len;
        g_mb_mmap[i].type = pcinfo->mmap[i].type;
    }

    g_mb_info.mmap_addr = (uint32_t)(uintptr_t)g_mb_mmap;
    g_mb_info.mmap_length = (uint32_t)(entry_count * sizeof(multiboot_mmap_entry_t));
    flags |= MULTIBOOT_INFO_MEM_MAP;

    // Framebuffer
    // буду брать из graphics.c т.к. там есть и VBE и фолбек на BGA.
    if (vbe_is_initialized())
    {
        g_mb_info.framebuffer_addr = (uint64_t)vbe_get_fb_addr();
        g_mb_info.framebuffer_pitch = vbe_get_pitch();
        g_mb_info.framebuffer_bpp = vbe_get_bpp();
        g_mb_info.framebuffer_width = vbe_get_width();
        g_mb_info.framebuffer_height = vbe_get_height();
        g_mb_info.framebuffer_type = MULTIBOOT_FRAMEBUFFER_TYPE_RGB;
        flags |= MULTIBOOT_INFO_FRAMEBUFFER_INFO;
    }

    g_mb_info.flags = flags;
    return &g_mb_info;
}