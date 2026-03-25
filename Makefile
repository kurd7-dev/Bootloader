# ARM64 bootloader Makefile (QEMU virt)

CROSS_COMPILE ?= aarch64-linux-gnu-
CC      := $(CROSS_COMPILE)gcc
LD      := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy

BUILD_DIR := build
TARGET_ELF := bootloader
TARGET_BIN := bootloader.bin

CFLAGS := -Wall -Wextra -Werror -std=c11 -O2 \
	-ffreestanding -fno-stack-protector -fno-builtin -fno-pic \
	-mcpu=cortex-a72 -mgeneral-regs-only -nostdlib -nostartfiles \
	-Iinclude

ASFLAGS := -D__ASSEMBLY__
LDFLAGS := -T linker.ld -nostdlib

RWILDCARD = $(wildcard $(1)/*.c) $(wildcard $(1)/*.s) $(foreach d,$(wildcard $(1)/*),$(call RWILDCARD,$(d)))
SOURCES := $(call RWILDCARD,src)
OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(patsubst %.s,$(BUILD_DIR)/%.o,$(SOURCES)))

.PHONY: all clean run

all: $(TARGET_ELF) $(TARGET_BIN)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(ASFLAGS) -c $< -o $@

$(TARGET_ELF): $(OBJECTS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

$(TARGET_BIN): $(TARGET_ELF)
	$(OBJCOPY) -O binary $< $@

run: $(TARGET_ELF)
	qemu-system-aarch64 -machine virt -cpu cortex-a72 -m 2048 -nographic -kernel $(TARGET_ELF)

clean:
	rm -rf $(BUILD_DIR) $(TARGET_ELF) $(TARGET_BIN)