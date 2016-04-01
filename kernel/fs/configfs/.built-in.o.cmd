cmd_fs/configfs/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/configfs/built-in.o fs/configfs/configfs.o ; scripts/mod/modpost fs/configfs/built-in.o
