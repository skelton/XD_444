cmd_fs/debugfs/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/debugfs/built-in.o fs/debugfs/debugfs.o ; scripts/mod/modpost fs/debugfs/built-in.o
