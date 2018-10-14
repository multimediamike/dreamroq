# Put the filename of the output binary here
TARGET = dreamroq-player.elf
DIR=$(PWD)

# List all of your C files here, but change the extension to ".o"
OBJS = dreamroq-player.o dreamroqlib.o

all: rm-elf $(TARGET)

include $(KOS_BASE)/Makefile.rules

clean:
	-rm -f $(TARGET) $(OBJS)

rm-elf:
	-rm -f $(TARGET)

# If you don't need a ROMDISK, then remove "romdisk.o" from the next few
# lines. Also change the -l arguments to include everything you need,
# such as -lmp3, etc.. these will need to go _before_ $(KOS_LIBS)
$(TARGET): $(OBJS)
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $(TARGET) $(KOS_START) \
		$(OBJS) $(OBJEXTRA) $(KOS_LIBS)

cd: $(TARGET) $(OBJS)
	#elf transform ---- make sure the *.elf name matches your .elf file.
	sh-elf-objcopy -R .stack -O binary $(TARGET) output.bin
	#scraming process
	$(KOS_BASE)/utils/scramble/scramble output.bin 1ST_READ.bin
	#creating iso -> -o outputname.iso
	mkisofs -C 0,11702 -V DC_GAME -G IP.BIN -r -J -l -m '*.o' -x $(DIR)/builds -o $(DIR)/builds/$(TARGET).iso $(DIR)

run: $(CD)
	lxdream builds/$(TARGET).iso

dist:
	rm -f $(OBJS) romdisk.o romdisk.img
	$(KOS_STRIP) $(TARGET)
