cmd_drivers/staging/android/ion/rockchip/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o drivers/staging/android/ion/rockchip/built-in.o drivers/staging/android/ion/rockchip/rockchip_ion.o drivers/staging/android/ion/rockchip/rockchip_ion_snapshot.o ; scripts/mod/modpost drivers/staging/android/ion/rockchip/built-in.o
