#! /bin/sh
#automating script for making CD games

DIR=$PWD
PROJECT_NAME="Reaperi_Cycle_V3_Hideout_LastStretch"
SD_NAME="Reaperi_SD"

# go to  build directory
# make clean
make clean
make

#elf transform??
#sh-elf-objcopy -R .stack -O binary main.elf output.bin

#scraming process
#$KOS_BASE/utils/scramble/scramble output.bin 1ST_READ.BIN

#creating a copy for SDISO - for somereason it tries to find the 1ST_READ there.
#cp 1ST_READ.bin data/1ST_READ.bin

#creating iso -> -o outputname.cdi / build files path
#mkisofs -C 0,11702 -V Reaperi_Cycle -G IP.BIN -r -J -l -m '*.o' -x $DIR/.git -o ../$PROJECT_NAME.iso $DIR

#creating sdiso -> -o outputname.cdi / build files path
#mkisofs -V Reaperi_Cycle -G IP.BIN -r -J -l -m '*.o' -x $DIR/.git -o ../$SD_NAME.iso $DIR


#mkisofs -C 0,11702 -V DC_GAME -G IP.BIN -m '*.o' -x $DIR/.git -o ../$PROJECT_NAME.iso $DIR
#mkisofs -C 0,45000 -V DC_GAME -G IP.BIN -r -J -l -x $DIR/.git -o  ../$PROJECT_NAME.iso $DIR
#mkisofs -C 0,0 -V DC_GAME -G IP.BIN -r -J -l -o ../$PROJECT_NAME.iso $DIR

#transform iso into a CDI
#$KOS_BASE/utils/cdi4dc/cdi4dc ../$PROJECT_NAME.iso ../$PROJECT_NAME.cdi -d > cdi4dc.log


#start the game
sudo arp -s 192.168.0.99 00:d0:f1:03:14:02
sudo ../dc-tool-ip -t dreamcast -c ./ -x dreamroq-player.elf

#lxdream ../$PROJECT_NAME.iso
#reicast ../$PROJECT_NAME.cdi
#redream ../$PROJECT_NAME.cdi
##../redream ../$PROJECT_NAME.cdi
