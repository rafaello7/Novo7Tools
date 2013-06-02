#include <stdio.h>
#include <string.h>
#include <libusb.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include "allwindisk.h"


static const char MBR_MAGIC[] = "softw311";
enum {
    MBR_SIZE            = 1024,     /* size of MBR structure */
    MBR_MAX_PART_COUNT  = 15
};

typedef struct tag_PARTITION
{
	uint32_t            addrhi;
	uint32_t            addrlo;
	uint32_t            lenhi;
	uint32_t            lenlo;
	char                classname[12];
	char                name[12];
	uint32_t            user_type;
	uint32_t            ro;
	char                res[16];
} __attribute__ ((packed)) PARTITION;


typedef struct tag_MBR
{
	uint32_t            crc32;
	uint32_t            version;
	char 	            magic[8];
	unsigned  char 	    copy;
	unsigned  char 	    index;
	uint16_t            PartCount;
	PARTITION           array[MBR_MAX_PART_COUNT];
	char                res[MBR_SIZE - 20 -
                            (MBR_MAX_PART_COUNT * sizeof(PARTITION))];
} __attribute__ ((packed)) MBR;


struct MbrPartitionData {
    int version;
    int partitionCount;
    struct MbrPartition partitions[MBR_MAX_PART_COUNT];
};

static libusb_device_handle *usb;

/* Response header read by last send_command().
 * datasize value equal 0 means that whole response is read
 */
static struct bootdisk_resp_header cur_resp_header;


void print_response_data(void)
{
    int status, transferred, toRead;
    char buf[0x40000];

    while( cur_resp_header.datasize > 0 ) {
        toRead = sizeof(buf) < cur_resp_header.datasize ?
            sizeof(buf) : cur_resp_header.datasize;
        if( (status = libusb_bulk_transfer(usb, 1 | LIBUSB_ENDPOINT_IN,
                (unsigned char*)buf, toRead, &transferred, 0)) != 0)
        {
            printf("read error: status=%d\n", status);
            exit(1);
        }
        fwrite(buf, transferred, 1, stdout);
        cur_resp_header.datasize -= transferred;
    }
}


/* Sends command to device and reads response header
 */
int send_command(enum BootdiskCommand cmd, unsigned param1,
        unsigned long long start, unsigned count,
        const void *data, unsigned datasize)
{
    struct bootdisk_cmd_header cmd_header;
    int status, transferred;

    if( cur_resp_header.datasize > 0 ) {
        printf("internal error: unread data of previous command\n");
        exit(1);
    }
    cmd_header.magic = htole16(0x1234);
    cmd_header.cmd = htole16(cmd);
    cmd_header.param1 = htole32(param1);
    cmd_header.start = htole64(start);
    cmd_header.count = htole32(count);
    cmd_header.datasize = datasize;

    if( libusb_bulk_transfer(usb, 2 | LIBUSB_ENDPOINT_OUT,
                (unsigned char*)&cmd_header, sizeof(cmd_header),
                &transferred, 0) != 0 )
    {
        printf("write error\n");
        exit(1);
    }

    if( datasize > 0 && libusb_bulk_transfer(usb, 2 | LIBUSB_ENDPOINT_OUT,
                (unsigned char*)data, datasize, &transferred, 0) != 0 )
    {
        printf("write error\n");
        exit(1);
    }

    do {
        if( (status = libusb_bulk_transfer(usb, 1 | LIBUSB_ENDPOINT_IN,
                (void*)&cur_resp_header, sizeof(cur_resp_header),
                &transferred, 0)) != 0)
        {
            printf("read error: status=%d\n", status);
            exit(1);
        }
        if( transferred != sizeof(cur_resp_header) ) {
            printf("read error: bad header size=%d\n", transferred);
            exit(1);
        }
        cur_resp_header.magic = le16toh(cur_resp_header.magic);
        cur_resp_header.status = le16toh(cur_resp_header.status);
        cur_resp_header.datasize = le32toh(cur_resp_header.datasize);
        if( cur_resp_header.magic != 0x1234 ) {
            printf("read error: bad magic=%x\n", cur_resp_header.magic);
        }
        if( cur_resp_header.status == BRST_MSG )
            print_response_data();
    }while( cur_resp_header.status == BRST_MSG );
    if( cur_resp_header.status == BRST_FAIL ) {
        printf("command execution failed on device\n");
        exit(1);
    }
    return cur_resp_header.datasize;
}

