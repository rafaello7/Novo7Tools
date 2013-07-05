#include "syscalls.h"
#include "display_interface.h"

extern unsigned L428126B4[4];

int wlibc_uprintf(const char *fmt, ...);

int De_IsLCDOpen(void)
{
    int sp0[3];

    if( L428126B4[1] == 0 ) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = 0;
    sp0[1] = 0;
    sp0[2] = 0;
    return svc_56(L428126B4[1], 0x14a, 0, sp0);
}

int De_IsLCDClose(void)
{
    int sp0[3];

    if( L428126B4[1] == 0 ) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = 0;
    sp0[1] = 0;
    sp0[2] = 0;
    return svc_56(L428126B4[1], 0x14b, 0, sp0);
}

int De_RequestLayer(int var4)
{
    int var5;
    int sp0[3];

    var5 = 0;

    if(L428126B4[1] == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return 0;
    }
    sp0[0] = var4;
    sp0[1] = 0;
    sp0[2] = 0;
    var5 = svc_56(L428126B4[1], 0x40, 0, sp0);
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

    if(L428126B4[1] == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = var4;
    sp0[1] = 0;
    sp0[2] = 0;
    var5 = svc_56(L428126B4[1], 0x41, 0, sp0);
    if(var5 != 0) {
        wlibc_uprintf(
                "ERR: wBoot_driver_ioctl DISP_CMD_LAYER_RELEASE failed\n");
    }
    return var5;
}

int De_OpenLayer(int var4)
{
    int var5;
    int sp0[3];

    var5 = 0;
    if(L428126B4[1] == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = var4;
    sp0[1] = 0;
    sp0[2] = 0;
    var5 = svc_56(L428126B4[1], 0x42, 0, sp0);
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
    if(L428126B4[1] == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = var4;
    sp0[1] = 0;
    sp0[2] = 0;
    var5 = svc_56(L428126B4[1], 0x43, 0, sp0);
    if(var5 != 0) {
        wlibc_uprintf("ERR: wBoot_driver_ioctl DISP_CMD_LAYER_CLOSE failed\n");
    }
    return var5;
}

int De_GetSceenWidth(void)
{
    int sp0[3];

    if(L428126B4[1] == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return 0;
    }
    sp0[0] = 0;
    sp0[1] = 0;
    sp0[2] = 0;
    return svc_56(L428126B4[1], 0x8, 0, sp0);
}

int De_GetSceenHeight(void)
{
    int sp0[3];

    if(L428126B4[1] == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return 0;
    }
    sp0[0] = 0;
    sp0[1] = 0;
    sp0[2] = 0;
    return svc_56(L428126B4[1], 0x9, 0, sp0);
}

int De_GetLayerPara(int var4, int var5)
{
    int var6;
    int sp0[3];

    var6 = 0;
    if(L428126B4[1] == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = var4;
    sp0[1] = var5;
    sp0[2] = 0;
    var6 = svc_56(L428126B4[1], 0x4b, 0, sp0);
    if(var6 != 0) {
        wlibc_uprintf(
                "ERR: wBoot_driver_ioctl DISP_CMD_LAYER_GET_PARA failed\n");

    }
    return var6;
}

int De_SetLayerPara(int var4, struct LayerPara *var5)
{
    int var6;
    int sp0[3];

    var6 = 0;
    if(L428126B4[1] == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = var4;
    sp0[1] = (int)var5;
    sp0[2] = 0;
    var6 = svc_56(L428126B4[1], 0x4a, 0, sp0);
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

    if(L428126B4[1] == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = var5;
    sp0[1] = 0;
    sp0[2] = 0;
    var4 = svc_56(L428126B4[1], 0x3f, 0, sp0);
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

    if(L428126B4[1] == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = 0;
    sp0[1] = 0;
    sp0[2] = 0;
    var7 = 0xffff & var4 >> 16;

    var8 = var4 & 0xffff;
    if(var7 == 1) {
        var5 = svc_56(L428126B4[1], 0x140, 0, sp0);
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
            svc_56(L428126B4[1], 0x182, 0, sp0);
            var5 = svc_56(L428126B4[1], 0x180, 0, 0);
        } else if(var7 == 4) {
            sp0[0] = var8 & 0xff;
            svc_56(L428126B4[1], 0x1c2, 0, sp0);
            var5 = svc_56(L428126B4[1], 0x1c0, 0, 0);
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
    if(L428126B4[1] == 0) {
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
    var5 = svc_56(L428126B4[1], var7, 0, sp0);
    if(var5 != 0) {
        wlibc_uprintf("ERR: display driver turn off display device failed\n");
    }
    return var5;
}

int De_GetFrameBuffer(int /*unsure*/ var4, int /*unsure*/ var5)
{
    int var6;
    int sp0[3];

    if(L428126B4[1] == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = var4;
    sp0[1] = var5;
    sp0[2] = 0;
    var6 = svc_56(L428126B4[1], 0x45, 0, sp0);
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
    if(L428126B4[1] == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
        return -1;
    }
    sp0[0] = var4;
    sp0[1] = var5;
    sp0[2] = 0;
    var6 = svc_56(L428126B4[1], 0x44, 0, sp0);
    if(var6 != 0) {
        wlibc_uprintf("ERR: wBoot_driver_ioctl DISP_CMD_LAYER_SET_FB failed\n");
    }
    return var6;
}
