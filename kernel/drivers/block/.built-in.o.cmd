cmd_drivers/block/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o drivers/block/built-in.o drivers/block/loop.o ; scripts/mod/modpost drivers/block/built-in.o
