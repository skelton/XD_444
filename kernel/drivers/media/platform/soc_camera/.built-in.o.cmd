cmd_drivers/media/platform/soc_camera/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o drivers/media/platform/soc_camera/built-in.o drivers/media/platform/soc_camera/soc_camera.o drivers/media/platform/soc_camera/soc_mediabus.o ; scripts/mod/modpost drivers/media/platform/soc_camera/built-in.o
