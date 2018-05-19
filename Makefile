Q?=@
QQ=@

RM?=rm

PREFIX_ARCH?=arm-none-eabi
AS=$(PREFIX_ARCH)-as
CC=$(PREFIX_ARCH)-gcc
LD=$(PREFIX_ARCH)-ld
OBJDUMP=$(PREFIX_ARCH)-objdump
OBJCOPY=$(PREFIX_ARCH)-objcopy

ASFLAGS:=--warn
ASFLAGS+=--fatal-warnings
ASFLAGS+=-mcpu=cortex-m3

CFLAGS:=-Wall
CFLAGS+=-O0
CFLAGS+=-nostdlib
CFLAGS+=-nostartfiles
CFLAGS+=-ffreestanding
CFLAGS+=-mcpu=cortex-m3
CFLAGS+=-mthumb

SRC=src
BUILD=build

SOURCES:=$(wildcard $(SRC)/*.c)
SOURCES+=$(wildcard $(SRC)/*.s)
OBJECTS:=$(patsubst $(SRC)/%,$(BUILD)/%.o,$(SOURCES))

TARGET=notmain

all: $(TARGET).elf $(TARGET).bin

$(OBJECTS): | $(BUILD)

$(BUILD):
	$(Q) mkdir -p $(BUILD)

$(BUILD)/%.c.o: $(SRC)/%.c
	$(QQ) echo "  CC      $@"
	$(Q) $(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.s.o: $(SRC)/%.s
	$(QQ) echo "  AS      $@"
	$(Q) $(AS) $(ASFLAGS) $< -o $@

$(TARGET).elf: $(OBJECTS)
	$(QQ) echo "  LD      $@"
	$(Q) $(LD) -o $@ -T flash.ld $^
	$(QQ) echo "  OBJDUMP $@"
	$(Q) $(OBJDUMP) -D $@ >$@.list

$(TARGET).bin: $(TARGET).elf
	$(QQ) echo "  OBJCOPY $@"
	$(Q) $(OBJCOPY) $< $@ -O binary

clean:
	$(QQ) echo "  RM      $(OBJECTS)"
	$(Q) $(RM) -r $(OBJECTS)
	$(QQ) echo "  RM      $(TARGET).elf $(TARGET).bin $(TARGET).elf.list"
	$(Q) $(RM) -r $(TARGET).elf $(TARGET).bin $(TARGET).elf.list

.PHONY: clean all
