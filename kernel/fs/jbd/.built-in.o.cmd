cmd_fs/jbd/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/jbd/built-in.o fs/jbd/jbd.o ; scripts/mod/modpost fs/jbd/built-in.o
