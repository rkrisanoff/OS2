
MODULE_NAME = lab_kernel_module
KERNELDIR ?= "/lib/modules/$(shell uname -r)/build"

SRCS = lab_km.c	lkmfs.c

OBJS = $(SRCS:.c=.o)

obj-m += $(MODULE_NAME).o
$(MODULE_NAME)-objs := lab_km.o lkmfs.o
obj-m += lab_kernel_module.o
lab_kernel_module-objs := lab_km.o lkmfs.o
$(MODULE_NAME)-y = $(OBJS)

CFLAGS=-std=gnu99 -Wno-declaration-after-statement
ccflags-y=-std=gnu99 -Wno-declaration-after-statement
USER_APP=user_app
LAB_CHARACTER_DEVICE=lab_character_device

all:
	make -C $(KERNELDIR) M=$(PWD) modules
	sudo insmod ./$(MODULE_NAME).ko
	sudo mknod $(LAB_CHARACTER_DEVICE) c 100 0
	gcc $(USER_APP).c -o $(USER_APP) -std=c99
	sudo ./$(USER_APP)
module:
	make -C $(KERNELDIR) M=$(PWD) modules
install:
	sudo insmod ./$(MODULE_NAME).ko
clean:
	make -C $(KERNELDIR) M=$(PWD) clean
remove:
	rm -f $(USER_APP)
	rm -f $(LAB_CHARACTER_DEVICE)
	sudo rmmod $(MODULE_NAME)