#include "syscalls.h"
#include "picture.h"
#include "wlibc.h"
#include "script.h"
#include "power.h"
#include "board.h"
#include "usb_device.h"
#include "bootos.h"
#include "bootpara.h"
#include "io.h"
#include "bootmain.h"
#include "string.h"


struct BootPara gBootPara;
unsigned L428126B4[4];


void keyval_test(void)
{
    int var4;

    wlibc_uprintf("welcome to key value test\n");
    wlibc_uprintf("press any key, and the value would be printed\n");
    wlibc_uprintf("press power key to exit\n\n");
    while(1) {
        var4 = svc_keyval();
        if(var4 > 0)
            wlibc_uprintf("key value = %d\n", var4);
        var4 = 0;
    }
}

int check_file_to_fel(const char *fname)
{
    int hdle;

    hdle = svc_open(fname, "r");
    if(hdle != 0) {
        svc_close(hdle);
        svc_deletefile(fname);
        hdle = svc_open(fname, "r");
        if(hdle == 0) {
            wlibc_uprintf("dest file is not exist\n");
        } else { 
            svc_close(hdle);
        }
        return 0;
    }
    return -1;
}

int check_natch_time(const char *fname, unsigned var5)
{
    int fdesc;
    int res = -1;

    fdesc = svc_open(fname, "r");
    if(fdesc != 0) {
        if(var5 == 4) {
            res = 0;
        }
        svc_close(fdesc);
        svc_deletefile(fname);
    }
    return res;
}

int BootMain(int argc, char *argv[])
{
    int status;
    struct BootIni *bootIni;
    int var9;
    int logoOff = 0;
    void *sp4;
    void *bootAddr;

    wlibc_uprintf("BootMain start\n");
    while(1) {
        int serialKeyVal = svc_read_uart(1);
        wlibc_uprintf("%d\n", serialKeyVal);
        if(serialKeyVal == '2') {
            wlibc_uprintf("Jump to fel\n");
            sdelay(10000);
            svc_gofel();
        } else { 
            if(serialKeyVal == '-') {
                wlibc_uprintf("for debug\n");
                var9 = 85;
                do {
                } while(var9 == 85);
                break;
            }
            if(serialKeyVal == '1') {
                usb_start(0);
                usb_run();
                break;
            }
            if(serialKeyVal == '3') {
                keyval_test();
                break;
            }
            break;
        }
    }
    if( (status = svc_getpara(1, &gBootPara)) != 0 ) {
        wlibc_uprintf("ERR: wBoot_get_para failed\n");
    } else { 
        if(check_file_to_fel("c:\\rabbit.bot") == 0) {
            wlibc_uprintf("jump to fel because of update file found\n");
        } else {
            if(check_natch_time("c:\\natch.ini",
                        gBootPara.bpparam32768[38]) != 0)
            {
                power_set_init();
                wlibc_uprintf("init to usb pc\n");
                power_set_usbpc();
                bootIni = svc_alloc(sizeof(struct BootIni));
                if(bootIni == 0) {
                    wlibc_uprintf("unable to malloc memory for bootini\n");
                    return -1;
                }
                memset(bootIni, 0, sizeof(struct BootIni));
                if( script_patch("c:\\boot.ini", bootIni, 0) < 0 ) {
                    wlibc_uprintf("unable to parse boot.ini\n");
                } else { 
                    BoardInit_Display(bootIni->displayDevice,
                          bootIni->displayMode);
                    if(check_power_status() != 0) {
                        status = -1;
                    } else { 
                        if(BootOS_detect_os_type(&sp4, &bootAddr,
                                    bootIni, &logoOff) !=0)
                        {
                            status = -2;
                        }
                    }
                    BoardExit(logoOff, status);
                    power_int_rel();
                    usb_detect_exit();
                    if(status == 0) {
                        BootOS(sp4, bootAddr);
                    } else if(status == -2) {
                        wlibc_uprintf("try to fel in 1 secend\n");
                        svc_delay(50);
                        svc_gofel();
                    }
                }
            }
        }
    }
    wlibc_uprintf("try to power off\n");
    svc_delay(50);
    svc_poweroff();
    return 0;
}

