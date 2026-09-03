# © Realix > Thirdix > Makefile
# =============================

CC = gcc
AS = nasm
LD = ld
OC = objcopy

CFLAGS  = -m32 -ffreestanding -fno-pic -fno-stack-protector -Wall -Wextra -IInclude
ASMFLAGS = -f elf32
LDFLAGS  = -m elf_i386 -T link.ld -nostdlib

SRC := Source/
SRC_SRC  := $(wildcard $(SRC)src/*.c)
DRV      := $(wildcard $(SRC)drivers/*.c)
FS       := $(wildcard $(SRC)fs/*.c)
LIBC     := $(wildcard $(SRC)libc/*/*.c)
LIBC_ASM := $(wildcard $(SRC)libc/*/*.asm)
ASM      := $(wildcard $(SRC)asm/*.asm)

OBJ := $(patsubst %.c,build/%.o,$(notdir $(SRC_SRC))) \
       $(patsubst %.c,build/%.o,$(notdir $(DRV))) \
       $(patsubst %.c,build/%.o,$(notdir $(FS))) \
       $(patsubst %.c,build/%.o,$(notdir $(LIBC))) \
       $(patsubst %.asm,build/%.o,$(notdir $(LIBC_ASM))) \
       $(patsubst %.asm,build/%.o,$(notdir $(ASM)))

vpath %.c   $(SRC)src $(SRC)drivers $(SRC)fs $(SRC)libc/string $(SRC)libc/stdlib $(SRC)libc/atomic
vpath %.asm $(SRC)libc/string $(SRC)asm

LIBGCC := $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

ASSETS_DIR = assets
ASSETS_TAR = build/fs.tar

all: thirdix_bin

build:
	mkdir -p build

$(ASSETS_DIR):
	mkdir -p $(ASSETS_DIR)

build/%.o: %.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: %.asm | build
	$(AS) $(ASMFLAGS) $< -o $@

build/thirdix_img.elf: $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) $(LIBGCC) -o $@

build/thirdix_img.bin: build/thirdix_img.elf
	$(OC) -O binary $< $@

$(ASSETS_TAR): $(wildcard $(ASSETS_DIR)/*) | $(ASSETS_DIR) build
	tar --format=ustar -cf $@ -C $(ASSETS_DIR) 

thirdix_bin: build/thirdix_img.bin $(ASSETS_TAR)
	cat build/thirdix_img.bin $(ASSETS_TAR) > $@

clean:
	rm -rf build thirdix_bin

.PHONY: all clean
