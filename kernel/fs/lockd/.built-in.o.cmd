cmd_fs/lockd/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/lockd/built-in.o fs/lockd/lockd.o ; scripts/mod/modpost fs/lockd/built-in.o
