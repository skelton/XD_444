cmd_net/rfkill/rfkill.o := ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o net/rfkill/rfkill.o net/rfkill/core.o ; scripts/mod/modpost net/rfkill/rfkill.o
