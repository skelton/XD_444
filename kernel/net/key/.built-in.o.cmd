cmd_net/key/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o net/key/built-in.o net/key/af_key.o ; scripts/mod/modpost net/key/built-in.o
