OBJS = start.o main.o fastboot.o string.o usbc.o usbc_dev.o

CROSS_COMPILE = arm-none-linux-gnueabi-
#GCCINC =  $$($(CROSS_COMPILE)gcc -print-file-name=include)
#GCCLIB = $$($(CROSS_COMPILE)gcc -print-libgcc-file-name)

CFLAGS = -Os -fno-common -Iinclude -ffreestanding -nostdinc -marm -mabi=aapcs-linux -march=armv6

u-boot.bin: u-boot
	$(CROSS_COMPILE)objcopy --gap-fill=0xff -O binary u-boot u-boot.bin
	ls -l u-boot.bin

u-boot: $(OBJS) u-boot.lds
	$(CROSS_COMPILE)ld -T u-boot.lds -Bstatic -Ttext 0x4A000000 $(OBJS)  -o u-boot
#	$(CROSS_COMPILE)readelf -S u-boot

.c.o:
	$(CROSS_COMPILE)gcc  $(CFLAGS) -Wall -Wstrict-prototypes -fno-stack-protector -Wno-format-nonliteral -Wno-format-security -c $<

.S.o:
	$(CROSS_COMPILE)gcc $(CFLAGS) -c $<

#$(OBJS): include/cpu.h include/io.h include/string.h include/types.h include/usbc_i.h include/fastboot.h include/usbdevice.h include/usb_bsp.h include/sw_usb_typedef.h include/usbdescriptors.h 

clean:
	cd cli; make clean
	rm -f $(OBJS) u-boot u-boot.bin

sc:
	make clean
	make
