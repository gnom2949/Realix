# © Realix > Makefile: Bootloader
# (27.07.26) v0.1
# ================

# Конфигурация
BUILD_DIR ?= $(abspath ../../build)
SRC_DIR   ?= $(abspath ../../source)
ASM       := nasm
ASMFLAGS  := -f bin -i $(SRC_DIR)
ASM_SRC := $(shell find $(SRC_DIR) -name '*.asm')

.PHONY: all bootloader bootix initrix clean
SUBMAKE_VARS := BUILD_DIR=$(abspath $(BUILD_DIR)) SRC_DIR=$(abspath $(SRC_DIR))
# Запуск по умолчанию
all: bootix initrix thirdix

# Сборка Bootix
bootix: $(BUILD_DIR)/bootix.bin

$(BUILD_DIR)/bootix.bin: $(ASM_SRC) | $(BUILD_DIR)
	$(ASM) $(ASMFLAGS) $(SRC_DIR)/bootloader/bootix/main.asm -o $@

# Сборка Initrix
initrix: $(BUILD_DIR)/initrix.bin

$(BUILD_DIR)/initrix.bin: $(ASM_SRC) | $(BUILD_DIR)
	$(ASM) $(ASMFLAGS) $(SRC_DIR)/bootloader/initrix/main.asm -o $@

# Сборка Thirdix
thirdix: $(BUILD_DIR)/thirdix.bin

$(BUILD_DIR)/thirdix.bin:
	$(MAKE) -C thirdix $(SUBMAKE_VARS) 

$(BUILD_DIR):
	mkdir -p $@

# Очистка
clean:
	rm -f $(BOOTIX_BIN) $(INITRIX_BIN)
