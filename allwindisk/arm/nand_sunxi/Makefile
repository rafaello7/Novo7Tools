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
	/home/rafal/Android/lichee/buildroot/output/external-toolchain/bin/arm-none-linux-gnueabi-gcc  -g  -Os   -fno-common -ffixed-r8 -msoft-float   -D__KERNEL__ -DCONFIG_SYS_TEXT_BASE=0x4A000000 -I../include -fno-builtin -ffreestanding -nostdinc -isystem /home/rafal/Android/lichee/buildroot/output/external-toolchain/bin/../lib/gcc/arm-none-linux-gnueabi/4.5.1/include -pipe  -DCONFIG_ARM -D__ARM__ -marm  -mabi=aapcs-linux -mno-thumb-interwork -march=armv5 -Wall -Wstrict-prototypes -fno-stack-protector -Wno-format-nonliteral -Wno-format-security -c -o $@ $<

