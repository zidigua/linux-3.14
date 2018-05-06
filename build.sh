#!/bin/bash

# make exynos_defconfig

make uImage
make dtbs
cp `pwd`/arch/arm/boot/dts/exynos4412-origen.dtb /tftpboot -f
cp `pwd`/arch/arm/boot/uImage /tftpboot/ -f



