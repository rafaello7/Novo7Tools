#ifndef BOOTDISK_INTERFACE_H
#define BOOTDISK_INTERFACE_H

enum BootdiskCommand {
    BCMD_NONE               = 0,    /* no command */
    BCMD_FLASHMEM_PARAMS    = 'F',
    BCMD_BOARD_EXIT         = 'E',
    BCMD_PING               = 'I',
    BCMD_MEMJUMPTO          = 'J',
    BCMD_GETLOG             = 'L',
    BCMD_SLEEPTEST          = 'S',
    BCMD_DISKREAD           = 'R',
    BCMD_DISKWRITE          = 'W',
    BCMD_MEMREAD            = 'r',
    BCMD_MEMWRITE           = 'w',
    BCMD_MEMEXEC            = 'x',
    BCMD_MOUNT              = 'M',
    BCMD_GETMOUNT_STATUS    = 'm',
};

enum BootdiskResponseStatus {
    BRST_FAIL,
    BRST_OK,
    BRST_MSG
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
    uint32_t datasize;      /* size of response data after header */
};

enum FlashMemoryArea {
    FMAREA_BOOT0,
    FMAREA_BOOT1,
    FMAREA_PHYDISK,
    FMAREA_LOGDISK,
    FMAREA_COUNT
};

struct flashmarea_properties {
    uint32_t sectors_per_page;
    uint32_t first_pageno;
    uint32_t page_count;
};

struct flashmem_properties {
    struct flashmarea_properties areas[FMAREA_COUNT];
	uint32_t sectors_per_page;
	uint32_t pages_per_block;
	uint32_t blocks_per_chip;
	uint32_t chip_cnt;
	uint32_t pagewithbadflag; /*bad block flag was written at the first byte of spare area of this page*/
    uint32_t block_cnt_of_zone;
};

enum BoardExitMode {
    BE_GO_FEL       = 1,
    BE_BOARD_RESET  = 2
};

enum MountMode {
    MOUNTM_NOMOUNT,
    MOUNTM_RO,
    MOUNTM_RW
};

struct diskmount_status {
    uint32_t    mountMode;
    uint32_t    firstSector;
    uint32_t    sectorCount;
};

#endif /* BOOTDISK_INTERFACE_H */
