cmd_net/ipv6/netfilter/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o net/ipv6/netfilter/built-in.o net/ipv6/netfilter/ip6_tables.o net/ipv6/netfilter/ip6table_filter.o net/ipv6/netfilter/ip6table_mangle.o net/ipv6/netfilter/ip6table_raw.o net/ipv6/netfilter/nf_conntrack_ipv6.o net/ipv6/netfilter/nf_defrag_ipv6.o net/ipv6/netfilter/ip6t_REJECT.o ; scripts/mod/modpost net/ipv6/netfilter/built-in.o
