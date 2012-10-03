#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

/* Android bootimage file format (from u-boot/fastboot.h) */
#define FASTBOOT_BOOT_MAGIC "ANDROID!"
#define FASTBOOT_BOOT_MAGIC_SIZE 8
#define FASTBOOT_BOOT_NAME_SIZE 16
#define FASTBOOT_BOOT_ARGS_SIZE 512

struct fastboot_boot_img_hdr {
	unsigned char magic[FASTBOOT_BOOT_MAGIC_SIZE];

	unsigned kernel_size;  /* size in bytes */
	unsigned kernel_addr;  /* physical load addr */

	unsigned ramdisk_size; /* size in bytes */
	unsigned ramdisk_addr; /* physical load addr */

	unsigned second_size;  /* size in bytes */
	unsigned second_addr;  /* physical load addr */

	unsigned tags_addr;    /* physical addr for kernel tags */
	unsigned page_size;    /* flash page size we assume */
	unsigned unused[2];    /* future expansion: should be 0 */

	unsigned char name[FASTBOOT_BOOT_NAME_SIZE]; /* asciiz product name */

	unsigned char cmdline[FASTBOOT_BOOT_ARGS_SIZE];

	unsigned id[8]; /* timestamp / checksum / sha1 / etc */
};

static void extract_img(const char *image_name, const char *fname,
        int fd, unsigned offset, unsigned size)
{
    char out_fname[1024], buf[32768];
    int fdOut, rd;

    if( size == 0 )
        return;
    if( lseek(fd, offset, SEEK_SET) < 0 ) {
        fprintf(stderr, "lseek: %s\n", strerror(errno));
        return;
    }
    rd = read(fd, buf, sizeof(buf) > size ? size : sizeof(buf));
    if( rd > 0 ) {
        snprintf(out_fname, sizeof(out_fname), "%s_%s", image_name, fname);
        printf("extracting %s into %s\n", image_name, out_fname);
        if( (fdOut = open(out_fname, O_WRONLY | O_CREAT, 0664)) < 0 ) {
            fprintf(stderr, "unable to create %s: %s\n", out_fname,
                    strerror(errno));
            return;
        }
        while( rd > 0 ) {
            if( write(fdOut, buf, rd) != rd ) {
                fprintf(stderr, "write: %s\n", strerror(errno));
                close(fdOut);
                return;
            }
            size -= rd;
            if( size > 0 ) {
                rd = read(fd, buf, sizeof(buf) > size ? size : sizeof(buf));
            }else{
                rd = 0;
            }
        }
        close(fdOut);
    }
    if( rd < 0 ) {
        fprintf(stderr, "read: %s\n", strerror(errno));
    }

}

int main(int argc, char *argv[])
{
    int fd, rd;
    struct fastboot_boot_img_hdr hdr;
    int extract;
    const char *fname;

    if( argc == 1 ) {
        printf("usage: %s [-e] <image file>\n", argv[0]);
        printf("    -e      - extract kernel and initrd\n");
        return 0;
    }
    fname = argv[1];
    if( extract = !strcmp(argv[1], "-e") ) {
        if( argc < 3 ) {
            printf("file name?\n");
            return 1;
        }
        fname = argv[2];
    }
    if( (fd = open(fname, O_RDONLY)) < 0 ) {
        fprintf(stderr, "failed to open %s: %s\n", fname, strerror(errno));
        return 1;
    }
    memset(&hdr, 0, sizeof(hdr));
    if( (rd = read(fd, &hdr, sizeof(hdr))) < 0 ) {
        fprintf(stderr, "fatal: read failed: %s\n", strerror(errno));
        return 1;
    }
    if( rd < sizeof(hdr) ) {
        printf("warn: header is truncated, file size=%d, header size=%d\n",
                rd, sizeof(hdr));
    }
    if( memcmp(hdr.magic, FASTBOOT_BOOT_MAGIC, FASTBOOT_BOOT_MAGIC_SIZE) ) {
        printf("warn: file doest not seem to be a fastboot image\n");
    }
	printf("    kernel size (bytes):  %u\n", hdr.kernel_size);
	printf("    kernel addr:          0x%x\n", hdr.kernel_addr);
	printf("    ramdisk size (bytes): %u\n", hdr.ramdisk_size);
	printf("    ramdisk addr:         0x%x\n", hdr.ramdisk_addr);
	printf("    second size:          %u\n", hdr.second_size);
	printf("    second addr:          0x%x\n", hdr.second_addr);
	printf("    tags addr:            0x%x\n", hdr.tags_addr);
	printf("    page size:            %u\n", hdr.page_size);
	printf("    unused1:              %u\n", hdr.unused[0]);
	printf("    unused2:              %u\n", hdr.unused[1]);
	printf("    product name:         %.*s\n", FASTBOOT_BOOT_NAME_SIZE, hdr.name);
	printf("    cmdline:              %.*s\n", FASTBOOT_BOOT_ARGS_SIZE,
            hdr.cmdline);
    printf("\n");
    if( extract && hdr.page_size > 0 ) {
        extract_img("bImage", fname, fd, hdr.page_size, hdr.kernel_size);
        extract_img("initrd", fname, fd, 2 * hdr.page_size + hdr.kernel_size -
                hdr.kernel_size % hdr.page_size, hdr.ramdisk_size);
        printf("done.\n\n");
    }
    return 0;
}
