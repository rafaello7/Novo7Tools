#include <stdio.h>
#include <string.h>
#include <libusb.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include "arm/include/bootdisk_interface.h"


#define     MBR_SIZE			1024
#define   	MBR_MAGIC			"softw311"
#define     MBR_MAX_PART_COUNT	15
#define     MBR_RESERVED        (MBR_SIZE - 20 - (MBR_MAX_PART_COUNT * sizeof(PARTITION)))

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
	char                res[MBR_RESERVED];
} __attribute__ ((packed)) MBR;


static libusb_device_handle *usb;

/* datasize equal 0 means that whole response is read */
static struct bootdisk_resp_header cur_resp_header;

int fel_ainol(void);


static void print_response_data(void)
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
static void send_command(unsigned cmd, unsigned param1,
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
}

/* read data of command response to buffer
 */
static void get_response_data(void *buf, unsigned bufsize)
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

static void read_disk(enum FlashMemoryArea fmarea,
        unsigned long long firstSector,
        unsigned long long count, void *rdbuf)
{
    send_command(BCMD_DISKREAD, fmarea, firstSector, count, NULL, 0);
    get_response_data(rdbuf, count * 512);
}

static int read_mbr(MBR *mbr)
{
    int i;

    read_disk(FMAREA_LOGDISK, 0, 2, mbr);
    if( memcmp(mbr->magic, MBR_MAGIC, 8) ) {
        printf("disk seems to not have allwinner partition table\n");
        return 0;
    }
    mbr->crc32 = le32toh(mbr->crc32);
    mbr->version = le32toh(mbr->version);
    mbr->PartCount = le16toh(mbr->PartCount);
    if( mbr->PartCount < 0 || mbr->PartCount > MBR_MAX_PART_COUNT ) {
        printf("error: wrong partition count=%d\n", mbr->PartCount);
        return 0;
    }
    for(i = 0; i < mbr->PartCount; ++i) {
        mbr->array[i].addrhi = le32toh(mbr->array[i].addrhi);
        mbr->array[i].addrlo = le32toh(mbr->array[i].addrlo);
        mbr->array[i].lenhi = le32toh(mbr->array[i].lenhi);
        mbr->array[i].lenlo = le32toh(mbr->array[i].lenlo);
        mbr->array[i].user_type = le32toh(mbr->array[i].user_type);
        mbr->array[i].ro = le32toh(mbr->array[i].ro);
    }
    return 1;
}

/* returns disk size in 512-byte sectors
 */
static struct flashmem_properties *get_flashmem_props(void)
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

static int getParitionParams(const char *partName,
        enum FlashMemoryArea *pFMArea, unsigned long long *pFirstSector,
        unsigned long long *pPartSize)
{
    MBR mbr;
    struct flashmem_properties *props = get_flashmem_props();

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
        *pPartSize = 1ULL * props->areas[*pFMArea].sectors_per_page *
            props->areas[*pFMArea].page_count;
    }else{
        int i;
        unsigned long long diskSize;

        if( ! read_mbr(&mbr) )
            return 0;
        for( i = 0; i < mbr.PartCount; ++i ) {
            if( ! strncmp(partName, mbr.array[i].name,
                        sizeof(mbr.array[0].name)) )
                break;
        }
        if( i == mbr.PartCount ) {
            printf("partition with name %s not found\n", partName);
            return 0;
        }
        *pFMArea = FMAREA_LOGDISK;
        *pFirstSector =
            ((unsigned long long)mbr.array[i].addrhi << 32)
            + mbr.array[i].addrlo;
        *pPartSize = ((unsigned long long)mbr.array[i].lenhi << 32)
                + mbr.array[i].lenlo;
        diskSize = 1ULL * props->areas[FMAREA_LOGDISK].sectors_per_page *
            props->areas[FMAREA_LOGDISK].page_count;
        if( *pFirstSector >= diskSize || *pFirstSector + *pPartSize > diskSize)
        {
            printf("ERROR: corrupted MBR, partition %s is beyond disk size\n",
                    mbr.array[i].name);
            return 0;
        }
    }
    return 1;
}

