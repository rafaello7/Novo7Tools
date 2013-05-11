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

/* Forward decl */
static int rx_handler (const char *buffer, unsigned int buffer_size);
static void reset_handler (void);

static struct cmd_fastboot_interface interface = 
{
	.rx_handler            = rx_handler,
	.reset_handler         = reset_handler,
};

static int go_fel;


static void reset_handler (void)
{
}

static int rx_handler(const char *buffer, unsigned int buffer_size)
{
    if( buffer_size == 3 && !memcmp(buffer, "fel", 3) ) {
        fastboot_txstr("going to FEL");
        go_fel = 1;
    }else if( buffer_size == 5 && !memcmp(buffer, "sleep", 5) ) {
        sdelay(500000);
        fastboot_txstr("sleep");
    }else{
        char response[1024];
        strcpy(response, "GOT: ");
        memcpy(response + 5, buffer, buffer_size);
        fastboot_tx(response, buffer_size + 5);
    }
    return 0;
}

void main_loop(void)
{
    sdelay(0x4000);
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
    sdelay(0x4000);
    // jump to FEL
    asm("ldr r0, =0xffff0020; bx r0");
}

