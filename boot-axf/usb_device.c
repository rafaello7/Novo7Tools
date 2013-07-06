#include "usb_device.h"
#include "io.h"
#include "wlibc.h"
#include "syscalls.h"
#include "power.h"
#include "string.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned u32;

struct usb_device_request {
	u8 bmRequestType;
	u8 bRequest;
	u16 wValue;                 /* #2 */
	u16 wIndex;                 /* #4 */
	u16 wLength;                /* #6 */
} __attribute__ ((packed));

struct usb_generic_descriptor {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubtype;
} __attribute__ ((packed));

struct usb_device_descriptor {
	u8 bLength;                     /* #0 */
	u8 bDescriptorType;	/* 0x01 */  /* #1 */
	u16 bcdUSB;                     /* #2 */
	u8 bDeviceClass;                /* #4 */
	u8 bDeviceSubClass;             /* #5 */
	u8 bDeviceProtocol;             /* #6 */
	u8 bMaxPacketSize0;             /* #7 */
	u16 idVendor;                   /* #8 */
	u16 idProduct;                  /* #10 */
	u16 bcdDevice;                  /* #12 */
	u8 iManufacturer;               /* #14 */
	u8 iProduct;                    /* #15 */
	u8 iSerialNumber;               /* #16 */
	u8 bNumConfigurations;          /* #17 */
} __attribute__ ((packed));

struct usb_configuration_descriptor {   /* 9 bytes */
	u8 bLength;
	u8 bDescriptorType;	/* 0x2 */
	u16 wTotalLength;               /* #2 */
	u8 bNumInterfaces;              /* #4 */
	u8 bConfigurationValue;         /* #5 */
	u8 iConfiguration;
	u8 bmAttributes;
	u8 bMaxPower;
} __attribute__ ((packed));

struct usb_interface_descriptor {
	u8 bLength;
	u8 bDescriptorType;	/* 0x04 */
	u8 bInterfaceNumber;
	u8 bAlternateSetting;
	u8 bNumEndpoints;
	u8 bInterfaceClass;
	u8 bInterfaceSubClass;
	u8 bInterfaceProtocol;
	u8 iInterface;
} __attribute__ ((packed));

struct usb_endpoint_descriptor {
	u8 bLength;
	u8 bDescriptorType;	/* 0x5 */
	u8 bEndpointAddress;
	u8 bmAttributes;
	u16 wMaxPacketSize;
	u8 bInterval;
} __attribute__ ((packed));

struct usb_string_descriptor {
	u8 bLength;
	u8 bDescriptorType;	/* 0x03 */
	u16 wData[];
} __attribute__ ((packed));

struct usb_qualifier_descriptor {
	u8  bLength;
	u8  bDescriptorType;  /* 0x06 */
	u16 bcdUSB;
	u8  bDeviceClass;
	u8  bDeviceSubClass;
	u8  bDeviceProtocol;
	u8  bMaxPacketSize0;
	u8  bNumConfigurations;
	u8  bRESERVED;
} __attribute__ ((packed));

struct SCSICommandBlockWrapper {
    u32  dCBWSignature;             /* always 0x43425355 */
    u32  dCBWTag;
    u32  dCBWDataTransferLength;
    u8   bmCBWFlags;
    u8   bCBWLUN;
    u8   bCBWCBLength;
    u8   CBWCB[16];
} __attribute__((packed));

struct SCSICommandStatusWrapper {
    u32 dCSWSignature;      /* always 0x53425355 */
    u32 dCSWTag;            /* equal to dCBWTag in SCSICommandBlockWrapper */
    u32 dCSWDataResidue;    /* difference between dCBWDataTransferLength and
                             * data really processed by the device */
    u8  bCSWStatus;         /* 0 - success, 1 - failure, 2 - phase error */
} __attribute__((packed));

struct SCSIInquiryResponse {
    u8 peripherial;
    u8 rmb;
    u8 version;
    u8 respDataFormat;
    u8 additionalLength;
    u8 flags1;
    u8 flags2;
    u8 flags3;
    char vendor[8];
    char product[16];
    char revision[4];
};

struct SCSIRequestSenseResponse {
    u8 responseCode;
    u8 obsolete;
    u8 senseKey;
    u8 information[4];
    u8 additionalSenseLength;
    u8 cmdSpecific[4];
    u8 addSenseCode;
    u8 addSenseCodeQual;
    u8 fieldReplUnitCode;
    u8 senseKeySpec[3];
} __attribute__((packed));

struct ConfigDesc {
    struct usb_configuration_descriptor conf;
    struct usb_interface_descriptor interf;
    struct usb_endpoint_descriptor ep[2];
} __attribute__ ((packed));

static const struct usb_device_descriptor USB_HS_BULK_DevDesc = {
    .bLength = 0x12,
    .bDescriptorType = 0x01,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 0x40,
    .idVendor = 0x1f1a,
    .idProduct = 0x0700,
    .bcdDevice = 0x100,
    .iManufacturer = 0x2,
    .iProduct = 0x3,
    .iSerialNumber = 0x1,
    .bNumConfigurations = 1
};

static const struct ConfigDesc USB_HS_BULK_ConfigDesc = {
    .conf = {
        .bLength = 0x9,
        .bDescriptorType = 0x2,
        .wTotalLength = 0x20,
        .bNumInterfaces = 1,
        .bConfigurationValue = 0x8,
        .iConfiguration = 0,
        .bmAttributes = 0x80,
        .bMaxPower = 0x32
    },
    .interf = {
        .bLength = 0x9,
        .bDescriptorType = 0x4,
        .bInterfaceNumber = 0,
        .bAlternateSetting = 0,
        .bNumEndpoints = 2,
        .bInterfaceClass = 0x8,
        .bInterfaceSubClass = 0x6,
        .bInterfaceProtocol = 0x50,
        .iInterface = 0
    },
    .ep = {
        {
            .bLength = 0x7,
            .bDescriptorType = 0x5,
            .bEndpointAddress = 0x81,
            .bmAttributes = 0x2,
            .wMaxPacketSize = 0x200,
            .bInterval = 0
        },{
            .bLength = 0x7,
            .bDescriptorType = 0x5,
            .bEndpointAddress = 0x2,
            .bmAttributes = 0x2,
            .wMaxPacketSize = 0x200,
            .bInterval = 0
        }
    }
};

static const struct usb_device_descriptor USB_FS_BULK_DevDesc = {
    .bLength = 0x12,
    .bDescriptorType = 0x1,
    .bcdUSB = 0x200,
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 8,
    .idVendor = 0x1f1a,
    .idProduct = 0x0700,
    .bcdDevice = 0x100,
    .iManufacturer = 2,
    .iProduct = 3,
    .iSerialNumber = 1,
    .bNumConfigurations = 1
};


static const struct ConfigDesc USB_FS_BULK_ConfigDesc = {
    .conf = {
        .bLength = 0x9,
        .bDescriptorType = 0x2,
        .wTotalLength = 0x20,
        .bNumInterfaces = 1,
        .bConfigurationValue = 0x8,
        .iConfiguration = 0,
        .bmAttributes = 0x80,
        .bMaxPower = 0x32
    },
    .interf = {
        .bLength = 0x9,
        .bDescriptorType = 0x4,
        .bInterfaceNumber = 0,
        .bAlternateSetting = 0,
        .bNumEndpoints = 2,
        .bInterfaceClass = 0x8,
        .bInterfaceSubClass = 0x6,
        .bInterfaceProtocol = 0x50,
        .iInterface = 0,
    },
    .ep = {
        {
            .bLength = 0x7,
            .bDescriptorType = 0x5,
            .bEndpointAddress = 0x81,
            .bmAttributes = 0x2,
            .wMaxPacketSize = 0x40,
            .bInterval = 0
        },{
            .bLength = 0x7,
            .bDescriptorType = 0x5,
            .bEndpointAddress = 0x2,
            .bmAttributes = 0x2,
            .wMaxPacketSize = 0x40,
            .bInterval = 0
        }
    }
};

static const struct usb_qualifier_descriptor USB_DevQual = {
	.bLength = 0xA,
	.bDescriptorType = 0x6,
	.bcdUSB = 0x200,
	.bDeviceClass = 0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 0x40,
	.bNumConfigurations = 0x1,
	.bRESERVED = 0,
};

static const struct usb_generic_descriptor USB_OTGDesc = {
    .bLength = 0x3,
    .bDescriptorType = 0x9,
    .bDescriptorSubtype = 0x3
};

static struct SCSIInquiryResponse InquiryData = {
    .peripherial = 0,
    .rmb = 0x80,
    .version = 0,
    .respDataFormat = 0,
    .additionalLength = 0x20,
    .flags1 = 0,
    .flags2 = 0,
    .flags3 = 0,
    .vendor = "AWTech  ",
    .product = "USB Storage     ",
    .revision = "0100"
};

static unsigned SenseData = 0x00000003;

static struct SCSIRequestSenseResponse RequestSense = {
    /* is set to SS_MEDIUM_NOT_PRESENT == 0x023a00 */
    .responseCode = 0x70,
    .obsolete = 0,
    .senseKey = 0x2,
    .information = { 0, 0, 0, 0 },
    .additionalSenseLength = 0xa,
    .cmdSpecific = { 0, 0, 0, 0 },
    .addSenseCode = 0x3a,
    .addSenseCodeQual = 0,
    .fieldReplUnitCode = 0,
    .senseKeySpec = { 0, 0, 0 }
};

