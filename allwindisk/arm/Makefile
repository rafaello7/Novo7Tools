OBJS = start.o main.o fastboot.o string.o usbc.o usbc_dev.o sunxi_dma.o malloc.o vsprintf.o eabi_compat.o board.o

CROSS_COMPILE = arm-none-linux-gnueabi-
GCCINC =  -I$$($(CROSS_COMPILE)gcc -print-file-name=include)
GCCLIB = $$($(CROSS_COMPILE)gcc -print-libgcc-file-name)

CFLAGS = -Os -fno-common -Iinclude -ffreestanding -nostdinc -marm -mabi=aapcs-linux -march=armv6 $(GCCINC)

diskboot.bin: diskboot
	$(CROSS_COMPILE)objcopy --gap-fill=0xff -O binary diskboot diskboot.bin
	ls -l diskboot.bin

diskboot: $(OBJS) nand_sunxi/libsunxi-nand.a diskboot.lds
	$(CROSS_COMPILE)ld -T diskboot.lds -Bstatic -Ttext 0x4A000000 $(OBJS) nand_sunxi/libsunxi-nand.a $(GCCLIB) -o diskboot
#	$(CROSS_COMPILE)readelf -S diskboot

nand_sunxi/libsunxi-nand.a::
	cd nand_sunxi; make

.c.o:
	$(CROSS_COMPILE)gcc  $(CFLAGS) -Wall -Wstrict-prototypes -fno-stack-protector -Wno-format-nonliteral -Wno-format-security -c $<

.S.o:
	$(CROSS_COMPILE)gcc $(CFLAGS) -c $<


clean:
	cd nand_sunxi; make clean
	rm -f $(OBJS) diskboot diskboot.bin

sc:
	make clean
	make
