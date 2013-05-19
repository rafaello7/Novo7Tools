include config.mk

OBJS_DISKBOOT = start.o main.o fastboot.o string.o usbc.o usbc_dev.o sunxi_dma.o malloc.o vsprintf.o eabi_compat.o board.o nandinit.o
OBJS_DRAMINIT = draminit.o

GCCINC =  -I$$($(CROSS_COMPILE)gcc -print-file-name=include)
GCCLIB = $$($(CROSS_COMPILE)gcc -print-libgcc-file-name)

CFLAGS = $(CFLAGS_CMN) -Iinclude $(GCCINC)

all:: diskboot.bin draminit.bin

diskboot.bin: diskboot
	$(CROSS_COMPILE)objcopy --gap-fill=0xff -O binary diskboot diskboot.bin
	ls -l diskboot.bin

diskboot: $(OBJS_DISKBOOT) nand_sunxi/libsunxi-nand.a diskboot.lds
	$(CROSS_COMPILE)ld -T diskboot.lds -Bstatic -Ttext 0x4A000000 $(OBJS_DISKBOOT) nand_sunxi/libsunxi-nand.a $(GCCLIB) -o diskboot
#	$(CROSS_COMPILE)readelf -S diskboot

nand_sunxi/libsunxi-nand.a::
	cd nand_sunxi; make

draminit.bin: draminit
	$(CROSS_COMPILE)objcopy --gap-fill=0xff -O binary draminit draminit.bin

draminit: $(OBJS_DRAMINIT) Makefile
	$(CROSS_COMPILE)ld -T draminit.lds $(OBJS_DRAMINIT) -o draminit

.c.o:
	$(CROSS_COMPILE)gcc  $(CFLAGS) -c $<

.S.o:
	$(CROSS_COMPILE)gcc $(CFLAGS) -c $<


clean:
	cd nand_sunxi; make clean
	rm -f $(OBJS_DISKBOOT) diskboot diskboot.bin draminit draminit.bin $(OBJS_DRAMINIT)

sc:
	make clean
	make
