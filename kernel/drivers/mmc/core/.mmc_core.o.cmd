cmd_drivers/mmc/core/mmc_core.o := ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o drivers/mmc/core/mmc_core.o drivers/mmc/core/core.o drivers/mmc/core/bus.o drivers/mmc/core/host.o drivers/mmc/core/mmc.o drivers/mmc/core/mmc_ops.o drivers/mmc/core/sd.o drivers/mmc/core/sd_ops.o drivers/mmc/core/sdio.o drivers/mmc/core/sdio_ops.o drivers/mmc/core/sdio_bus.o drivers/mmc/core/sdio_cis.o drivers/mmc/core/sdio_io.o drivers/mmc/core/sdio_irq.o drivers/mmc/core/quirks.o drivers/mmc/core/slot-gpio.o drivers/mmc/core/debugfs.o ; scripts/mod/modpost drivers/mmc/core/mmc_core.o
