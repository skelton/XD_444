cmd_fs/ramfs/ramfs.o := ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/ramfs/ramfs.o fs/ramfs/inode.o fs/ramfs/file-mmu.o ; scripts/mod/modpost fs/ramfs/ramfs.o