static void cmd_diskread(int argc, char *argv[])
{
    enum FlashMemoryArea fmarea;
    unsigned long long firstSector, count, toRdTot, partOffset, partSize;
    int fd;
    char buf[0x80000];
    struct timeval tvpre, tvpost;
    double transferTime;

    if( argc != 2 && argc != 4 ) {
        printf("error: wrong number of parameters\n");
        return;
    }
    if( ! getParitionParams(argv[0], &fmarea, &firstSector, &partSize) )
        return;
    if( argc == 4 ) {
        partOffset = strtoull(argv[1], NULL, 0) << 1;
        count = strtoull(argv[2], NULL, 0) << 1;
        if( partOffset + count > partSize ) {
            printf("error: requested read beyond disk size\n");
            return;
        }
    }else{
        partOffset = 0;
        count = partSize;
    }
    firstSector += partOffset;
    fd = open(argv[argc-1], O_RDWR | O_CREAT | O_TRUNC, 0664);
    if( fd < 0 ) {
        printf("failed to open %s: %s\n", argv[argc-1],
                strerror(errno));
        return;
    }
    gettimeofday(&tvpre, NULL);
    toRdTot = count;
    while( toRdTot > 0 ) {
        unsigned toRead = sizeof(buf) / 512;
        if( toRdTot < toRead )
            toRead = toRdTot;
        read_disk(fmarea, firstSector, toRead, buf);
        if( write(fd, buf, toRead * 512) != toRead * 512 ) {
            printf("file write error\n");
            close(fd);
            return;
        }
        firstSector += toRead;
        toRdTot -= toRead;
        putchar('.');
        fflush(stdout);
    }
    printf("\n");
    gettimeofday(&tvpost, NULL);
    transferTime = (tvpost.tv_sec - tvpre.tv_sec) +
        (tvpost.tv_usec - tvpre.tv_usec) * 1e-9;
    if( transferTime < 1.0 )
        printf("%lld kB transferred\n", count / 2);
    else
        printf("%lld kB transferred in %.0f seconds, %.0f kB/s\n",
                count / 2, transferTime, count / transferTime / 2);

}

static void cmd_diskwrite(int argc, char *argv[])
{
    enum FlashMemoryArea fmarea;
    unsigned long long firstSector, toWrMax, count, partOffset, partSize;
    int fd, rd, bufsize = 0x80000;
    char *buf;
    struct timeval tvpre, tvpost;
    double transferTime;

    if( argc != 2 && argc != 3 ) {
        printf("error: wrong number of parameters\n");
        return;
    }
    if( ! getParitionParams(argv[0], &fmarea, &firstSector, &partSize) )
        return;
    if( argc == 3 ) {
        partOffset = strtoull(argv[1], NULL, 0) << 1;
        if( partOffset > partSize ) {
            printf("error: requested write beyond disk size\n");
            return;
        }
    }else
        partOffset = 0;
    firstSector += partOffset;
    toWrMax = partSize - partOffset;
    fd = open(argv[argc-1], O_RDONLY);
    if( fd < 0 ) {
        printf("failed to open %s: %s\n", argv[argc-1],
                strerror(errno));
        return;
    }
    if( fmarea != FMAREA_LOGDISK ) {
        struct flashmem_properties *props = get_flashmem_props();
        int blocksize = 512 * props->areas[fmarea].sectors_per_page *
            props->pages_per_block;
        if( blocksize > bufsize )
            bufsize = blocksize;
    }
    buf = malloc(bufsize);
    gettimeofday(&tvpre, NULL);
    count = 0;
    while( (rd = read(fd, buf, bufsize)) > 0 ) {
        unsigned toWr;
        if( rd & 0x1FF ) {
            int toPad = 512 - (rd & 0x1ff);
            printf("warning: file size is not a multiple of sectors\n");
            printf("warning: padding with 0xff\n");
            memset(buf + rd, 0xff, toPad);
            rd += toPad;
        }
        toWr = rd >> 9;
        if( toWr + count > toWrMax ) {
            printf("warning: file size exceed disk space, cut\n");
            if( toWrMax > count ) {
                send_command(BCMD_DISKWRITE, fmarea, firstSector,
                        toWrMax - count, buf, (toWrMax-count)*512);
                count = toWrMax;
            }
            break;
        }
        send_command(BCMD_DISKWRITE, fmarea, firstSector, toWr, buf, toWr*512);
        firstSector += toWr;
        count += toWr;
        putchar('.');
        fflush(stdout);
    }
    printf("\n");
    free(buf);
    gettimeofday(&tvpost, NULL);
    transferTime = (tvpost.tv_sec - tvpre.tv_sec) +
        (tvpost.tv_usec - tvpre.tv_usec) * 1e-9;
    if( transferTime < 1.0 )
        printf("%lld kB transferred\n", count / 2);
    else
        printf("%lld kB transferred in %.0f seconds, %.0f kB/s\n",
                count / 2, transferTime, count / transferTime / 2);
}

