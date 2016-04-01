cmd_fs/fuse/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/fuse/built-in.o fs/fuse/fuse.o ; scripts/mod/modpost fs/fuse/built-in.o
