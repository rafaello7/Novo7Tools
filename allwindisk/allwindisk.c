#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include "allwindisk.h"


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
                write_disk(fmarea, firstSector, toWrMax - count, buf);
                count = toWrMax;
            }
            break;
        }
        write_disk(fmarea, firstSector, toWr, buf);
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
    struct flashmem_properties *props = get_flashmem_props();
    struct MbrPartition *part;
    unsigned long long dsize;
    int i, partCount, pgsize;

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
    partCount = getPartitionCount();
    if( partCount >= 0 ) {
        printf("logical disk partitions:                              "
                "(mbr ver: %x)\n", getMbrVersion());
        if( partCount == 0 ) {
            printf("No partitions.\n");
        }else{
            printf(" #  NAME        kB-offset   kB-length     ro   usertype        class\n");
            for(i = 0; i < partCount; ++i) {
                part = getPartitionNoParams(i);
                printf("%2d  %-12.12s %8lld%s  %8lld%s %4d   %8d %12.12s\n",
                        i,
                        part->name,
                        part->firstSector / 2,
                        part->firstSector & 1 ? ".5" : "  ",
                        part->sectorCount / 2,
                        part->sectorCount & 1 ? ".5" : "  ",
                        part->ro,
                        part->user_type,
                        part->classname);
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
    int logSize = send_command(BCMD_GETLOG, doclear, 0, 0, NULL, 0);
    printf("log size: %d\n\n", logSize);
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

static void prdump_addr(unsigned address, const char *fmtParam,
        unsigned *prInLine)
{
    int prMaxInLine = 20;
    const char *fmt = "%4d  ";

    if( fmtParam != NULL && fmtParam[0] == '0' && fmtParam[1] != '\0' ) {
        if( fmtParam[1] == 'x' || fmtParam[1] == 'X' )
            fmt = "%04X  ";
        else
            fmt = "%04o  ";
        prMaxInLine = 16;
    }
    if(*prInLine == 0 || *prInLine >= prMaxInLine) {
        if( *prInLine != 0 )
            printf("\n");
        printf(fmt, address);
        *prInLine = 0;
    }
    ++*prInLine;
}

static void prdump_char(char c)
{
    if( c >= 32 && c < 127 ) {
        printf("  %c", c);
    }else if( c == '\r' ) {
        printf(" \\r");
    }else if( c == '\n' ) {
        printf(" \\n");
    }else{
        printf(" %02X", (unsigned char)c);
    }
}

static void cmd_memdump(int argc, char *argv[])
{
    unsigned address, size, i, prInLine = 0;
    char buf[0x4000], *endptr;

    if( argc != 1 && argc != 2 ) {
        printf("error: wrong number of parameters\n");
        return;
    }
    address = strtoul(argv[0], &endptr, 0);
    if( *endptr ) {
        printf("wrong address\n");
        return;
    }
    if( argc == 2 ) {
        size = strtoul(argv[1], &endptr, 0);
        if( *endptr == 'k' )
            size *= 0x400;
        else if( *endptr == 'M' )
            size *= 0x100000;
    }else{
        size = 256;
    }
    while( size > 0 ) {
        unsigned toRead = sizeof(buf);
        if( size < toRead )
            toRead = size;
        send_command(BCMD_MEMREAD, 0, address, toRead, NULL, 0);
        get_response_data(buf, toRead);
        for(i = 0; i < toRead; ++i) {
            prdump_addr(address+i, argv[0], &prInLine);
            prdump_char(buf[i]);
        }
        address += toRead;
        size -= toRead;
    }
    printf("\n");
}

static void cmd_diskdump(int argc, char *argv[])
{
    enum FlashMemoryArea fmarea;
    unsigned long long firstSector, count, partOffset, partSize;
    unsigned i, prInLine = 0;
    char buf[0x4000], *endptr;

    if( argc < 1 || argc > 3 ) {
        printf("error: wrong number of parameters\n");
        return;
    }
    if( ! getParitionParams(argv[0], &fmarea, &firstSector, &partSize) )
        return;
    count = 1;
    if( argc > 1 ) {
        partOffset = strtoull(argv[1], &endptr, 0) << 1;
        if( *endptr ) {
            printf("wrong address\n");
            return;
        }
        if( argc > 2 ) {
            count = strtoull(argv[2], NULL, 0) << 1;
        }
        if( partOffset + count > partSize ) {
            printf("error: requested read beyond disk size\n");
            return;
        }
    }else{
        partOffset = 0;
    }
    while( count > 0 ) {
        unsigned toRead = sizeof(buf) >> 9;
        if( count < toRead )
            toRead = count;
        read_disk(fmarea, firstSector + partOffset, toRead, buf);
        for(i = 0; i < toRead << 9; ++i) {
            prdump_addr((partOffset << 9) + i, argv[1], &prInLine);
            prdump_char(buf[i]);
        }
        partOffset += toRead;
        count -= toRead;
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

static void cmd_exposepart(int argc, char *argv[])
{
    enum FlashMemoryArea fmarea;
    unsigned long long firstSector, sectorCount;

    if( argc == 0 ) {
        printf("error: partition name not provided\n");
        return;
    }
    if( ! getParitionParams(argv[0], &fmarea, &firstSector, &sectorCount) )
        return;
    if( fmarea != FMAREA_LOGDISK ) {
        printf("error: only logical disk partitions are supported\n");
        return;
    }
    send_command(BCMD_MOUNT, argc == 2 && !strcmp(argv[1], "rw"),
            firstSector, sectorCount, NULL, 0);
}

int main(int argc, char *argv[])
{
    if( argc == 1 ) {
        printf("usage:\n");
        printf("    allwindisk dp                   - print existing disk partitions\n");
        printf("    allwindisk dr <partname> [<offsetkB> <sizekB>] <file>   - read partition\n");
        printf("    allwindisk dw <partname> [<offsetkB>]          <file>   - write partition\n");
        printf("    allwindisk dm <partname> [rw]   - make partition available as disk on host\n");
        printf("    allwindisk dd <partname> [<offsetkB> [<sizekB>]]        - dump partition\n");
        printf("    allwindisk dg <eGON_imgfile>    - embed eGON image (must have valid magic\n");
        printf("                                      and the place for size and checksum)\n");
        printf("    allwindisk mr <address> <size[k|M]> <file>  - memory read\n");
        printf("    allwindisk mw <address> <file>  - memory write\n");
        printf("    allwindisk mx <address> [<file>]- optionally load file to the specified\n");
        printf("                                      address; execute code\n");
        printf("    allwindisk mj <address> [<file>]- optionally load file to the specified\n");
        printf("                                      address; jump to code\n");
        printf("    allwindisk md <address> [<size[k|M]>]       - memory dump\n");
        printf("    allwindisk mf <address> <string> [<size[k|M|x]>] - memory fill\n");
        printf("    allwindisk mg <eGON_imgfile>    - load the image into memory and jump to it\n");
        printf("    allwindisk i                    - ping (check if alive)\n");
        printf("    allwindisk q                    - board reset\n");
        printf("    allwindisk f                    - go back to FEL mode\n");
        printf("    allwindisk l                    - print debug log from device\n");
        printf("\n");
        printf("The <partname> may be a pseudo-partition, one of: boot0, boot1, disk-logic\n");
        printf("\n");
        return 0;
    }
    if( usbcomm_init() != 0 )
        return 1;
    switch( argv[1][0] ) {
    case 'd':
        switch( argv[1][1] ) {
        case 'd':
            cmd_diskdump(argc-2, argv+2);
            break;
        case 'g':
            // not implemented yet
            break;
        case 'm':
            cmd_exposepart(argc-2, argv+2);
            break;
        case 'p':
            cmd_partitions();
            break;
        case 'r':
            cmd_diskread(argc - 2, argv + 2);
            break;
        case 'w':
            cmd_diskwrite(argc - 2, argv + 2);
            break;
        default:
            printf("unknown command %s\n", argv[1]);
            break;
        }
        break;
    case 'f':
        cmd_board_exit(BE_GO_FEL);
        break;
    case 'i':
        cmd_ping();
        break;
    case 'l':
        cmd_log(argv[2]);
        break;
    case 'm':
        switch( argv[1][1] ) {
        case 'd':
            cmd_memdump(argc-2, argv+2);
            break;
        case 'f':
            cmd_memfill(argc-2, argv+2);
            break;
        case 'g':
            cmd_egonjumpto(argv[2]);
            break;
        case 'j':
            cmd_memwrexec(argc-2, argv+2, BCMD_MEMJUMPTO);
            break;
        case 'r':
            cmd_memread(argc-2, argv+2);
            break;
        case 'w':
            cmd_memwrexec(argc-2, argv+2, BCMD_NONE);
            break;
        case 'x':
            cmd_memwrexec(argc-2, argv+2, BCMD_MEMEXEC);
            break;
        default:
            printf("unknown command %s\n", argv[1]);
            break;
        }
        break;
    case 'q':
        cmd_board_exit(BE_BOARD_RESET);
        break;
    case 'S':
        cmd_sdelay(argv[2]);
        break;
    default:
        printf("unknown command %s\n", argv[1]);
        break;
    }
    usbcomm_exit();
    return 0;
}
