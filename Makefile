obj-m += lab_km.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	sudo insmod ./lab_km.ko
	sudo mknod lab_character_device c 100 0
	gcc user_app.c -o user_app
	gcc testee.c -o testee
	sudo ./user_app
module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f user_app
	rm -f lab_character_device
	sudo rmmod lab_km