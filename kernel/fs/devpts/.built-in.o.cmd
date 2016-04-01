cmd_fs/devpts/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/devpts/built-in.o fs/devpts/devpts.o ; scripts/mod/modpost fs/devpts/built-in.o
