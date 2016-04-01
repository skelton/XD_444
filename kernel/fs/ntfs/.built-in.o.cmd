cmd_fs/ntfs/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/ntfs/built-in.o fs/ntfs/ntfs.o ; scripts/mod/modpost fs/ntfs/built-in.o
