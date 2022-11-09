obj-m += drukhary_km.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	sudo insmod ./drukhary_km.ko
	sudo mknod drukhary_character_device c 100 0
	gcc ioctl.c -o drukhary_app
	gcc testee.c -o testee
	sudo ./drukhary_app
module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f drukhary_app
	rm -f drukhary_character_device
	sudo rmmod drukhary_km