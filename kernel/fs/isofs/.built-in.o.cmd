cmd_fs/isofs/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/isofs/built-in.o fs/isofs/isofs.o ; scripts/mod/modpost fs/isofs/built-in.o
