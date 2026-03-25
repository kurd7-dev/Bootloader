# ARM64 bootloader Makefile with lightweight Kconfig/defconfig flow

CONFIG_FILE := .config
CONFIG_MK   := include/generated/config.mk
AUTOCONF_H  := include/generated/autoconf.h

.DEFAULT_GOAL := all

ifeq ($(wildcard $(CONFIG_FILE)),)
$(shell cp configs/qemu_virt_defconfig $(CONFIG_FILE))
endif

$(AUTOCONF_H) $(CONFIG_MK): $(CONFIG_FILE) scripts/genconfig.py
	python3 scripts/genconfig.py $(CONFIG_FILE) $(AUTOCONF_H) $(CONFIG_MK)

-include $(CONFIG_MK)

ifeq ($(strip $(CROSS_COMPILE)),)
CROSS_COMPILE := $(shell if command -v aarch64-none-elf-gcc >/dev/null 2>&1; then echo aarch64-none-elf-; else echo aarch64-linux-gnu-; fi)
endif

CC      := $(CROSS_COMPILE)gcc
LD      := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy

BUILD_DIR := build
TARGET_ELF := bootloader.elf
TARGET_IMAGE := $(subst ",,$(CONFIG_OUTPUT_IMAGE))

CFLAGS := -Wall -Wextra -Werror -std=c11 -O2 \
	-ffreestanding -fno-stack-protector -fno-builtin -fno-pic \
	-mcpu=cortex-a53 -mgeneral-regs-only -nostdlib -nostartfiles \
	-Iinclude

ASFLAGS := -D__ASSEMBLY__
LDFLAGS := -T linker.ld -nostdlib --defsym=BOOT_BASE=$(CONFIG_BOOTLOADER_BASE)

RWILDCARD = $(wildcard $(1)/*.c) $(wildcard $(1)/*.s) $(foreach d,$(wildcard $(1)/*),$(call RWILDCARD,$(d)))
SOURCES := $(call RWILDCARD,src)
OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(patsubst %.s,$(BUILD_DIR)/%.o,$(SOURCES)))

.PHONY: all clean qemu_virt_defconfig rpi3_defconfig olddefconfig printconfig run

all: $(AUTOCONF_H) $(TARGET_ELF) $(TARGET_IMAGE)

qemu_virt_defconfig:
	cp configs/qemu_virt_defconfig $(CONFIG_FILE)
	python3 scripts/genconfig.py $(CONFIG_FILE) $(AUTOCONF_H) $(CONFIG_MK)

rpi3_defconfig:
	cp configs/rpi3_defconfig $(CONFIG_FILE)
	python3 scripts/genconfig.py $(CONFIG_FILE) $(AUTOCONF_H) $(CONFIG_MK)

olddefconfig: $(AUTOCONF_H)

printconfig: $(AUTOCONF_H)
	@echo "CROSS_COMPILE=$(CROSS_COMPILE)"
	@echo "TARGET_IMAGE=$(TARGET_IMAGE)"
	@echo "CONFIG_TARGET_QEMU_VIRT=$(CONFIG_TARGET_QEMU_VIRT)"
	@echo "CONFIG_TARGET_RPI3=$(CONFIG_TARGET_RPI3)"

$(BUILD_DIR)/%.o: %.c $(AUTOCONF_H)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s $(AUTOCONF_H)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(ASFLAGS) -c $< -o $@

$(TARGET_ELF): $(OBJECTS) linker.ld $(AUTOCONF_H)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

$(TARGET_IMAGE): $(TARGET_ELF)
	$(OBJCOPY) -O binary $< $@

ifeq ($(CONFIG_ENABLE_QEMU_RUN_TARGET),y)
run: $(TARGET_ELF)
	qemu-system-aarch64 -machine virt -cpu cortex-a72 -m 2048 -nographic -kernel $(TARGET_ELF)
else
run:
	@echo "run target is disabled for current board config"
	@false
endif

clean:
	rm -rf $(BUILD_DIR) $(TARGET_ELF) bootloader.bin kernel8.img