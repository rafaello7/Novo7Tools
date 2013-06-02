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

#ifndef DISKBOOT_H
#define DISKBOOT_H


/* USB device status values */
enum {
    USBDCOMM_OK			= 0,
    USBDCOMM_ERROR		= -1,
    USBDCOMM_DISCONNECT	= 1,
    USBDCOMM_INACTIVE	= 2
};

/* Returns 0 on success
 * Returns 1 on failure */
extern int usbdcomm_init(void);

extern void usbdcomm_shutdown(void);

/*
 * Handles board specific usb protocol exchanges
 * Returns 0 on success
 * Returns 1 on disconnects, break out of loop
 * Returns 2 if no USB activity detected
 * Returns -1 on failure, unhandled usb requests and other error conditions
*/
extern int usbdcomm_poll(void);

/* Send data to the client app */
extern void usbdcomm_diskboot_tx(const void *buffer, unsigned buffer_size);
extern void usbdcomm_masstorage_tx(const void *buffer, unsigned buffer_size);


int diskboot_rx_handler(const char *message, unsigned count);
void diskboot_reset_handler(void);


int masstorage_rx_handler(const char *message, unsigned count);
int masstorage_mount(unsigned firstSector, unsigned sectorCount,
        int read_write);
int masstorage_getstatus(unsigned *firstSector, unsigned *sectorCount,
        int *read_write);


void sdelay(unsigned loops);
void clock_init(void);
void clock_restore(void);
void reset_board(void);

void dolog(const char *fmt, ...);

#endif /* DISKBOOT_H */