/* read data of command response to buffer
 */
void get_response_data(void *buf, unsigned bufsize)
{
    int status, transferred;

    if( bufsize != cur_resp_header.datasize ) {
        printf("error: data size to receive doesn't match expected\n");
        exit(1);
    }
    while( cur_resp_header.datasize > 0 ) {
        if( (status = libusb_bulk_transfer(usb, 1 | LIBUSB_ENDPOINT_IN,
                buf, cur_resp_header.datasize, &transferred, 0)) != 0)
        {
            printf("read error: status=%d\n", status);
            exit(1);
        }
        buf += transferred;
        cur_resp_header.datasize -= transferred;
    }
}

void read_disk(enum FlashMemoryArea fmarea,
        unsigned long long firstSector,
        unsigned long long count, void *rdbuf)
{
    send_command(BCMD_DISKREAD, fmarea, firstSector, count, NULL, 0);
    get_response_data(rdbuf, count * 512);
}

void write_disk(enum FlashMemoryArea fmarea,
        unsigned long long firstSector,
        unsigned long long count, const void *wrbuf)
{
    send_command(BCMD_DISKWRITE, fmarea, firstSector, count, wrbuf, count*512);
}

/* returns disk size in 512-byte sectors
 */
struct flashmem_properties *get_flashmem_props(void)
{
    static struct flashmem_properties props;
    static int is_initialized;

    if( ! is_initialized ) {
        int i;

        send_command(BCMD_FLASHMEM_PARAMS, 0, 0, 0, NULL, 0);
        get_response_data(&props, sizeof(props));
        for(i = 0; i < FMAREA_COUNT; ++i) {
            props.areas[i].sectors_per_page =
                le32toh(props.areas[i].sectors_per_page);
            props.areas[i].first_pageno =
                le32toh(props.areas[i].first_pageno);
            props.areas[i].page_count =
                le32toh(props.areas[i].page_count);
        }
        props.sectors_per_page = le32toh(props.sectors_per_page);
        props.pages_per_block = le32toh(props.pages_per_block);
        props.blocks_per_chip = le32toh(props.blocks_per_chip);
        props.chip_cnt = le32toh(props.chip_cnt);
        props.pagewithbadflag = le32toh(props.pagewithbadflag);
        props.block_cnt_of_zone = le32toh(props.block_cnt_of_zone);
        is_initialized = 1;
    }
    return &props;
}

static struct MbrPartitionData *getPartitionData(void)
{
    static struct MbrPartitionData partitionData;
    static int isValid = -1;

    if( isValid == -1 ) {
        MBR mbr;
        int i;

        isValid = 0;
        read_disk(FMAREA_LOGDISK, 0, 2, &mbr);
        if( memcmp(mbr.magic, MBR_MAGIC, 8) ) {
            printf("disk seems to not have allwinner partition table\n");
            return NULL;
        }
        partitionData.version = le32toh(mbr.version);
        partitionData.partitionCount = le16toh(mbr.PartCount);
        if( partitionData.partitionCount < 0 ||
                partitionData.partitionCount > MBR_MAX_PART_COUNT )
        {
            printf("error: wrong partition count=%d\n",
                    partitionData.partitionCount);
            return NULL;
        }
        for(i = 0; i < partitionData.partitionCount; ++i) {
            partitionData.partitions[i].firstSector =
                ((unsigned long long)le32toh(mbr.array[i].addrhi) << 32) +
                le32toh(mbr.array[i].addrlo);
            partitionData.partitions[i].sectorCount =
                ((unsigned long long)le32toh(mbr.array[i].lenhi) << 32) +
                le32toh(mbr.array[i].lenlo);
            memset(partitionData.partitions[i].classname, 0,
                    sizeof(partitionData.partitions[i].classname));
            strncpy(partitionData.partitions[i].classname,
                    mbr.array[i].classname,
                    sizeof(mbr.array[i].classname));
            memset(partitionData.partitions[i].name, 0,
                    sizeof(partitionData.partitions[i].name));
            strncpy(partitionData.partitions[i].name, mbr.array[i].name,
                    sizeof(mbr.array[i].name));
            partitionData.partitions[i].user_type =
                le32toh(mbr.array[i].user_type);
            partitionData.partitions[i].ro = le32toh(mbr.array[i].ro);
        }
        isValid = 1;
    }
    return isValid ? &partitionData : NULL;
}

