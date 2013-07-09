#CROSS_COMPILE = arm-none-linux-gnueabi-
fbecho: fbecho.o
	$(CROSS_COMPILE)gcc fbecho.o -o fbecho

.c.o:
	$(CROSS_COMPILE)gcc -g -c fbecho.c

clean:
	rm -f fbecho.o fbecho
