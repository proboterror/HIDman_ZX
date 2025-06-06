#OSC_TYPE = OSC_INTERNAL
OSC_TYPE = OSC_EXTERNAL

BUILD_OPTIONS = OPT_DEFAULT

CC = sdcc
GCC = gcc
OBJCOPY = sdobjcopy
PACK_HEX = packihx
WCHISP = wchisptool
OBJDIR = ./build

TARGET = hidman

COMMON = \
$(OBJDIR)/keyboardled.rel \
$(OBJDIR)/parsedescriptor.rel \
$(OBJDIR)/system.rel \
$(OBJDIR)/usbhost.rel \
$(OBJDIR)/data.rel \
$(OBJDIR)/ps2protocol.rel \
$(OBJDIR)/andyalloc.rel \
$(OBJDIR)/mouse.rel \
$(OBJDIR)/settings.rel \
$(OBJDIR)/usbll.rel \
$(OBJDIR)/preset.rel \
$(OBJDIR)/ps2mapping.rel \
$(OBJDIR)/linkedlist.rel \
$(OBJDIR)/scancode.rel \
$(OBJDIR)/CH446Q.rel \
$(OBJDIR)/ps2_keyboard.rel \
$(OBJDIR)/ps2_mouse.rel \
$(OBJDIR)/zx_keyboard.rel \
$(OBJDIR)/gotek_buttons.rel \
$(OBJDIR)/kempston_joy.rel \
$(OBJDIR)/custom_handlers.rel \
$(OBJDIR)/ps3.rel \
$(OBJDIR)/xbox360_usb.rel \
$(OBJDIR)/xbox_one.rel

COMMON += $(OBJDIR)/uart.rel
COMMON += $(OBJDIR)/uart1.rel

ifndef FREQ_SYS
FREQ_SYS = 48000000
endif

ifndef XRAM_SIZE
XRAM_SIZE = 0x1600
endif

ifndef XRAM_LOC
XRAM_LOC = 0x0200
endif

ifndef CODE_SIZE
CODE_SIZE = 0xEFFF
endif

CFLAGS := -V -mmcs51 --model-large --stack-auto \
	--xram-size $(XRAM_SIZE) --xram-loc $(XRAM_LOC) \
	--code-size $(CODE_SIZE) \
	-I/ -DFREQ_SYS=$(FREQ_SYS) -D$(OSC_TYPE) \
	-DENABLE_WHEEL -D$(BUILD_OPTIONS) $(EXTRA_FLAGS)

LFLAGS := $(CFLAGS)

TESTCFLAGS := -V -mmcs51 --model-large --stack-auto \
	--xram-size 0x8000 --xram-loc $(XRAM_LOC) \
	--code-size 0xEFFF \
	-I/ -DFREQ_SYS=$(FREQ_SYS) -D$(OSC_TYPE) \
	$(EXTRA_FLAGS)

print-%  : ; @echo $* = $($*)

makebuilddir:
	@-mkdir build

$(OBJDIR)/%.rel : %.c
	$(CC) -c $(CFLAGS) -o $@ $<


$(OBJDIR)/$(TARGET).ihx: makebuilddir $(COMMON) $(OBJDIR)/main.rel
	$(CC) $(COMMON) $(OBJDIR)/main.rel $(LFLAGS) -o $(OBJDIR)/$(TARGET).ihx
	

$(OBJDIR)/$(TARGET).hex: $(OBJDIR)/$(TARGET).ihx
	$(PACK_HEX) $(OBJDIR)/$(TARGET).ihx > $(OBJDIR)/$(TARGET).hex

$(OBJDIR)/$(TARGET).bin: $(OBJDIR)/$(TARGET).ihx
	$(OBJCOPY) -I ihex -O binary $(OBJDIR)/$(TARGET).ihx $(OBJDIR)/$(TARGET).bin

size: $(OBJDIR)/$(TARGET).ihx
	@echo '---------- Segments ----------'
	@egrep '(ABS,CON)|(REL,CON)' $(OBJDIR)/$(TARGET).map | gawk --non-decimal-data '{dec = sprintf("%d","0x" $$2); print dec " " $$0}' | /usr/bin/sort -n -k1 | cut -f2- -d' ' | uniq
	@echo '---------- Memory ----------'
	@egrep 'available|EXTERNAL|FLASH' $(OBJDIR)/$(TARGET).mem

flash: $(OBJDIR)/$(TARGET).bin
	$(WCHISP) -f $(OBJDIR)/$(TARGET).bin -g

.DEFAULT_GOAL := all
all: $(OBJDIR)/$(TARGET).bin $(OBJDIR)/$(TARGET).hex

clean:
	-rm -f $(OBJDIR)/*

test: $(COMMON) $(OBJDIR)/test.rel $(OBJDIR)/testdata.rel
	$(CC) $(COMMON) $(OBJDIR)/test.rel $(OBJDIR)/testdata.rel $(TESTCFLAGS) -o $(OBJDIR)/test.ihx
	s51 -s - build/test 
