cmd_arch/arm/boot/compressed/piggy.lzo.o := ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-gcc -Wp,-MD,arch/arm/boot/compressed/.piggy.lzo.o.d  -nostdinc -isystem /home/skelton/xd/XD-EN-V335/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/../lib/gcc/arm-eabi/4.6.x-google/include -I/home/skelton/xd/XD-EN-V335/kernel/arch/arm/include -Iarch/arm/include/generated  -Iinclude -I/home/skelton/xd/XD-EN-V335/kernel/arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I/home/skelton/xd/XD-EN-V335/kernel/include/uapi -Iinclude/generated/uapi -include /home/skelton/xd/XD-EN-V335/kernel/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian  -D__ASSEMBLY__ -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -marm -D__LINUX_ARM_ARCH__=7 -march=armv7-a  -include asm/unified.h -msoft-float     -DZIMAGE     -c -o arch/arm/boot/compressed/piggy.lzo.o arch/arm/boot/compressed/piggy.lzo.S

source_arch/arm/boot/compressed/piggy.lzo.o := arch/arm/boot/compressed/piggy.lzo.S

deps_arch/arm/boot/compressed/piggy.lzo.o := \
  /home/skelton/xd/XD-EN-V335/kernel/arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
    $(wildcard include/config/thumb2/kernel.h) \

arch/arm/boot/compressed/piggy.lzo.o: $(deps_arch/arm/boot/compressed/piggy.lzo.o)

$(deps_arch/arm/boot/compressed/piggy.lzo.o):
