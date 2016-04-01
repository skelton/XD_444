cmd_fs/udf/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/udf/built-in.o fs/udf/udf.o ; scripts/mod/modpost fs/udf/built-in.o