struct usb_devparams {
    int uparam0;
    unsigned usbRegBase;   /* == 0x01C13000 */
    int interruptNo;
    int uparam16;
    int uparam20;
    int usbSpeed;
    int intusb;
    int uparam40;
    int uparam44;
    int uparam48;
    int uparam52;
    int uparam60;
    int ep0TxInterruptCount;    /* unhandled tx interrupt count for ep0 */
    int uparam588;
    int uparam592;
    const struct usb_generic_descriptor *uparam596;
    int uparam600;
    int txInterruptCount[5];  /* unhandled tx interrupt count for ep1 .. ep5 */
    int rxInterruptCount[5];  /* unhandled rx interrupt count for ep1 .. ep5 */
    int eptx_xfer_state[5];
    int eprx_xfer_state[5];
    int uparam688;
    const struct usb_device_descriptor *devDesc;
    const struct ConfigDesc *configDesc;
    const struct usb_string_descriptor *uparam700[4];
    const struct ConfigDesc *hsBulkConfigDesc;
    const struct ConfigDesc *hsBulkConfigDesc2;
    const struct usb_qualifier_descriptor *devQualDesc;
    const struct usb_generic_descriptor *otgDesc;
    int lastLUNnum;
    int uparam736[5];
    int uparam756[5];
    int uparam776[5];
    int uparam796[5];
    char *processedData;
    int unprocessedSize;
    int processedSize;
    int transferState;
    int epTransfer;
    int epRead;
    void *writeData;
    int uparam848;
    int uparam852;
    void *uparam856;
    int uparam864;
    int uparam868;
    int uparam876;
    int uparam884;
    char readBuf[256];
    int uparam1144;
};

extern unsigned L4280A3A8[];
static int      L4280A3F0[1];
int      L4280A408;
static unsigned L4280A40C;
static void *timerHandle;
static unsigned gDiskWriteBytes;
static unsigned gDiskWriteOffset;

const struct usb_string_descriptor LangID = {
    .bLength = 0x4,
    .bDescriptorType = 0x3,
    .wData = { 0x0409 }
};

const struct usb_string_descriptor iSerialNum0 = {
    .bLength = 0x1E,
    .bDescriptorType = 0x3,
    .wData = { '2', '0', '1', '0', '1', '2', '0',
        '1', '1', '2', '0', '0', '0', '1' }
};

const struct usb_string_descriptor iManufacturer = {
    .bLength = 0x28,
    .bDescriptorType = 0x3,
    .wData = {
        'A', 'l', 'l', 'W', 'i', 'n', 'n', 'e', 'r', ' ',
        'T', 'e', 'c', 'h', 'n', 'o', 'l', 'o', 'g', 'y'
    }
};

const struct usb_string_descriptor iProduct_new = {
    .bLength = 0x22,
    .bDescriptorType = 0x3,
    .wData = {
        'U', 'S', 'B', ' ', 'S', 't', 'o', 'r', 'a', 'g', 'e', ' ',
        'T', 'o', 'o', 'l'
    }
};

const struct usb_string_descriptor *StringDescriptors[] = {
    &LangID,
    &iSerialNum0,
    &iManufacturer,
    &iProduct_new
};
int             L428126E8[4];
struct usb_devparams gDevParams;
static unsigned L4280A3F8;
static unsigned L4280A3FC;
static unsigned gTxInterruptCount;
static unsigned gRxInterruptCount;
static int L4280A3EC;
static int L4280A3F4;
unsigned L42806340[] = { 0xffffffff, 0x00000000 };

static const unsigned TestPkt[] = { /* 53 bytes */
    0x00000000, 0x00000000, 0xaaaaaa00, 0xaaaaaaaa,
    0xeeeeeeaa, 0xeeeeeeee, 0xfffffeee, 0xffffffff,
    0xffffffff, 0xdfbf7fff, 0xfdfbf7ef, 0xdfbf7efc,
    0xfdfbf7ef, 0x7e
};

void usb_eprx_flush_fifo(struct usb_devparams *var0)
{
    writew(0x10, var0->usbRegBase + 0x86);
}

void usb_eptx_flush_fifo(struct usb_devparams *var0)
{
    writew(8, var0->usbRegBase + 0x82);
}

void usb_fifo_accessed_by_cpu(struct usb_devparams *var0)
{
    writeb(readb(var0->usbRegBase + 0x43) & ~1, var0->usbRegBase + 0x43);
}

int usb_get_active_ep(struct usb_devparams *var0)
{
    return readb(var0->usbRegBase + 0x42) & 0xf;
}

int usb_get_ep0_count(struct usb_devparams *var0)
{
    return readw(var0->usbRegBase + 0x88) & 0x7f;
}

int usb_get_eprx_csr(struct usb_devparams *var0)
{
    return readw(var0->usbRegBase + 0x86);
}

int usb_get_eptx_csr(struct usb_devparams *var0)
{
    return readw(var0->usbRegBase + 0x82);
}

int usb_get_fifo_access_config(struct usb_devparams *var0)
{
    return readb(var0->usbRegBase + 0x43);
}

void usb_select_ep(struct usb_devparams *var0, int var1)
{
    if( var1 > 5 )
        return;
    writeb(var1, var0->usbRegBase + 0x42);
}

void usb_set_dev_addr(struct usb_devparams *var0, int var1)
{
    writeb(var1 & 0x7f, var0->usbRegBase + 0x98);
}

void usb_set_ep0_csr(struct usb_devparams *var0, int var1)
{
    writew(var1, var0->usbRegBase + 0x82);
}

void usb_set_eprx_csr(struct usb_devparams *var0, int var1)
{
    writew(var1, var0->usbRegBase + 0x86);
}

void usb_set_eptx_csr(struct usb_devparams *var0, int var1)
{
    writew(var1, var0->usbRegBase + 0x82);
}

void usb_set_fifo_access_config(struct usb_devparams *var0, int var1)
{
    writeb(var1, var0->usbRegBase + 0x43);
}

void usb_set_test_mode(struct usb_devparams *var0, int var1)
{
    writeb(var1, var0->usbRegBase + 0x7c);
}

static int aw_log2(unsigned var2)
{
    int var0 = 0;

    while(var2 > 1) {
        var2 >>= 1;
        ++var0;
    }
    return var0;
}

static int usb_get_bus_interrupt_status(struct usb_devparams *var1)
{
    return readl(var1->usbRegBase + 0x4c);
}

static void usb_clear_bus_interrupt_status(struct usb_devparams *var0,
        unsigned var1)
{
    writel(var1, var0->usbRegBase + 0x4c);
}

void usb_irq_handler(struct usb_devparams *var4)
{
    int status;
    int ep;

    status = usb_get_bus_interrupt_status(var4);
    usb_clear_bus_interrupt_status(var4, status);
    if(status & 8) {  /* USBC_INTUSB_SOF */
        ++var4->uparam16;
        status &= ~8;
    }
    if(readb(var4->usbRegBase + 0x50) & status) { /* if enabled */
        var4->intusb |= status;
        ++var4->uparam40;
        ++L4280A3F8;
    }
    status = readw(var4->usbRegBase + 0x44);   /* tx interrupts */
    writew(status, var4->usbRegBase + 0x44);
    if(status & 1) {    /* tx interrupt for ep0 */
        ++var4->ep0TxInterruptCount;
        ++L4280A3FC;
    }
    if(0xfffe & status) {   /* tx interrupt for ep1 .. ep5 */
        for(ep = 0; ep < 5; ++ep) {
            if(status & 2 << ep) {
                ++var4->txInterruptCount[ep];
            }
        }
        ++gTxInterruptCount;
    }
    status = readw(var4->usbRegBase + 0x46); /* rx interrupts */
    writew(status, var4->usbRegBase + 0x46);
    if((0xfffe & status) == 0) /* return if no rx interrupt for ep1 .. ep5 */
        return;
    for(ep = 0; ep < 5; ++ep) {
        if(status & 2 << ep) {
            ++var4->rxInterruptCount[ep];
        }
    }
    ++gRxInterruptCount;
}

void usb_read_ep_fifo(struct usb_devparams *var4, int ep,
        char *buf, int count)
{
    char *bufp;
    int i;
    int vafp;

    if(ep > 5) {
        return;
    }
    vafp = usb_get_fifo_access_config(var4);
    usb_fifo_accessed_by_cpu(var4);
    bufp = buf;
    for(i = 0; i < count; ++i) {
        *bufp++ = readb(var4->usbRegBase + (ep << 2));
    }
    usb_set_fifo_access_config(var4, vafp);
}

void usb_write_ep_fifo(struct usb_devparams *var4, int ep, const void *data,
        int count)
{
    const char *datap;
    int i;
    int vafp;

    if(ep > 5) {
        return;
    }
    vafp = usb_get_fifo_access_config(var4);
    usb_fifo_accessed_by_cpu(var4);
    datap = data;
    for(i = 0; i < count; ++i) {
        writeb(*datap++, var4->usbRegBase + (ep << 2));
    }
    usb_set_fifo_access_config(var4, vafp);
}

