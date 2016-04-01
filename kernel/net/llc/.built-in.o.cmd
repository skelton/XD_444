cmd_net/llc/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o net/llc/built-in.o net/llc/llc.o ; scripts/mod/modpost net/llc/built-in.o
