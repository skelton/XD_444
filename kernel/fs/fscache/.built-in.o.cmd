cmd_fs/fscache/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/fscache/built-in.o fs/fscache/fscache.o ; scripts/mod/modpost fs/fscache/built-in.o
