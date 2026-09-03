; © Realix => CMOS Third Stage Skip Flagg
; =======================================
;   Флаг в CMOS RTC RAM, сообщает Second Stage, нужно ли загрузить третий этап или загрузить 16 битное ядро.
;
;   Используется область CMOS вне зоны BIOS Setup (0x10-0x2D) и чексуммы (0x2E-0x2F).
; 
;   Хранятся два байта (флаг и его "подтверждение"), чтобы на случай сдохшей батарейки отличить реальный флаг от мусора.

bits 16

; порты доступа CMOS
CMOS_PORT_INDEX equ 0x70
CMOS_PORT_DATA equ 0x71

; индексы байт в CMOS RAM
CMOS_FLAG_INDEX equ 0x31
CMOS_FLAG_CHECK equ 0x32

; значения
CMOS_FLAG_LOAD_RMKERNEL equ 0xAA
CMOS_FLAG_RMKERNEL_CHECK equ 0x55 ; ~CMOS_FLAG_LOAD_RMKERNEL

; Чтение байта CMOS
; Параметры:
; - al: индекс байта (0x00-0x7F) 
; Возвращает:
; - al: значение байта
cmos_read_byte:
    and al, 0x7F
    cli
    out CMOS_PORT_INDEX, al
    sti
    ret

; Запись байта CMOS
; Параметры:
; - al: индекс байта (0x00-0x7F) 
; - ah: значение для записи
cmos_write_byte:
    push cx
    mov cl, ah ; здравствуйте
    and al, 0x7F
    cli
    out CMOS_PORT_INDEX, al
    mov al, cl
    out CMOS_PORT_DATA, al
    sti
    pop cx
    ret

; Проверка: нужно пропустить Stage 3 и грузить 16 битное ядро
; Возвращает:
;   - CF=1: грузин 16-битное ядро.
;   - CF=0: грузин Stage 3.
check_flagz_lrmk:
    mov al, CMOS_FLAG_INDEX
    call cmos_read_byte
    cmp al, CMOS_FLAG_LOAD_RMKERNEL
    jne .not_set

    mov al, CMOS_FLAG_CHECK
    call cmos_read_byte
    cmp al, CMOS_FLAG_RMKERNEL_CHECK
    jne .not_set
    
    stc
    ret

.not_set:
    clc
    ret

; Установка флага Load Real Mode Kernel
set_flagz_lrmk:
    push ax

    mov al, CMOS_FLAG_INDEX
    mov ah, CMOS_FLAG_LOAD_RMKERNEL
    call cmos_write_byte

    mov al, CMOS_FLAG_CHECK
    mov ah, CMOS_FLAG_RMKERNEL_CHECK
    call cmos_write_byte

    pop ax
    ret

; Сброс флага Load Real Mode Kernel
clear_flagz_lrmk:
    push ax

    mov al, CMOS_FLAG_INDEX
    mov ah, 0x00
    call cmos_write_byte

    mov al, CMOS_FLAG_CHECK
    mov ah, 0x00
    call cmos_write_byte

    pop ax
    ret