OBJS = usbtest.o

usbtest: $(OBJS)
	gcc $(OBJS) $$(pkg-config --libs libusb-1.0) -o usbtest

.c.o:
	gcc -g -c $$(pkg-config --cflags libusb-1.0) $<


clean:
	rm -f $(OBJS) usbtest
