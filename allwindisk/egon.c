#include "allwindisk.h"
#include <stdio.h>
#include <string.h>
#include <libusb.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

static unsigned round_up(unsigned val, unsigned round)
{
    if( val % round )
        val += round - val % round;
    return val;
}

static void setBootImgChecksum(char *image, int imageSize)
{
    uint32_t val = htole32(0x5F0A6C39);
    unsigned checksum = 0;
    char *addr = image;

    memcpy(image+12, &val, 4);
    while(addr - image < imageSize) {
        memcpy(&val, addr, 4);
        checksum += le32toh(val);
        addr += 4;
    }
    val = htole32(checksum);
    memcpy(image+12, &val, 4);
}


static void egon0_embed(const char *image, unsigned imageSize)
{
    struct flashmem_properties *props = get_flashmem_props();
    unsigned pageSize = props->areas[FMAREA_BOOT0].sectors_per_page * 512;
    unsigned areaSize = props->areas[FMAREA_BOOT0].page_count * pageSize;
    unsigned i, repCount;
    char *buf;
    uint32_t imgSizeLe;

    if( imageSize > areaSize ) {
        printf("error: boot0 image file too big\n");
        return;
    }
    buf = malloc(areaSize);
    memcpy(buf, image, imageSize);
    memset(buf + imageSize, '\xff', round_up(imageSize, 1024));
    imageSize = round_up(imageSize, 1024);
    imgSizeLe = htole32(imageSize);
    memcpy(buf + 16, &imgSizeLe, 4);
    setBootImgChecksum(buf, imageSize);
    memset(buf + imageSize, '\xff', round_up(imageSize, 64 * pageSize));
    imageSize = round_up(imageSize, 64 * pageSize);
    repCount = areaSize / imageSize;
    for(i = 1; i < repCount; ++i) {
        memcpy(buf + i * imageSize, buf, imageSize);
    }
    memset(buf + repCount * imageSize, '\xff', areaSize - repCount * imageSize);
    write_disk(FMAREA_BOOT0, 0, areaSize / 512, buf);
    free(buf);
}

int egonfile_embed(const char *fname)
{
    int fd, imageSize;
    char buf[0x200001];

    fd = open(fname, O_RDONLY);
    if( fd < 0 ) {
        printf("failed to open %s: %s\n", fname, strerror(errno));
        return -1;
    }

    if( (imageSize = read(fd, buf, sizeof(buf))) < 0 ) {
        printf("image file read failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);
    if( imageSize == sizeof(buf) ) {
        printf("image file too big\n");
        return -1;
    }
    if( imageSize >= 20 && ! memcmp(buf + 4, "eGON.BT0", 8) ) {
        egon0_embed(buf, imageSize);
    }else if( imageSize >= 20 && !memcmp(buf + 4, "eGON.BT1", 8) ) {
        printf("error: boot1 image embedding not implemented\n");
    }else{
        printf("error: %s is not eGON image\n", fname);
        return -1;
    }
    return 0;
}

