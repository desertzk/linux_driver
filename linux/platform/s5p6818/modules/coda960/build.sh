#!/bin/sh

export CROSS_COMPILE=/home/zr/kernel/6818/s5p6818/x6818_lollipop/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin/arm-eabi-

make ARCH=arm clean
make ARCH=arm -j4
cp nx_vpu.ko ../../../../../hardware/samsung_slsi/slsiap/prebuilt/modules/
