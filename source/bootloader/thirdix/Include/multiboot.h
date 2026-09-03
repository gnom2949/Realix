/* multiboot.h the multiboot header for Realix, from the official GNU GRUB Source Code(on archived github<https://github.com/coreos/grub>).
    © 2026 Realix
    © 2026 Alexander Silaev <thebinaryblob@gmail.com>
    This is the Multiboot 1.    
    The Realix Bootloader Thirdix.
    Licensed under GNU GPLv3, see more in <https://www.gnu.org/licenses/gpl-3.0.html>
*/

#ifndef REALIX_MULTIBOOT
#define REALIX_MULTIBOOT 1

#include "stdint.h"
#include "pcinfo.h"

/* Сколько байт от начала файла мы ищем заголовок. */
#define MULTIBOOT_SEARCH_TO_HEADER 8192
#define MULTIBOOT_HEADER_ALIGN 4

/* Магическое число должно содержать это. */
#define MULTIBOOT_HEADER_MAGIC  0x1BADB002
/* это должно быть в eax. */
#define MULTIBOOT_BL_MAGIC      0x2BADB002 /* Пиздец я тут тупую ошибку совершил, взял и вместо 0 поствил O, я ебал. */

/* Выравнивание модулей multiboot. */
#define MULTIBOOT_MOD_ALIGN     0x00001000
/* Выравнивание структуры info. */
#define MULTIBOOT_INFO_ALIGN    0x00000004
/* Флаги устанавливаются в поля 'flags' в заголовке multiboot. */

/* Выровнять все загрузочные модули по границам страниц i386 (4 КБ). */
#define MULTIBOOT_PAGE_ALIGN    0x00000001
/* Отдаёт ОС информацию о памяти. */
#define MULTIBOOT_MEMORY_INFO   0x00000002
/* Отдаёт ОС информацию о видеоадаптерах. */
#define MULTIBOOT_VIDEO_MODE    0x00000004

/* Этот флаг указывает на использование адресов в заголовке. */
#define MULTIBOOT_AOUT_KLUDGE   0x00010000

/* Флаги устанавливаются в поля 'flags' в структуре info. */

/* Есть ли базовая информация он нижней/верхней памяти? */
#define MULTIBOOT_INFO_MEMORY   0x00000001
/* Есть ли загрузочное устройство? */
#define MULTIBOOT_INFO_BOOTDEV  0x00000002
/* Есть ли консоль? */
#define MULTIBOOT_INFO_CMDLINE  0x00000004
/* Есть ли модули с которыми можно взаимодействовать? */
#define MULTIBOOT_INFO_MODS     0x00000008

/* Следующие два взаимоисключающие, то есть можно выбрать только один. */

/* Загружена ли таблица символов?(symtab). */
#define MULTIBOOT_INFO_AOUT_SYMS 0x00000010
/* Существует ли таблица заголовка раздела ELF? */
#define MULTIBOOT_INFO_ELF_SHDR  0x00000020

/* Существует ли полная карта памяти? */
#define MULTIBOOT_INFO_MEM_MAP   0x00000040

/* Есть ли информация о диске? */
#define MULTIBOOT_INFO_DRIVE_INFO 0x00000080

/* Есть ли таблица конфигураций? */
#define MULTIBOOT_INFO_CONFIG_TABLE 0x00000100

/* Есть ли имя загрузчика? */
#define MULTIBOOT_INFO_BOOTLOADER_NAME 0x00000200

/* Есть ли таблица APM? */
#define MULTIBOOT_INFO_APM_TABLE 0x00000400

/* Есть ли информация о графике?*/
#define MULTIBOOT_INFO_VBE_INFO  0x00000800
#define MULTIBOOT_INFO_FRAMEBUFFER_INFO 0x00001000

#ifndef ASM_FILE /* Я хз зачем это было в GRUB, но я тоже добавлю, вдруг это важно. */
typedef struct {
    /* Магическое число(0x1BADBOO2).*/
    uint32_t magic;
    /* Флаги. */
    uint32_t flags;
    /* Сумма значений вышеуказанных полей и этого должна равняться 0 по модулю 2^32. */
    uint32_t checksum;

    /* ТОЛЬКО Если выставлен MULTIBOOT_AOUT_KLUDGE. */
    uint32_t header_addr;
    uint32_t load_addr;
    uint32_t bss_end_addr;
    uint32_t entry_addr;

    /* ТОЛЬКО Если выставлен MULTIBOOT_VIDEO_MODE. */
    uint32_t mode_type;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
} multiboot_t;

/* Таблица символов(symtab) для a.out. */
typedef struct {
    uint32_t tabsize;
    uint32_t strsize;
    uint32_t addr;
    uint32_t reserved;
} multiboot_aout_symbol_table_t;

/* Таблица заголовков разделов для ELF. */
typedef struct {
    uint32_t num;
    uint32_t size;
    uint32_t addr;
    uint32_t shndx;
} multiboot_elf_section_header_table_t;

typedef struct {
    /* Ну флаги. */
    uint32_t flags;
    /* Доступная память из под BIOS'а.*/
    uint32_t mem_lower;
    uint32_t mem_upper;
    /* Корневой раздел. */
    uint32_t boot_device;
    /* Консоль ядра ну или коммандная строка. */
    uint32_t cmdline;
    /* Список загрузочных модулей. */
    uint32_t mods_count;
    uint32_t mods_addr;

    union
    {
        multiboot_aout_symbol_table_t aout_sym;
        multiboot_elf_section_header_table_t elf_sec;
    }u;
    /* Буфер карт памяти(mmap). */
    uint32_t mmap_length;
    uint32_t mmap_addr;

    /* Буфер информации диска(исправьте если перевёл неправильно).*/
    uint32_t drives_length;
    uint32_t drives_addr;

    /* таблица конфигурации ROM. */
    uint32_t config_table;
    /* Имя загрузчика. */
    uint32_t bootloader_name;
    /* Таблица APM. */
    uint32_t apm_table;
    /* Графика. */
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;

    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB 1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT 2
    uint8_t framebuffer_type;
    union
    {
        struct {
            uint32_t framebuffer_palette_addr;
            uint16_t framebuffer_palette_num_colors;
        };

        struct {
            uint8_t framebuffer_red_field_position;
            uint8_t framebuffer_red_mask_size;
            uint8_t framebuffer_green_field_position;
            uint8_t framebuffer_green_mask_size;
            uint8_t framebuffer_blue_field_position;
            uint8_t framebuffer_blue_mask_size;
        };
    };
} multiboot_info_t;

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} multiboot_color_t;

typedef struct {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_RESERVED  2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS 4
#define MULTIBOOT_MEMORY_BADRAM 5
    uint32_t type;
} __attribute__((packed)) multiboot_mmap_entry_t;

typedef struct {
    /* Используемая память начинается с байтов 
    'mod_start' и заканчивается 
    на 'mod_end-1' включительно.*/
    uint32_t mod_start;
    uint32_t mod_end;

    /* Командная строка модуля. */
    uint32_t cmdline;
    /* Отступ до 16 байт(должно быть равно 0)*/
    uint32_t pad;
} multiboot_module_t;

struct multiboot_apm_info {
    uint16_t version;
    uint16_t cseg;
    uint32_t offset;
    uint16_t cseg_16;
    uint16_t dseg;
    uint16_t flags;
    uint16_t cseg_len;
    uint16_t cseg_16_len;
    uint16_t dseg_len;
};

#endif /*ASM_FILE*/

multiboot_info_t *mb_fill_info(pcinfo_t *pcinfo);

#endif /* REALIX_MULTIBOOT */