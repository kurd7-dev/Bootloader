# Custom ARM64 Bootloader Makefile

CROSS_COMPILE ?= aarch64-linux-gnu-
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS = -Wall -Wextra -O2 -ffreestanding -nostdlib -nostartfiles
ASFLAGS = -march=armv8-a
LDFLAGS = -T linker.ld

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.c)
ASMS = $(wildcard $(SRC_DIR)/*.s)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS)) \
       $(patsubst $(SRC_DIR)/%.s, $(BUILD_DIR)/%.o, $(ASMS))

TARGET = bootloader.bin

.PHONY: all clean

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(TARGET): $(OBJS) linker.ld
	$(LD) $(LDFLAGS) $(OBJS) -o bootloader.elf
	$(OBJCOPY) -O binary bootloader.elf $(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET) bootloader.elf