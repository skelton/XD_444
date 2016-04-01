cmd_net/ethernet/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o net/ethernet/built-in.o net/ethernet/eth.o ; scripts/mod/modpost net/ethernet/built-in.o
