cmd_crypto/crypto.o := ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o crypto/crypto.o crypto/api.o crypto/cipher.o crypto/compress.o ; scripts/mod/modpost crypto/crypto.o
