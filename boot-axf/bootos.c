#include "wlibc.h"
#include "syscalls.h"
#include "picture.h"
#include "bootos.h"
#include "script.h"
#include "bootpara.h"
#include "string.h"
#include "fbprint.h"


struct ImageHeader {
    int param0;
    int param4;
    union {
        char param8c[4];
        int  param8i[1];
    };
};

static struct ImageHeader *gImageHeader;


/* volume down: 43
 * volume up:   30
 * back:        37
 */
static int GetKeyVal(int timeout)
{
    static int keyvalOld = -1;
    int keyval, rep;
    unsigned delayCounter = 1;
    static const struct ColorPair timeoutColors = { .bg = 0, .fg = 0xffffff };

    do {
        while( (keyval = svc_keyval()) == -1 ) {
            if( --delayCounter == 0 ) {
                if( timeout == 0 ) {
                    return -1;
                }else if( timeout > 0 ) {
                    char buf[40];
                    wlibc_sprintf(buf, "Timeout: %d ", timeout);
                    fbprint_line(-1, 40, &timeoutColors, 1, buf);
                    --timeout;
                }
                delayCounter = 50;
            }
            svc_delay(20);
        }
    }while(keyval == keyvalOld);
    if( timeout >= 0 ) {
        fbprint_line(-1, 40, &timeoutColors, 1, "               ");
    }
    while(keyvalOld != keyval) {
        keyvalOld = keyval;
        for(rep = 0; keyval == keyvalOld && rep < 5; ++rep) {
            while( (keyval = svc_keyval()) == -1 )
                ;
        }
    }
    return keyval;
}

static void PrintMenuItem(int lineNo, const char *item, int highlightLine)
{
    static const struct ColorPair menuColors[2] = {
        { .bg = 0, .fg = 0xffffff },
        { .bg = 0x0d1e54, .fg = 0xffffff }
    };
    char buf[28];
    int len;

    buf[0] = ' ';
    buf[1] = ' ';
    strncpy(buf+2, item, sizeof(buf) - 2);
    len = strlen(item) + 2;
    while( len < sizeof(buf) )
        buf[len++] = ' ';
    buf[sizeof(buf)-1] = '\0';
    fbprint_line(2*lineNo+2, 40, &menuColors[lineNo==highlightLine], 2, buf);
}

/* -4 - go fel
 * -3 - power off
 * -2 - boot recovery
 * -1 - boot fastboot
 *  >=0 - BootIni item number
 */
static int SelectBoot(struct BootIni *bootIni)
{
    int curItemNo = bootIni->startOSNum, timeout = bootIni->timeout;
    int isSubMenu = 0, i, lastItemNo;
    char *subMenuItems[] = {
        "boot fastboot",
        "boot recovery",
        "power off",
        "go fel",
        "< back"
    };

    while(1) {
        fbclear();
        if( isSubMenu ) {
            lastItemNo = sizeof(subMenuItems) / sizeof(subMenuItems[0]) - 1;
            for(i = 0; i <= lastItemNo; ++i)
            {
                PrintMenuItem(i, subMenuItems[i], curItemNo);
            }
        }else{
            PrintMenuItem(0, "android", curItemNo);
            lastItemNo = bootIni->osCount;
            for(i = 1; i < lastItemNo; ++i) {
                PrintMenuItem(i, bootIni->osNames[i], curItemNo);
            }
            PrintMenuItem(lastItemNo, "other options >", curItemNo);
        }
        switch( GetKeyVal(timeout) ) {
        case 30:    /* volume up */
            curItemNo = curItemNo == 0 ? lastItemNo : curItemNo-1;
            break;
        case 43:    /* volume down */
            curItemNo = curItemNo == lastItemNo ? 0 : curItemNo+1;
            break;
        case -1:    /* timeout */
        case 37:    /* back */
            if( curItemNo == lastItemNo ) {
                isSubMenu = 1 - isSubMenu;
                curItemNo = 0;
            }else{
                fbclear();
                return isSubMenu ? -curItemNo - 1 : curItemNo;
            }
            break;
        }
        timeout = -1;
    }
    return 0;
}

