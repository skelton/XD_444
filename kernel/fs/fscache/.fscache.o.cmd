cmd_fs/fscache/fscache.o := ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/fscache/fscache.o fs/fscache/cache.o fs/fscache/cookie.o fs/fscache/fsdef.o fs/fscache/main.o fs/fscache/netfs.o fs/fscache/object.o fs/fscache/operation.o fs/fscache/page.o fs/fscache/proc.o ; scripts/mod/modpost fs/fscache/fscache.o
