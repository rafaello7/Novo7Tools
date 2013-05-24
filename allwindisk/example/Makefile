CROSS_COMPILE = arm-none-linux-gnueabi-
CFLAGS = -Os -fno-common -ffreestanding -nostdinc -marm -mabi=aapcs-linux -march=armv7-a -Wall -Wstrict-prototypes -fno-stack-protector 


all:: hello.bin asmhello.bin


hello.bin: hello
	$(CROSS_COMPILE)objcopy --gap-fill=0xff -O binary hello hello.bin

hello: hello.o
	$(CROSS_COMPILE)ld -T hello.lds hello.o -o hello

asmhello.bin: asmhello
	$(CROSS_COMPILE)objcopy --gap-fill=0xff -O binary asmhello asmhello.bin

asmhello: asmhello.o
	$(CROSS_COMPILE)ld -Ttext 0x0 asmhello.o -o asmhello

.c.o:
	$(CROSS_COMPILE)gcc $(CFLAGS) -c $<

.S.o:
	$(CROSS_COMPILE)gcc $(CFLAGS) -c $<


clean:
	rm -f asmhello asmhello.bin asmhello.o hello hello.bin hello.o
