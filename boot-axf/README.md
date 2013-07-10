boot-axf
========

The _boot.axf_ binary is located in first partition (_nanda_) of Ainol flash
disk. The _nanda_ partition contains FAT filesystem. The binary is loaded
from _nanda_ partition by _boot1_ loader.

The binary is in ELF format, but it is a special binary, incompatible
with e.g. Linux ELF binaries. It must contain special section named
_EGON2\_MAGIC_ and does not support shared libraries.  The boot1 serves as a
very simple operating system for the binary. It allows to display images on
LCD, manipulate files in the _nanda_ partition, read/write other partitions of
NAND flash, and a few more things. But, unlike Linux programs, the binary
runs in kernel mode.

This version of _boot.axf_ has some improvements in comparison to original
one from Ainol.

Original _boot.axf_ binary discovers first the wakeup reason. If the tablet
was waken up because e.g. power supply was plugged in, the program displays
battery charge state and then powers off back the device. If the device was
waken up because power button was pressed, the program reads _boot.ini_ file,
then _linux\\linux.ini_, then loads and jumps to image specified in the
_linux.ini_ file.

It seems the Ainol binary was stripped down from some features of generic
version delivered by Allwinner, like possibility to choose operating system to
boot. This binary restores the possibility. Although does this in different way
than original one before pruning.

Note that this program is distributed in the hope that will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.

Example of _boot.ini_ file:

    [system]
    start_os_name = debian
    timeout       = 15
    display_device= 0
    display_mode  = 0
    erase_flash   = 1

    [linux]

    [debian]

The configuration allows to choose among two operating systems. First one
is displayed as _android_, second one as _debian_. Note that \[linux\] entry
should be always first one. The program assumes that the \[linux\] entry
boots Android.

When _android_ is chosen, file _linux\\linux.ini_ is read. When _debian_
is chosen, file _debian\\debian.ini_ is read.

The program allows also to boot into _recovery_ or _fastboot_. This is achieved
by write _boot-recovery_ or _boot-fastboot_ on _misc_ partition and then
boot _android_. Such way works provided that _u-boot_ loader loaded by this
program handles the stored information properly. Original _u-boot_ from Ainol
does this.

My _debian\\debian.ini_ file looks very similar to original
_linux\\linux.ini_:

    [segment]
    img_name = c:\debian\u-boot.bin
    img_size = 0x80000
    img_base = 0x4A000000

    [script_info]
    script_base = 0x43000000
    script_size = 0x10000

    [logo_info]
    logo_name = c:\debian\debian.bmp
    logo_show = 1

I have Debian on SD card, so I'm using _u-boot_ from
[linux-sunxi](https://github.com/linux-sunxi/u-boot-sunxi). The SPL loader
is not needed. My SD card has only one partition, which is _ext4_ one, so
I had to modify _include/configs/sunxi-common.h_. Instead of original
_CONFIG\_BOOTCOMMAND_ and _CONFIG\_EXTRA\_ENV\_SETTINGS_ I have:


    #define CONFIG_BOOTCOMMAND \
        "run setargs boot_mmc;" \

    #define CONFIG_EXTRA_ENV_SETTINGS \
        "console=ttyS0,115200\0" \
        "root=/dev/mmcblk0p1 rootwait\0" \
        "panicarg=panic=10\0" \
        "extraargs=\0" \
        "loglevel=8\0" \
        "setargs=setenv bootargs console=${console} root=${root}" \
        " loglevel=${loglevel} ${panicarg} ${extraargs}\0" \
        "boot_mmc=ext4load mmc 0 0x48000000 boot/uImage; ext4load mmc 0 0x49000000 boot/uInitrd; bootm 0x48000000 0x49000000\0"

My _uImage_ is compiled from
[linux-sunxi](https://github.com/linux-sunxi/linux-sunxi). The _uInitrd_
is usual Debian ramdisk created by _update-initramfs_ command and then
packed by _mkimage_ command. Debian image itself was created by me using
_debootstrap_.

