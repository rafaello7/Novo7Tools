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

/* From fastboot client.. */
#define FASTBOOT_INTERFACE_CLASS     0xff
#define FASTBOOT_INTERFACE_SUB_CLASS 0x42
#define FASTBOOT_INTERFACE_PROTOCOL  0x03

struct cmd_fastboot_interface
{
	/* This function is called when a buffer has been 
	   recieved from the client app.
	   The buffer is a supplied by the board layer and must be unmodified. 
	   The buffer_size is how much data is passed in. 
	   Returns 0 on success
	   Returns 1 on failure	

	   Set by cmd_fastboot	*/
	int (*rx_handler)(const unsigned char *buffer,
			  unsigned int buffer_size);
	
	/* This function is called when an exception has
	   occurred in the device code and the state
	   off fastboot needs to be reset 

	   Set by cmd_fastboot */
	void (*reset_handler)(void);
  
	/* A getvar string for the product name
	   It can have a maximum of 60 characters 

	   Set by board	*/
	char *product_name;
	
	/* A getvar string for the serial number 
	   It can have a maximum of 60 characters 

	   Set by board */
	char *serial_no;

	/* Transfer buffer, for handling flash updates
	   Should be multiple of the nand_block_size 
	   Care should be take so it does not overrun bootloader memory	
	   Controlled by the configure variable CFG_FASTBOOT_TRANSFER_BUFFER 

	   Set by board */
	unsigned char *transfer_buffer;

	/* How big is the transfer buffer
	   Controlled by the configure variable
	   CFG_FASTBOOT_TRANSFER_BUFFER_SIZE

	   Set by board	*/ 
	unsigned int transfer_buffer_size;

};

/* Status values */
#define FASTBOOT_OK			0
#define FASTBOOT_ERROR			-1
#define FASTBOOT_DISCONNECT		1
#define FASTBOOT_INACTIVE		2

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

/* Send a status reply to the client app 
   buffer does not have to be null terminated. 
   buffer_size must be not be larger than what is returned by
   fastboot_fifo_size 
   Returns 0 on success
   Returns 1 on failure */
extern int fastboot_tx_status(const char *buffer, unsigned int buffer_size);

/*
 * Send some data to the client app
 * buffer does not have to be null terminated.
 * buffer_size can be larger than what is returned by
 * fastboot_fifo_size
 * Returns number of bytes written
 */
extern int fastboot_tx(unsigned char *buffer, unsigned int buffer_size);

void sdelay(unsigned long loops);

#endif /* FASTBOOT_H */
