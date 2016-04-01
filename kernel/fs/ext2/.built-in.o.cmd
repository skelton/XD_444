cmd_fs/ext2/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/ext2/built-in.o fs/ext2/ext2.o ; scripts/mod/modpost fs/ext2/built-in.o
