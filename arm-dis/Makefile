
OBJS = arm-dis.o floatformat.o

arm-dis: $(OBJS)
	gcc $(OBJS) -o arm-dis

.c.o:
	gcc -g -c -Wall $<

clean:
	rm -f $(OBJS) arm-dis

arm-dis.o: dis-asm.h
