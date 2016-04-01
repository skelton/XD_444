cmd_pie/pie_stage1.o := ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-objcopy  --weaken-symbols=pie/pie_weaken.syms --redefine-syms=pie/pie_rename.syms pie/../vmlinux.o pie/pie_stage1.o
