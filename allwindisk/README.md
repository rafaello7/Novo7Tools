A tool to read/write NAND flash disk on allwinner device without need to have
Android operating system on device.

License
-------

  This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation, either version 2 of the License, or (at your option)
any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.


Compilation
-----------

The tool needs both arm toolchain and native compiler. Ensure libusb-1.0-0-dev
package is installed.

Run "make" without parameters.


Usage
-----

Enter device into FEL mode (a mode used by LiveSuit flash tool), then run

    ./allwindisk i

At first run, the tool loads to the device file "arm/diskboot.bin". After
load, the program should print out "OK".

Invoke allwindisk without parameters for possible operations.

On Ainol Novo 7 Aurora the FEL mode may be entered as follows:

1. Turn off the device by press "power" button and hold it for at least
   10 seconds.
2. Press "volume down" button (don't release).
3. Plug in USB cable to device (the cable should be already connected
   to computer).
4. Press and release "power" button at least 5 times
5. Release "volume down" button.

The device should be visible using e.g. lsusb command as 1f3a:efe8.


Reading/writing flash memory of the device.
-------------------------------------------

####    allwindisk dp

This command prints out information about flash memory installed on
device, division of the flash memory into areas and logical disk partitions.

Command displays table similar to the following:

    flash properties:
            page        pages      blocks     chip          pages    block count
         kB-size    per block    per chip    count     marked bad        of zone
               8          256        4096        1              1            950
    
    flash areas:
                                     page       erase block
        NAME          kB-size     kB-size           kB-size
        boot0             512           1               256
        boot1           10240           8              2048
        disk-phy      8374272           8              2048
        disk-logic    7782400          16  
    
    logical disk partitions:                              (mbr ver: 100)
     #  NAME        kB-offset   kB-length     ro   usertype        class
     0  bootloader      16384       16384      0          0         DISK
     1  env             32768       16384      0          0         DISK
     2  boot            49152       16384      0          0         DISK
     3  system          65536      524288      0          0         DISK
     4  data           589824     1048576      0          1         DISK
     5  misc          1638400       16384      0          0         DISK
     6  recovery      1654784       32768      0          0         DISK
     7  cache         1687552      524288      0          0         DISK
     8  UDISK         2211840     5570560      0          0         DISK


The "flash properties" table shows the installed flash memory information.

The flash memory consists of pages. One page is the smallest amount of
memory which may be written. The above table specifies that page size is 8 kB.

The pages are assembled into erase blocks. One block is the smallest
piece of memory which may be erased. When the block is erased, all bits
in the block are set to 1 - all bytes are set to 0xFF. When a page is
written, bits "1" may be changed to "0", but not vice versa. This way behaves
NAND flash memory. Once a block erased, it may be written page by page.

Whole memory of the device may be calculated as:

      <page size> * <pages per block> * <blocks per chip> * <chip count>

It seems the device has 8kB * 256 * 4096 * 1 = 8GB of flash memory.

The "block count of zone" parameter value is used in calculations of logical
disk size, will be explained later.

The flash memory is divided into three parts (areas):

1. First part contains boot0 image, the image loaded by SoC BROM.
2. Second part contains boot1 image, which is loaded by boot0
3. Third part contains logical disk. The logical disk is created on the disk
   area, marked as "disk-phy". As can be seen, the logical disk size is smaller
   than the disk area size because certain percentage of pages is reserved for
   "bad pages" (pages which are flashed with bit distortions). Also a few pages
   are occupied by the disk administrative data.

The boot0 area is flashed using smaller page size. Really the pages have 8kB,
too, but only first kilobyte of every page is used. This is because of BROM
code, which does not know the page size, so it reads only 1kB of every page.
The BROM may farther attempt to read 512 bytes of every page, if reading of
1kB fails, but does never attempt to read 2kB or more. Regardless of the
page size, the boot0 area occupies always 512 pages. In the table above,
because single block contains 256 pages, the erase block size if 256 *
1kB = 256kB.

The boot1 area may be flashed with real page size because boot0 code knows
(has embedded) the real page size. Number of pages included in the
boot1 area depend on page size and may vary.

The rest of flash memory is designated for logical disk. The logical disk
algorithm has built-in wear leveling of the flash, so the disk may be
formatted using e.g. ext4 format and not jffs, which would be needed when
"raw" flash would be in use.

The logical disk size is calculated using "block count of zone" parameter.
This parameters expresses expected minimum of good blocks per 1024
blocks. The disk size formula is:

      <whole flash size> * <block count of zone> / 1024

Note that the whole flash memory size is taken into account, not the size
of area designated for logical disk. According to parameters shown in
above table, the logical disk size is: 8192MB * 950 / 1024 = 7600MB.

The logical disk has page size two times larger than physical page size
because the pages are written with interleaving (whatever it means ;-) ).

