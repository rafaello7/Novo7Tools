#include "power.h"
#include "syscalls.h"
#include "wlibc.h"
#include "io.h"
#include "usb_device.h"
#include "picture.h"
#include "display_interface.h"


static char L4280A3B4[5];
static int L4280A3C8[1];
static int L4280A3CC[1] = { 0x00000FA0 };
static int L4280A3C0[1];
static int L4280A3C4[1] = { 0x00000FA0 };
extern int L4280A408[];
unsigned L4280A3A8[1];
static unsigned L4280A3B0[1];

int BOOT_TWI_Read(int var4, int *var5, char *var6)
{
    int sp4[8];

    sp4[0] = var4;
    sp4[1] = 0;
    sp4[2] = (unsigned)var5;
    sp4[3] = 1;
    sp4[4] = 1;
    sp4[6] = (unsigned)var6;
    sp4[5] = 1;
    return svc_2a(sp4);
}

int BOOT_TWI_Write(int var4, int *var5, char *var6)
{
    int sp4[8];

    sp4[0] = var4;
    sp4[1] = 0;
    sp4[2] = (unsigned)var5;
    sp4[3] = 1;
    sp4[4] = 1;
    sp4[6] = (unsigned)var6;
    return svc_2b(sp4);
}

int boot_power_int_enable(void)
{
    int var4;
    char sp0[5];
    int sp8;

    for(var4 = 0; var4 < 5; ++var4) {
        sp8 = (var4 + 64) & 0xff;
        if(BOOT_TWI_Read(52, &sp8, L4280A3B4 + var4) != 0) {
            wlibc_uprintf("twi read err\n");
            return -1;
        }
    }
    sp0[0] = 12;
    sp0[1] = 0;
    sp0[2] = 3;
    sp0[3] = 0;
    sp0[4] = 0;
    for(var4 = 0; var4 < 5; ++var4) {
        sp8 = (var4 + 64) & 0xff;
        if(BOOT_TWI_Write(52, &sp8, &sp0[var4]) != 0) {
            wlibc_uprintf("twi write err\n");
            return -1;
        }
    }
    return 0;
}

int boot_power_int_disable(void)
{
    int var4;
    int sp0;

    for(var4 = 0; var4 < 5; ++var4) {
        sp0 = (var4 + 64) & 0xff;
        if(BOOT_TWI_Write(52, &sp0, L4280A3B4 + var4) != 0) {
            wlibc_uprintf("twi write err\n");
            return -1;
        }
    }
    return 0;
}

int eGon2_power_int_query(char *var4)
{
    int var5;
    int sp0;

    for(var5 = 0; var5 < 5; ++var5) {
        sp0 = (var5 + 72) & 0xff;
        if(BOOT_TWI_Read(52, &sp0, var4 + var5) != 0) {
            wlibc_uprintf("twi read err\n");
            return -1;
        }
        if(BOOT_TWI_Write(52, &sp0, var4 + var5) != 0) {
            wlibc_uprintf("twi write err\n");
            return -1;
        }
    }
    writel(readl(0x01C20000 + 0x410) | 1, 0x01C20000 + 0x410);
    return 0;

}

void power_set_usbdc(void)
{
    wlibc_uprintf("set dc\n");
    svc_41(*L4280A3C8);
    svc_42(*L4280A3CC);
}

void power_set_usbpc(void)
{
    wlibc_uprintf("set pc\n");
    svc_41(*L4280A3C0);
    svc_42(*L4280A3C4);
}

static void power_int_handler(void)
{
    char sp0[5];

    eGon2_power_int_query(sp0);
    if(sp0[0] & 4) {
        wlibc_uprintf("vbus remove\n");
        *L4280A408 = 0;
        usb_detect_exit();
        power_set_usbpc();
    } else { 
        if(sp0[0] & 8) {
            wlibc_uprintf("vbus insert\n");
            *L4280A3A8 |= 4;
            usb_detect_enter();
        }
    }
    if(sp0[2] & 2) {
        *L4280A3A8 |= 2;
    }else if(sp0[2] & 1)
        *L4280A3A8 |= 1;
}

void power_int_reg(void)
{
    wlibc_uprintf("power start detect\n");
    if(*L4280A3B0 != 0)
        return;
    wlibc_uprintf("power enter detect\n");
    *L4280A3A8 = 0;
    *L4280A3B0 = 1;
    boot_power_int_enable();
    svc_interrupt_sethandler(0, power_int_handler, 0);
    svc_interrupt_enable(0);
}

