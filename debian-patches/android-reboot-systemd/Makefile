OBJS = android-reboot.o hddown.o ifdown.o


android-reboot: $(OBJS)
	gcc $(OBJS) -o android-reboot


.c.o:
	gcc -Wall -c $<

clean:
	rm -f $(OBJS) android-reboot

install:
	install android-reboot /sbin
	install -m 644 android-reboot.service /etc/systemd/system
	systemctl enable android-reboot

uninstall:
	rm -f /sbin/android-reboot
	systemctl disable android-reboot
	rm -f /etc/systemd/system/android-reboot.service

