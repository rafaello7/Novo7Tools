#include "wlibc.h"
#include "syscalls.h"
#include "picture.h"
#include "bootos.h"
#include "script.h"
#include "bootpara.h"
#include "string.h"


struct ImageHeader {
    int param0;
    int param4;
    union {
        char param8c[4];
        int  param8i[1];
    };
};

static struct ImageHeader *L4280A3BC;

static int fnL428015CC(void)
{
    int var4;
    int var5;
    int var6;
    int sp0;
    int sp4;

    var6 = gBootPara.bpparam0[2];
    wlibc_uprintf("key value = %d\n", var6);
    var4 = svc_b0("recovery_key", "key_max", &sp4, 1);
    var5 = svc_b0("recovery_key", "key_min", &sp0, 1);
    if(var4 != 0) {
        wlibc_uprintf("unable to find recovery_key key_max value\n");
        return -1;

    }
    if(var5 != 0) {
        wlibc_uprintf("unable to find recovery_key key_min value\n");
        return -1;
    }
    wlibc_uprintf("recovery key high %d, low %d\n", sp4, sp0);
    if(var6 >= sp0) {
        if(var6 <= sp4) {
            wlibc_uprintf("key found, android recovery\n");
            return 2;
        }
    }
    var4 = svc_b0("fastboot_key", "key_max", &sp4, 1);
    var5 = svc_b0("fastboot_key", "key_min", &sp0, 1);
    if(var4 != 0) {
        wlibc_uprintf("unable to find fastboot_key key_max value\n");
        return -1;
    }
    if(var5 != 0) {
        wlibc_uprintf("unable to find fastboot_key key_min value\n");
        return -1;
    }
    wlibc_uprintf("fastboot key high %d, low %d\n", sp4, sp0);
    if(var6 >= sp0) {
        if(var6 <= sp4) {
            wlibc_uprintf("key found, android fastboot\n");
            return 1;
        }
    }
    wlibc_uprintf("key invalid\n");
    return -1;
}

static int fnL42801848(const char *var4, int var5, void *var6)
{
    if(var5 == 0 || var4 == 0) {
        wlibc_uprintf("logo name is invalid or dont need show logo\n");
        return -1;
    }
    ShowPictureEx(var4, var6);
    wlibc_uprintf("show pic finish\n");
    return 0;
}

static int fnL42801F44(void)
{
    L4280A3BC->param4 = 0;
    L4280A3BC->param0 = 0;
    return 0;
}

static void fnL42801F64(char *var4, int var6)
{
    char *var5;

    if(var4 == 0) {
        return;
    }
    var5 = var4;

    while(*var5 == ' ') {
        ++var5;
    }
    if(*var5 == 0)
        return;
    var4[var6] = '\0';
    L4280A3BC->param4 = 0x54410009;
    L4280A3BC->param0 = (strlen(var5) + 13) >> 2;
    strcpy(L4280A3BC->param8c, var5);
    L4280A3BC = (void*)((int*)L4280A3BC + L4280A3BC->param0);
}

static void fnL42802010(struct ImageHeader *imgHeader)
{
    L4280A3BC = imgHeader;
    L4280A3BC->param4 = 0x54410001;
    L4280A3BC->param0 = 5;
    L4280A3BC->param8i[0] = 0;
    L4280A3BC->param8i[1] = 0;
    L4280A3BC->param8i[2] = 0;
    L4280A3BC = (void*)(((int*)&L4280A3BC) + L4280A3BC->param0);
}

int do_boot_linux(struct ImageHeader *imgHeader, char *loadAddr, int var6)
{
    fnL42802010(imgHeader);
    fnL42801F64(loadAddr, var6);
    fnL42801F44();
    return 0;
}

