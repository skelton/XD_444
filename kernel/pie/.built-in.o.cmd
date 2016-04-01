cmd_pie/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL   --no-undefined -X  -r -o pie/built-in.o pie/pie.bin.o ; scripts/mod/modpost pie/built-in.o