static void cmd_partitions(void)
{
    MBR mbr;
    struct flashmem_properties *props = get_flashmem_props();
    unsigned long long dsize;
    int i, pgsize;

    printf("flash properties:\n");
    printf("        page        pages      blocks     chip          pages    block count\n");
    printf("     kB-size    per block    per chip    count     marked bad        of zone\n");
    printf("    %8u     %8u    %8u %8u       %8u       %8u\n",
            props->sectors_per_page / 2, props->pages_per_block,
            props->blocks_per_chip, props->chip_cnt, props->pagewithbadflag,
            props->block_cnt_of_zone);
    printf("\n");
    printf("flash areas:\n");
    printf("                                 page       erase block\n");
    printf("    NAME          kB-size     kB-size           kB-size\n");
    for(i = 0; i < FMAREA_COUNT; ++i) {
        const char *area_name = "unknown";
        switch(i) {
        case FMAREA_BOOT0:      area_name = "boot0"; break;
        case FMAREA_BOOT1:      area_name = "boot1"; break;
        case FMAREA_PHYDISK:    area_name = "disk-phy"; break;
        case FMAREA_LOGDISK:    area_name = "disk-logic"; break;
        }
        pgsize = props->areas[i].sectors_per_page;
        dsize = (unsigned long long)props->areas[i].page_count * pgsize;
        printf("    %-10s   %8llu    %8u%s",
                area_name, dsize / 2, pgsize / 2, pgsize & 1 ? ".5" : "  ");
        if( i != FMAREA_LOGDISK )
            printf("        %8u", pgsize * props->pages_per_block / 2);
        printf("\n");
    }
    printf("\n");
    if( read_mbr(&mbr) ) {
        printf("logic disk partitions:                                "
                "(mbr ver: %x)\n", mbr.version);
        if( mbr.PartCount == 0 ) {
            printf("No partitions.\n");
        }else{
            printf(" #  NAME        kB-offset   kB-length     ro   usertype        class\n");
            for(i = 0; i < mbr.PartCount; ++i) {
                printf("%2d  %-12.12s %8lld%s  %8lld%s %4d   %8d %12.12s\n",
                        i,
                        mbr.array[i].name,
                        (((unsigned long long)mbr.array[i].addrhi << 32)
                            + mbr.array[i].addrlo) / 2,
                        mbr.array[i].addrlo & 1 ? ".5" : "  ",
                        (((unsigned long long)mbr.array[i].lenhi << 32)
                            + mbr.array[i].lenlo) / 2,
                        mbr.array[i].lenlo & 1 ? ".5" : "  ",
                        mbr.array[i].ro,
                        mbr.array[i].user_type,
                        mbr.array[i].classname);
            }
        }
    }
    printf("\n");
}

static void cmd_board_exit(enum BoardExitMode mode)
{
    send_command(BCMD_BOARD_EXIT, mode, 0, 0, NULL, 0);
    print_response_data();
    printf("\n");
}

static void cmd_log(const char *par)
{
    int doclear = par != NULL && par[0] == 'c';
    send_command(BCMD_GETLOG, doclear, 0, 0, NULL, 0);
    printf("log size: %d\n\n", cur_resp_header.datasize);
    print_response_data();
    printf("\n");
}

/* sleep time test */
static void cmd_sdelay(const char *tm)
{
    send_command(BCMD_SLEEPTEST,
            tm == NULL ? 1000000 : strtoul(tm, NULL, 0), 0, 0, NULL, 0);
    printf("OK\n");
}

static void cmd_ping(void)
{
    send_command(BCMD_PING, 0, 0, 0, NULL, 0);
    printf("OK\n");
}

