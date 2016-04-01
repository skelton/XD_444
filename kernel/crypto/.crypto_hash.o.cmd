cmd_crypto/crypto_hash.o := ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o crypto/crypto_hash.o crypto/ahash.o crypto/shash.o ; scripts/mod/modpost crypto/crypto_hash.o
