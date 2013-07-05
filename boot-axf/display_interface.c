#include "syscalls.h"
#include "display_interface.h"
#include "wlibc.h"
#include "string.h"

static unsigned gLayerHandle;
static unsigned gDisplayHandle;
static int gDispDeviceNo;


struct LayerPara {
    char lpparam0;
    char lpparam1;
    char lpparam2;
    char lpparam3;
    char lpparam4;
    char lpparam5;
    short lpparam6;
    char lpparam8;
    char lpparam9;
    short lpparam10;
    int lpparam12;
    int lpparam16;
    int width2;
    int height2;
    int horizShift;
    int vertShift;
    int width;
    int height;
    char *imgdata;
    int lpparam48;
    int lpparam52;
    int dispWidth;
    int lpparam60;
    char lpparam64;
    char lpparam65;
    char lpparam66;
    char lpparam67;
    int lpparam68;
    int lpparam72;
    int lpparam76;
    int lpparam80;
    int lpparam84;
};

static struct LayerPara gLayerPara;

int De_IsInit(void)
{
    return gLayerPara.imgdata != 0;
}

int De_IsLCDOpen(void)
{
    int sp0[3];

    if( gDisplayHandle == 0 ) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = 0;
    sp0[1] = 0;
    sp0[2] = 0;
    return svc_56(gDisplayHandle, 0x14a, 0, sp0);
}

int De_IsLCDClose(void)
{
    int sp0[3];

    if( gDisplayHandle == 0 ) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = 0;
    sp0[1] = 0;
    sp0[2] = 0;
    return svc_56(gDisplayHandle, 0x14b, 0, sp0);
}

int De_RequestLayer(int var4)
{
    int var5;
    int sp0[3];

    var5 = 0;

    if(gDisplayHandle == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return 0;
    }
    sp0[0] = var4;
    sp0[1] = 0;
    sp0[2] = 0;
    var5 = svc_56(gDisplayHandle, 0x40, 0, sp0);
    if(var5 == 0) {
        wlibc_uprintf(
                "ERR: wBoot_driver_ioctl DISP_CMD_LAYER_REQUEST failed\n");
    }
    return var5;
}

