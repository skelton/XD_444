cmd_kernel/sched/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o kernel/sched/built-in.o kernel/sched/core.o kernel/sched/clock.o kernel/sched/cputime.o kernel/sched/idle_task.o kernel/sched/fair.o kernel/sched/rt.o kernel/sched/stop_task.o kernel/sched/cpupri.o kernel/sched/stats.o kernel/sched/debug.o kernel/sched/cpuacct.o ; scripts/mod/modpost kernel/sched/built-in.o
