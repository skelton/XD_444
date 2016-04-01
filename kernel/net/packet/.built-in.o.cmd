cmd_net/packet/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o net/packet/built-in.o net/packet/af_packet.o ; scripts/mod/modpost net/packet/built-in.o
