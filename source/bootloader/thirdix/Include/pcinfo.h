/* pcinfo.h the PCINFO Header for Realix   
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    The Realix Bootloader Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/
#ifndef PCINFO_H
#define PCINFO_H

#include "stdint.h"

#define PCINFO_E820_ENTRY_SIZE  24
#define PCINFO_E820_MAX_ENTRIES 64

typedef struct __attribute__((packed)) {
    uint64_t base;
    uint64_t len;
    uint32_t type; // 1 = usable, 2 = reserved, 3 = ACPI reclaimable, etc.
    uint32_t acpi_ext; // Расширенные атрибуты.
} e820_entry_t;

typedef struct __attribute__((packed)) {
    uint32_t all_mem_mb;    // PCINFO_ALL_MEM: суммарная доступная память, MB
    uint16_t low_mem_kb;    // PCINFO_LOW_MEM: нижняя память, KB
    uint16_t mmap_entries;  // PCINFO_MMAP_ENTRIES: количество записей карты памяти
    uint16_t drive;         // PCINFO_DRIVE: номер загрузочного диска
    uint16_t video_mode;    // PCINFO_VIDEOMODE: 0 - текстовый, 1 - видеорежим

    // VBE (заполняется bios-api/video/vbe.asm)
    uint32_t fb_addr;       // PCINFO_FB_ADDR: физический адрес (0 - не установлен)
    uint16_t fb_pitch;      // PCINFO_FB_PITCH: байт на строку
    uint16_t fb_width;      // PCINFO_FB_WIDTH: ширина
    uint16_t fb_height;     // PCINFO_FB_HEIGHT: высота
    uint8_t fb_bpp;         // PCINFO_FB_BPP: байт на пиксель
    uint8_t fb_valid;       // PCINFO_FB_VALID: 1 - framebuffer установлен через VESA BIOS Exts

    e820_entry_t mmap[];    // PCINFO_MMAP (смещение 24): карта памяти 
} pcinfo_t;

#endif