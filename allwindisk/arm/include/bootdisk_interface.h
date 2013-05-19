#ifndef BOOTDISK_INTERFACE_H
#define BOOTDISK_INTERFACE_H

enum BootdiskCommand {
    BCMD_NONE       = 0,    /* no command */
    BCMD_DISKSIZE   = 'D',
    BCMD_BOARD_EXIT = 'E',
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
    uint32_t param1;        /* meaning dependent on command */
    uint64_t start;
    uint32_t count;
    uint32_t datasize;      /* size of data after header */
};

/* response header sent from device to host
 */
struct bootdisk_resp_header {
    uint16_t magic;         /* 0x1234 */
    uint16_t status;        /* 1 - OK, 0 - FAIL */
    uint32_t datasize;      /* size of response data after heder */
};

enum FlashMemoryArea {
    FMAREA_BOOT0,
    FMAREA_BOOT1,
    FMAREA_DISK
};

enum BoardExitMode {
    BE_GO_FEL       = 1,
    BE_BOARD_RESET  = 2
};

#endif /* BOOTDISK_INTERFACE_H */
