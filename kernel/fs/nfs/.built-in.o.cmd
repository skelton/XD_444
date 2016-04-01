cmd_fs/nfs/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/nfs/built-in.o fs/nfs/nfs.o fs/nfs/nfsv2.o fs/nfs/nfsv3.o ; scripts/mod/modpost fs/nfs/built-in.o