int De_ReleaseLayer(int var4)
{
    int var5;
    int sp0[3];

    var5 = 0;

    if(gDisplayHandle == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = var4;
    sp0[1] = 0;
    sp0[2] = 0;
    var5 = svc_56(gDisplayHandle, 0x41, 0, sp0);
    if(var5 != 0) {
        wlibc_uprintf(
                "ERR: wBoot_driver_ioctl DISP_CMD_LAYER_RELEASE failed\n");
    }
    return var5;
}

static int De_OpenLayer(int var4)
{
    int var5;
    int sp0[3];

    var5 = 0;
    if(gDisplayHandle == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = var4;
    sp0[1] = 0;
    sp0[2] = 0;
    var5 = svc_56(gDisplayHandle, 0x42, 0, sp0);
    if(var5 != 0) {
        wlibc_uprintf("ERR: wBoot_driver_ioctl DISP_CMD_LAYER_OPEN failed\n");
    }
    return var5;
}

int De_CloseLayer(int var4)
{
    int var5;
    int sp0[3];

    var5 = 0;
    if(gDisplayHandle == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = var4;
    sp0[1] = 0;
    sp0[2] = 0;
    var5 = svc_56(gDisplayHandle, 0x43, 0, sp0);
    if(var5 != 0) {
        wlibc_uprintf("ERR: wBoot_driver_ioctl DISP_CMD_LAYER_CLOSE failed\n");
    }
    return var5;
}

int De_GetSceenWidth(void)
{
    int sp0[3];

    if(gDisplayHandle == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return 0;
    }
    sp0[0] = 0;
    sp0[1] = 0;
    sp0[2] = 0;
    return svc_56(gDisplayHandle, 0x8, 0, sp0);
}

int De_GetSceenHeight(void)
{
    int sp0[3];

    if(gDisplayHandle == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return 0;
    }
    sp0[0] = 0;
    sp0[1] = 0;
    sp0[2] = 0;
    return svc_56(gDisplayHandle, 0x9, 0, sp0);
}

int De_GetLayerPara(int var4, int var5)
{
    int var6;
    int sp0[3];

    var6 = 0;
    if(gDisplayHandle == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = var4;
    sp0[1] = var5;
    sp0[2] = 0;
    var6 = svc_56(gDisplayHandle, 0x4b, 0, sp0);
    if(var6 != 0) {
        wlibc_uprintf(
                "ERR: wBoot_driver_ioctl DISP_CMD_LAYER_GET_PARA failed\n");

    }
    return var6;
}

static int De_SetLayerPara(int var4, struct LayerPara *var5)
{
    int var6;
    int sp0[3];

    var6 = 0;
    if(gDisplayHandle == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = var4;
    sp0[1] = (int)var5;
    sp0[2] = 0;
    var6 = svc_56(gDisplayHandle, 0x4a, 0, sp0);
    if(var6 != 0) {
        wlibc_uprintf(
                "ERR: wBoot_driver_ioctl DISP_CMD_LAYER_SET_PARA failed\n");
    }
    return var6;
}

int De_SetBlkColor(int var5)
{
    int var4;
    int sp0[3];

    if(gDisplayHandle == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = var5;
    sp0[1] = 0;
    sp0[2] = 0;
    var4 = svc_56(gDisplayHandle, 0x3f, 0, sp0);
    if(var4 != 0) {
        wlibc_uprintf("ERR: wBoot_driver_ioctl DISP_CMD_SET_BKCOLOR failed\n");
    }
    return var4;
}

int De_OpenDevice(int var4)
{
    int var5;
    int var6;
    int var7;
    int var8;
    int sp0[3];

    var5 = 0;

    if(gDisplayHandle == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = 0;
    sp0[1] = 0;
    sp0[2] = 0;
    var7 = 0xffff & var4 >> 16;

    var8 = var4 & 0xffff;
    if(var7 == 1) {
        var5 = svc_56(gDisplayHandle, 0x140, 0, sp0);
    } else { 
        if(var7 == 2) {
            if(var8 == 1) {
                var6 = 4;
            } else if(var8 == 2) {
                var6 = 11;
            } else { 
                return -1;
            }
            sp0[0] = var6;
            svc_56(gDisplayHandle, 0x182, 0, sp0);
            var5 = svc_56(gDisplayHandle, 0x180, 0, 0);
        } else if(var7 == 4) {
            sp0[0] = var8 & 0xff;
            svc_56(gDisplayHandle, 0x1c2, 0, sp0);
            var5 = svc_56(gDisplayHandle, 0x1c0, 0, 0);
        } else { 
            return -1;

        }
    }
    if(var5 != 0) {
        wlibc_uprintf("ERR: wBoot_driver_ioctl DISP_CMD_HDMI_ON failed\n");
    }
    return var5;
}

int De_CloseDevice(int var4)
{
    int var5;
    int var6;
    int var7;
    int sp0[3];

    var5 = 0;
    if(gDisplayHandle == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = 0;
    sp0[1] = 0;
    sp0[2] = 0;
    var6 = 0xffff & var4 >> 16;

    if(var6 == 1) {
        var7 = 0x141;
    } else if(var6 == 2) {
        var7 = 0x181;
    } else if(var6 == 4) {
        var7 = 0x1c1;
    } else { 
        return -1;
    }
    var5 = svc_56(gDisplayHandle, var7, 0, sp0);
    if(var5 != 0) {
        wlibc_uprintf("ERR: display driver turn off display device failed\n");
    }
    return var5;
}

int De_GetFrameBuffer(int /*unsure*/ var4, int /*unsure*/ var5)
{
    int var6;
    int sp0[3];

    if(gDisplayHandle == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = var4;
    sp0[1] = var5;
    sp0[2] = 0;
    var6 = svc_56(gDisplayHandle, 0x45, 0, sp0);
    if(var6 != 0) {
        wlibc_uprintf("ERR: wBoot_driver_ioctl DISP_CMD_LAYER_GET_FB failed\n");
    }
    return var6;
}

int De_SetFrameBuffer(int var4, int var5)
{
    int var6;
    int sp0[3];

    var6 = 0;
    if(gDisplayHandle == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = var4;
    sp0[1] = var5;
    sp0[2] = 0;
    var6 = svc_56(gDisplayHandle, 0x44, 0, sp0);
    if(var6 != 0) {
        wlibc_uprintf("ERR: wBoot_driver_ioctl DISP_CMD_LAYER_SET_FB failed\n");
    }
    return var6;
}

int De_DegradeBrightness(void)
{
    int brightness;
    int var8;

    var8 = gLayerPara.lpparam6;
    wlibc_uprintf("De_DegradeBrightness beg: lpparam6=%d", var8);
    for(brightness = 255; brightness > 0; brightness -= 5) {
        wlibc_uprintf("  %d", brightness);
        gLayerPara.lpparam4 = 1;
        var8 -= 5;
        if(var8 > 0) {
            De_SetLayerPara(gLayerHandle, &gLayerPara);
            svc_delay(50);
            gLayerPara.lpparam6 = var8;
        } else { 
            break;
        }
    }
    wlibc_uprintf("De_DegradeBrightness end");
    return 0;
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

static unsigned De_InitLayerPara(struct LayerPara *layerPara)
{
    int screenWidth;
    int screenHeight;
    int bufSize;
    unsigned layerHandle;

    layerHandle = De_RequestLayer(0);
    if(layerHandle == 0) {
        wlibc_uprintf("ERR: De_RequestLayer failed\n");
        return 0;
    }
    memset(layerPara, 0, sizeof(struct LayerPara));
    screenWidth = De_GetSceenWidth();
    screenHeight = De_GetSceenHeight();
    bufSize = screenWidth * screenHeight * 4;
    layerPara->imgdata = svc_alloc(bufSize);
    if(layerPara->imgdata == 0) {
        wlibc_uprintf("De_InitLayerPara fail: malloc memory for display buffer failed\n");
        return 0;
    }
    memset(layerPara->imgdata, 0, bufSize);
    layerPara->dispWidth = screenWidth;
    layerPara->lpparam60 = screenHeight;
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
    if(gDispDeviceNo == 0)
        WaitForDeInitFinish();
    if( De_SetLayerPara(layerHandle, layerPara) != 0 ) {
        wlibc_uprintf("ERR: De_SetLayerPara failed\n");
        return 0;
    }
    De_OpenLayer(layerHandle);
    return layerHandle;
}

void De_GetPictBuf(struct MultiLayerPara *mlp)
{
    // upper half of the screen
    mlp->width = gLayerPara.width;
    mlp->height = gLayerPara.height / 2;
    mlp->dispbuf = gLayerPara.imgdata;
}

void De_GetConsoleBuf(struct MultiLayerPara *mlp)
{
    // lower half of the screen
    mlp->width = gLayerPara.width;
    mlp->height = gLayerPara.height / 2;
    mlp->dispbuf = gLayerPara.imgdata + gLayerPara.width * gLayerPara.height * 2;
}

int BoardInit_Display(int displayDevice, int displayMode)
{
    int var6;
    int var7;
    int var8;

    gDispDeviceNo = displayDevice;
    if(displayDevice < 0 || displayMode < 0) {
        gDispDeviceNo = -1;
        return 0;
    }
    var6 = svc_50("c:\\drv_de.drv");
    if(var6 != 0) {
        wlibc_uprintf("ERR: wBoot_driver_install display driver failed\n");
        return -1;
    }
    if( (gDisplayHandle = svc_52(3, 0)) == 0 ) {
        wlibc_uprintf("ERR: open display driver failed\n");
        return -1;
    }
    var7 = 0;
    var8 = 0;
    switch( displayDevice ) {
    case 1:
        var7 = 2;
        var8 = 1;
        break;
    case 2:
        var7 = 2;
        var8 = 2;
        break;
    case 3:
        var7 = 4;
        break;
    case 4:
        var7 = 8;
        break;
    default:
        var7 = 1;
    }
    var6 = De_OpenDevice(var7 << 16 | var8 << 8 | displayMode);
    if(var6 != 0) {
        wlibc_uprintf("ERR: De Open LCD or TV failed\n");
        return -1;
    }
    gLayerHandle = De_InitLayerPara(&gLayerPara);
    return 0;
}

int BoardExit_Display(int logoOff, int exitStatus)
{
    if(gDispDeviceNo < 0)
        return 0;
    if(gDisplayHandle == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
    } else { 
        if(exitStatus != 0) {
            /* before boot OS */
            svc_56(gDisplayHandle, 0x141, 0, 0);
            sdelay(200000);
        } else { 
            if(logoOff == 0) {
                svc_56(gDisplayHandle, 0xc, 1, 0);
            } else { 
                svc_56(gDisplayHandle, 0x141, 0, 0);
            }
        }
    }
    return svc_51(3);
}

int BoardExit(int logoOff, int exitStatus)
{
    BoardExit_Display(logoOff, exitStatus);
    return 0;
}