static int usb_read(struct usb_devparams *var4, int ep, char *buf,
        int toRead, int var8)
{
    int var9;
    int vasl;
    int vafp;
    int epSave;
    int res;

    res = 0;
    epSave = usb_get_active_ep(var4);
    usb_select_ep(var4, ep);
    var9 = readw(var4->usbRegBase + 0x84);
    var9 = (var9 & 0x7ff) * ((var9 >> 11) + 1);
    switch( var4->eprx_xfer_state[ep - 1] ) {
    case 0:
        var4->processedData = buf;
        var4->unprocessedSize = toRead;
        var4->processedSize = 0;
        if(var9 == 0) {
            return 1;
        }
        if(toRead >= var9) {
            vasl = var4->unprocessedSize < var9 ?
                var4->unprocessedSize : var9;
            if(usb_get_eprx_csr(var4) & 1) {
                usb_fifo_accessed_by_cpu(var4);
                if(usb_get_fifo_access_config(var4) & 1) {
                    wlibc_uprintf("Error: CPU Access Failed!!\n");
                }
                usb_read_ep_fifo(var4, ep, var4->processedData, vasl);
                vafp = usb_get_eprx_csr(var4) & 0x4000;
                usb_set_eprx_csr(var4, vafp);
                var4->unprocessedSize -= vasl;
                var4->processedData += vasl;
                var4->processedSize += vasl;
                var4->eprx_xfer_state[ep - 1] = 1;
                L4280A3F4 = 0;
            } else { 
                ++L4280A3F4;
                if(L4280A3F4 < 4096) {
                    res = 0;
                } else { 
                    res = 2;
                    wlibc_uprintf("Error: RxPktRdy Timeout!!\n");
                }
            }
        } else { 
            if(usb_get_eprx_csr(var4) & 1) {
                usb_fifo_accessed_by_cpu(var4);
                if(usb_get_fifo_access_config(var4) & 1) {
                    wlibc_uprintf("Error: CPU Access Failed!!\n");
                }
                usb_read_ep_fifo(var4, ep, var4->processedData, toRead);
                vasl = usb_get_eprx_csr(var4) & 0x4000;
                usb_set_eprx_csr(var4, vasl);
                var4->unprocessedSize -= toRead;
                var4->processedSize += toRead;
                var4->eprx_xfer_state[ep - 1] = 0;
                res = 1;
                L4280A3F4 = 0;
            } else { 
                ++L4280A3F4;
                if(L4280A3F4 < 4096) {
                    res = 0;
                } else { 
                    res = 2;
                    wlibc_uprintf("Error: RxPktRdy Timeout!!\n");
                }
            }
        }
        break;
    case 1:
        if(var4->unprocessedSize != 0) {
            if(usb_get_eprx_csr(var4) & 1) {
                vasl = var4->unprocessedSize < var9 ?
                    var4->unprocessedSize : var9;
                usb_fifo_accessed_by_cpu(var4);
                if(usb_get_fifo_access_config(var4) & 1) {
                    wlibc_uprintf("Error: CPU Access Failed!!\n");
                }
                usb_read_ep_fifo(var4, ep, var4->processedData, vasl);
                vafp = usb_get_eprx_csr(var4) & 0x4000;
                usb_set_eprx_csr(var4, vafp);
                var4->unprocessedSize -= vasl;
                var4->processedData += vasl;
                var4->processedSize += vasl;
                var4->eprx_xfer_state[ep - 1] = 1;
                L4280A3F4 = 0;
            } else { 
                ++L4280A3F4;
                if(L4280A3F4 < 4096) {
                    res = 0;
                } else { 
                    res = 2;
                    wlibc_uprintf("Error: RxPktRdy Timeout!!\n");
                }
            }
        } else { 
            var4->eprx_xfer_state[ep - 1] = 0;
            res = 1;
        }
        break;
    default:
        wlibc_uprintf("Error: Wrong eprx_xfer_state=%d\n",
                var4->eprx_xfer_state[ep - 1]);
        var4->eprx_xfer_state[ep - 1] = 0;
    }
    usb_select_ep(var4, epSave);
    return res;
}

static int usb_transfer(struct usb_devparams *var4, int epNum, void *data,
        int var7, int var8)
{
    int var9 = 0;
    int vasl;
    int vafp;
    int sp0;

    sp0 = usb_get_active_ep(var4);
    usb_select_ep(var4, epNum);
    vasl = readw(var4->usbRegBase + 0x80);
    vasl = (vasl & 0x7ff) * ((vasl >> 11) + 1);
    switch( var4->eptx_xfer_state[epNum - 1] ) {
    case 0:
        var4->processedData = data;
        var4->unprocessedSize = var7;
        var4->processedSize = 0;
        if(vasl == 0) {
            return 1;
        }
        if(var7 >= vasl) {
            usb_fifo_accessed_by_cpu(var4);
            usb_write_ep_fifo(var4, epNum, var4->processedData, vasl);
            var4->unprocessedSize -= vasl;
            var4->processedSize += vasl;
            var4->processedData += vasl;
            usb_set_eptx_csr(var4, 0x2001);
            var4->eptx_xfer_state[epNum - 1] = 1;
        } else { 
            usb_fifo_accessed_by_cpu(var4);
            usb_write_ep_fifo(var4, epNum, var4->processedData, var7);
            if(usb_get_fifo_access_config(var4) & 1) {
                wlibc_uprintf("Error: FIFO Access Config Error!!\n");
            }
            usb_set_eptx_csr(var4, 8193);
            var4->eptx_xfer_state[epNum - 1] = 2;
            var4->unprocessedSize = 0;
            var4->processedSize = var7;
        }
        break;
    case 1:
        if((usb_get_eptx_csr(var4) & 1) == 0) {
            if(var4->unprocessedSize != 0) {
                vafp = var4->unprocessedSize < vasl ?
                    var4->unprocessedSize : vasl;
                usb_fifo_accessed_by_cpu(var4);
                usb_write_ep_fifo(var4, epNum, var4->processedData, vafp);
                var4->unprocessedSize -= vafp;
                var4->processedSize += vafp;
                var4->processedData += vafp;
                usb_set_eptx_csr(var4, 8193);
                var4->eptx_xfer_state[epNum - 1] = 1;
            } else { 
                if((usb_get_eptx_csr(var4) & 2) == 0) {
                    var4->eptx_xfer_state[epNum - 1] = 2;
                }
            }
        }
        break;
    case 2:
        if((usb_get_eptx_csr(var4) & 3) == 0) {
            usb_get_eptx_csr(var4);
            vafp = usb_get_eptx_csr(var4) & 0x4000;
            usb_set_eptx_csr(var4, vafp);
            var4->eptx_xfer_state[epNum - 1] = 0;
            var9 = 1;
        }
        break;
    default:
        wlibc_uprintf("Error: Wrong eptx_xfer_state=%d\n",
                var4->eptx_xfer_state[epNum - 1]);
        var4->eptx_xfer_state[epNum - 1] = 0;
    }
    usb_select_ep(var4, sp0);
    return var9;
}