static int ShowLogo(const char *fname, int logoShow, void *imgdatabuf)
{
    if(logoShow == 0 || fname == 0) {
        wlibc_uprintf("logo name is invalid or dont need show logo\n");
        return -1;
    }
    ShowPictureEx(fname, imgdatabuf);
    wlibc_uprintf("show pic finish\n");
    return 0;
}

static int fnL42801F44(void)
{
    gImageHeader->param4 = 0;
    gImageHeader->param0 = 0;
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
    gImageHeader->param4 = 0x54410009;
    gImageHeader->param0 = (strlen(var5) + 13) >> 2;
    strcpy(gImageHeader->param8c, var5);
    gImageHeader = (void*)((int*)gImageHeader + gImageHeader->param0);
}

static void fnL42802010(struct ImageHeader *imgHeader)
{
    gImageHeader = imgHeader;
    gImageHeader->param4 = 0x54410001;
    gImageHeader->param0 = 5;
    gImageHeader->param8i[0] = 0;
    gImageHeader->param8i[1] = 0;
    gImageHeader->param8i[2] = 0;
    gImageHeader = (void*)(((int*)&gImageHeader) + gImageHeader->param0);
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
    ShowLogo(imgScript->logoFileName, imgScript->logoShow,
            imgScript->logoAddress);
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


static void WriteMiscPart(const char *val)
{
    struct MBR mbr;
    char miscPartBuf[1024];
    int partNo;
    unsigned miscPartFistSect = 0;

    svc_diskread(0, 2, &mbr);
    for(partNo = 0; partNo < mbr.PartCount; ++partNo) {
        if(strcmp("misc", mbr.array[partNo].name) == 0) {
            miscPartFistSect = mbr.array[partNo].addrlo;
            svc_diskread(miscPartFistSect, 1, miscPartBuf);
            if( strcmp(miscPartBuf, val) ) {
                memset(miscPartBuf, 0, 32);
                strcpy(miscPartBuf, val);
                svc_diskwrite(miscPartFistSect, 1, miscPartBuf);
            }
            break;
        }
    }
}

int BootOS_detect_os_type(void **var4, void **bootAddrBuf,
        struct BootIni *bootIni, int *logoOff)
{
    int res;
    struct OSImageScript imgScript;
    int requestedBootMode;
    char osIni[100];

    res = -1;
    memset(&imgScript, 0, sizeof(imgScript));
    requestedBootMode = SelectBoot(bootIni);
    switch( requestedBootMode ) {
    case -1:
        WriteMiscPart("boot-fastboot");
        requestedBootMode = 0;
        break;
    case -2:
        WriteMiscPart("boot-recovery");
        requestedBootMode = 0;
        break;
    default:
        if( requestedBootMode >= 0 ) {
            WriteMiscPart("");
        }
        break;
    }
    if( requestedBootMode >= 0) {
        wlibc_sprintf(osIni, "c:\\%s\\%s.ini",
                bootIni->osNames[requestedBootMode],
                bootIni->osNames[requestedBootMode]);
        res = script_patch(osIni, &imgScript, 1);
        if(res < 0) {
            wlibc_uprintf("NO OS to Boot\n");
        } else { 
            wlibc_uprintf("test for multi os boot with display\n");
            res = PreBootOS(&imgScript, bootAddrBuf, var4, logoOff);
        }
    }else{
        res = requestedBootMode + 2;
    }
    svc_free(bootIni);
    return res;
}

void BootOS(void *var4, void *bootAddr)
{
    volatile int var0;

    wlibc_uprintf("jump to: %x, %x\n", var4, bootAddr);
    svc_bootos(0, 3495, var4, bootAddr);
    var0 = 85;
    while( var0 == 85 ) {
    }
}

