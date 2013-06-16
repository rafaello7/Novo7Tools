#include "syscalls.h"
#include "ui.h"
#include "wlibc.h"
#include "string.h"

struct LayerPara *ui_AllocLayerPara(struct BatteryChargeImg *img)
{
    struct LayerPara *layerPara;
    int screenWidth;
    int screenHeight;

    layerPara = svc_alloc(sizeof(struct LayerPara));
    if(layerPara == 0) {
        wlibc_uprintf("ERR: wBoot_malloc failed\n");
        return 0;
    }
    memset(layerPara, 0, sizeof(struct LayerPara));
    screenWidth = De_GetSceenWidth();
    screenHeight = De_GetSceenHeight();
    layerPara->dispWidth = (img->lineSize * 8) /  img->depth;
    layerPara->imgdata = img->imgdata;
    layerPara->lpparam66 = 1;
    layerPara->lpparam64 = 10;
    layerPara->lpparam67 = 0;
    layerPara->lpparam65 = 0;
    layerPara->lpparam8 = 0;
    layerPara->lpparam0 = 0;
    layerPara->lpparam4 = 1;
    layerPara->lpparam6 = 255;
    layerPara->lpparam2 = 0;
    layerPara->lpparam12 = 0;
    layerPara->lpparam16 = 0;
    layerPara->width2 = img->width;
    layerPara->height2 = img->height;
    layerPara->horizShift = (screenWidth - img->width) >> 1;
    layerPara->vertShift = (screenHeight - img->height) >> 1;
    layerPara->width = img->width;
    layerPara->height = img->height;
    return layerPara;
}

struct LayerPara *ui_multi_AllocLayerPara(struct MultiLayerPara *var4,
        void *var5)
{
    struct LayerPara *layerPara;
    int screenWidth;
    int screenHeight;
    int bufSize;

    layerPara = svc_alloc(sizeof(struct LayerPara));
    if(layerPara == 0) {
        wlibc_uprintf("ERR: wBoot_malloc failed\n");
        return 0;
    }
    memset(layerPara, 0, sizeof(struct LayerPara));
    screenWidth = De_GetSceenWidth();
    screenHeight = De_GetSceenHeight();
    var4->width = screenWidth;
    var4->height = screenHeight;
    bufSize = screenWidth * screenHeight * 4;
    if(var5 == 0) {
        var4->dispbuf = svc_alloc(bufSize);
        if(var4->dispbuf == 0) {
            wlibc_uprintf("multi displya fail: malloc memory for display buffer failed\n");
            svc_free(layerPara);
            return 0;
        }
    } else { 
        var4->dispbuf = var5;
    }
    memset(var4->dispbuf, 0, bufSize);
    layerPara->dispWidth = screenWidth;
    layerPara->lpparam60 = screenHeight;
    layerPara->imgdata = var4->dispbuf;
    layerPara->lpparam66 = 1;
    layerPara->lpparam64 = 10;
    layerPara->lpparam67 = 0;
    layerPara->lpparam65 = 0;
    layerPara->lpparam8 = 0;
    layerPara->lpparam0 = 0;
    layerPara->lpparam4 = 1;
    layerPara->lpparam6 = 255;
    layerPara->lpparam2 = 0;
    layerPara->lpparam12 = 0;
    layerPara->lpparam16 = 0;
    layerPara->width2 = screenWidth;
    layerPara->height2 = screenHeight;
    layerPara->horizShift = 0;
    layerPara->vertShift = 0;
    layerPara->width = screenWidth;
    layerPara->height = screenHeight;
    return layerPara;
}

int ui_FreeLayerPara(struct LayerPara *layerPara)
{
    if( layerPara != 0 )
        svc_free(layerPara);
    return 0;
}

