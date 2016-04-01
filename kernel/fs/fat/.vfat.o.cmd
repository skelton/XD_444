cmd_fs/fat/vfat.o := ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/fat/vfat.o fs/fat/namei_vfat.o ; scripts/mod/modpost fs/fat/vfat.o
