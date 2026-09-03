// © Realix > Thirdix
// © 2026 Alexander Silaev <thebinaryblob@gmail.com>
// ===================
// Основа для Thirdix или Stage 3 загрузчика Realix.

#include "../../Include/io.h"
#include "../../Include/stdint.h"
#include "../../Include/pcinfo.h"
#include "../../Include/screen.h"
#include "../../Include/cmos.h"
#include "../../Include/idt.h"
#include "../../Include/bump.h"
#include "../../Include/multiboot.h"
#include "../../Include/ui.h"
#include "../../Include/bootcfg.h"
#include "../../Include/thrfs.h"

extern uint8_t _image_end;
extern uint8_t _bss_start;

#define KERNEL32_PHYS_ADDR 0x00010000
#define THIRDIX_ARENA_SIZE (64 + 1024)
static uint8_t thirdix_arena[THIRDIX_ARENA_SIZE];

static void delay(void) { inb(0x80); }

static void reboot(void)
{
    outb(0x0CF9, 0x02);
    delay();
    outb(0x0CF9, 0x06);
    delay();

    for (volatile int i = 0; i < 100000; i++)
        if ((inb(0x64) & 0x02) == 0) break;

    outb(0x64, 0xFE);
    delay();

    __asm__ __volatile__(
        "pushl $0\n\t"
        "pushw $0\n\t"
        "lidt (%esp)\n\t"
        "int $3\n\t"
        "hlt"
    );
}

__attribute__((noreturn))
static void jump_to_kernel32(pcinfo_t *pcinfo)
{
    multiboot_info_t *mb_info = mb_fill_info(pcinfo);

    __asm__ volatile (
        "movl %0, %%eax\n\t"
        "movl %1, %%ebx\n\t"
        "jmp *%2\n\t"
        :
        : "i"(MULTIBOOT_BL_MAGIC), "r"(mb_info), "r"((uint32_t)KERNEL32_PHYS_ADDR)
        : "eax", "ebx"
    );
    __builtin_unreachable();
}

void thirdix_entry(pcinfo_t *pcinfo)
{
    idt_new();
    binit(thirdix_arena, THIRDIX_ARENA_SIZE);
    screen_new(pcinfo);
    thrfs_new(&_image_end, (size_t)(&_bss_start - &_image_end));

    ui_choice_t choice = ui_run(pcinfo);
    
    if (choice == UI_CHOICE_16BIT)
    {
        cmos_unset_flagz();
        cmos_set_lrmk();
        reboot();
    } else jump_to_kernel32(pcinfo);
}