cmd_fs/ceph/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o fs/ceph/built-in.o fs/ceph/ceph.o ; scripts/mod/modpost fs/ceph/built-in.o
