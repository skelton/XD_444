cmd_drivers/i2c/busses/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o drivers/i2c/busses/built-in.o drivers/i2c/busses/i2c-rockchip.o ; scripts/mod/modpost drivers/i2c/busses/built-in.o
