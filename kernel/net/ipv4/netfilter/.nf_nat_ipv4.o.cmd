cmd_net/ipv4/netfilter/nf_nat_ipv4.o := ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o net/ipv4/netfilter/nf_nat_ipv4.o net/ipv4/netfilter/nf_nat_l3proto_ipv4.o net/ipv4/netfilter/nf_nat_proto_icmp.o ; scripts/mod/modpost net/ipv4/netfilter/nf_nat_ipv4.o
