cmd_arch/arm/boot/dts/rk3288-tb.dtb := ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-gcc -E -Wp,-MD,arch/arm/boot/dts/.rk3288-tb.dtb.d.pre.tmp -nostdinc -I/home/skelton/xd/XD-EN-V335/kernel/arch/arm/boot/dts -I/home/skelton/xd/XD-EN-V335/kernel/arch/arm/boot/dts/include -I/home/skelton/xd/XD-EN-V335/kernel/include -undef -D__DTS__ -x assembler-with-cpp -o arch/arm/boot/dts/.rk3288-tb.dtb.dts arch/arm/boot/dts/rk3288-tb.dts ; /home/skelton/xd/XD-EN-V335/kernel/scripts/dtc/dtc -O dtb -o arch/arm/boot/dts/rk3288-tb.dtb -b 0 -i arch/arm/boot/dts/  -d arch/arm/boot/dts/.rk3288-tb.dtb.d.dtc.tmp arch/arm/boot/dts/.rk3288-tb.dtb.dts ; cat arch/arm/boot/dts/.rk3288-tb.dtb.d.pre.tmp arch/arm/boot/dts/.rk3288-tb.dtb.d.dtc.tmp > arch/arm/boot/dts/.rk3288-tb.dtb.d

source_arch/arm/boot/dts/rk3288-tb.dtb := arch/arm/boot/dts/rk3288-tb.dts

deps_arch/arm/boot/dts/rk3288-tb.dtb := \
  arch/arm/boot/dts/rk3288.dtsi \
  /home/skelton/xd/XD-EN-V335/kernel/arch/arm/boot/dts/include/dt-bindings/clock/rk_system_status.h \
  /home/skelton/xd/XD-EN-V335/kernel/arch/arm/boot/dts/include/dt-bindings/interrupt-controller/arm-gic.h \
  /home/skelton/xd/XD-EN-V335/kernel/arch/arm/boot/dts/include/dt-bindings/interrupt-controller/irq.h \
  /home/skelton/xd/XD-EN-V335/kernel/arch/arm/boot/dts/include/dt-bindings/rkfb/rk_fb.h \
  /home/skelton/xd/XD-EN-V335/kernel/arch/arm/boot/dts/include/dt-bindings/rkmipi/mipi_dsi.h \
    $(wildcard include/config/mipi/dsi/ft.h) \
  /home/skelton/xd/XD-EN-V335/kernel/arch/arm/boot/dts/include/dt-bindings/suspend/rockchip-pm.h \
  /home/skelton/xd/XD-EN-V335/kernel/arch/arm/boot/dts/include/dt-bindings/sensor-dev.h \
  arch/arm/boot/dts/skeleton.dtsi \
  arch/arm/boot/dts/rk3288-pinctrl.dtsi \
  /home/skelton/xd/XD-EN-V335/kernel/arch/arm/boot/dts/include/dt-bindings/gpio/gpio.h \
  /home/skelton/xd/XD-EN-V335/kernel/arch/arm/boot/dts/include/dt-bindings/pinctrl/rockchip.h \
    $(wildcard include/config/to/pull.h) \
    $(wildcard include/config/to/vol.h) \
    $(wildcard include/config/to/drv.h) \
    $(wildcard include/config/to/tri.h) \
  /home/skelton/xd/XD-EN-V335/kernel/arch/arm/boot/dts/include/dt-bindings/pinctrl/rockchip-rk3288.h \
  arch/arm/boot/dts/rk3288-clocks.dtsi \
  /home/skelton/xd/XD-EN-V335/kernel/arch/arm/boot/dts/include/dt-bindings/clock/rockchip,rk3288.h \
  /home/skelton/xd/XD-EN-V335/kernel/arch/arm/boot/dts/include/dt-bindings/clock/rockchip.h \
  arch/arm/boot/dts/lcd-wqxga-mipi.dtsi \
  arch/arm/boot/dts/act8846.dtsi \
  arch/arm/boot/dts/rk808.dtsi \
  arch/arm/boot/dts/ricoh619.dtsi \
  arch/arm/boot/dts/rk818.dtsi \

arch/arm/boot/dts/rk3288-tb.dtb: $(deps_arch/arm/boot/dts/rk3288-tb.dtb)

$(deps_arch/arm/boot/dts/rk3288-tb.dtb):
