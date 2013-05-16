OBJS = allwindisk.o fel.o

all: allwindisk
	cd arm; make

allwindisk: $(OBJS)
	gcc $(OBJS) $$(pkg-config --libs libusb-1.0) -o allwindisk

.c.o:
	gcc -g -c -Wall $$(pkg-config --cflags libusb-1.0) $<


clean:
	cd arm; make clean
	rm -f $(OBJS) allwindisk

sc:
	make clean
	make
