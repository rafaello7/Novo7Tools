#include "syscalls.h"
#include "picture.h"
#include "wlibc.h"
#include "string.h"

extern unsigned L428126B4[4];

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

static int WaitForDeInitFinish(void)
{
    int var4;
    int var6;

    var6 = 100;
    while(1) {
        if( (var4 = De_IsLCDOpen()) == 1 )
            break;
        if(var4 == -1)
            return -1;
        svc_delay(50);
        if(--var6 <= 0)
            return -1;
    }
    return 0;
}

static int ShowLayer(int var4, struct LayerPara *layerPara, int var7)
{
    if(var7 == 0)
        WaitForDeInitFinish();
    if( De_SetLayerPara(var4, layerPara) != 0 ) {
        wlibc_uprintf("ERR: De_SetLayerPara failed\n");
        return -1;
    }
    De_OpenLayer(var4);
    return 0;
}

struct LayerPara *ShowPictureEx(const char *fname, void *imgdatabuf)
{
    struct LayerPara *layerPara;
    struct BatteryChargeImg img;

    layerPara = 0;
    if( *L428126B4 == 0 ) {
        return 0;
    }
    memset(&img, 0, sizeof(img));
    if( Parse_Pic_BMP_ByPath(fname, &img, imgdatabuf) != 0 ) {
        wlibc_uprintf("ERR: Parse_Pic_BMP failed\n");
        return 0;
    }
    layerPara = ui_AllocLayerPara(&img);
    ShowLayer(*L428126B4, layerPara, L428126B4[3]);
    svc_delay(50);
    return layerPara;
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

struct MultiPicture *ShowMultiPicture(struct MultiPictureParam *mppar, int itemCount)
{
    int itemNo;
    int i;
    int horizDist;
    int vertDist;
    int imgWidth;
    int imgHeight;
    struct MultiPicture *multiPict;
    char *imgdata;
    char *dispbuf;
    char *imgline;
    int itemWidth;
    int sp24;

    multiPict = 0;

    if(*L428126B4 == 0) {
        return 0;
    }
    if(itemCount > 8)
        itemCount = 8;
    multiPict = svc_alloc(sizeof(struct MultiPicture));
    if(multiPict == 0) {
        return 0;
    }
    memset(multiPict, 0, sizeof(struct MultiPicture));
    multiPict->itemCount = itemCount;
    for(itemNo = 0; itemNo < itemCount; ++itemNo) {
        for(i = 0; i < mppar[itemNo].filecount; ++i) {
            sp24 = Parse_Pic_BMP_ByPath(mppar[itemNo].fnames[i],
                    &multiPict->items[itemNo].battImg[multiPict->items[itemNo].imgCount], 0);
            if(sp24 != 0) {
                wlibc_uprintf("decode %s bmp file failed\n",
                        mppar[itemNo].fnames[i]);
            } else { 
                ++multiPict->items[itemNo].imgCount;
            }
        }
    }
    multiPict->layerPara = ui_multi_AllocLayerPara(&multiPict->mlp, 0);
    imgWidth = multiPict->items[0].battImg[0].width;
    imgHeight = multiPict->items[0].battImg[0].height;
    vertDist = (multiPict->mlp.height - multiPict->items[0].battImg[0].height) >> 1;
    itemWidth = multiPict->mlp.width / itemCount;
    horizDist = (itemWidth - imgWidth) >> 1;
    for(itemNo = 0; itemNo < itemCount; ++itemNo) {
        multiPict->items[itemNo].offsetHoriz = itemNo * itemWidth + horizDist;
        multiPict->items[itemNo].offsetVert = vertDist;
    }
    dispbuf = multiPict->mlp.dispbuf;
    dispbuf += multiPict->mlp.width * vertDist * 4;
    for(itemNo = 0; itemNo < itemCount; ++itemNo) {
        imgline = dispbuf + multiPict->items[itemNo].offsetHoriz * 4;
        imgdata = multiPict->items[itemNo].battImg[0].imgdata;
        for(i = 0; i < imgHeight; ++i) {
            memcpy(imgline, imgdata, imgWidth * 4);
            imgline += multiPict->mlp.width * 4;
            imgdata += imgWidth * 4;
        }
    }
    ShowLayer(*L428126B4, multiPict->layerPara, L428126B4[3]);
    svc_delay(50);
    return multiPict;
}

int ShutPictureEx(struct LayerPara *layerPara)
{
    if(layerPara == 0) {
        return 0;
    }
    De_CloseLayer(*L428126B4);
    ui_FreeLayerPara(layerPara);
    return 0;
}

struct ShowBatteryChangeHandle *ShowBatteryCharge_init(void)
{
    struct ShowBatteryChangeHandle *hdle;
    int i;
    char fname[32];

    hdle = 0;
    if( L428126B4[0] == 0 )
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
    hdle->layerPara = ui_AllocLayerPara(&hdle->lp);
    ShowLayer(L428126B4[0], hdle->layerPara, L428126B4[3]);
    return hdle;
}

int ShowBatteryCharge_rate(struct ShowBatteryChangeHandle *hdle, int rate)
{
    void *var7;

    if(hdle == 0)
        return -1;
    if(rate != 100) {
        var7 = hdle->images[rate / 10].imgdata;
        memcpy(hdle->lp.imgdata, var7, hdle->imgsize);
    } else { 
        memcpy(hdle->lp.imgdata, hdle->images[10].imgdata,
                hdle->imgsize);
    }
    return 0;

}

int ShowBatteryCharge_degrade(struct ShowBatteryChangeHandle *hdle)
{
    int var7;
    int var8;

    if(hdle == 0)
        return -1;

    var8 = hdle->layerPara->lpparam6;
    for(var7 = 255; var7 > 0; var7 -= 5) {
        hdle->layerPara->lpparam4 = 1;
        var8 -= 5;
        if(var8 > 0) {
            De_SetLayerPara(L428126B4[0], hdle->layerPara);
            svc_delay(50);
            hdle->layerPara->lpparam6 = var8;
        } else { 
            break;
        }
    }
    return 0;
}

int ShowBatteryCharge_exit(struct ShowBatteryChangeHandle *hdle)
{
    if(hdle == 0)
        return -1;
    De_CloseLayer( L428126B4[0] );
    ui_FreeLayerPara(hdle->layerPara);
    return 0;
}

