#include "syscalls.h"
#include "picture.h"
#include "wlibc.h"
#include "script.h"
#include "power.h"
#include "usb_device.h"
#include "bootos.h"
#include "bootpara.h"
#include "io.h"
#include "bootmain.h"
#include "string.h"


struct BootPara gBootPara;

/*  Key values:
 *      30 - volume up press
 *      43 - volume down press
 *      37 - back press
 */
void dotest2(void)
{
    int keyval = -1;

    wlibc_uprintf("> press back button to go fel, volume btn to boot");
    while( keyval != 30 && keyval != 43 && keyval != 37 ) {
        keyval = svc_keyval();
    }
    //if( keyval != 37 ) {
    //    usb_start(0);
    //    usb_run();
    //}
    if( keyval == 37 ) {
        BoardExit(0, -1);
        power_int_rel();
        usb_detect_exit();
        svc_gofel();
    }
    while( svc_keyval() <= 0 )
        ;
}


int BootMain(int argc, char *argv[])
{
    int status;
    struct BootIni *bootIni;
    int logoOff = 0;
    void *sp4;
    void *bootAddr;

    BoardInit_Display(0, 0);
    dotest2();
    if( (status = svc_getpara(1, &gBootPara)) != 0 ) {
        wlibc_uprintf("ERR: wBoot_get_para failed\n");
    } else { 
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
            //BoardInit_Display(bootIni->displayDevice,
            //      bootIni->displayMode);
            if(check_power_status() != 0) {
                status = -1;
            } else{
                status = BootOS_detect_os_type(&sp4, &bootAddr, bootIni,
                        &logoOff);
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
    wlibc_uprintf("try to power off\n");
    svc_delay(50);
    svc_poweroff();
    return 0;
}

