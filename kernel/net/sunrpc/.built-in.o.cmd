cmd_net/sunrpc/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o net/sunrpc/built-in.o net/sunrpc/sunrpc.o ; scripts/mod/modpost net/sunrpc/built-in.o
