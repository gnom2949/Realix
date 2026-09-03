; © Realix > Config
; (15.08.26) v0.12
; ================

; Защита от повторного включения
%ifndef CONFIG_ASM
%define CONFIG_ASM


; ! Не забывайте менять эту строку
%define OS_VERSION 'v0.12'

; Управляющие символы и ASCII коды
%define ENTER 0x0D, 0x0A

; Системные адреса памяти
PCINFO_ADDR          equ 0x4500
FAT_BUFFER_ADDR      equ 0x0500
BOOTIX_LOAD_OFFSET   equ 0x7C00
INITRIX_LOAD_SEGMENT equ 0x07E0
INITRIX_LOAD_OFFSET  equ 0
KERNEL_LOAD_SEGMENT  equ 0x1000
KERNEL_LOAD_OFFSET   equ 0
KERNEL32_PHYS_ADDR   equ (KERNEL_LOAD_SEGMENT*16 + KERNEL_LOAD_OFFSET)
THIRDIX_LOAD_SEGMENT equ 0x4000
THIRDIX_LOAD_OFFSET  equ 0
THIRDIX_PHYS_ADDR    equ (THIRDIX_LOAD_SEGMENT*16 + THIRDIX_LOAD_OFFSET)

; CMOS-флаг "пропустить Thirdix", грузить 16-битное ядро сразу, пропуская Thirdix.
; (bios-api/memory/cmos.asm).
CMOS_FLAG_INDEX     equ 0x31
CMOS_FLAG_CHECK     equ 0x32
CMOS_FLAG_LRMK      equ 0xAA
CMOS_MAGIC_CHECK    equ 0x55

; Раскладка структуры PCINFO
PCINFO_ALL_MEM      equ 0   ; Размер всей памяти (МБ)
PCINFO_LOW_MEM      equ 4   ; Размер "нижней" памяти (КБ)
PCINFO_MMAP_ENTRIES equ 6   ; Кол-во записей карты памяти
PCINFO_DRIVE        equ 8   ; Номер загрузочного диска
PCINFO_VIDEOMODE    equ 10  ; Номер видеорежима (0 - текстовый, 1 - видеорежим)
; --- VBE Framebuffer(bios-api/video/vbe.asm) ---
PCINFO_FB_ADDR      equ 12  ; физ. адрес framebuffer (u32).
PCINFO_FB_PITCH     equ 16  ; байт на строку (u16).
PCINFO_FB_WIDTH     equ 18  ; Ширина в пикселях (u16).
PCINFO_FB_HEIGHT    equ 20  ; Высота в пикселях (u16).
PCINFO_FB_BPP       equ 22  ; Бит на пиксель (u8).
PCINFO_FB_VALID     equ 23  ; 1 = framebuffer через Vesa BIOS Ext's, 0 = нет (u8).
PCINFO_MMAP         equ 24  ; Массив записей E820, сдвинул на 24 из-за VBE.

; VBE: желаемый режим и адреса временных буферов для BIOS-вызовов
VBE_DESIRED_WIDTH   equ 1024
VBE_DESIRED_HEIGHT  equ 768
VBE_DESIRED_BPP     equ 32
VBE_INFO_BUFFER     equ 0x6000
VBE_MODEINFO_BUFFER equ 0x6200

; Маркеры кластеров FAT12 (общие для bootix и bios-api/fat12)
CHAIN_END   equ 0x0FF8  ; Кластер >= этого - конец цепочки (EOF)
BAD_CLUSTER equ 0x0FF7  ; Дефектный кластер

; Видеорежимы BIOS: ah - Установка режима, al - Выбранный режим
TEXT_MODE_80x25    equ 0x0003
VIDEO_MODE_320x200 equ 0x0013

; Константы карты памяти (❗️ MAX_ENTRIES используется и из Rust)
E820_ENTRY_SIZE  equ 24
E820_MAX_ENTRIES equ 64

; Магический заголовок файла .RLX
%define RLX_MAGIC 0xFC26

; Флаги режима архитектуры
; (16-бит: Real Mode / System API, 32-бит: Protected Mode / Ring 3)
%define RLX_MODE_16 0x16
%define RLX_MODE_32 0x32

; Системные вызовы (int 0x80)
%define SYS_PRINT_STRING 1  ; Печать строки (DS:SI для 16-бит, ESI для 32-бит)
%define SYS_PUTCHAR      2  ; Печать символа (AL для 16/32-бит)
%define SYS_EXIT         3  ; Завершение работы программы
%define SYS_READ_KEY     4  ; Ожидание нажатия клавиши (AL - символ)
%define SYS_CLEAR        5  ; Очистка экрана

%endif