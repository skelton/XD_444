cmd_drivers/ptp/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o drivers/ptp/built-in.o drivers/ptp/ptp.o ; scripts/mod/modpost drivers/ptp/built-in.o
