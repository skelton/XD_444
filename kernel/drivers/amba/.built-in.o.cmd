cmd_drivers/amba/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o drivers/amba/built-in.o drivers/amba/bus.o ; scripts/mod/modpost drivers/amba/built-in.o
