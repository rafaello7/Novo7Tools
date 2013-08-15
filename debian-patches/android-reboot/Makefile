OBJS = android-reboot.o hddown.o ifdown.o


android-reboot: $(OBJS)
	gcc $(OBJS) -o android-reboot


.c.o:
	gcc -Wall -c $<

clean:
	rm -f $(OBJS) android-reboot

install:
	install android-reboot /sbin
	install android-reboot.init.d /etc/init.d/android-reboot
	update-rc.d android-reboot defaults

uninstall:
	rm -f /sbin/android-reboot
	rm -f /etc/init.d/android-reboot
	update-rc.d android-reboot remove

