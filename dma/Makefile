obj-m := dma.o
KER := $(shell uname -r)
KER_DIR := /lib/modules/$(KER)/build

all:
	make -C ${KER_DIR} M=$(PWD) modules

clean:
	make -C ${KER_DIR} M=$(PWD) clean
