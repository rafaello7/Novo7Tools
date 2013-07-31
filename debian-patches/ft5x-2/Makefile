ft5x_drv.so: ft5x.o
	gcc -shared  ft5x.o  $$(pkg-config --libs xorg-server) -o ft5x_drv.so

ft5x.o: ft5x.c
	gcc -g -O2 $$(pkg-config --cflags xorg-server) -c ft5x.c  -fPIC -DPIC -o ft5x.o

install:
	install -m 644 ft5x_drv.so /usr/lib/xorg/modules/input/
	install -D -m 644 ft5x-xorg.conf /etc/X11/xorg.conf.d/50-ft5x-xorg.conf

clean:
	rm -f ft5x.o ft5x_drv.so
