cmd_fs/nfs/nfsv2.o := ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/nfs/nfsv2.o fs/nfs/nfs2super.o fs/nfs/proc.o fs/nfs/nfs2xdr.o ; scripts/mod/modpost fs/nfs/nfsv2.o
