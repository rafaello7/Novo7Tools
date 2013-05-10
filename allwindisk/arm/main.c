/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <fastboot.h>
#include <string.h>
#include <nand_bsp.h>
#include <malloc.h>
#include <vsprintf.h>

void clock_init(void);

/* Forward decl */
static int rx_handler (const char *buffer, unsigned int buffer_size);
static void reset_handler (void);

static struct cmd_fastboot_interface interface = 
{
	.rx_handler            = rx_handler,
	.reset_handler         = reset_handler,
};

static char *transfer_buffer    = (char*)0x41000000;
static char *malloc_base        = (char*)0x42000000;
static char *log_addr           = (char*)0x49000000;
// code is at 0x4a000000

static int go_fel;
static char *to_write_first = NULL;
static unsigned to_write_bytes = 0;
static uint64_t firstSector, count;

static void dolog(const char *fmt, ...)
{
    if( log_addr <= (char*)0x49800000 ) {
        va_list args;
        va_start(args, fmt);
        vsprintf(log_addr, fmt, args);
        va_end(args);
        log_addr += strlen(log_addr);
    }
}

static void reset_handler (void)
{
    dolog("in reset handler\n");
    to_write_first = NULL;
    to_write_bytes = 0;
}

static int rx_handler(const char *buffer, unsigned int buffer_size)
{
    int res;
    uint64_t diskSize;
    char buf[40];

    if( to_write_bytes != 0 ) {
        /* a data for "disk write" */
        if( buffer_size > to_write_bytes ) {
            dolog("rx_handler: ignore data after bytes to write\n");
            buffer_size = to_write_bytes;
        }
        memcpy(to_write_first, buffer, buffer_size);
        to_write_first += buffer_size;
        to_write_bytes -= buffer_size;
        if( to_write_bytes == 0 ) {
            dolog("rx_handler: store data to write, first=0x%llx, count=0x%llx\n",
                firstSector, count);
            if( (res = NAND_LogicWrite(firstSector, count, transfer_buffer)) == 0 )
            {
                fastboot_txstr("OKAY");
                dolog("rx_handler write OK\n");
            }else{
                dolog("rx_handler write error: %d\n", res);
                fastboot_txstr("FAIL");
            }
        }
        return 0;
    }
    dolog("rx_handler: request=%c\n", buffer[0]);
    switch(buffer[0]) {
    case 'D':   /* disk size */
        diskSize = NAND_GetDiskSize();
        fastboot_tx(&diskSize, 8);
        break;
    case 'F':   /* go to FEL */
        fastboot_txstr("going to FEL");
        go_fel = 1;
        break;
    case 'I':   /* ping */
        fastboot_txstr("OKAY");
        break;
    case 'L':   /* write log */
        sprintf(buf, "OKAY%X", log_addr - (char*)0x49000000);
        fastboot_txstr(buf);
        fastboot_tx((char*)0x49000000, log_addr - (char*)0x49000000);
        break;
    case 'S':   /* sleep time test */
        res = simple_strtoul(buffer + 1, NULL, 16);
        dolog("sleep time test: delay=%d\n", res);
        sdelay(res);
        fastboot_txstr("OKAY");
        break;
    case 'U':   /* usb speed test */
        firstSector = simple_strtoul(buffer + 1, NULL, 16);
        count = simple_strtoul(buffer + 18, NULL, 16);
        dolog("rx_handler: usb speed test first=0x%llx, count=0x%llx\n",
                firstSector, count);
        fastboot_txstr("OKAY");
        fastboot_tx(transfer_buffer, count << 9);
        dolog("rx_handler read OK\n");
        break;
    case 'R':   /* read disk */
        firstSector = simple_strtoul(buffer + 1, NULL, 16);
        count = simple_strtoul(buffer + 18, NULL, 16);
        dolog("rx_handler: read first=0x%llx, count=0x%llx\n",
                firstSector, count);
        if( (res = NAND_LogicRead(firstSector, count, transfer_buffer)) == 0 )
        {
            fastboot_txstr("OKAY");
            fastboot_tx(transfer_buffer, count << 9);
            dolog("rx_handler read OK\n");
        }else{
            dolog("rx_handler read error: %d\n", res);
            fastboot_txstr("FAIL");
        }
        break;
    case 'W':   /* write to disk */
        firstSector = simple_strtoul(buffer + 1, NULL, 16);
        count = simple_strtoul(buffer + 18, NULL, 16);
        dolog("rx_handler: write first=0x%llx, count=0x%llx\n",
                firstSector, count);
        to_write_first = transfer_buffer;
        to_write_bytes = count << 9;
        break;
    default:
        fastboot_txstr("FAIL");
        break;
    }
    return 0;
}

void main_loop(void)
{
    clock_init();
    malloc_init(malloc_base, 1 << 22);
    NAND_Init();
    sdelay(0x100);
    if (0 == fastboot_init(&interface)) {
        while (1) {
            int poll_status = fastboot_poll();

            if (FASTBOOT_ERROR == poll_status) {
                /* Error */
                break;
            } else if (FASTBOOT_DISCONNECT == poll_status) {
                /* beak, cleanup and re-init */
                break;
            }
            if( go_fel ) {
                break;
            }
        }
    }
    fastboot_shutdown();
    sdelay(0x200);
    // jump to FEL
    asm("ldr r0, =0xffff0020; bx r0");
}

