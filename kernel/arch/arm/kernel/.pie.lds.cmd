cmd_arch/arm/kernel/pie.lds := ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-gcc -E -Wp,-MD,arch/arm/kernel/.pie.lds.d  -nostdinc -isystem /home/skelton/xd/XD-EN-V335/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/../lib/gcc/arm-eabi/4.6.x-google/include -I/home/skelton/xd/XD-EN-V335/kernel/arch/arm/include -Iarch/arm/include/generated  -Iinclude -I/home/skelton/xd/XD-EN-V335/kernel/arch/arm/include/uapi -Iarch/arm/include/generated/uapi -I/home/skelton/xd/XD-EN-V335/kernel/include/uapi -Iinclude/generated/uapi -include /home/skelton/xd/XD-EN-V335/kernel/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian     -P -C -Uarm -D__ASSEMBLY__ -DLINKER_SCRIPT -o arch/arm/kernel/pie.lds arch/arm/kernel/pie.lds.S

source_arch/arm/kernel/pie.lds := arch/arm/kernel/pie.lds.S

deps_arch/arm/kernel/pie.lds := \
  include/asm-generic/pie.lds.h \
  include/asm-generic/vmlinux.lds.h \
    $(wildcard include/config/hotplug.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/event/tracing.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/ftrace/syscalls.h) \
    $(wildcard include/config/clksrc/of.h) \
    $(wildcard include/config/irqchip.h) \
    $(wildcard include/config/common/clk.h) \
    $(wildcard include/config/of/reserved/mem.h) \
    $(wildcard include/config/function/graph/tracer.h) \
    $(wildcard include/config/constructors.h) \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/pm/trace.h) \
    $(wildcard include/config/blk/dev/initrd.h) \
  include/linux/export.h \
    $(wildcard include/config/have/underscore/symbol/prefix.h) \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \

arch/arm/kernel/pie.lds: $(deps_arch/arm/kernel/pie.lds)

$(deps_arch/arm/kernel/pie.lds):
