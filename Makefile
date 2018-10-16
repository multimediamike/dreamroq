# Put the filename of the output binary here
TARGET = dreamroq-player.elf
PROJECT_NAME = dreamroq_ph3nom
DIR=$(PWD)

# List all of your C files here, but change the extension to ".o"
OBJS = dreamroq-player.o dreamroqlib.o

#AICA Audio Driver
KOS_CFLAGS += -I. -Ilibdcmc/
OBJS += libdcmc/snddrv.o

#PVR Video Driver
OBJS += libdcmc/pvrdrv.o

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

run: $(TARGET)
	$(KOS_LOADER) $(TARGET)

cd: $(TARGET)
	@sh-elf-objcopy -R .stack -O binary $(TARGET) output.bin
	@$(KOS_BASE)/utils/scramble/scramble output.bin 1ST_READ.BIN
	@mkisofs -C 0,11702 -V Reaperi_Cycle -G $(KOS_BASE)/IP.BIN -r -J -l -m '*.o' -o ../$(PROJECT_NAME).iso $(DIR)
	@$(KOS_BASE)/utils/cdi4dc/cdi4dc ../$(PROJECT_NAME).iso ../$(PROJECT_NAME).cdi -d > cdi4dc.log
	../redream ../$(PROJECT_NAME).cdi

dist:
	rm -f $(OBJS) romdisk.o romdisk.img
	$(KOS_STRIP) $(TARGET)
