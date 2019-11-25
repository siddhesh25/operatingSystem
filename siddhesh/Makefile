#obj-m := lwnfs.o
obj-m := s2fs.o

#CFLAGS_lwnfs.o := -DDEBUG
CFLAGS_s2fs.o := -DDEBUG

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
