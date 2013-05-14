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
#include <bootdisk_interface.h>

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

static struct bootdisk_cmd_header cur_cmd;
static unsigned cur_cmd_bytes_written;

static int go_fel;

static void dolog(const char *fmt, ...)
{
    static unsigned logno;

    if( log_addr <= (char*)0x49800000 ) {
        va_list args;

        sprintf(log_addr, "%-5u ", ++logno);
        log_addr += strlen(log_addr);
        va_start(args, fmt);
        vsprintf(log_addr, fmt, args);
        va_end(args);
        log_addr += strlen(log_addr);
    }
}

static void reset_handler (void)
{
    dolog("in reset handler\n");
    cur_cmd.cmd = BCMD_NONE;
}

static void send_resp_fail(void)
{
    struct bootdisk_resp_header resp;
    resp.magic = 0x1234;
    resp.status = 0;
    resp.datasize = 0;
    fastboot_tx(&resp, sizeof(resp));
}

static void send_resp_OK(const void *data, int datasize)
{
    struct bootdisk_resp_header resp;

    if( datasize < 0 )
        datasize = strlen(data);
    resp.magic = 0x1234;
    resp.status = 1;
    resp.datasize = datasize;
    fastboot_tx(&resp, sizeof(resp));
    if( datasize > 0 )
        fastboot_tx(data, datasize);
}

static void dispatch_cmd(uint16_t cmd, uint64_t param1, uint64_t param2,
        const char *data, unsigned datasize)
{
    uint64_t diskSize;
    int res;

    dolog("dispatch_cmd: command=%c, param1=%lld (0x%llx),"
           " param2=%lld (0x%llx), datasize=%d\n",
           cmd, param1, param1, param2, param2, datasize);
    switch(cmd) {
    case BCMD_DISKSIZE:
        diskSize = NAND_GetDiskSize();
        send_resp_OK(&diskSize, 8);
        break;
    case BCMD_GO_FEL:
        send_resp_OK("going to FEL", -1);
        go_fel = 1;
        break;
    case BCMD_PING:
        send_resp_OK(NULL, 0);
        break;
    case BCMD_GETLOG:
        send_resp_OK((void*)0x49000000, log_addr - (char*)0x49000000);
        if( param1 != 0 )
            log_addr = (char*)0x49000000;
        break;
    case BCMD_SLEEPTEST:
        dolog("sleep time test: delay=%lld\n", param1);
        sdelay(param1);
        send_resp_OK(NULL, 0);
        break;
    case BCMD_USBSPDTEST:
        dolog("rx_handler: usb speed test first=0x%llx, count=0x%llx\n",
                param1, param2);
        send_resp_OK(transfer_buffer, param2 << 9);
        break;
    case BCMD_READ:
        dolog("rx_handler: read first=0x%llx, count=0x%llx\n",
                param1, param2);
        if( (res = NAND_LogicRead(param1, param2, transfer_buffer)) == 0 )
        {
            send_resp_OK(transfer_buffer, param2 << 9);
            dolog("rx_handler read OK\n");
        }else{
            dolog("rx_handler read error: %d\n", res);
            send_resp_fail();
        }
        break;
    case BCMD_WRITE:
        dolog("rx_handler: write first=0x%llx, count=0x%lx\n",
                param1, datasize);
        if( (res = NAND_LogicWrite(param1, datasize / 512,
                        transfer_buffer)) == 0 )
        {
            send_resp_OK(NULL, 0);
            dolog("rx_handler write OK\n");
        }else{
            dolog("rx_handler write error: %d\n", res);
            send_resp_fail();
        }
        break;
    default:
        send_resp_fail();
        break;
    }
}

static int rx_handler(const char *buffer, unsigned int buffer_size)
{
    if( cur_cmd.cmd == BCMD_NONE ) {
        if( buffer_size != sizeof(cur_cmd) ) {
            dolog("rx_handler FATAL: wrong data size on input=%d\n",
                    buffer_size);
            return 0;
        }
        memcpy(&cur_cmd, buffer, sizeof(cur_cmd));
        if( cur_cmd.magic != 0x1234 ) {
            dolog("rx_handler FATAL: wrong magic on cmd=%x\n", cur_cmd.magic);
            return 0;
        }
        if( cur_cmd.datasize > 0x1000000 ) {    /* max 16 MB */
            dolog("rx_handler FATAL: too big data size on cmd=0x%x\n",
                    cur_cmd.datasize);
            return 0;
        }
        cur_cmd_bytes_written = 0;
    }else{
        int to_write = cur_cmd.datasize - cur_cmd_bytes_written;
        if( buffer_size <= to_write )
            to_write = buffer_size;
        else
            dolog("rx_handler: ignore extra %d bytes on input\n",
                    buffer_size - to_write);
        memcpy(transfer_buffer + cur_cmd_bytes_written, buffer, to_write);
        cur_cmd_bytes_written += to_write;
    }
    if( cur_cmd_bytes_written == cur_cmd.datasize ) {
        dispatch_cmd(cur_cmd.cmd, cur_cmd.cmd_param1, cur_cmd.cmd_param2,
                transfer_buffer, cur_cmd.datasize);
        cur_cmd.cmd = BCMD_NONE;
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