int usb_dev_bulk_xfer(struct usb_devparams *var4)
{
    int var5 = 0;
    struct SCSICommandBlockWrapper *scsiCmd;
    int var7 = 0;
    int var8;
    int var9;
    int vasl;
    int vafp;
    int sp4;
    int sp8;
    unsigned sp12;
    char sp16[32];
    int sp28;
    int sp32;
    char sp36[12];
    char sp40[8];
    int sp36i;
    int epSaved;
    int res = 0;
    struct SCSICommandStatusWrapper *cmdStatus;

    epSaved = usb_get_active_ep(var4);
    scsiCmd = (struct SCSICommandBlockWrapper*)var4->readBuf;
    cmdStatus = (struct SCSICommandStatusWrapper*)var4->readBuf;
    switch( var4->transferState ) {
    case 0:
    case 1:
        if(var4->rxInterruptCount[var4->epRead - 1] == 0) {
            break;
        }
        --var4->rxInterruptCount[var4->epRead - 1];
        usb_select_ep(var4, var4->epRead);
        if((usb_get_eprx_csr(var4) & 1) == 0) {
            break;
        }
        var5 = readw(var4->usbRegBase + 0x88) & 0x1fff;
        if(var5 != 31) {
            usb_eprx_flush_fifo(var4);
            wlibc_uprintf("Error: Not CBW, RX Data Length=%d\n", var5);
            break;
        }
        do {
            res = usb_read(var4, var4->epRead, var4->readBuf, var5, 2);
        } while(res == 0);
        if(res == 2) {
            wlibc_uprintf("Error: RX CBW Error\n");
            break;
        }
        res = 0;
        if(scsiCmd->dCBWSignature != 0x43425355) {
            wlibc_uprintf("Error: Not CBW, Error Signature=0x%x\n",
                    scsiCmd->dCBWSignature);
            break;
        }
        switch( scsiCmd->CBWCB[0] ) {
        case 0: /* TEST_UNIT_READY */
            cmdStatus->dCSWSignature = 0x53425355;
            cmdStatus->dCSWDataResidue = 0;
            if(L4280A40C == 0) {
                cmdStatus->bCSWStatus = 0;
            } else { 
                cmdStatus->bCSWStatus = 1;
            }
            var4->writeData = cmdStatus;
            var4->uparam848 = 13;
            var4->uparam852 = 0;
            var7 = usb_transfer(var4, var4->epTransfer, var4->writeData,
                    var4->uparam848, 2);
            if(var7 == 0) {
                var4->transferState = 4;
            } else { 
                if(var7 == 1) {
                    var4->uparam852 = var4->uparam848;
                    var4->uparam848 = 0;
                    var4->transferState = 1;
                } else { 
                    var4->transferState = 1;
                    wlibc_uprintf("Error: CSW Send Error!!\n");
                }
            }
            break;
        case 0x1e:  /* ALLOW_MEDIUM_REMOVAL */
            cmdStatus->dCSWSignature = 0x53425355;
            cmdStatus->dCSWDataResidue = 0;
            cmdStatus->bCSWStatus = 0;
            var4->writeData = cmdStatus;
            var4->uparam848 = 13;
            var4->uparam852 = 0;
            var7 = usb_transfer(var4, var4->epTransfer, var4->writeData,
                    var4->uparam848, 2);
            if(var7 == 0) {
                var4->transferState = 4;
            } else { 
                var4->transferState = 1;
                wlibc_uprintf("Error: CSW Send Error!!\n");
            }
            break;
        case 0x2f:  /* VERIFY */
            cmdStatus->dCSWSignature = 0x53425355;
            cmdStatus->dCSWDataResidue = 0;
            cmdStatus->bCSWStatus = 0;
            var4->writeData = cmdStatus;
            var4->uparam848 = 13;
            var4->uparam852 = 0;
            var7 = usb_transfer(var4, var4->epTransfer, var4->writeData,
                    var4->uparam848, 2);
            if(var7 == 0) {
                var4->transferState = 4;
            } else { 
                var4->transferState = 1;
                wlibc_uprintf("Error: CSW Send Error!!\n");
            }
            break;
        case 0x12:  /* INQUIRY */
            var4->writeData = &InquiryData;
            var4->uparam848 = scsiCmd->dCBWDataTransferLength < 36 ?
                scsiCmd->dCBWDataTransferLength : 36;
            var4->uparam852 = 0;
            var7 = usb_transfer(var4, var4->epTransfer, var4->writeData,
                    var4->uparam848, 2);
            if(var7 == 0) {
                var4->transferState = 3;
            } else { 
                var4->transferState = 1;
                wlibc_uprintf("Error: Data Send Error!!\n");
            }
            break;
        case 0x23:  /* READ_FORMAT_CAPACITIES */
            sp36[0] = 0;
            sp36[1] = 0;
            sp36[2] = 0;
            sp36[3] = 8;
            sp36[8] = 2;
            sp36[9] = 0;
            sp36[10] = 2;
            sp36[11] = 0;
            sp28 = svc_partsectcount(scsiCmd->bCBWLUN & 0xf);
            sp36[4] = sp28 >> 24;
            sp36[5] = sp28 >> 16;
            sp36[6] = sp28 >> 8;
            sp36[7] = sp28;
            var4->writeData = sp36;
            if(L4280A40C == 0) {
                var4->uparam848 = scsiCmd->dCBWDataTransferLength < 12 ?
                    scsiCmd->dCBWDataTransferLength : 12;
            } else { 
                var4->uparam864 = 1;
                var4->uparam848 = 0;
            }
            var4->uparam852 = 0;
            var7 = usb_transfer(var4, var4->epTransfer, var4->writeData,
                    var4->uparam848, 2);
            if(var7 == 0) {
                var4->transferState = 3;
            } else { 
                var4->transferState = 1;
                wlibc_uprintf("Error: Data Send Error!!\n");
            }
            break;
        case 0x25:  /* READ_CAPACITY */
            sp40[4] = 0;
            sp40[5] = 0;
            sp40[6] = 2;
            sp40[7] = 0;
            sp32 = svc_partsectcount(scsiCmd->bCBWLUN & 0xf) - 1;
            sp40[0] = sp32 >> 24;
            sp40[1] = sp32 >> 16;
            sp40[2] = sp32 >> 8;
            sp40[3] = sp32;
            var4->writeData = sp40;
            if(L4280A40C == 0) {
                var4->uparam848 = scsiCmd->dCBWDataTransferLength < 8 ?
                    scsiCmd->dCBWDataTransferLength : 8;
            } else { 
                var4->uparam864 = 1;
                var4->uparam848 = 0;
            }
            var4->uparam852 = 0;
            var7 = usb_transfer(var4, var4->epTransfer, var4->writeData,
                    var4->uparam848, 2);
            if(var7 == 0) {
                var4->transferState = 3;
            } else { 
                var4->transferState = 1;
                wlibc_uprintf("Error: Data Send Error!!\n");
            }
            break;
        case 0x28:  /* READ_10 */
            var8 = 0;
            var9 = 0;
            var8 = scsiCmd->CBWCB[7];
            var8 = var8 << 8;
            var8 = var8 | scsiCmd->CBWCB[8];
            var9 = scsiCmd->CBWCB[2];
            var9 = var9 << 8;
            var9 = var9 | scsiCmd->CBWCB[3];
            var9 = var9 << 8;
            var9 = var9 | scsiCmd->CBWCB[4];
            var9 = var9 << 8;
            var9 = var9 | scsiCmd->CBWCB[5];
            var4->writeData = var4->uparam856;
            if(L4280A40C == 0) {
                vasl = svc_partfirstsect(scsiCmd->bCBWLUN & 15);
                if( svc_diskread(var9 + vasl, var8, var4->uparam856) < 0) {
                    wlibc_uprintf("part index = %d, start = %d, "
                            "read_start = %d, len = %d\n",
                            scsiCmd->bCBWLUN & 15, vasl, var9 + vasl, var8);
                    cmdStatus->dCSWSignature = 0x53425355;
                    cmdStatus->dCSWDataResidue = scsiCmd->dCBWDataTransferLength;
                    cmdStatus->bCSWStatus = 1;
                    var4->writeData = cmdStatus;
                    var4->uparam848 = 13;
                    var4->uparam852 = 0;
                    usb_transfer(var4, var4->epTransfer, var4->writeData,
                            var4->uparam848, 2);
                    var4->transferState = 4;
                    wlibc_uprintf("Error: Flash Read Fail\n");
                    break;
                }
                var8 = var8 << 9;
                var9 = var9 << 9;
                var4->uparam848 = var8 < 0x1000000 ? var8 : 0x1000000;
            } else { 
                var4->uparam864 = 1;
                var4->uparam848 = 0;
            }
            var4->uparam852 = 0;
            var7 = usb_transfer(var4, var4->epTransfer, var4->writeData,
                    var4->uparam848, 2);
            if(var7 == 0) {
                var4->transferState = 3;
            } else { 
                var4->transferState = 1;
                wlibc_uprintf("Error: Data Send Error!!\n");
            }
            break;
        case 0x1a:  /* MODE_SENSE */
            var4->writeData = &SenseData;
            var4->uparam848 = scsiCmd->dCBWDataTransferLength < 4 ?
                scsiCmd->dCBWDataTransferLength : 4;
            var4->uparam852 = 0;
            var7 = usb_transfer(var4, var4->epTransfer, var4->writeData,
                    var4->uparam848, 2);
            if(var7 == 0) {
                var4->transferState = 3;
            } else { 
                var4->transferState = 1;
                wlibc_uprintf("Error: Data Send Error!!\n");
            }
            break;
        case 3: /* REQUEST_SENSE */
            var4->writeData = &RequestSense;
            var4->uparam848 = scsiCmd->dCBWDataTransferLength < 18 ?
                scsiCmd->dCBWDataTransferLength : 18;
            var4->uparam852 = 0;
            var7 = usb_transfer(var4, var4->epTransfer, var4->writeData,
                    var4->uparam848, 2);
            if(var7 == 0) {
                var4->transferState = 3;
            } else { 
                var4->transferState = 1;
                wlibc_uprintf("Error: Data Send Error!!\n");
            }
            break;
        case 0x2a:  /* WRITE_10 */
            gDiskWriteBytes = scsiCmd->CBWCB[7];
            gDiskWriteBytes <<= 8;
            gDiskWriteBytes |= scsiCmd->CBWCB[8];
            gDiskWriteBytes <<= 9;
            gDiskWriteOffset = scsiCmd->CBWCB[2];
            gDiskWriteOffset <<= 8;
            gDiskWriteOffset |= scsiCmd->CBWCB[3];
            gDiskWriteOffset <<= 8;
            gDiskWriteOffset |= scsiCmd->CBWCB[4];
            gDiskWriteOffset <<= 8;
            gDiskWriteOffset |= scsiCmd->CBWCB[5];
            gDiskWriteOffset <<= 9;
            var4->writeData = var4->uparam856;
            var4->uparam848 = gDiskWriteBytes < 0x1000000 ?
                gDiskWriteBytes : 0x1000000;
            var4->uparam852 = 0;
            var7 = usb_read(var4, var4->epRead, var4->writeData,
                    var4->uparam848, 2);
            if(var7 == 1) {
                var9 = svc_partfirstsect(scsiCmd->bCBWLUN & 15)
                    + (gDiskWriteOffset >> 9);
                vasl = gDiskWriteBytes >> 9;
                var8 = svc_diskwrite(var9, vasl, var4->uparam856);
                wlibc_uprintf("part index = %d, start = %d\n",
                        scsiCmd->bCBWLUN & 15, var9);
                if(var8 < 0) {
                    wlibc_uprintf("flash write start %d sector %d failed\n",
                            var9, vasl);
                    cmdStatus->dCSWSignature = 0x53425355;
                    cmdStatus->dCSWDataResidue = scsiCmd->dCBWDataTransferLength;
                    cmdStatus->bCSWStatus = 1;
                    var4->writeData = cmdStatus;
                    var4->uparam848 = 13;
                    var4->uparam852 = 0;
                    usb_transfer(var4, var4->epTransfer, var4->writeData,
                            var4->uparam848, 2);
                    var4->transferState = 4;
                    wlibc_uprintf("Error: Flash Write Fail\n");
                    break;
                }
                var4->uparam852 = var4->uparam848;
                var4->uparam848 = 0;
                cmdStatus->dCSWSignature = 0x53425355;
                cmdStatus->dCSWDataResidue = scsiCmd->dCBWDataTransferLength -
                    var4->uparam852;
                cmdStatus->bCSWStatus = 0;
                var4->writeData = cmdStatus;
                var4->uparam848 = 13;
                var4->uparam852 = 0;
                var7 = usb_transfer(var4, var4->epTransfer, var4->writeData,
                        var4->uparam848, 2);
                if(var7 == 0) {
                    var4->transferState = 4;
                } else { 
                    var4->transferState = 1;
                    wlibc_uprintf("Error: CSW Send Error!!\n");
                }
            } else { 
                if(var7 == 0) {
                    var4->transferState = 2;
                } else { 
                    var4->transferState = 1;
                    wlibc_uprintf("Error: Rx Data Error!!\n");
                }
            }
            break;
        case 0xf3:
            if(L4280A40C == 0) {
                break;
            }
            wlibc_uprintf("usb handshake\n");
            wlibc_uprintf("usb command = %d\n", scsiCmd->CBWCB[1]);
            switch( scsiCmd->CBWCB[1] ) {
            case 0:
                wlibc_uprintf("usb command = 0\n");
                memset(sp16, 0, sizeof(sp16));
                strcpy(sp16, "usbhandshake");
                sp12 = L428126E8[3] & L42806340[1];
                sp8 = L428126E8[2] & L42806340[0];
                sp32 = L428126E8[2] & L42806340[0];
                sp8 = L428126E8[3];
                sp12 = L428126E8[3] >> 31;
                sp4 = sp12 & L42806340[1];
                sp36i = L428126E8[3] & L42806340[0];
                wlibc_uprintf("part sectors low = %d, high = %d\n",
                        sp32, sp36i);
                var4->uparam848 = 32;
                memcpy(var4->uparam856, &sp16, 32);
                var4->writeData = var4->uparam856;
                var7 = usb_transfer(var4, var4->epTransfer, var4->writeData,
                        var4->uparam848, 2);
                if(var7 == 0) {
                    var4->transferState = 3;
                } else { 
                    var4->transferState = 1;
                    wlibc_uprintf("Error: Data Send Error!!\n");
                }
                L4280A3EC = L428126E8[0];
                break;
            case 1:
                wlibc_uprintf("usb command = 1\n");
                if(L4280A3EC == 0) {
                    var4->transferState = 1;
                    wlibc_uprintf("Error: CSW Send Error!!\n");
                    break;
                }
                var8 = L4280A3EC;
                gDiskWriteBytes = scsiCmd->CBWCB[8] << 9;
                var9 = gDiskWriteBytes;
                var4->writeData = var4->uparam856;
                var4->uparam848 = var9;
                wlibc_uprintf("write size = %d\n", var9);
                var7 = usb_read(var4, var4->epRead, var4->writeData,
                        var4->uparam848, 2);
                if(var7 == 1) {
                    vafp = var9 >> 9;
                    wlibc_uprintf("MSG:L%d(%s):", 1180,
                            "usb_device/usb_storage.c");
                    wlibc_ntprintf("write start = %d, count = %d\n", var8, vafp);
                    vasl = svc_diskwrite(var8, vafp, var4->uparam856);
                    L4280A3EC += vafp;
                    if(vasl < 0) {
                        wlibc_uprintf(
                                "flash write start %d sector %d failed\n",
                                var8, vafp);
                        cmdStatus->dCSWSignature = 0x53425355;
                        cmdStatus->dCSWDataResidue =
                            scsiCmd->dCBWDataTransferLength;
                        cmdStatus->bCSWStatus = 1;
                        var4->writeData = cmdStatus;
                        var4->uparam848 = 13;
                        var4->uparam852 = 0;
                        usb_transfer(var4, var4->epTransfer, var4->writeData,
                                var4->uparam848, 2);
                        var4->transferState = 4;
                        wlibc_uprintf("Error: Flash Write Fail\n");
                        break;
                    }
                    var4->uparam852 = var4->uparam848;
                    var4->uparam848 = 0;
                    cmdStatus->dCSWSignature = 0x53425355;
                    cmdStatus->dCSWDataResidue =
                        scsiCmd->dCBWDataTransferLength - var4->uparam852;
                    cmdStatus->bCSWStatus = 0;
                    var4->writeData = cmdStatus;
                    var4->uparam848 = 13;
                    var4->uparam852 = 0;
                    var7 = usb_transfer(var4, var4->epTransfer, var4->writeData,
                            var4->uparam848, 2);
                    if(var7 == 0) {
                        var4->transferState = 4;
                    } else { 
                        var4->transferState = 1;
                        wlibc_uprintf("Error: CSW Send Error!!\n");
                    }
                } else { 
                    if(var7 == 0) {
                        var4->transferState = 2;
                    } else { 
                        var4->transferState = 1;
                        wlibc_uprintf("Error: Rx Data Error!!\n");
                    }
                }
                break;
            case 2:
                wlibc_uprintf("usb command = 2\n");
                memset(sp16, 0, sizeof(sp16));
                strcpy(sp16, "usb_dataok");
                sp32 = 0;
                sp36i = 0;
                var4->uparam848 = 32;
                memcpy(var4->uparam856, sp16, sizeof(sp16));
                var4->writeData = var4->uparam856;
                var7 = usb_transfer(var4, var4->epTransfer, var4->writeData,
                        var4->uparam848, 2);
                if(var7 == 0) {
                    var4->transferState = 3;
                    *L4280A3F0 = 1;
                } else { 
                    var4->transferState = 1;
                    wlibc_uprintf("Error: Data Send Error!!\n");
                }
                break;
            }
            break;
        }
        break;
    case 2:
        var7 = usb_read(var4, var4->epRead, var4->writeData,
                var4->uparam848, 2);
        if(var7 == 1) {
            if(L4280A40C == 0) {
                var9 = svc_partfirstsect(scsiCmd->bCBWLUN & 15)
                    + (gDiskWriteOffset >> 9);
            } else { 
                var9 = L4280A3EC;
            }
            vasl = gDiskWriteBytes >> 9;
            wlibc_uprintf("MSG:L%d(%s):", 1285, "usb_device/usb_storage.c");
            wlibc_ntprintf("write start = %d, count = %d\n", var9, vasl);
            if(L4280A40C != 0) {
                L4280A3EC += vasl;
            }
            var8 = svc_diskwrite(var9, vasl, var4->uparam856);
            if(var8 < 0) {
                wlibc_uprintf("flash write start %d sector %d failed\n",
                        var9, vasl);
                cmdStatus->dCSWSignature = 0x53425355;
                cmdStatus->dCSWDataResidue = scsiCmd->dCBWDataTransferLength;
                cmdStatus->bCSWStatus = 1;
                var4->writeData = cmdStatus;
                var4->uparam848 = 13;
                var4->uparam852 = 0;
                usb_transfer(var4, var4->epTransfer, var4->writeData,
                        var4->uparam848, 2);
                var4->transferState = 4;
                wlibc_uprintf("Error: Flash Write Fail\n");
                break;
            }
            var4->uparam852 = var4->uparam848;
            var4->uparam848 = 0;
            cmdStatus->dCSWSignature = 0x53425355;
            cmdStatus->dCSWDataResidue = scsiCmd->dCBWDataTransferLength -
                var4->uparam852;
            cmdStatus->bCSWStatus = 0;
            if(L4280A40C == 1) {
                if(var4->uparam864 == 1) {
                    cmdStatus->bCSWStatus = 1;
                }
            }
            var4->writeData = cmdStatus;
            var4->uparam848 = 13;
            var4->uparam852 = 0;
            usb_transfer(var4, var4->epTransfer, var4->writeData,
                    var4->uparam848, 2);
            var4->transferState = 4;
        } else { 
            if(var7 == 2) {
                wlibc_uprintf("Error: RxData Error\n");
                var4->transferState = 1;
            }
        }
        break;
    case 3:
        var7 = usb_transfer(var4, var4->epTransfer, var4->writeData,
                var4->uparam848, 2);
        if(var7 == 1) {
            var4->uparam852 = var4->uparam848;
            var4->uparam848 = 0;
            cmdStatus->dCSWSignature = 0x53425355;
            cmdStatus->dCSWDataResidue = scsiCmd->dCBWDataTransferLength -
                var4->uparam852;
            cmdStatus->bCSWStatus = 0;
            if(L4280A40C == 1) {
                if(var4->uparam864 == 1) {
                    cmdStatus->bCSWStatus = 1;
                }
            }
            var4->writeData = cmdStatus;
            var4->uparam848 = 13;
            var4->uparam852 = 0;
            usb_transfer(var4, var4->epTransfer, var4->writeData,
                    var4->uparam848, 2);
            var4->transferState = 4;
        } else if(var7 == 2) {
            wlibc_uprintf("Error: TxData Error\n");
            var4->transferState = 1;
        }
        break;
    case 4:
        var7 = usb_transfer(var4, var4->epTransfer, var4->writeData,
                var4->uparam848, 2);
        if(var7 == 1) {
            var4->uparam852 = var4->uparam848;
            var4->uparam848 = 0;
            var4->transferState = 1;
            if(L4280A40C == 1) {
                var4->uparam864 = 0;
                if(*L4280A3F0 == 1) {
                    wlibc_uprintf("MSG:L%d(%s):", 1368,
                            "usb_device/usb_storage.c");
                    wlibc_ntprintf("1\n");
                    *L4280A3F0 = 2;
                }
            }
        } else if(var7 == 2) {
            var4->transferState = 1;
            wlibc_uprintf("Error: Tx CSW Error!!\n");
        }
        break;
    }
    usb_select_ep(var4, epSaved);
    return res;
}

