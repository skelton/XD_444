cmd_fs/cifs/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/cifs/built-in.o fs/cifs/cifs.o ; scripts/mod/modpost fs/cifs/built-in.o
