#ifndef ALLWINDISK_H
#define ALLWINDISK_H

#include <stdint.h>
#include "arm/include/bootdisk_interface.h"

struct MbrPartition
{
    unsigned long long  firstSector;
    unsigned long long  sectorCount;
    char                classname[13];
    char                name[13];
    unsigned            user_type;
    unsigned            ro;
};


/* usbcomm.c */

int usbcomm_init(void);
void usbcomm_exit(void);

/* Sends bootdisk_cmd_header and data via usb; reads bootdisk_resp_header
 * Returns: number of bytes to receive in response data.
 */
int send_command(enum BootdiskCommand cmd, /* command sent through usb */
        unsigned param1,                    /* header param1 value */
        unsigned long long start,           /* header start param value */
        unsigned count,                     /* header count param value */
        const void *data,                   /* data to send after header;
                                               may be NULL */
        unsigned datasize);                 /* the data size */


/* Reads response data sent by device after response header;
 * the bufsize must match exactly the amount of data send by device
 * after header.
 */
void get_response_data(void *buf, unsigned bufsize);


/* Prints to stdout the response data sent by device after response header
 */
void print_response_data(void);


struct flashmem_properties *get_flashmem_props(void);
int getMbrVersion(void);
int getPartitionCount(void);
struct MbrPartition *getPartitionNoParams(int partNo);

int getParitionParams(const char *partName,
        enum FlashMemoryArea *pFMArea,
        unsigned long long *pFirstSector,
        unsigned long long *pSectorCount);


/* Performs communication with the device in order to read their disk data.
 * The data read is stored in rdbuf.
 */
void read_disk(enum FlashMemoryArea fmarea,
        unsigned long long firstSector,
        unsigned long long count, void *rdbuf);

/* Performs communication with the device in order to store the data
 * on their disk.
 */
void write_disk(enum FlashMemoryArea fmarea,
        unsigned long long firstSector,
        unsigned long long count, const void *wrbuf);



/* fel.c */
int fel_ainol(void);


#endif /* ALLWINDISK_H */
