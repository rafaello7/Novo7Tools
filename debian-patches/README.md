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
Drag using two fingers simulates scrolling with wheel mouse button.

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