static int fnL42801894(struct OSImageScript *imgScript,
        void **bootAddrBuf,
        void **var6)
{
    int fdesc = 0;
    int imgNo;
    int fileSize = 0;
    void *loadAddr;

    wlibc_uprintf("load kernel start\n");
    for(imgNo = 0; imgNo < imgScript->segmentSectionCount; ++imgNo) {
        if(imgNo != 1) {
            loadAddr = imgScript->images[imgNo].baseAddr;
        } else { 
            loadAddr = svc_alloc(65536);
        }
        if(loadAddr == 0) {
            wlibc_uprintf("img file %s base addres is NULL\n", loadAddr);
            return -1;
        }
        fdesc = svc_open(imgScript->images[imgNo].name, "rb");
        if(fdesc == 0) {
            wlibc_uprintf("open img file %s failed\n",
                    imgScript->images[imgNo].name);
            goto fail;
        }
        fileSize = svc_filesize(fdesc);
        if( fileSize > imgScript->images[imgNo].maxSize ) {
            wlibc_uprintf(
                "the img file %s length %d is larger than img max size %d\n",
                imgScript->images[imgNo].name, fileSize,
                imgScript->images[imgNo].maxSize);
            goto fail;

        }
        svc_read(loadAddr, 1, fileSize, fdesc);
        svc_close(fdesc);
        imgScript->images[imgNo].fileLen = fileSize;
        if(imgNo == 1) {
            do_boot_linux(imgScript->images[1].baseAddr, loadAddr, fileSize);
            svc_free(loadAddr);
        }
        loadAddr = 0;
        fdesc = 0;
        fileSize = 0;
    }
    loadAddr = imgScript->baseAddr;
    if(loadAddr == 0) {
        wlibc_uprintf("no script could be filed\n");
    } else { 
        /* 0x42F00000 - c:\script.bin contents */
        memcpy(loadAddr, (const void*)0x42F00000, imgScript->maxSize);
        imgScript->param776 = imgScript->maxSize;
    }
    *bootAddrBuf = imgScript->images[0].baseAddr;
    *var6 = imgScript->images[1].baseAddr;
    wlibc_uprintf("load kernel successed\n");
    return 0;

fail:
    if(fdesc != 0) {
        svc_close(fdesc);
    }
    wlibc_uprintf("load kernel failed\n");
    return -1;
}

int PreBootOS(struct OSImageScript *imgScript, void **bootAddrBuf,
        void **var6, int *logoOff)
{
    int var9;

    var9 = -1;
    *logoOff = imgScript->logoOff;
    fnL42801848(imgScript->logoFileName, imgScript->logoShow, imgScript->logoAddress);
    var9 = fnL42801894(imgScript, bootAddrBuf, var6);
    if(var9 >= 0) {
        wlibc_uprintf("start address = %x\n", *bootAddrBuf);
        return 0;
    }
    return -1;
}

struct PARTITION {
	unsigned            addrhi;
	unsigned            addrlo;
	unsigned            lenhi;
	unsigned            lenlo;
	char                classname[12];
	char                name[12];
	unsigned            user_type;
	unsigned            ro;
	char                res[16];
} __attribute__ ((packed));

struct MBR
{
	unsigned            crc32;
	unsigned            version;
	char 	            magic[8];
	unsigned  char 	    copy;
	unsigned  char 	    index;
	unsigned short      PartCount;
	struct PARTITION    array[15];
	char                res[1024 - 20 - (15 * sizeof(struct PARTITION))];
} __attribute__ ((packed));


int BootOS_detect_os_type(void **var4, void **bootAddrBuf,
        struct BootIni *bootIni, int *logoOff)
{
    int var8;
    unsigned vasl;
    int sp4;
    char sp8[1024];
    struct MBR sp2048;
    struct OSImageScript imgScript;
    int sp3916;

    var8 = -1;
    sp3916 = 0;
    memset(&imgScript, 0, sizeof(imgScript));
    sp3916 = fnL428015CC();
    if(sp3916 > 0) {
        vasl = 0;
        svc_diskread(0, 2, &sp2048);
        for(sp4 = 0; sp4 < sp2048.PartCount; ++sp4) {
            if(strcmp("misc", sp2048.array[sp4].name) == 0) {
                vasl = sp2048.array[sp4].addrlo;
                svc_diskread(vasl, 1, sp8);
                memset(sp8, 0, 32);
                if(sp3916 == 1) {
                    strcpy(sp8, "boot-fastboot");
                    wlibc_uprintf("fastboot mode\n");
                } else if(sp3916 == 2) {
                    strcpy(sp8, "boot-recovery");
                    wlibc_uprintf("recovery mode\n");
                }
                svc_diskwrite(vasl, 1, sp8);
                break;
            }
        }
    }
    var8 = script_patch("c:\\linux\\linux.ini", &imgScript, 1);
    if(var8 < 0) {
        wlibc_uprintf("NO OS to Boot\n");
    } else { 
        wlibc_uprintf("test for multi os boot with display\n");
        var8 = PreBootOS(&imgScript, bootAddrBuf, var4, logoOff);
    }
    svc_free(bootIni);
    return var8;
}

void BootOS(void *var4, void *bootAddr)
{
    volatile int var0;

    wlibc_uprintf("jump to\n");
    svc_bootos(0, 3495, var4, bootAddr);
    var0 = 85;
    while( var0 == 85 ) {
    }
}

