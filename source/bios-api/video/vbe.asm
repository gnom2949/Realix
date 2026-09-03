; bios-api/video/vbe.asm
; © Realix > VBE (VESA BIOS Extensions) - real-mode init.
; =======================
; ! Вызывать последним действием, после всего текстового вывода и перед jmp boot_switcher - как только режим будет в LFB,
; текстовый буфер будет не актуален для вывода.

; Ни при какой ошибке не роняет загрузку - просто оставляет PCINFO_FB_VALID=0 и
; PCINFO_VIDEOMODE=0, код идёт дальше как ни в чём не бывало.
; Thirdix обязан проверять валидность PCINFO_FB_VALID.

; vbe_try_init
; Возвращает:
;   - CF=0: успех - PCINFO_FB_ заполнены.
;   - CF=1: не удалось.
vbe_try_init:
    push es
    push di
    push si
    push ax
    push bx
    push cx 
    push dx 

    ; гарантируем валидное состояние по умолчанию
    xor ax, ax
    mov es, ax
    mov byte [es:PCINFO_ADDR + PCINFO_FB_VALID], 0
    mov word [es:PCINFO_ADDR + PCINFO_VIDEOMODE], 0

    ; VBE Controller Info
    push ds ; нинтенда
    pop es
    mov di, VBE_INFO_BUFFER
    mov byte [es:di + 0], 'V'
    mov byte [es:di + 1], 'B'
    mov byte [es:di + 3], 'E'
    mov byte [es:di + 4], '2'

    mov ax, 0x4F00
    int 0x10
    cmp ax, 0x004F
    jne .fail

    ; указатель на список видеорежимов (VbeInfoBlock+0x0E: off, +0x10: seg)
    mov si, [es:di + 0x0E]
    mov ax, [es:di + 0x10]
    mov fs, ax

.mode_scan:
    mov cx, [fs:si]
    add si, 2
    cmp cx, 0xFFFF
    je .fail

    ; получение Mode Info для тек. режима (перезап. VBE_MODEINFO_BUFFER)
    push cx
    mov ax, 0x4F01
    mov di, VBE_MODEINFO_BUFFER
    int 0x10
    pop cx
    cmp ax, 0x004F
    jne .mode_scan  ; этот режим не читается, пробуем следующий

    ; attributes (offset 0x00), бит 7 - поддержка LFB
    mov ax, [es:di + 0x00]
    test ax, 0x0080
    jz .mode_scan

    ; XRes / YRes / BPP
    mov ax, [es:di + 0x12]
    cmp ax, VBE_DESIRED_WIDTH
    jne .mode_scan
    mov ax, [es:di + 0x14]
    cmp ax, VBE_DESIRED_HEIGHT
    jne .mode_scan
    mov al, [es:di + 0x19]
    cmp al, VBE_DESIRED_BPP
    jne .mode_scan

    ; нашли подходящий режим - сохраняем pitch/physbase, пока ModeInfoBlock ещё в буфере
    mov ax, [es:di + 0x10]  ; pitch
    mov [found_fb_pitch], ax

    mov eax, [es:di + 0x28] ; PhysBasePtr (валиден только у VBE2+)
    mov [found_fb_physaddr], eax

    ; установка режима (бит 14 - использовать LFB)
    mov bx, cx
    or bx, 0x4000
    mov ax, 0x4F02
    int 0x10
    cmp ax, 0x004F
    jne .fail

    ; записываем результат в PCINFO (сегмент 0)
    xor ax, ax
    mov es, ax

    mov eax, [found_fb_physaddr]
    mov [es:PCINFO_ADDR + PCINFO_FB_ADDR], eax

    mov ax, [found_fb_pitch]
    mov [es:PCINFO_ADDR + PCINFO_FB_PITCH], ax

    mov word [es:PCINFO_ADDR + PCINFO_FB_WIDTH], VBE_DESIRED_WIDTH
    mov word [es:PCINFO_ADDR + PCINFO_FB_HEIGHT], VBE_DESIRED_HEIGHT
    mov byte [es:PCINFO_ADDR + PCINFO_FB_BPP], VBE_DESIRED_BPP
    mov byte [es:PCINFO_ADDR + PCINFO_FB_VALID], 1
    mov word [es:PCINFO_ADDR + PCINFO_VIDEOMODE], 1

    pop dx
    pop cx
    pop bx
    pop ax
    pop si
    pop di
    pop es
    clc
    ret

.fail:
    pop dx
    pop cx
    pop bx
    pop ax
    pop si
    pop di
    pop es
    stc
    ret

found_fb_physaddr:  dd 0
found_fb_pitch:     dw 0