#include "board.h"
#include "syscalls.h"
#include "wlibc.h"
#include "display_interface.h"

extern unsigned L428126B4[4];


int BoardInit_Display(int displayDevice, int displayMode)
{
    int var6;
    int var7;
    int var8;

    L428126B4[3] = displayDevice;
    if(displayDevice < 0 || displayMode < 0) {
        L428126B4[3] = -1;
        return 0;
    }
    var6 = svc_50("c:\\drv_de.drv");
    if(var6 != 0) {
        wlibc_uprintf("ERR: wBoot_driver_install display driver failed\n");
        return -1;
    }
    if( (L428126B4[1] = svc_52(3, 0)) == 0 ) {
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
    *L428126B4 = De_RequestLayer(0);
    if(*L428126B4 == 0) {
        wlibc_uprintf("ERR: De_RequestLayer failed\n");
        return -1;
    }
    return 0;
}

int BoardExit_Display(int logoOff, int exitStatus)
{
    if(L428126B4[3] < 0)
        return 0;
    if(L428126B4[1] == 0) {
        wlibc_uprintf("ERR: display driver not open yet\n");
    } else { 
        if(exitStatus != 0) {
            /* before boot OS */
            svc_56(L428126B4[1], 0x141, 0, 0);
            sdelay(200000);
        } else { 
            if(logoOff == 0) {
                svc_56(L428126B4[1], 0xc, 1, 0);
            } else { 
                svc_56(L428126B4[1], 0x141, 0, 0);
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

