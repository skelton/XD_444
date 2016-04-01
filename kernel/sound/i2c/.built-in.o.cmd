cmd_sound/i2c/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o sound/i2c/built-in.o sound/i2c/other/built-in.o ; scripts/mod/modpost sound/i2c/built-in.o
