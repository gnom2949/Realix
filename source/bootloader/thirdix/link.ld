STAGE3_LOAD_ADDR = 0x40000;

STAGE3_BSS_ADDR = 0x60000;

ENTRY(_start)

SECTIONS
{
    . = STAGE3_LOAD_ADDR;

    .text : {
        *(.text.entry) 
        *(.text*)
    }

    .rodata : { *(.rodata*) }
    .data   : { *(.data*) }

    . = ALIGN(4);
    _image_end = .; 

    . = STAGE3_BSS_ADDR;
    _bss_start = .;
    .bss (NOLOAD) : { *(.bss*) *(COMMON) }
    _bss_end = .;

    /DISCARD/ : { *(.comment) *(.eh_frame) *(.note*) }
}
