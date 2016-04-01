cmd_net/ceph/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o net/ceph/built-in.o net/ceph/libceph.o ; scripts/mod/modpost net/ceph/built-in.o
