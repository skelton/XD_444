cmd_crypto/cryptomgr.o := ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o crypto/cryptomgr.o crypto/algboss.o crypto/testmgr.o ; scripts/mod/modpost crypto/cryptomgr.o
