cmd_init/mounts.o := ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o init/mounts.o init/do_mounts.o init/do_mounts_initrd.o ; scripts/mod/modpost init/mounts.o