int getMbrVersion(void)
{
    struct MbrPartitionData *pdata = getPartitionData();
    return pdata == NULL ? -1 : pdata->version;
}

int getPartitionCount(void)
{
    struct MbrPartitionData *pdata = getPartitionData();
    return pdata == NULL ? -1 : pdata->partitionCount;
}

struct MbrPartition *getPartitionNoParams(int partNo)
{
    struct MbrPartitionData *pdata = getPartitionData();
    return pdata == NULL || partNo >= pdata->partitionCount ? NULL :
        pdata->partitions + partNo;
}

int getParitionParams(const char *partName,
        enum FlashMemoryArea *pFMArea,
        unsigned long long *pFirstSector,
        unsigned long long *pSectorCount)
{
    struct flashmem_properties *props = get_flashmem_props();
    struct MbrPartitionData *pdata;

    *pFMArea = FMAREA_COUNT;
    if( ! strcmp(partName, "boot0") ) {
        *pFMArea = FMAREA_BOOT0;
    }else if( ! strcmp(partName, "boot1") ) {
        *pFMArea = FMAREA_BOOT1;
    }else if( ! strcmp(partName, "disk-logic") ) {
        *pFMArea = FMAREA_LOGDISK;
    }
    if( *pFMArea != FMAREA_COUNT ) {
        *pFirstSector = 0LL;
        *pSectorCount = 1ULL * props->areas[*pFMArea].sectors_per_page *
            props->areas[*pFMArea].page_count;
    }else{
        int i;
        unsigned long long diskSize;

        if( (pdata = getPartitionData()) == NULL )
            return 0;
        for( i = 0; i < pdata->partitionCount; ++i ) {
            if( ! strcmp(partName, pdata->partitions[i].name) )
                break;
        }
        if( i == pdata->partitionCount ) {
            printf("partition with name %s not found\n", partName);
            return 0;
        }
        *pFMArea = FMAREA_LOGDISK;
        *pFirstSector = pdata->partitions[i].firstSector;
        *pSectorCount = pdata->partitions[i].sectorCount;
        diskSize = 1ULL * props->areas[FMAREA_LOGDISK].sectors_per_page *
            props->areas[FMAREA_LOGDISK].page_count;
        if( *pFirstSector >= diskSize ||
                *pFirstSector + *pSectorCount > diskSize)
        {
            printf("ERROR: corrupted MBR, partition %s is beyond disk size\n",
                    pdata->partitions[i].name);
            return 0;
        }
    }
    return 1;
}

void get_mountstatus(struct diskmount_status *stat)
{
    send_command(BCMD_GETMOUNT_STATUS, 0, 0, 0, NULL, 0);
    get_response_data(stat, sizeof(*stat));
    stat->mountMode = le32toh(stat->mountMode);
    stat->firstSector = le32toh(stat->firstSector);
    stat->sectorCount = le32toh(stat->sectorCount);
}

int usbcomm_init(void)
{
    int i;

    if( libusb_init(NULL) != 0 ) {
        printf("libusb_init fail\n");
        return -1;
    }
    if( (usb = libusb_open_device_with_vid_pid(NULL, 0xbb4, 0xfff)) == NULL )
    {
        printf("open device failed, trying to fel...");
        fflush(stdout);
        if( fel_ainol() != 0 ) {
            fprintf(stderr, "\nfailed to open A10 USB FEL device\n");
            return -1;
        }
        for(i = 0; usb == NULL && i < 10; ++i) {
            printf(".");
            fflush(stdout);
            usleep(500000);
            usb = libusb_open_device_with_vid_pid(NULL, 0xbb4, 0xfff);
        }
        printf("\n");
        if( usb == NULL ) {
            printf("open device failed\n");
            return -1;
        }
    }
    libusb_claim_interface(usb, 0);
    return 0;
}

void usbcomm_exit(void)
{
    libusb_release_interface(usb, 0);
    libusb_close(usb);
    libusb_exit(NULL);
}