static void cmd_memread(int argc, char *argv[])
{
    unsigned address, size, toRdTot;
    int fd;
    char buf[0x40000], *endptr;

    if( argc != 3 ) {
        printf("error: wrong number of parameters\n");
        return;
    }
    address = strtoul(argv[0], &endptr, 0);
    if( *endptr ) {
        printf("wrong address\n");
        return;
    }
    size = strtoul(argv[1], &endptr, 0);
    if( *endptr == 'k' )
        size *= 0x400;
    else if( *endptr == 'M' )
        size *= 0x100000;
    fd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0664);
    if( fd < 0 ) {
        printf("failed to open %s: %s\n", argv[2], strerror(errno));
        return;
    }
    toRdTot = size;
    while( toRdTot > 0 ) {
        unsigned toRead = sizeof(buf);
        if( toRdTot < toRead )
            toRead = toRdTot;
        send_command(BCMD_MEMREAD, 0, address, toRead, NULL, 0);
        get_response_data(buf, toRead);
        if( write(fd, buf, toRead) != toRead ) {
            printf("file write error\n");
            close(fd);
            return;
        }
        address += toRead;
        toRdTot -= toRead;
    }
    printf("%d bytes transferred\n", size);
}

static void cmd_memdump(int argc, char *argv[])
{
    unsigned address, size, i, prInLine, prMaxInLine = 20;
    char buf[0x4000], *endptr;
    const char *fmt = "%4d  ";

    if( argc != 1 && argc != 2 ) {
        printf("error: wrong number of parameters\n");
        return;
    }
    address = strtoul(argv[0], &endptr, 0);
    if( *endptr ) {
        printf("wrong address\n");
        return;
    }
    if( argv[0][0] == '0' && argv[0][1] != '\0' ) {
        if( argv[0][1] == 'x' || argv[0][1] == 'X' )
            fmt = "%04X  ";
        else
            fmt = "%04o  ";
        prMaxInLine = 16;
    }
    if( argc == 2 ) {
        size = strtoul(argv[1], &endptr, 0);
        if( *endptr == 'k' )
            size *= 0x400;
        else if( *endptr == 'M' )
            size *= 0x100000;
    }else{
        size = prMaxInLine == 20 ? 260 : 256;
    }
    printf(fmt, address);
    prInLine = 0;
    while( size > 0 ) {
        unsigned toRead = sizeof(buf);
        if( size < toRead )
            toRead = size;
        send_command(BCMD_MEMREAD, 0, address, toRead, NULL, 0);
        get_response_data(buf, toRead);
        for(i = 0; i < toRead; ++i) {
            if(prInLine == prMaxInLine) {
                printf("\n");
                printf(fmt, address+i);
                prInLine = 0;
            }
            if( buf[i] >= 32 && buf[i] < 127 ) {
                printf("  %c", buf[i]);
            }else if( buf[i] == '\r' ) {
                printf(" \\r");
            }else if( buf[i] == '\n' ) {
                printf(" \\n");
            }else{
                printf(" %02X", (unsigned char)buf[i]);
            }
            ++prInLine;
        }
        address += toRead;
        size -= toRead;
    }
    printf("\n");
}