void power_int_rel(void)
{
    wlibc_uprintf("power exit detect\n");
    svc_interrupt_disable(0);
    boot_power_int_disable();
    *L4280A3B0 = 0;
    *L4280A3A8 = 0;
}

void power_set_init(void)
{
    svc_b0("pmu_para", "pmu_usbvol", L4280A3CC, 1);
    svc_b0("pmu_para", "pmu_usbcur", L4280A3C8, 1);
    svc_b0("pmu_para", "pmu_usbvol_pc", L4280A3C4, 1);
    svc_b0("pmu_para", "pmu_usbcur_pc", L4280A3C0, 1);
    wlibc_uprintf("usbdc_vol = %d, usbdc_cur = %d\n", *L4280A3CC, *L4280A3C8);
    wlibc_uprintf("usbpc_vol = %d, usbpc_cur = %d\n", *L4280A3C4, *L4280A3C0);
}

static int fnL428008D0(int *var4)
{
    if(*var4 == 0) {
        *var4 = 1;
        ShowPictureEx("c:\\os_show\\bat10.bmp", 0);
    } else { 
        //De_OpenLayer(gDeParam0);
    }
    return 0;
}

static int fnL42800848(void)
{
    De_DegradeBrightness();
    return 0;
}

int check_power_status(void)
{
    int var4;
    int var5;
    struct ShowBatteryChangeHandle *var6;
    int var7;
    int var8;
    int var9;
    int vasl;
    int sp0;
    int sp4;
    int sp8;
    int sp12;

    sp12 = svc_3e();
    if(sp12 == 1) {
        wlibc_uprintf(
                "battery low power with no dc or ac, should set power off\n");
        ShowPictureEx("c:\\os_show\\low_pwr.bmp", 0);
        svc_delay(3000);
        return -1;
    }
    if(sp12 == 3) {
        wlibc_uprintf(
                "battery low power with dc or ac, should charge longer\n");
        ShowPictureEx("c:\\os_show\\bempty.bmp", 0);
        svc_delay(3000);
        return -1;

    }
    sp12 = -1;
    svc_b0("target", "power_start", &sp12, 1);
    if(sp12 == 1) {
        return 0;
    }
    sp12 = svc_3b();
    wlibc_uprintf("startup status = %d\n", sp12);
    if( sp12 != 0)
        return 0;
    var6 = 0;
    sp0 = 0;
    power_int_reg();
    usb_detect_enter();
    var6 = ShowBatteryCharge_init();

    svc_delay(1500);
    sp8 = 0;
    sp4 = 0;
    svc_38(&sp8, &sp4);
    if(sp4 == 0) {
        wlibc_uprintf("no battery exist\n");
        ShowBatteryCharge_exit(var6);
        power_int_rel();
        usb_detect_exit();
        return 0;

    }
    wlibc_int_disable();
    var5 = svc_40();
    wlibc_int_enable();
    wlibc_uprintf("base bat_cal = %d\n", var5);
    if(var5 > 95)
        var5 = 100;
    if(var5 == 100) {
        ShowBatteryCharge_exit(var6);
        var6 = 0;
        fnL428008D0(&sp0);
        var7 = 0;
        while(var7 < 12) {
            if( *L4280A3A8 & 2 ) {
                *L4280A3A8 &= ~2;
                var8 = 0;
                wlibc_uprintf("short key\n");
            } else if( *L4280A3A8 & 1 ) {
                wlibc_int_disable();
                power_int_rel();
                usb_detect_exit();
                *L4280A3A8 &= ~1;
                wlibc_int_enable();
                power_int_reg();
                wlibc_uprintf("long key\n");
                return 0;
            }
            svc_delay(250);
            var7 = var7 + 1;
        }
    } else { 
        vasl = 10 - var5 / 10;
        var9 = 1000 / vasl;
        var8 = 0;
        while(var8 < 3) {
            var7 = var5;
            while(var7 < 110) {
                ShowBatteryCharge_rate(var6, var7);
                svc_delay(var9);
                if( *L4280A3A8 & 2 ) {
                    *L4280A3A8 &= ~2;
                    var8 = 0;
                    wlibc_uprintf("short key\n");
                } else if( *L4280A3A8 & 1 ) {
                    ShowBatteryCharge_exit(var6);
                    wlibc_int_disable();
                    power_int_rel();
                    usb_detect_exit();
                    *L4280A3A8 &= ~1;
                    wlibc_int_enable();
                    power_int_reg();
                    wlibc_uprintf("long key\n");
                    return 0;
                }
                var7 = var7 + 10;
            }
            var8 = var8 + 1;
        }
        ShowBatteryCharge_rate(var6, var5);
        svc_delay(1000);
    }
    svc_3a();
    wlibc_uprintf("extenal power low go high startup\n");
    while(1) {
        wlibc_uprintf("enter standby\n");
        if( *L4280A3A8 & 4 ) {
            sp12 = 8;
            *L4280A3A8 &= ~4;
        } else { 
            wlibc_int_disable();
            power_int_rel();
            usb_detect_exit();
            wlibc_int_enable();
            //De_CloseLayer(gDeParam0);
            sp12 = svc_05();
            wlibc_uprintf("exit standby by %d\n", sp12);
            wlibc_int_disable();
            var4 = svc_40();
            wlibc_int_enable();
            wlibc_uprintf("current bat_cal = %d\n", var4);
            if(var4 > var5) {
                var5 = var4;
                var4 = var5;
                if(var5 > 95) {
                    var5 = 100;
                }
            }
        }
        switch( sp12 - 2 ) {
        case 0:
            power_int_reg();
            //De_OpenLayer(gDeParam0);

            if(var5 == 100) {
                if(var6 != 0) {
                    ShowBatteryCharge_exit(var6);
                    var6 = 0;
                }
                fnL428008D0(&sp0);
                var7 = 0;
                while(var7 < 12) {
                    if( *L4280A3A8 & 2 ) {
                        *L4280A3A8 &= ~2;
                        var7 = 0;
                        wlibc_uprintf("MSG:L%d(%s):", 321,
                                "Board/fel_detect/fel_detect.c");
                        wlibc_ntprintf("short key\n");
                    } else { 
                        if( *L4280A3A8 & 1 ) {
                            ShowBatteryCharge_exit(var6);
                            wlibc_int_disable();
                            power_int_rel();
                            usb_detect_exit();
                            *L4280A3A8 &= ~1;
                            wlibc_int_enable();
                            power_int_reg();
                            wlibc_uprintf("long key\n");
                            return 0;
                        }
                    }
                    svc_delay(250);
                    var7 = var7 + 1;
                }
            } else { 
                vasl = 10 - var5 / 10;
                var9 = 1000 / vasl;
                var8 = 0;
                while(var8 < 3) {
                    var7 = var5;
                    while(var7 < 110) {
                        ShowBatteryCharge_rate(var6, var7);
                        svc_delay(var9);
                        if( *L4280A3A8 & 2 ) {
                            *L4280A3A8 &= ~2;
                            var8 = 0;
                            wlibc_uprintf("MSG:L%d(%s):", 354,
                                    "Board/fel_detect/fel_detect.c");
                            wlibc_ntprintf("short key\n");
                        } else if( *L4280A3A8 & 1 ) {
                            ShowBatteryCharge_exit(var6);
                            wlibc_int_disable();
                            power_int_rel();
                            usb_detect_exit();
                            *L4280A3A8 &= ~1;
                            wlibc_int_enable();
                            power_int_reg();
                            wlibc_uprintf("long key\n");
                            return 0;
                        }
                        var7 = var7 + 10;
                    }
                    var8 = var8 + 1;
                }
                ShowBatteryCharge_rate(var6, var5);
                svc_delay(1000);
            }
            break;
        case 1:
            ShowBatteryCharge_exit(var6);
            power_int_reg();
            return 0;
        case 2:
        case 3:
            //De_OpenLayer(gDeParam0);
            ShowBatteryCharge_rate(var6, var5);
        case 4:
        case 5:
            power_int_reg();
            if(sp12 != 4) {
                if(sp12 != 5) {
                    //De_OpenLayer(gDeParam0);
                    ShowBatteryCharge_rate(var6, var5);
                }
            }
            svc_delay(500);
            if(var6 != 0) {
                ShowBatteryCharge_degrade(var6);
                ShowBatteryCharge_exit(var6);
            } else { 
                fnL42800848();
            }
            return -1;
        case 6:
            usb_detect_enter();
            svc_delay(600);
            usb_detect_exit();
            break;
        case 7:
            power_set_usbpc();
            break;
        default:
            break;
        }
    }
}

