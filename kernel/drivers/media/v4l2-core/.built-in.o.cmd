cmd_drivers/media/v4l2-core/built-in.o :=  ../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-ld -EL    -r -o drivers/media/v4l2-core/built-in.o drivers/media/v4l2-core/videodev.o drivers/media/v4l2-core/v4l2-common.o drivers/media/v4l2-core/videobuf-core.o drivers/media/v4l2-core/videobuf-dma-contig.o drivers/media/v4l2-core/videobuf2-core.o drivers/media/v4l2-core/videobuf2-memops.o drivers/media/v4l2-core/videobuf2-vmalloc.o ; scripts/mod/modpost drivers/media/v4l2-core/built-in.o
