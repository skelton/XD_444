cmd_drivers/misc/inv_mpu/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o drivers/misc/inv_mpu/built-in.o drivers/misc/inv_mpu/mpu6050.o drivers/misc/inv_mpu/timerirq.o drivers/misc/inv_mpu/accel/built-in.o drivers/misc/inv_mpu/compass/built-in.o drivers/misc/inv_mpu/pressure/built-in.o ; scripts/mod/modpost drivers/misc/inv_mpu/built-in.o