static int fnL42806DF0(struct usb_devparams *var4)
{
    int var5 = 0;
    struct usb_device_request *devReq;

    devReq = (struct usb_device_request*)var4->readBuf;
    if((devReq->bmRequestType & 0x60) == 0) {
        switch( devReq->bRequest ) {
        case 0:
            wlibc_uprintf("usb_device: Get Status\n");
        case 6:
            switch( devReq->wValue >> 8 ) {
            case 1:
                var4->uparam588 = var4->devDesc->bMaxPacketSize0;
                var4->uparam596 =
                    (const struct usb_generic_descriptor*)var4->devDesc;
                var4->uparam600 = var4->uparam596->bLength < devReq->wLength ?
                    var4->uparam596->bLength : devReq->wLength;
                break;
            case 2:
                var5 = var4->configDesc->conf.wTotalLength;
                var4->uparam596 =
                    (struct usb_generic_descriptor*)var4->configDesc;
                var4->uparam600 = var5 < devReq->wLength ? var5 : devReq->wLength;
                break;
            case 3:
                var5 = devReq->wValue & 255;
                if(var5 < 4) {
                    var4->uparam596 =
                        (struct usb_generic_descriptor*)var4->uparam700[var5];
                    var4->uparam600 =
                        var4->uparam596->bLength < devReq->wLength ?
                        var4->uparam596->bLength : devReq->wLength;
                } else { 
                    wlibc_uprintf("Unkown String Desc!!\n");
                }
                break;
            case 4:
                var4->uparam600 = 0;
                wlibc_uprintf("usb_device: Get Interface Descriptor\n");
                break;
            case 5:
                var4->uparam600 = 0;
                wlibc_uprintf("usb_device: Get Endpoint Descriptor\n");
                break;
            case 6:
                var4->uparam596 =
                    (struct usb_generic_descriptor*)var4->devQualDesc;
                var4->uparam600 = var4->uparam596->bLength < devReq->wLength ?
                    var4->uparam596->bLength : devReq->wLength;
                break;
            case 9:
                var4->uparam596 = var4->otgDesc;
                var4->uparam600 = var4->uparam596->bLength < devReq->wLength ?
                    var4->uparam596->bLength : devReq->wLength;
                break;
            case 0:
            case 7:
            case 8:
            default:
                var4->uparam600 = 0;
                wlibc_uprintf("usb_device: Get Unkown Descriptor\n");
            }
            break;
        case 8:
            var4->uparam600 = 0;
            wlibc_uprintf("usb_device: Get Configuration\n");
            break;
        case 10:
            var4->uparam600 = 0;
            wlibc_uprintf("usb_device: Get Interface\n");
            break;
        case 12:
            var4->uparam600 = 0;
            wlibc_uprintf("usb_device: Sync Frame\n");
            break;
        default:
            var4->uparam600 = 0;
            wlibc_uprintf("usb_device: Unkown Standard Request = 0x%x\n",
                    devReq->bRequest);
        }
    } else { 
        if((devReq->bmRequestType & 0x60) == 32) {
            if(devReq->bRequest == 0xfe) {
                var4->uparam596 =
                    (struct usb_generic_descriptor*)&var4->lastLUNnum;
                var4->uparam600 = 1;
                wlibc_uprintf("usb_device: Get MaxLUN\n");
            } else { 
                var4->uparam600 = 0;
                wlibc_uprintf(
                        "usb_device: Unkown Class-Specific Request = 0x%x\n",
                        devReq->bRequest);
            }
        } else { 
            var4->uparam600 = 0;
            wlibc_uprintf("usb_device: Unkown EP0 IN!!\n");
        }
    }
    return 0;
}

