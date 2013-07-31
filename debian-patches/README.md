A collection of Debian patches, for touchscreen, battery etc.
=============================================================

patch-upower
------------

Patch for "upower" package. Causes to display correctly battery
charge rate on xfce desktop (and possibly on other desktops).

ft5x
----

Touchscreen support.
Tap with 2nd finger simulates right-click.
Drag with 2nd finger simulates scrolling with mouse wheel.

ft5x-2
------

Another way of touchscreen support. See [ft5x-2 README](ft5x-2).


fbexec
------

Executes a program, output prints on framebuffer (fb0).
Useful to display some logs at boot time.

fbecho
------

Like "echo", output displayed on framebuffer (fb0)

disp
----

A tool to manipulate display output: turn on/off hdmi, lcd,
set resolution.

adbd\_emu
--------

Provides a very limited functionality of _adbd_ Android daemon.
Allows to run shell using "adb shell". All other _adb_ commands are refused.

A more versatile solution is to turn on _rndis_ in Linux kernel and set
network via USB.

pm
--

Corrections in "suspend" functionality of pm-utils package.

patch-rtl8192cu
---------------

A patch for [rtl8192cu driver from realtek](http://www.realtek.com/downloads/downloadsView.aspx?Langid=1&PNid=48&PFid=48&Level=5&Conn=4&DownTypeID=3&GetDown=false&Downloads=true#2772),
useful when replacing driver incorporated into kernel on [linux-sunxi](https://github.com/linux-sunxi/linux-sunxi/tree/sunxi-3.0/drivers/net/wireless/rtl8192cu).


sunxi-disp-brightness
---------------------

A tool to set LCD display brightness on sunxi platform.

