sunxi_suspend: sunxi_suspend.c
	gcc -Wall sunxi_suspend.c -o sunxi_suspend

install:
	install sunxi_suspend /usr/sbin/
	install -m 644 pm-sunxi /etc/pm/config.d/
	install -m 644 systemd-suspend.service /etc/systemd/system/

clean:
	rm -f sunxi_suspend
