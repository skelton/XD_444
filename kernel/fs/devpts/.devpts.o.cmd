cmd_fs/devpts/devpts.o := ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/devpts/devpts.o fs/devpts/inode.o ; scripts/mod/modpost fs/devpts/devpts.o