The logical disk is divided into partitions. Initial segment of disk contains
MBR with partition table. Mostly it is not accessible on Android. Usually the
MBR size is 1MB. In the table above it has 16MB ("bootloader" partition is at
16MB offset). This way the disk was divided by Feiyu for Android 4.1.

The partition table has Allwinner's own format.

####    allwindisk dr

        allwindisk dr <partname> [<offsetkB> <sizekB>] <file>

Command reads partition from device and stores in file. Offset and size are
expressed in kilobytes. When omitted, whole partition is read.

Also flash memory area may be specified as "partname" (except "disk-phy").


####    allwindisk dw

        allwindisk dw <partname> [<offsetkB>] <file>

Writes disk partition with data from the file. The file may be shorter
than partition size. In this case the disk is filled with 0xff up to nearest
sector (512 byte) boundary. Rest of partition remains unchanged.
Offset is in kilobytes. When omitted, assume 0.

####    allwindisk dm

        allwindisk dm <partname> [rw]
  
Makes partition available as disk on host. If "rw" is specified, the disk
may be mounted for both reading and writing. By default the partition
may be mounted for reading only.

Be careful with writing, although seems to work but not tested well.


####    allwindisk dd

        allwindisk dd <partname> [<offsetkB> [<sizekB>]]
    
Prints partition data on stdout, in human-readable form. When size is
omitted, 512 bytes are printed.


Other Commands
--------------

####    allwindisk dg

        allwindisk dg <eGON_imgfile>
 
Not implemented yet.
Embed eGON image (must have valid magic and the place for size and checksum).


####    allwindisk mr

        allwindisk mr <address> <size[k|M]> <file>
   
Read memory contents to file.


####    allwindisk mw

        allwindisk mw <address> <file>
 
Write memory using data from file.


####    allwindisk mx

        allwindisk mx <address> [<file>]
    
If a file is specified, loads the file contents to the specified address
(like "mw" command does). Next, executes code at the specified address.

####    allwindisk mj

        allwindisk mj <address> [<file>]
   
If a file is specified, loads the file contents to the specified address
(like "mw" command does). Next, jumps to the code at the specified address.

The difference between "mx" and "mj" commands is that code executed by "mx" is
expected to return to caller and that doesn't alter device registry settings
which may have influence on USB communication or disk operations. Also, the
"allwindisk" command waits for the code to finish.
On the other hand, code executed by "mj" command is not expected to return.
The allwindisk command does not wait for the execution finish.

Currently the following memory areas are free for such programs:

* 0x0 .. 0x6fff
* 0x40000000 .. 0x40ffffff
* 0x42000000 .. 0x42ffffff
* 0x4b000000 and above


####    allwindisk md

        allwindisk md <address> [<size[k|M>]
  
Dump memory piece in human-readable form.

####    allwindisk mf

        allwindisk mf <address> <string> [<size[k|M|x>]
 
Fill memory piece with string. It is possible to specify amount of memory
to fill or number of string repeats (using "x" suffix). When the size parameter
is omitted, "1x" is assumed.

For example, to fill 1MB of memory at address 0x40000000 with 0's, invoke:

        allwindisk mf 0x40000000 '\0' 1M


####    allwindisk mg

        allwindisk mg <eGON_imgfile>
 
Load the eGON image into memory and jump to it. When it is boot0 image,
it is loaded at address 0. When boot1, it is loaded at address 0x42400000.


####    allwindisk i


Ping (check if alive).


####    allwindisk q


Perform the device reset.


####    allwindisk f

Goes back to FEL mode.


####    allwindisk l


Prints debug log from device.


Have fun :-)
