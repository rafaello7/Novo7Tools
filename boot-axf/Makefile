CROSS_COMPILLE = arm-none-linux-gnueabi-

CFLAGS = -Os -fno-common -ffreestanding -nostdinc -marm -mabi=aapcs-linux -march=armv7-a -Wall -Wstrict-prototypes -fno-stack-protector
GCCINC =  -I`$(CROSS_COMPILLE)gcc -print-file-name=include`

OBJS = egon2_magic.o bootmain.o wlibc.o syscalls.o display_interface.o ui.o picture.o script.o bootos.o power.o usb_device.o board.o eabi_compat.o string.o


# gcc sets .rodata section flags to "A", but ELF loader included in boot1 does
# not load section having "A" flag, hence objcopy call which changes the flags
# to "WA"
boot-axf: $(OBJS)
	$(CROSS_COMPILLE)ld $(OBJS) -o boot-axf -T boot-axf.lds
	$(CROSS_COMPILLE)objcopy --set-section-flags .rodata=alloc boot-axf

$(OBJS): board.h bootos.h display_interface.h io.h picture.h power.h script.h syscalls.h ui.h usb_device.h wlibc.h bootmain.h string.h

.S.o:
	$(CROSS_COMPILLE)gcc $(CFLAGS) -c $<

.c.o:
	$(CROSS_COMPILLE)gcc $(CFLAGS) $(GCCINC) -c $<

clean:
	rm -f $(OBJS) boot-axf

sc:
	make clean
	make
