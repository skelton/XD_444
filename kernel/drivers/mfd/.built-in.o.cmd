cmd_drivers/mfd/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o drivers/mfd/built-in.o drivers/mfd/rt5025-i2c.o drivers/mfd/rt5025-core.o drivers/mfd/rt5025-misc.o drivers/mfd/rt5025-irq.o drivers/mfd/rt5025-debug.o drivers/mfd/rt5036-i2c.o drivers/mfd/rt5036-core.o drivers/mfd/rt5036-misc.o drivers/mfd/rt5036-irq.o drivers/mfd/mfd-core.o drivers/mfd/rk808.o drivers/mfd/rk808-irq.o drivers/mfd/rk818.o drivers/mfd/rk818-irq.o drivers/mfd/ricoh619.o drivers/mfd/ricoh619-irq.o ; scripts/mod/modpost drivers/mfd/built-in.o
