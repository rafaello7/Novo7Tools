#include <io.h>
#include <fastboot.h>

static int LFFFF557C(int var1)
{
    do {
        if((readl(0x1C03000 + 0x4) & 2) == 0) {
            return 0;
        }
    }while(--var1 != 0);
    return 2;
}

static int LFFFF55B0(int var1)
{
    do {
        if(readl(0x1C03000 + 0x4) & 8) {
            return 0;
        }
    } while(--var1 != 0);
    return 2;

}

static int LFFFF5644(void)
{
    writel(12583167, 0x1C03000 + 0x24);
    if(LFFFF55B0(4800) == 2) {
        return 2;
    }
    if(LFFFF557C(4920) == 2) {
        return 2;
    }
    return 0;

}

static void LFFFF59E8(void)
{
    writel(572662306, 0x1C20000 + 0x848);
    writel(572662306, 0x1C20000 + 0x84C);
    writel(572662306 >> 4, 0x1C20000 + 0x850);
    writel(2, 0x1C20000 + 0x854);
    writel(1431655765, 0x1C20000 + 0x85C);
    writel(1431655765 >> 14, 0x1C20000 + 0x860);
    writel(20800, 0x1C20000 + 0x864);
    writel(16406, 0x1C20000 + 0x868);
}

static void LFFFF5A2C(void)
{
    unsigned var5;

    LFFFF59E8();
    writel(readl(0x1C20000 + 0x60) & ~0x2000, 0x1C20000 + 0x60);
    writel(readl(0x1C20000 + 0x60) | 0x2000, 0x1C20000 + 0x60);
    writel(readl(0x1C20000 + 0x80) & ~0x80000000, 0x1C20000 + 0x80);
    var5 = (readl(0x1C20000 + 0x80) & ~0x303000F) | 1;
    writel(var5, 0x1C20000 + 0x80);
    writel(readl(0x1C20000 + 0x80) | 0x80000000, 0x1C20000 + 0x80);
    writel(readl(0x1C20000 + 0x60) | 64, 0x1C20000 + 0x60);
    writel(3, 0x1C03000 + 0x0);
    sdelay(240);
}

//LFFFF5B68
static void set_ecc(unsigned ecc_bit_count, unsigned block_size,
        unsigned nfc_random_en)
{
    unsigned ecc_mode, ecc_block_size, var4;

    switch( block_size ) {
    case 512:   ecc_block_size = 1; break;
    case 1024:  ecc_block_size = 0; break;
    default:    ecc_block_size = 0; break;
    }
    switch( ecc_bit_count ) {
    case 24:    ecc_mode = 1; break;
    case 40:    ecc_mode = 4; break;
    case 64:    ecc_mode = 8; break;
    default:    ecc_mode = 8; break;
    }
    var4 = readl(0x1C03000 + 0x34); // NFC_REG_ECC_CTL
    // "and" 0xf220 clears the bits being set
#define NFC_ECC_EN				(1 << 0)
    writel((var4 & ~0xf220) | ecc_mode << 12 | ecc_block_size << 5 |
            NFC_ECC_EN | nfc_random_en << 9, 0x1C03000 + 0x34);
}

static int LFFFF5C14(const unsigned *var4)
{
    unsigned var5;
    unsigned var6;
    unsigned var7;
    unsigned var8;
    unsigned var9;

    LFFFF5A2C();
    if(var4[2] != 0) {
        LFFFF5644();
    }
    set_ecc(var4[7], var4[9], var4[8]);
    var5 = readl(0x1C03000 + 0x0) & 0xFFF3B0FB;
    switch( var4[5] ) {
    case 2:
        var6 = 0;
        break;
    case 4:
        var6 = 1;
        break;
    case 8:
        var6 = 2;
        break;
    case 16:
        var6 = 3;
        break;
    case 32:
        var6 = 4;
        break;
    default:
        var6 = 0;
    }
    var7 = var4[1] & 3;

    writel(((var5 | var7 << 18) | var6 << 8) | 16384, 0x01C03000 + 0x0);
    var5 = readl(0x1C03000 + 0xC) & ~3903;
    if(var4[1] == 3) {
        var8 = var4[3] & 15;
        var9 = var4[4] & 63;
    } else { 
        var8 = 0;
        var9 = 0;
    }
    writel((var5 | var8 << 8) | var9, 0x1C03000 + 0xC);
    writel(var4[5] << 9, 0x1C03000 + 0xA0);
    return var4[5] << 9;
}

void dram_lowlevel_init(void)
{
    static const unsigned param[] = {
        0xFFFF5914,
        0x00000000,
        0x00000001,
        0x00000000,
        0x00000000,
        0x00000002,
        0x00000001,
        0x00000040,
        0x00000001,
        0x00000400
    };
    LFFFF5C14(param);
}

