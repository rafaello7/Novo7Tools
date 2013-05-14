#ifndef BOOTDISK_INTERFACE_H
#define BOOTDISK_INTERFACE_H

enum BootdiskCommand {
    BCMD_NONE       = 0,    /* no command */
    BCMD_DISKSIZE   = 'D',
    BCMD_GO_FEL     = 'F',
    BCMD_PING       = 'I',
    BCMD_GETLOG     = 'L',
    BCMD_SLEEPTEST  = 'S',
    BCMD_USBSPDTEST = 'U',
    BCMD_READ       = 'R',
    BCMD_WRITE      = 'W',
};

/* command header sent from host to device
 * 24 bytes structure, integer values are little endian
 */
struct bootdisk_cmd_header {
    uint16_t magic;         /* 0x1234 */
    uint16_t cmd;           /* BootdiskCommand */
    uint32_t datasize;      /* size of command data after header */
    uint64_t cmd_param1;    /* meaning dependent on command */
    uint64_t cmd_param2;
};

/* response header sent from device to host
 */
struct bootdisk_resp_header {
    uint16_t magic;         /* 0x1234 */
    uint16_t status;        /* 1 - OK, 0 - FAIL */
    uint32_t datasize;      /* size of response data after heder */
};

#endif /* BOOTDISK_INTERFACE_H */