static void cmd_memfill(int argc, char *argv[])
{
    unsigned address, size, repeat, i, sizeOnce, fillLen;
    char buf[0x4000], *dest, *endptr;
    const char *fill;

    if( argc != 2 && argc != 3 ) {
        printf("error: wrong number of parameters\n");
        return;
    }
    address = strtoul(argv[0], &endptr, 0);
    if( *endptr ) {
        printf("wrong address\n");
        return;
    }
    fill = argv[1];
    if( argc == 3 ) {
        repeat = 0;
        size = strtoul(argv[2], &endptr, 0);
        if( *endptr == 'k' )
            size *= 0x400;
        else if( *endptr == 'M' )
            size *= 0x100000;
        else if( *endptr == 'x' )
            repeat = 1;
    }else{
        size = 1;
        repeat = 1;
    }
    dest = buf;
    while( *fill ) {
        if( dest - buf >= sizeof(buf) - 1 ) {
            printf("fill string too long\n");
            return;
        }
        if( *fill == '\\' ) {
            switch( *++fill ) {
            case 'a':   *dest++ = '\a'; ++fill; break;
            case 'b':   *dest++ = '\b'; ++fill; break;
            case 'f':   *dest++ = '\f'; ++fill; break;
            case 'n':   *dest++ = '\n'; ++fill; break;
            case 'r':   *dest++ = '\r'; ++fill; break;
            case 't':   *dest++ = '\t'; ++fill; break;
            case 'v':   *dest++ = '\v'; ++fill; break;
            case '\\':  *dest++ = '\\'; ++fill; break;
            case '\0':  *dest++ = '\\'; break;
            case 'x':
                ++fill;
                {
                    unsigned val = 0;
                    for(i = 0; i < 2 && ((*fill >= '0' && *fill <= '9') ||
                            (*fill >= 'A' && *fill <= 'F') ||
                            (*fill >= 'a' && *fill <= 'f')); ++i)
                    {
                        unsigned char c = *fill++;
                        val = 16 * val + c - (c >= '0' && c <= '9' ?  '0' :
                                (c >= 'A' && c <= 'F' ? 'A' : 'a') - 10);
                    }
                    *dest++ = val;
                }
                break;
            default:
                if( *fill >= '0' && *fill <= '7' ) {
                    unsigned val = 0;
                    for(i = 0; i < 3 && *fill >= '0' && *fill <= '7'; ++i) {
                        val = 8 * val + *fill++ - '0';
                    }
                    *dest++ = val;
                }else{
                    *dest++ = '\\';
                    *dest++ = *fill++;
                }
                break;
            }
        }else
            *dest++ = *fill++;
    }
    fillLen = dest - buf;
    if( fillLen == 0 ) {
        printf("error: empty fill string\n");
        return;
    }
    if( repeat )
        size *= fillLen;
    sizeOnce = fillLen;
    while( sizeOnce < size && 2 * sizeOnce < sizeof(buf) ) {
        memcpy(buf + sizeOnce, buf, sizeOnce);
        sizeOnce *= 2;
    }
    while( size > 0 ) {
        if( sizeOnce > size )
            sizeOnce = size;
        send_command(BCMD_MEMWRITE, 0, address, 0, buf, sizeOnce);
        address += sizeOnce;
        size -= sizeOnce;
    }
}

/* upload file to memory at the specified address
 * if defaultAddr is non-zero, the file should be a eGON image;
 * in this case upload address is default address of the image,
 * i.e. 0 for eGON.BT0, 0x42400000 for eGON.BT1
 */
static unsigned mem_write(const char *fname, int defaultAddr, unsigned startAddr)
{
    unsigned address, wrTot;
    int fd, rd;
    char buf[0x40000];

    fd = open(fname, O_RDONLY);
    if( fd < 0 ) {
        printf("failed to open %s: %s\n", fname, strerror(errno));
        return 0xffffffff;
    }
    wrTot = 0;
    address = startAddr;
    while( (rd = read(fd, buf, sizeof(buf))) > 0 ) {
        if( defaultAddr ) {
            if( rd >= 20 && ! memcmp(buf + 4, "eGON.BT0", 8) ) {
                startAddr = 0;
            }else if( rd >= 20 && !memcmp(buf + 4, "eGON.BT1", 8) ) {
                startAddr = 0x42400000;
            }else{
                printf("error: %s is not eGON image\n", fname);
                close(fd);
                return 0xffffffff;
            }
            address = startAddr;
            defaultAddr = 0;
        }
        send_command(BCMD_MEMWRITE, 0, address, 0, buf, rd);
        address += rd;
        wrTot += rd;
    }
    close(fd);
    if( wrTot < 1024 )
        printf("%d bytes transferred\n", wrTot);
    else
        printf("%d%s kB transferred\n", wrTot / 1024, wrTot % 1024 ? "+" : "");
    return startAddr;
}

static void cmd_memwrexec(int argc, char *argv[], enum BootdiskCommand cmd)
{
    unsigned address;

    if( argc != 1 && argc != 2 ) {
        printf("error: wrong number of parameters\n");
        return;
    }
    address = strtoul(argv[0], NULL, 0);
    if( (argc == 1 || mem_write(argv[1], 0, address) != 0xffffffff) &&
            cmd != BCMD_NONE )
    {
        send_command(cmd, 0, address, 0, NULL, 0);
        if( cmd == BCMD_MEMEXEC )
            printf("OK\n");
        else
            printf("jumped to 0x%x...\n", address);
    }
}

