cmd_drivers/scsi/sd_mod.o := ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o drivers/scsi/sd_mod.o drivers/scsi/sd.o ; scripts/mod/modpost drivers/scsi/sd_mod.o
