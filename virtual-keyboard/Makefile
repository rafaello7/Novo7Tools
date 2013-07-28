OBJS = keyboard_layout.o cmdline.o main.o

virtual-keyboard: $(OBJS)
	gcc $$(pkg-config --libs gtk+-3.0) -lXtst -o virtual-keyboard $(OBJS)

.c.o:
	gcc -Wall $$(pkg-config --cflags gtk+-3.0) -c $<

clean:
	rm -f $(OBJS) virtual-keyboard

install:
	install virtual-keyboard /usr/local/bin