static void cmd_egonjumpto(const char *fname)
{
    unsigned address;

    if( fname == NULL ) {
        printf("no image file provided\n");
        return;
    }
    address = mem_write(fname, 1, 0);
    if( address != 0xffffffff ) {
        send_command(BCMD_MEMJUMPTO, 0, address, 0, NULL, 0);
        printf("jumped to 0x%x...\n", address);
    }
}

int main(int argc, char *argv[])
{
    int i;

    if( argc == 1 ) {
        printf("usage:\n");
        printf("    allwindisk r <partname> [<offsetkB> <sizekB>] <file>    - read partition\n");
        printf("    allwindisk w <partname> [<offsetkB>]          <file>    - write partition\n");
        printf("    allwindisk p                    - print existing disk partitions\n");
        printf("    allwindisk i                    - ping (check if alive)\n");
        printf("    allwindisk g <eGON_imgfile>     - load the image into memory and jump to it\n");
        printf("    allwindisk q                    - board reset\n");
        printf("    allwindisk f                    - go back to FEL mode\n");
        printf("    allwindisk l                    - print debug log from device\n");
        printf("    allwindisk e  <file>            - embed eGON image (must have valid magic\n");
        printf("                                      and the place for size and checksum)\n");
        printf("    allwindisk mr <address> <size[k|M]> <file>  - memory read\n");
        printf("    allwindisk mw <address> <file>  - memory write\n");
        printf("    allwindisk mx <address> [<file>]- optionally load file to the specified\n");
        printf("                                      address; execute code\n");
        printf("    allwindisk mj <address> [<file>]- optionally load file to the specified\n");
        printf("                                      address; jump to code\n");
        printf("    allwindisk md <address> [<size[k|M]>]       - memory dump\n");
        printf("    allwindisk mf <address> <string> [<size[k|M|x]>] - memory fill\n");
        printf("\n");
        printf("The <partname> may be a pseudo-partition, one of: boot0, boot1, disk-logic\n");
        printf("\n");
        return 0;
    }
    if( libusb_init(NULL) != 0 ) {
        printf("libusb_init fail\n");
        return 1;
    }
    if( (usb = libusb_open_device_with_vid_pid(NULL, 0xbb4, 0xfff)) == NULL )
    {
        printf("open device failed, trying to fel...");
        fflush(stdout);
        if( fel_ainol() != 0 ) {
            fprintf(stderr, "\nfailed to open A10 USB FEL device\n");
            return 1;
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
            return 1;
        }
    }
    libusb_claim_interface(usb, 0);
    switch( argv[1][0] ) {
    case 'e':
        // not implemented yet
        break;
    case 'f':
        cmd_board_exit(BE_GO_FEL);
        break;
    case 'g':
        cmd_egonjumpto(argv[2]);
        break;
    case 'i':
        cmd_ping();
        break;
    case 'l':
        cmd_log(argv[2]);
        break;
    case 'm':
        switch( argv[1][1] ) {
        case 'r':
            cmd_memread(argc-2, argv+2);
            break;
        case 'd':
            cmd_memdump(argc-2, argv+2);
            break;
        case 'f':
            cmd_memfill(argc-2, argv+2);
            break;
        case 'w':
            cmd_memwrexec(argc-2, argv+2, BCMD_NONE);
            break;
        case 'j':
            cmd_memwrexec(argc-2, argv+2, BCMD_MEMJUMPTO);
            break;
        case 'x':
            cmd_memwrexec(argc-2, argv+2, BCMD_MEMEXEC);
            break;
        }
        break;
    case 'p':
        cmd_partitions();
        break;
    case 'q':
        cmd_board_exit(BE_BOARD_RESET);
        break;
    case 'r':
        cmd_diskread(argc - 2, argv + 2);
        break;
    case 'w':
        cmd_diskwrite(argc - 2, argv + 2);
        break;
    case 'S':
        cmd_sdelay(argv[2]);
        break;
    default:
        printf("unknown command %s\n", argv[1]);
        break;
    }
    libusb_release_interface(usb, 0);
    libusb_close(usb);
    libusb_exit(NULL);
    return 0;
}
