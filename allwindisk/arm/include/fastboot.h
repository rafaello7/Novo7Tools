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

#ifndef FASTBOOT_H
#define FASTBOOT_H


struct cmd_fastboot_interface
{
	/* Called when a buffer has been recieved from the client app.
	 * The buffer is a supplied by the board layer and must be unmodified. 
	 * The buffer_size is how much data is passed in. 
	 * Returns 0 on success
	 * Returns 1 on failure	
     */
	int (*rx_handler)(const char *buffer,
			  unsigned int buffer_size);
	
	/* Called when an exception has occurred in the device code and the state
	   off fastboot needs to be reset */
	void (*reset_handler)(void);
};

/* Status values */
enum {
    FASTBOOT_OK			= 0,
    FASTBOOT_ERROR		= -1,
    FASTBOOT_DISCONNECT	= 1,
    FASTBOOT_INACTIVE	= 2
};

/* Initizes the board specific fastboot 
   Returns 0 on success
   Returns 1 on failure */
extern int fastboot_init(struct cmd_fastboot_interface *interface);

/* Cleans up the board specific fastboot */
extern void fastboot_shutdown(void);

/*
 * Handles board specific usb protocol exchanges
 * Returns 0 on success
 * Returns 1 on disconnects, break out of loop
 * Returns 2 if no USB activity detected
 * Returns -1 on failure, unhandled usb requests and other error conditions
*/
extern int fastboot_poll(void);

/* Is this high speed (2.0) or full speed (1.1) ? 
   Returns 0 on full speed
   Returns 1 on high speed */
extern int fastboot_is_highspeed(void);

/* Return the size of the fifo */
extern int fastboot_fifo_size(void);

/* Send data to the client app */
extern void fastboot_tx(const void *buffer, unsigned buffer_size);

/* Send string to the client app (without terminating null byte) */
void fastboot_txstr(char *str);

void sdelay(unsigned long loops);

#endif /* FASTBOOT_H */