static int fnL42806B14(struct usb_devparams *var4)
{
    const struct usb_device_request *devReq;
    const struct usb_interface_descriptor *var6;
    int var7;
    int var8;
    int var9;
    unsigned maxPacketSize;
    int vafp;
    int epDirection;
    int epNum;
    const struct usb_endpoint_descriptor *epDesc;
    const struct ConfigDesc *configDesc;

    devReq = (struct usb_device_request*)var4->readBuf;
    var8 = 0x400;
    configDesc = var4->configDesc;
    if(configDesc->conf.bConfigurationValue != devReq->wValue) {
        wlibc_uprintf("Error: Right Configval %d; Error Configval %d\n",
                configDesc->conf.bConfigurationValue, devReq->wValue);
        return 0;
    }
    var6 = &var4->configDesc->interf;
    var7 = 0;
    while(var7 < var6->bNumEndpoints) {
        epDesc = &var4->configDesc->ep[var7];
        epNum = epDesc->bEndpointAddress & 0xf;
        epDirection = epDesc->bEndpointAddress >> 7;
        var9 = epDesc->bmAttributes & 3;
        maxPacketSize = epDesc->wMaxPacketSize & 0x7ff;
        usb_select_ep(var4, epNum);
        if(epDirection != 0) {
            vafp = 512 / maxPacketSize;
            writew((maxPacketSize & 0x7ff) | (((vafp-1) & 31) << 11),
                    var4->usbRegBase + 0x80);
            writew((var8 >> 3) & 0x1fff, var4->usbRegBase + 0x92);
            vafp = 0;
            if(0) {
                vafp = vafp | 16;
            }
            vafp |= aw_log2(512 >> 3) & 0xf;
            writeb(vafp, var4->usbRegBase + 0x90);
            var4->uparam736[epNum - 1] = var9;
            var4->uparam776[epNum - 1] = (maxPacketSize & 0x7fff) |
                32768 | (var8 << 16);
            var8 = var8 + 1024;
            if(var6->bInterfaceProtocol == 80) {
                var4->epTransfer = epNum;
            }
            usb_eptx_flush_fifo(var4);
            usb_eptx_flush_fifo(var4);
        } else { 
            vafp = 512 / maxPacketSize;
            writew((maxPacketSize & 0x7ff) | (((vafp-1) & 31) << 11),
                    var4->usbRegBase + 0x84);
            writew((var8>>3) & 0x1fff, var4->usbRegBase + 0x96);
            vafp = 0;
            if(0) {
                vafp = vafp | 16;
            }
            vafp |= aw_log2(512 >> 3) & 0xf;
            writeb(vafp, var4->usbRegBase + 0x94);
            var4->uparam756[epNum-1] = var9;
            var4->uparam796[epNum - 1] = (maxPacketSize & 0x7fff) |
                32768 | (var8 << 16);
            var8 += 1024;
            if(var6->bInterfaceProtocol == 80) {
                var4->epRead = epNum;
            }
            usb_eprx_flush_fifo(var4);
            usb_eprx_flush_fifo(var4);
        }
        var7 = var7 + 1;
    }
    return 1;
}

static int fnL4280722C(struct usb_devparams *var5)
{
    struct usb_device_request *devReq;

    devReq = (struct usb_device_request*)var5->readBuf;
    switch( devReq->bRequest ) {
    case 1:
        break;
    case 3:
        switch( devReq->wValue ) {
        case 2:
            switch( devReq->wIndex ) {
            case 0x100:
                usb_set_test_mode(var5, 2);
                wlibc_uprintf("usb_device: Send Test J Now...\n");
                break;
            case 0x200:
                usb_set_test_mode(var5, 4);
                wlibc_uprintf("usb_device: Send Test K Now...\n");
                break;
            case 0x300:
                usb_set_test_mode(var5, 1);
                wlibc_uprintf("usb_device: Test SE0_NAK Now...\n");
                break;
            case 0x400:
                usb_write_ep_fifo(var5, 0, TestPkt, 53);
                usb_set_ep0_csr(var5, 2);
                usb_set_test_mode(var5, 8);
                wlibc_uprintf("usb_device: Send Test Packet Now...\n");
                break;
            default:
                wlibc_uprintf("usb_device: Unkown Test Mode: 0x%x\n",
                        devReq->wIndex);
            }
            break;
        case 3:
        case 4:
        case 5:
            wlibc_uprintf("usb_device: HNP Enable...\n");
            break;
        default:
            wlibc_uprintf("usb_device: Unkown SetFeature Value: 0x%x\n",
                    devReq->wValue);
        }
        break;
    case 5:
        usb_set_dev_addr(var5, devReq->wValue);
        var5->uparam688 = devReq->wValue;
        wlibc_uprintf("usb_device: Set Address 0x%x\n", devReq->wValue);
        break;
    case 7:
        wlibc_uprintf("usb_device: Set Descriptor\n");
        break;
    case 9:
        fnL42806B14(var5);
        break;
    case 11:
        wlibc_uprintf("usb_device: Set Interface\n");
        break;
    case 0:
    case 2:
    case 4:
    case 6:
    case 8:
    case 10:
    default:
        wlibc_uprintf("usb_device: Unkown EP0 OUT: 0x%x!!\n", devReq->bRequest);
    }
    return 0;
}

