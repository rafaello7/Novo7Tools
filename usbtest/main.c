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
static int tx_handler(void);
static int rx_handler (const unsigned char *buffer, unsigned int buffer_size);
static void reset_handler (void);

static struct cmd_fastboot_interface interface = 
{
	.rx_handler            = rx_handler,
	.reset_handler         = reset_handler,
	.product_name          = NULL,
	.serial_no             = NULL,
	.transfer_buffer       = (unsigned char *)0xffffffff,
	.transfer_buffer_size  = 0,
};

static unsigned int upload_size;
static unsigned int upload_bytes;
static unsigned int upload_error;

static void reset_handler (void)
{
	/* If there was a download going on, bail */
	upload_size = 0;
	upload_bytes = 0;
	upload_error = 0;
}


static int tx_handler(void)
{
	if (upload_size) {

		int bytes_written;
		bytes_written = fastboot_tx(interface.transfer_buffer +
					    upload_bytes, upload_size -
					    upload_bytes);
		if (bytes_written > 0) {

			upload_bytes += bytes_written;
			/* Check if this is the last */
			if (upload_bytes == upload_size) {

				/* Reset upload */
				upload_size = 0;
				upload_bytes = 0;
				upload_error = 0;
			}
		}
	}
	return upload_error;
}

static int go_fel;

static int rx_handler (const unsigned char *buffer, unsigned int buffer_size)
{
    char response[1024];

    if( buffer_size == 3 && !memcmp(buffer, "fel", 3) ) {
        strcpy(response, "going to FEL");
        fastboot_tx_status(response, strlen(response));
        go_fel = 1;
    }else if( buffer_size == 5 && !memcmp(buffer, "sleep", 5) ) {
        strcpy(response, "sleep");
        sdelay(50000);
        fastboot_tx_status(response, strlen(response));
    }else if( buffer_size == 5 && !memcmp(buffer, "reset", 5) ) {
        strcpy(response, "reset");
        fastboot_tx_status(response, strlen(response));
        sdelay(50);
    }else{
        strcpy(response, "GOT: ");
        memcpy(response + 5, buffer, buffer_size);
        fastboot_tx_status(response, buffer_size + 5);
    }
    return 0;
}

void main_loop(void)
{
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

            /* Check if there is something to upload */
            tx_handler();
        }
    }
    fastboot_shutdown();
    sdelay(0x200);
    // jump to FEL
    asm("ldr r0, =0xffff0020; bx r0");
}

