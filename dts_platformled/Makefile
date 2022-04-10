# KERNELDIR := /home/ye/individual/linux_imx6u/linux/kernel/NXP_EVK_linux/linux-imx-rel_imx_4.1.15_2.1.0_ga_alientek
KERNELDIR := /home/ye/individual/linux_imx6u/linux/kernel/alientek_linux

CURRENT_PATH := $(shell pwd)
obj-m := dts_platformledDev.o

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean



