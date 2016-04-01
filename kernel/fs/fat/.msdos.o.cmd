cmd_fs/fat/msdos.o := ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/fat/msdos.o fs/fat/namei_msdos.o ; scripts/mod/modpost fs/fat/msdos.o
