#include "syscalls.h"
#include "picture.h"
#include "wlibc.h"
#include "string.h"


struct ImageHeader {
    unsigned short magic;
    short param2;
    short param4;
    short param6;
    short param8;
    short bytesStart;
    short param12;
    int param14;
    int width;
    int height;
} __attribute__((packed));


static int Parse_Pic_BMP_ByPath(const char *fname,
        struct BatteryChargeImg *imgBuf, void* imgdatabuf)
{
    char *fileContents;
    char *imgBytes;
    struct ImageHeader *imgHeader;
    int hdle, lineNo;
    char buf[512];
    int fileSize, res = -1;

    fileContents = 0;
    hdle = svc_open(fname, "rb");
    if(hdle == 0)
        return -1;
    svc_read(buf, 1, 512, hdle);
    imgHeader = (struct ImageHeader*)buf;

    if( imgHeader->magic == 0x4d42 ) {
        imgBuf->height = imgHeader->height > 0 ? imgHeader->height :
            -imgHeader->height;
        imgBuf->width = imgHeader->width;
        imgBuf->imgsiz = imgBuf->height * imgBuf->width * 4;
        imgBuf->depth = 32;
        imgBuf->lineSize = imgBuf->width * 4;
        svc_65(hdle, 0, 0);
        fileSize = svc_filesize(hdle);
        fileContents = svc_alloc(fileSize);
        if(fileContents != 0) {
            svc_read(fileContents, 1, fileSize, hdle);
            if(imgdatabuf == 0) {
                imgBuf->imgdata = svc_alloc(imgBuf->imgsiz);
            } else { 
                imgBuf->imgdata = imgdatabuf;
            }
            imgBytes = fileContents + imgHeader->bytesStart;
            if(imgHeader->height > 0) {
                char *lineBuf = imgBuf->imgdata;
                for(lineNo = 0; lineNo < imgHeader->height; ++lineNo) {
                    memcpy(lineBuf, imgBytes +
                            (imgHeader->height - lineNo-1) *
                            imgHeader->width * 4, imgHeader->width * 4);
                    lineBuf += 4 * imgHeader->width;
                }
            } else { 
                memcpy(imgBuf->imgdata, imgBytes, imgBuf->imgsiz);
            }
            res = 0;
        }
    }
    if(hdle != 0)
        svc_close(hdle);
    if(fileContents != 0)
        svc_free(fileContents);
    return res;
}

static void CopyImage(struct BatteryChargeImg *img)
{
    int i;
    int firstLineImg, firstLineScreen, nLines;
    int firstColImg, firstColScreen, nCols;
    struct MultiLayerPara mlp;

    De_GetPictBuf(&mlp);
    if( img->height <= mlp.height ) {
        firstLineImg = 0;
        firstLineScreen = (mlp.height - img->height) / 2;
        nLines = img->height;
    }else{
        firstLineImg = (img->height - mlp.height) / 2;
        firstLineScreen = 0;
        nLines = mlp.height;
    }
    if( img->width <= mlp.width ) {
        firstColImg = 0;
        firstColScreen = (mlp.width - img->width) / 2;
        nCols = img->width;
    }else{
        firstColImg = (img->width - mlp.width) / 2;
        firstColScreen = 0;
        nCols = mlp.width;
    }
    wlibc_uprintf("CopyImage: depth=%d", img->depth);
    // assume 32bpp image
    for(i = 0; i < nLines; ++i) {
        memcpy(mlp.dispbuf +
                ((i+firstLineScreen) * mlp.width + firstColScreen) * 4,
                img->imgdata +
                ((i+firstLineImg) * img->width + firstColImg) * 4,
                nCols * 4);
    }
}

void ShowPictureEx(const char *fname, void *imgdatabuf)
{
    struct BatteryChargeImg img;

    memset(&img, 0, sizeof(img));
    if( Parse_Pic_BMP_ByPath(fname, &img, imgdatabuf) != 0 ) {
        wlibc_uprintf("ERR: Parse_Pic_BMP failed\n");
        return;
    }
    CopyImage(&img);
    svc_delay(50);
}

struct MultiPictureItem {   /* 112 bytes */
    int offsetHoriz;
    int offsetVert;
    int imgCount;
    int param12;
    struct BatteryChargeImg battImg[4];
};

struct MultiPicture {   /* 920 bytes */
    int itemCount;
    struct MultiLayerPara mlp;  /* #4 */
    int param16;
    struct LayerPara *layerPara;
    struct MultiPictureItem items[8];
};

struct ShowBatteryChangeHandle *ShowBatteryCharge_init(void)
{
    struct ShowBatteryChangeHandle *hdle;
    int i;
    char fname[32];

    if( ! De_IsInit() )
        return 0;
    hdle = svc_alloc(sizeof(*hdle));
    if(hdle == 0)
        return 0;
    memset(hdle, 0, sizeof(*hdle));
    memset(fname, 0, sizeof(fname));
    strcpy(fname, "c:\\os_show\\bat0.bmp");
    for(i = 0; i < 10; ++i) {
        fname[14] = '0' + i;
        if( Parse_Pic_BMP_ByPath(fname, hdle->images + i, 0) != 0 ) {
            wlibc_uprintf("parser bmp file %s failed\n", fname);
            return 0;
        }
    }
    if( Parse_Pic_BMP_ByPath("c:\\os_show\\bat10.bmp",
                hdle->images + 10, 0) != 0 )
    {
        wlibc_uprintf("parser bmp file c:\\os_show\\bat10.bmp failed\n");
        return 0;
    }
    hdle->lp.depth = hdle->images[0].depth;
    hdle->lp.height = hdle->images[0].height;
    hdle->lp.width = hdle->images[0].width;
    hdle->lp.imgsiz = hdle->lp.width * hdle->lp.height * 4;
    hdle->lp.imgdata = svc_alloc(hdle->lp.imgsiz);
    hdle->lp.lineSize = hdle->lp.width * 4;
    hdle->imgsize = hdle->lp.imgsiz;
    if(hdle->lp.imgdata == 0)
        return 0;
    memcpy(hdle->lp.imgdata, hdle->images[0].imgdata, hdle->lp.imgsiz);
    CopyImage(&hdle->lp);
    return hdle;
}

int ShowBatteryCharge_rate(struct ShowBatteryChangeHandle *hdle, int rate)
{
    if(hdle == 0)
        return -1;
    CopyImage(hdle->images + rate / 10);
    return 0;

}

int ShowBatteryCharge_degrade(struct ShowBatteryChangeHandle *hdle)
{
    if(hdle == 0)
        return -1;
    De_DegradeBrightness();
    return 0;
}

int ShowBatteryCharge_exit(struct ShowBatteryChangeHandle *hdle)
{
#if 0
    if(hdle == 0)
        return -1;
    De_CloseLayer( gDeParam0 );
    ui_FreeLayerPara(hdle->layerPara);
#endif
    return 0;
}