int usb_dev_ep0xfer(struct usb_devparams *var4)
{
    int var5 = 0;
    struct usb_device_request *var6;
    int var7;
    int var8;
    int var9;

    var6 = (struct usb_device_request*)var4->readBuf;
    if(var4->uparam20 != 2) {
        return 0;
    }
    if(var4->ep0TxInterruptCount == 0) {
        return 0;
    }
    --var4->ep0TxInterruptCount;
    usb_select_ep(var4, 0);
    var5 = readw(var4->usbRegBase + 0x82);
    if(var4->uparam592 == 1) {
        if(var5 & 1) {
            var4->uparam592 = 0;
        } else { 
            if(var5 & 0x10) {
                usb_set_ep0_csr(var4, 128);
                wlibc_uprintf("WRN: EP0 Setup End!!\n");
            } else { 
                if((var5 & 2) == 0) {
                    if(var4->uparam600 != 0) {
                        var4->uparam596 += var4->uparam588;
                        if(var4->uparam600 == -1) {
                            var7 = 1;
                            var8 = 0;
                            var4->uparam600 = 0;
                        } else { 
                            if(var4->uparam600 < var4->uparam588) {
                                var7 = 1;
                                var8 = var4->uparam600;
                                var4->uparam600 = 0;
                            } else { 
                                if(var4->uparam600 == var4->uparam588) {
                                    var7 = 0;
                                    var8 = var4->uparam600;
                                    var4->uparam600 = -1;
                                } else { 
                                    var7 = 0;
                                    var8 = var4->uparam588;
                                    var4->uparam600 -= var4->uparam588;
                                }
                            }
                        }
                        usb_write_ep_fifo(var4, 0, var4->uparam596, var8);
                        if(var7 != 0 || var8 == 0) {
                            usb_set_ep0_csr(var4, 10);
                        }else{
                            usb_set_ep0_csr(var4, 2);
                        }
                        if(usb_get_ep0_count(var4) != 0) {
                            var9 = usb_get_ep0_count(var4);
                            wlibc_uprintf("Error: COUNT0 = 0x%x\n", var9);
                        }
                    }
                } else { 
                    wlibc_uprintf(
                            "WRN: Unkown EP0 Interrupt, CSR=0x%x!!\n", var5);
                }
            }
        }
    }
    if(var4->uparam592 == 0) {
        if(var5 & 1) {
            var9 = usb_get_ep0_count(var4);
            if(var9 == 8) {
                var4->ep0TxInterruptCount = 0;
                usb_read_ep_fifo(var4, 0, var4->readBuf, 8);
                if(var6->bmRequestType & 0x80) {
                    usb_set_ep0_csr(var4, 64);
                    fnL42806DF0(var4);
                    if(var4->uparam600 < var4->uparam588) {
                        var7 = 1;
                        var8 = var4->uparam600;
                        var4->uparam600 = 0;
                    } else { 
                        if(var4->uparam600 == var4->uparam588) {
                            var7 = 0;
                            var8 = var4->uparam600;
                            var4->uparam600 = -1 - 0;
                        } else { 
                            var7 = 0;
                            var8 = var4->uparam588;
                            var4->uparam600 -= var4->uparam588;
                        }
                    }
                    usb_write_ep_fifo(var4, 0, var4->uparam596, var8);
                    if(var7 != 0 || var8 == 0) {
                        usb_set_ep0_csr(var4, 10);
                    }else{
                        usb_set_ep0_csr(var4, 2);
                    }
                    var4->uparam592 = 1;
                } else { 
                    usb_set_ep0_csr(var4, 72);
                    var4->uparam592 = 0;
                }
            } else { 
                writew(readw(var4->usbRegBase + 130) | 0x100, var4->usbRegBase + 130);
                wlibc_uprintf("Error: EP0 Rx Error Length = 0x%x\n", var9);
            }
        } else { 
            fnL4280722C(var4);
        }
    }
    return 1;
}

int usb_bus_irq_handler_dev(struct usb_devparams *var4)
{
    int intusbe;
    int ep;
    int epSave;

    epSave = usb_get_active_ep(var4);
    if(var4->uparam20 != 2) {
        return 0;
    }
    if(var4->uparam40 == 0) {
        return 0;
    }
    usb_select_ep(var4, 0);
    --var4->uparam40;
    intusbe = readb(var4->usbRegBase + 0x50); /* enabled USB interrupts */
    if( var4->intusb & intusbe & 1 ) { /* USBC_INTUSB_SUSPEND */
        var4->intusb &= ~1;
        var4->uparam48 = 1;
        if(svc_96(var4->uparam868) != 0) {
            wlibc_uprintf(
                    "Error: DMA for EP is not finished after Bus Suspend\n");
        }
        svc_93(var4->uparam868);
        wlibc_uprintf("uSuspend\n");
    }
    if( var4->intusb & intusbe & 2 ) { /* USBC_INTUSB_RESUME */
        var4->intusb &= ~2;
        var4->uparam48 = 0;
        wlibc_uprintf("uResume\n");
    }
    if( var4->intusb & intusbe & 4 ) { /* USBC_INTUSB_RESET */
        var4->intusb &= ~4;
        var4->uparam44 = 1;
        var4->uparam52 = 1;
        var4->uparam48 = 0;
        ++var4->uparam60;
        for(ep = 0; ep < 5; ++ep) {
            var4->txInterruptCount[ep] = 0;
            var4->rxInterruptCount[ep] = 0;
            var4->eptx_xfer_state[ep] = 0;
            var4->eprx_xfer_state[ep] = 0;
        }
        var4->uparam688 = 0;
        var4->transferState = 0;
        var4->uparam876 = 0;
        usb_set_dev_addr(var4, 0);
        writeb(readb(var4->usbRegBase + 0x50) | 247, var4->usbRegBase + 0x50);
        writew(readw(var4->usbRegBase + 0x48) | 0xffff, var4->usbRegBase + 0x48);
        writew(readw(var4->usbRegBase + 0x4a) | 0xfffe, var4->usbRegBase + 0x4a);
        if(svc_96(var4->uparam868) != 0) {
            wlibc_uprintf(
                    "Error: DMA for EP is not finished after Bus Reset\n");
        }
        svc_93(var4->uparam868);
        wlibc_uprintf("uReset\n");
    }
    if( var4->intusb & intusbe & 32 ) { /* USBC_INTUSB_DISCONNECT */
        var4->intusb &= ~32;
        var4->uparam44 = 0;
        var4->uparam52 = 0;
        var4->uparam48 = 1;
        wlibc_uprintf("uSessend\n");
    }
    usb_select_ep(var4, epSave);
    return 0;
}

static void usb_struct_init(struct usb_devparams *var0)
{
    int ep;

    var0->uparam16 = 0;
    var0->uparam40 = 0;
    var0->intusb = 0;
    var0->uparam44 = 0;
    var0->uparam52 = 0;
    var0->uparam48 = 1;
    var0->uparam60 = 0;
    var0->ep0TxInterruptCount = 0;
    var0->uparam592 = 0;
    var0->uparam588 = 64;
    for(ep = 0; ep < 5; ++ep) {
        var0->txInterruptCount[ep] = 0;
        var0->rxInterruptCount[ep] = 0;
        var0->eptx_xfer_state[ep] = 0;
        var0->eprx_xfer_state[ep] = 0;
    }
    var0->uparam688 = 0;
    var0->transferState = 0;
    var0->epTransfer = 1;
    var0->epRead = 1;
    var0->uparam876 = 0;
    var0->uparam884 = 0;
    L4280A3F8 = 0;
    L4280A3FC = 0;
    gTxInterruptCount = 0;
    gRxInterruptCount = 0;
}

static int usb_init(struct usb_devparams *var4)
{
    usb_struct_init(var4);
    if(var4->uparam0 == 0) {
        /*PIO #7, CFG1 register: PA14_SELECT := output */
        writel((readl(0x01C20800 + 0x100) & ~0x7000000) | 0x1000000,
                0x01C20800 + 0x100);
        /*PIO #7, DAT register */
        writel(readl(0x01C20800 + 0x10c) & ~0x4000, 0x01C20800 + 0x10c);
    }
    writel(readl(var4->usbRegBase + 0x400) | 0x4000, var4->usbRegBase + 0x400);
    writel(readl(var4->usbRegBase + 0x400) | 0x8000, var4->usbRegBase + 0x400);
    if(var4->usbSpeed == 2) {
        /*high speed disable */
        writeb(readb(var4->usbRegBase + 0x40) & ~0x20, var4->usbRegBase + 0x40);
    } else { 
        /*high speed enable */
        writeb(readb(var4->usbRegBase + 0x40) | 0x20, var4->usbRegBase + 0x40);
    }
    writeb(readb(var4->usbRegBase + 0x40) | 1, var4->usbRegBase + 0x40);
    writel(readl(var4->usbRegBase + 0x400) & ~0xc00, var4->usbRegBase + 0x400);
    writel(readl(var4->usbRegBase + 0x400) | 1 << 10, var4->usbRegBase + 0x400);
    writel(readl(var4->usbRegBase + 0x400) & ~0x3000, var4->usbRegBase + 0x400);
    writel(readl(var4->usbRegBase + 0x400) | 0x1000, var4->usbRegBase + 0x400);
    writel(readl(var4->usbRegBase + 0x400) | 0x2000, var4->usbRegBase + 0x400);
    writeb(0, var4->usbRegBase + 66);
    writew(readw(var4->usbRegBase + 0x82) | 0x100, var4->usbRegBase + 0x82);
    wlibc_uprintf("USB Device!!\n");
    var4->uparam20 = 2;
    writeb(readb(var4->usbRegBase + 0x50) & ~0xff, var4->usbRegBase + 0x50);
    writeb(readb(var4->usbRegBase + 0x50) | 0xf7, var4->usbRegBase + 0x50);
    writew(readw(var4->usbRegBase + 0x48) | 0x3f, var4->usbRegBase + 0x48);
    writew(readw(var4->usbRegBase + 0x4a) | 0x3e, var4->usbRegBase + 0x4a);
    return 1;
}

