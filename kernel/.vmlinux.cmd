cmd_vmlinux := /bin/bash scripts/link-vmlinux.sh ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL  -p --no-undefined -X --build-id --just-symbols=pie/pie.syms
