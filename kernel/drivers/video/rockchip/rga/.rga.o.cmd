cmd_drivers/video/rockchip/rga/rga.o := ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o drivers/video/rockchip/rga/rga.o drivers/video/rockchip/rga/rga_drv.o drivers/video/rockchip/rga/rga_mmu_info.o drivers/video/rockchip/rga/rga_reg_info.o drivers/video/rockchip/rga/RGA_API.o ; scripts/mod/modpost drivers/video/rockchip/rga/rga.o