static void usb_clock_init(void)
{
    writel(readl(0x01C20000 + 96) | 1, 0x01C20000 + 96);
    writel(0x100, 0x01C20000 + 204);
    sdelay(256);
    writel(0x101, 0x01C20000 + 204);
    writel(readl(0x1C00000 + 4) | 1, 0x1C00000 + 4);
}

static void usb_clock_exit(void)
{
    int var0;

    writel(readl(0x1C00000 + 4) & ~1, 0x1C00000 + 4);
    var0 = readl(0x1C20000 + 204) & ~0x100;
    writel(var0, 0x1C20000 + 204);
    sdelay(256);
    writel(var0 & ~1, 0x1C20000 + 204);
    writel(readl(0x01C20000 + 96) & ~1, 0x01C20000 + 96);
}

static void TimerHandler(unsigned unused)
{
    if(*L4280A3A8 & 8) {
        wlibc_uprintf("usb set pc\n");
        *L4280A3A8 &= ~8;
    } else { 
        usb_clock_exit();
        svc_interrupt_disable(gDevParams.interruptNo);
        power_set_usbdc();
    }
    L4280A408 = 0;
}

static void usb_interrupt_handler(void)
{
    struct usb_devparams *var4;
    int var5;

    var4 = &gDevParams;
    var5 = usb_get_bus_interrupt_status(var4);
    usb_clear_bus_interrupt_status(var4, var5);
    if((var5 & 4) == 0) /* USBC_BP_INTUSB_RESET */
        return;
    usb_clock_exit();
    svc_interrupt_disable(var4->interruptNo);
    *L4280A3A8 |= 8;
    svc_disable_timer(timerHandle);
    svc_release_timer(timerHandle);
    timerHandle = 0;
    power_set_usbpc();
    L4280A408 = 0;
}

int usb_detect_enter(void)
{
    gDevParams.uparam0 = 0;
    gDevParams.usbRegBase = 0x1C13000;
    gDevParams.interruptNo = 38;
    wlibc_uprintf("usb start detect\n");
    if(L4280A408 == 0) {
        wlibc_uprintf("usb enter detect\n");
        L4280A408 = 1;
        usb_clock_init();
        timerHandle = svc_request_timer(TimerHandler, 0);
        if(timerHandle == 0) {
            wlibc_uprintf("timer request fail\n");
        } else { 
            svc_enable_timer(timerHandle, 400, 0);
        }
        svc_interrupt_sethandler(gDevParams.interruptNo,
                usb_interrupt_handler, 0);
        svc_interrupt_enable(gDevParams.interruptNo);
        usb_init(&gDevParams);
        /* USBC_Dev_ConectSwitch(udc.bsp, USBC_DEVICE_SWITCH_ON); */
        writeb(readb(gDevParams.usbRegBase + 0x40) | 0x40,
                gDevParams.usbRegBase + 0x40);
    }
    return 0;
}

int usb_detect_exit(void)
{
    wlibc_uprintf("usb exit detect\n");
    L4280A408 = 0;
    usb_clock_exit();
    svc_interrupt_disable(gDevParams.interruptNo);
    if(timerHandle != 0) {
        svc_disable_timer(timerHandle);
        svc_release_timer(timerHandle);
        timerHandle = 0;
    }
    return 0;
}

static int checkPrivateKeyVal(int keyVal)
{
    int var5;
    int var6;
    int keyMin;
    int keyMax;

    var5 = svc_b0("private_key", "key_max", &keyMax, 1);
    var6 = svc_b0("private_key", "key_min", &keyMin, 1);
    if(var5 != 0) {
        wlibc_uprintf("unable to find private_key key_max value\n");
        return -1;
    }
    if(var6 != 0) {
        wlibc_uprintf("unable to find private_key key_min value\n");
        return -1;
    }
    wlibc_uprintf("key valye %d, usb key high %d, low %d\n",
            keyVal, keyMax, keyMin);
    if(keyVal >= keyMin && keyVal <= keyMax) {
        wlibc_uprintf("key found, try private mode\n");
        return 0;
    }
    return -1;
}

int check_private_part(int keyVal)
{
    int var5;

    var5 = svc_6c("DISK", "private", L428126E8);
    if(var5 == 0) {
        wlibc_uprintf("find private part\n");
        if(checkPrivateKeyVal(keyVal) == 0) {
            usb_start(1);
            usb_run();
        }
    }
    return 0;
}

void usb_power_polling_dev(struct usb_devparams *var4)
{
    if(var4->uparam20 != 2) {
        return;
    }
    if(var4->uparam44 != 0) {
        return;
    }
    if(((readb(var4->usbRegBase + 0x41) >> 3) & 0x3) == 3) {
        if(var4->uparam876 != 1) {
            if(var4->uparam876 != 0) {
                wlibc_uprintf("Error: Timer Ocuppied\n");
            }
            var4->uparam876 = 1;
            var4->uparam884 = 0;
            var4->uparam1144 = 80;
            return;
        }
        if(var4->uparam1144 != 0) {
            ++var4->uparam884;
            --var4->uparam1144;
            return;
        }
    } else { 
        var4->uparam876 = 0;
        var4->uparam884 = 0;
        return;
    }
    ++var4->uparam884;
    var4->uparam876 = 0;
    var4->uparam884 = 0;
    writeb(readb(var4->usbRegBase + 0x40) | 0x40, var4->usbRegBase + 0x40);
    var4->uparam44 = 1;
    wlibc_uprintf("USB Connect!!\n");
}

void usb_params_init(void)
{
    int var4;

    usb_clock_init();

    gDevParams.uparam0 = 0;
    gDevParams.usbRegBase = 0x01C13000;
    gDevParams.interruptNo = 38;
    gDevParams.uparam20 = 2;
    gDevParams.usbSpeed = 1;
    if(gDevParams.usbSpeed == 1) {
        gDevParams.devDesc = &USB_HS_BULK_DevDesc;
        gDevParams.configDesc = &USB_HS_BULK_ConfigDesc;
    } else { 
        gDevParams.devDesc = &USB_FS_BULK_DevDesc;
        gDevParams.configDesc = &USB_FS_BULK_ConfigDesc;
    }
    for(var4 = 0; var4 < 4; ++var4) {
        gDevParams.uparam700[var4] = StringDescriptors[var4];
    }
    gDevParams.hsBulkConfigDesc = &USB_HS_BULK_ConfigDesc;
    gDevParams.hsBulkConfigDesc2 = &USB_HS_BULK_ConfigDesc;
    gDevParams.devQualDesc = &USB_DevQual;
    gDevParams.otgDesc = &USB_OTGDesc;
    if(L4280A40C == 0) {
        gDevParams.lastLUNnum = svc_partcount(1) - 1;
        wlibc_uprintf("part count = %d\n", gDevParams.lastLUNnum + 1);
    } else { 
        gDevParams.uparam864 = 0;
        gDevParams.lastLUNnum = 0;
    }
    gDevParams.uparam856 = svc_alloc(131072);
    gDevParams.ep0TxInterruptCount = 0;
    gDevParams.uparam592 = 0;
    gDevParams.uparam868 = svc_90(1);
    if(gDevParams.uparam868 != 0)
        return;
    wlibc_uprintf("usb error: request dma fail\n");
}

static void interruptHandler(void)
{
    usb_irq_handler(&gDevParams);
}

static int enableInterrupts(void)
{
    svc_interrupt_sethandler(gDevParams.interruptNo, interruptHandler, 0);
    return svc_interrupt_enable(gDevParams.interruptNo);
}

int usb_start(int var4)
{
    L4280A40C = var4;
    usb_params_init();
    enableInterrupts();
    usb_init(&gDevParams);
    return 1;
}

int usb_device_function(struct usb_devparams *var4)
{
    if(var4->uparam20 != 2) {
        return 0;
    }
    usb_power_polling_dev(var4);
    usb_bus_irq_handler_dev(var4);
    usb_dev_ep0xfer(var4);
    usb_dev_bulk_xfer(var4);
    return 1;
}

int usb_run(void)
{
    int var4;

    while(1) {
        usb_device_function(&gDevParams);
        if(*L4280A3F0 == 2) {
            break;
        }
        var4 = svc_keypressed();
        if(var4 > 0) {
            break;
        }
    }
    usb_clock_exit();
    return 1;
}

