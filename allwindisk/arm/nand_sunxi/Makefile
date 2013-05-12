# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

LIB	= libsunxi-nand.a

CROSS_COMPILE = arm-none-linux-gnueabi-

OBJS	:= nand_drv/nand_for_uboot.o  		\
		   nand_drv/nand_osal_uboot.o 		\
		   src/format/nand_format.o			\
		   src/logic/bad_manage.o			\
		   src/logic/logic_cache.o			\
		   src/logic/logic_ctl.o			\
		   src/logic/mapping.o				\
		   src/logic/mapping_base.o			\
		   src/logic/merge.o				\
		   src/logic/read_reclaim.o			\
		   src/logic/wear_levelling.o		\
		   src/ndfc/nfc_r.o					\
		   src/ndfc/nfc_w.o					\
		   src/physic/nand_phy.o			\
		   src/physic/nand_simple_r.o		\
		   src/physic/nand_simple_w.o		\
		   src/scan/nand_id.o				\
		   src/scan/nand_scan.o

all:	$(LIB)

$(LIB):	$(OBJS)
	ar sru $(LIB) $(OBJS)

clean:
	rm -f $(OBJS) $(LIB)

sc:
	make clean
	make $(LIB)

.c.o:
	$(CROSS_COMPILE)gcc -Os -fno-common -I../include -ffreestanding -nostdinc -marm -mabi=aapcs-linux -march=armv6 -Wall -Wstrict-prototypes -fno-stack-protector -c -o $@ $<

