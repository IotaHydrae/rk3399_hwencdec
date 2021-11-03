
# local kernel build dir
KERN_DIR:=/lib/modules/$(shell uname -r)/build

# users kernel dir
# KERN_DIR:=/home/user/linux

MOD_NAME:=test_kmalloc_memcpy

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean

insmod:
	sudo insmod $(MOD_NAME).ko
	sh ./klogcat.sh

rmmod:
	sudo rmmod $(MOD_NAME).ko
	sh ./klogcat.sh

log:
	sh ./klogcat.sh

# obj-m+=eeprom.o
# obj-m+=test_eep.o

obj-m+=$(MOD_NAME).o
