
# ---------------------------------------------------------------------------
# Toolchain
# =============================================================================
# SENG21213-OS :: Makefile
# =============================================================================

AS      := nasm
ASFLAGS := -f elf32

ifneq (, $(shell which i686-elf-gcc 2>/dev/null))
	CC   := i686-elf-gcc
	LD   := i686-elf-ld
else
	CC   := gcc
	LD   := ld
endif

CFLAGS := -m32 -ffreestanding -fno-stack-protector -fno-pie -nostdlib \
	  -Wall -Wextra -O2 -I./include
LDFLAGS := -m elf_i386 -nostdlib

# ---------------------------------------------------------------------------
# Sources & Objects
# ---------------------------------------------------------------------------
BOOT_SRC  := boot/boot.asm
BOOT_BIN  := boot/boot.bin

KERNEL_ASM_SRC := kernel/kernel_entry.asm
KERNEL_ASM_OBJ := build/kernel_entry.o

KERNEL_C_SRCS  := kernel/kernel.c \
	          kernel/vga.c \
	          kernel/keyboard.c \
	          kernel/shell.c \
	          kernel/string.c \
	          kernel/process.c \
	          kernel/scheduler.c

BOOT_OBJS      := build/boot/switch.o
KERNEL_C_OBJS  := $(patsubst kernel/%.c, build/%.o, $(KERNEL_C_SRCS))

KERNEL_ELF     := build/kernel.elf
KERNEL_BIN     := build/kernel.bin
OS_IMAGE       := seng21213-os.img

# ---------------------------------------------------------------------------
# Default target
# ---------------------------------------------------------------------------
.PHONY: all clean run run-debug info

all: $(OS_IMAGE)
	@echo ""
	@echo "  ✓  Build successful → $(OS_IMAGE)"
	@echo "  →  Run with:  make run"
	@echo ""

# ---------------------------------------------------------------------------
# Bootloader
# ---------------------------------------------------------------------------
$(BOOT_BIN): $(BOOT_SRC)
	@mkdir -p build
	@echo "  [AS]  $<"
	$(AS) -f bin $< -o $@

# ---------------------------------------------------------------------------
# Kernel: Assembly objects
# ---------------------------------------------------------------------------
$(KERNEL_ASM_OBJ): $(KERNEL_ASM_SRC)
	@mkdir -p build
	@echo "  [AS]  $<"
	$(AS) $(ASFLAGS) $< -o $@

$(BOOT_OBJS): boot/switch.asm
	@mkdir -p build/boot
	@echo "  [AS]  $<"
	$(AS) $(ASFLAGS) $< -o $@

# ---------------------------------------------------------------------------
# Kernel: C objects
# ---------------------------------------------------------------------------
build/%.o: kernel/%.c
	@mkdir -p build
	@echo "  [CC]  $<"
	$(CC) $(CFLAGS) -c $< -o $@

# ---------------------------------------------------------------------------
# Link kernel ELF, then extract flat binary
# ---------------------------------------------------------------------------
$(KERNEL_ELF): $(KERNEL_ASM_OBJ) $(BOOT_OBJS) $(KERNEL_C_OBJS)
	@echo "  [LD]  $@"
	$(LD) $(LDFLAGS) -T linker.ld $^ -o $@

$(KERNEL_BIN): $(KERNEL_ELF)
	@echo "  [OBJCOPY] $@"
	objcopy -O binary $< $@

# ---------------------------------------------------------------------------
# Disk image
# ---------------------------------------------------------------------------
$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	@echo "  [IMG]  Creating $(OS_IMAGE)..."
	dd if=/dev/zero  bs=512 count=2880 of=$(OS_IMAGE) 2>/dev/null
	dd if=$(BOOT_BIN)    conv=notrunc bs=512 count=1    of=$(OS_IMAGE) 2>/dev/null
	dd if=$(KERNEL_BIN)  conv=notrunc bs=512 seek=1     of=$(OS_IMAGE) 2>/dev/null
	@echo "  [IMG]  $(OS_IMAGE) ready"

# ---------------------------------------------------------------------------
# Run in QEMU
# ---------------------------------------------------------------------------
QEMU      := qemu-system-i386
QEMUFLAGS := -drive format=raw,file=$(OS_IMAGE) -m 32M -display curses

run: $(OS_IMAGE)
	@echo "  Starting QEMU... (Close window to exit)"
	$(QEMU) $(QEMUFLAGS)

run-debug: $(OS_IMAGE)
	$(QEMU) $(QEMUFLAGS) -S -gdb tcp::1234 &
	@echo "  QEMU paused. Connect GDB: target remote :1234"

info:
	@echo "Toolchain: CC=$(CC)  AS=$(AS)  LD=$(LD)"

clean:
	rm -rf build $(OS_IMAGE)
	@echo "  Cleaned."
