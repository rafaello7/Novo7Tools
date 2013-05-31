/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Based on Linux USB mass-storage gadget driver.
 */

#include <types.h>
#include <string.h>
#include <vsprintf.h>
#include <nand_bsp.h>
#include <fastboot.h>

/*
 *      SCSI opcodes
 */
#define TEST_UNIT_READY       0x00
#define REQUEST_SENSE         0x03
#define READ_6                0x08
#define WRITE_6               0x0a
#define INQUIRY               0x12
#define MODE_SELECT           0x15
#define MODE_SENSE            0x1a
#define START_STOP            0x1b
#define ALLOW_MEDIUM_REMOVAL  0x1e
#define READ_FORMAT_CAPACITIES 0x23
#define READ_CAPACITY         0x25
#define READ_10               0x28
#define WRITE_10              0x2a
#define SEEK_10               0x2b
#define VERIFY                0x2f
#define SYNCHRONIZE_CACHE     0x35
#define MODE_SELECT_10        0x55
#define MODE_SENSE_10         0x5a
#define READ_12               0xa8
#define WRITE_12              0xaa

#define TYPE_DISK           0x00

#define LDBG(lun, fmt, args...)
#define DBG(d, fmt, args...)
#define VDBG(d, fmt, args...)


#define USB_BULK_IN_FLAG    0x80

#define USB_BULK_CS_WRAP_LEN    13
#define USB_BULK_CS_SIG     0x53425355  /* Spells out 'USBS' */
#define USB_STATUS_PASS     0
#define USB_STATUS_FAIL     1
#define USB_STATUS_PHASE_ERROR  2


/* SCSI Sense Key/Additional Sense Code/ASC Qualifier values */
#define SS_NO_SENSE             0
#define SS_COMMUNICATION_FAILURE        0x040800
#define SS_INVALID_COMMAND          0x052000
#define SS_INVALID_FIELD_IN_CDB         0x052400
#define SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE   0x052100
#define SS_LOGICAL_UNIT_NOT_SUPPORTED       0x052500
#define SS_MEDIUM_NOT_PRESENT           0x023a00
#define SS_MEDIUM_REMOVAL_PREVENTED     0x055302
#define SS_NOT_READY_TO_READY_TRANSITION    0x062800
#define SS_RESET_OCCURRED           0x062900
#define SS_SAVING_PARAMETERS_NOT_SUPPORTED  0x053900
#define SS_UNRECOVERED_READ_ERROR       0x031100
#define SS_WRITE_ERROR              0x030c02
#define SS_WRITE_PROTECTED          0x072700

#define SK(x)       ((u8) ((x) >> 16))  /* Sense Key byte, etc. */
#define ASC(x)      ((u8) ((x) >> 8))
#define ASCQ(x)     ((u8) (x))


/* Default size of buffer length. */
#define FSG_BUFLEN  ((u32)16384)

/* Maximal number of LUNs supported in mass storage function */
#define FSG_MAX_LUNS    8

enum data_direction {
    DATA_DIR_UNKNOWN = 0,
    DATA_DIR_FROM_HOST,
    DATA_DIR_TO_HOST,
    DATA_DIR_NONE
};

struct fsg_lun {
    int         is_open;
    u32      firstSector;
    u32      sectorCount;
    unsigned ro:1;          /* whether access to the LUN shall be read-only */
    unsigned removable:1;   /* whether LUN shall be indicated as removable */
    unsigned nofua:1;       /* whether ignore FUA flag in SCSI WRITE commands*/

    unsigned prevent_medium_removal:1;
    unsigned info_valid:1;
    u32     sense_data;
    u32     sense_data_info;
    u32     unit_attention_data;
};

struct fsg_common {
    unsigned int        nluns;
    struct fsg_lun      *luns;
    unsigned int        lun;
    struct fsg_lun      *curlun;

    u32         data_size_from_cmnd;
    u32         residue;

    unsigned int        phase_error;
    unsigned int        bad_lun_okay;
};

enum TransferErr {
    ERR_EINVAL  = 1,
    ERR_EIO
};

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

static struct SCSICommandBlockWrapper cur_cmd;
static unsigned cur_cmd_bytes_written;
extern char *masstorage_buffer;


static inline u32 get_unaligned_be32(const u8 *p)
{
    return p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
}

static inline void put_unaligned_be32(u32 val, u8 *p)
{
    p[0] = val >> 24;
    p[1] = val >> 16;
    p[2] = val >> 8;
    p[3] = val;
}

static inline u32 get_unaligned_be24(u8 *buf)
{
    return 0xffffff & (u32) get_unaligned_be32(buf - 1);
}

static inline u16 get_unaligned_be16(const u8 *p)
{
    return p[0] << 8 | p[1];
}

static inline void put_unaligned_be16(u16 val, u8 *p)
{
    p[0] = val >> 8;
    p[1] = val;
}

static inline u32 cpu_to_le32(u32 val)
{
    return val;
}

static void fsg_lun_close(struct fsg_lun *curlun)
{
    if (curlun->is_open) {
        //fput(curlun->filp);
        curlun->is_open = 0;
    }
}


/*
 * Sync the file data, don't bother with the metadata.
 * This code was copied from fs/buffer.c:sys_fdatasync().
 */
static int fsg_lun_fsync_sub(struct fsg_lun *curlun)
{
    if (curlun->ro || !curlun->is_open)
        return 0;
    //return vfs_fsync(filp, 1);
    return 0;
}

static int do_read(struct fsg_common *common)
{
    struct fsg_lun      *curlun = common->curlun;
    u32         lba, sectorCount;

    /*
     * Get the starting Logical Block Address and check that it's
     * not too big.
     */
    if (cur_cmd.CBWCB[0] == READ_6)
        lba = get_unaligned_be24(&cur_cmd.CBWCB[1]);
    else {
        lba = get_unaligned_be32(&cur_cmd.CBWCB[2]);

        /*
         * We allow DPO (Disable Page Out = don't save data in the
         * cache) and FUA (Force Unit Access = don't read from the
         * cache), but we don't implement them.
         */
        if ((cur_cmd.CBWCB[1] & ~0x18) != 0) {
            curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
            dolog("err: do_read, We allow DPO and FUA, but we don't implement them.\n");
            return -ERR_EINVAL;
        }
    }

    sectorCount = common->data_size_from_cmnd >> 9;
    if (lba >= curlun->sectorCount || lba + sectorCount > curlun->sectorCount){
        curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
        dolog("err: do_read, lba is too big. (%d, %d)\n", lba,
                (u32)curlun->sectorCount);
        return -ERR_EINVAL;
    }
    dolog(">> do_read: first=0x%x, count=0x%x\n", lba, sectorCount);
    if( NAND_LogicRead(curlun->firstSector + lba, sectorCount,
            masstorage_buffer) )
    {
        curlun->sense_data = SS_UNRECOVERED_READ_ERROR;
        curlun->sense_data_info = lba;
        curlun->info_valid = 1;
    }
    common->residue -= sectorCount << 9;
    return -ERR_EIO;        /* No default reply */
}


/*-------------------------------------------------------------------------*/

static int do_write(struct fsg_common *common)
{
    struct fsg_lun      *curlun = common->curlun;
    u32         lba, sectorCount;

    dolog(">> do_write\n");
    if (curlun->ro) {
        curlun->sense_data = SS_WRITE_PROTECTED;
        dolog("err: do_write, file is read only, can not write\n");
        return -ERR_EINVAL;
    }

    /*
     * Get the starting Logical Block Address and check that it's
     * not too big
     */
    if (cur_cmd.CBWCB[0] == WRITE_6)
        lba = get_unaligned_be24(&cur_cmd.CBWCB[1]);
    else {
        lba = get_unaligned_be32(&cur_cmd.CBWCB[2]);

        /*
         * We allow DPO (Disable Page Out = don't save data in the
         * cache) and FUA (Force Unit Access = write directly to the
         * medium).  We don't implement DPO; we implement FUA by
         * performing synchronous output.
         */
        if (cur_cmd.CBWCB[1] & ~0x18) {
            curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
            dolog("err: do_write, We allow DPO and FUA, We don't implement DPO.\n");
            return -ERR_EINVAL;
        }

        if (!curlun->nofua && (cur_cmd.CBWCB[1] & 0x08)) { /* FUA */
            //curlun->filp->f_flags |= O_SYNC;
        }
    }
    sectorCount = common->data_size_from_cmnd >> 9;
    if (lba >= curlun->sectorCount || lba + sectorCount > curlun->sectorCount) {
        curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
        dolog("err: do_write, lba is overwrite. (%d, %d)\n", (u32)lba, (u32)curlun->sectorCount);
        return -ERR_EINVAL;
    }

    if( NAND_LogicWrite(curlun->firstSector + lba, sectorCount, masstorage_buffer) ) {
        curlun->sense_data = SS_WRITE_ERROR;
        curlun->sense_data_info = lba;
        curlun->info_valid = 1;
        return -ERR_EIO;
    }
    common->residue -= sectorCount << 9;
    return -ERR_EIO;        /* No default reply */
}


/*-------------------------------------------------------------------------*/

static int do_synchronize_cache(struct fsg_common *common)
{
    struct fsg_lun  *curlun = common->curlun;
    int     rc;

    dolog(">> do_synchronize_cache\n");
    /* We ignore the requested LBA and write out all file's
     * dirty data buffers. */
    rc = fsg_lun_fsync_sub(curlun);
    if (rc)
        curlun->sense_data = SS_WRITE_ERROR;
    return 0;
}


/*-------------------------------------------------------------------------*/

static int do_verify(struct fsg_common *common)
{
    struct fsg_lun      *curlun = common->curlun;
    u32         lba;
    u32         verification_length;
    u32         amount_left;
    unsigned int        amount;

    dolog(">> do_verify\n");
    /*
     * Get the starting Logical Block Address and check that it's
     * not too big.
     */
    lba = get_unaligned_be32(&cur_cmd.CBWCB[2]);
    if (lba >= curlun->sectorCount) {
        curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
        return -ERR_EINVAL;
    }

    /*
     * We allow DPO (Disable Page Out = don't save data in the
     * cache) but we don't implement it.
     */
    if (cur_cmd.CBWCB[1] & ~0x10) {
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        return -ERR_EINVAL;
    }

    verification_length = get_unaligned_be16(&cur_cmd.CBWCB[7]);
    if (verification_length == 0)
        return -ERR_EIO;        /* No default reply */

    if(lba + verification_length > curlun->sectorCount) {
        curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
        curlun->sense_data_info = lba + verification_length - 1;
        curlun->info_valid = 1;
        return -ERR_EINVAL;
    }

    /* Prepare to carry out the file verify */
    amount_left = verification_length;

    /* Write out all the dirty buffers before invalidating them */
    fsg_lun_fsync_sub(curlun);

    /* Just try to read the requested blocks */
    while (amount_left > 0) {
        /* read no more than 2 MB (4096 sectors) to masstorage_buffer */
        amount = amount_left < 4096 ? amount_left : 4096;

        /* Perform the read */
        if( NAND_LogicRead(curlun->firstSector + lba, amount, masstorage_buffer))
        {
            LDBG(curlun, "error in file verify: %d\n", (int)amount);
            curlun->sense_data = SS_UNRECOVERED_READ_ERROR;
            curlun->sense_data_info = lba;
            curlun->info_valid = 1;
            break;
        }
        lba += amount;
        amount_left -= amount;
    }
    return 0;
}


/*-------------------------------------------------------------------------*/

static int do_inquiry(struct fsg_common *common)
{
    struct fsg_lun *curlun = common->curlun;
    u8  *buf = (u8 *) masstorage_buffer;
    char inquiry_string[8 + 16 + 4 + 1] = "Generic AllwinDisk      0.01";

    dolog(">> do_inquiry: curlun=%x, removable=%d\n", curlun, curlun->removable);
    if (!curlun) {      /* Unsupported LUNs are okay */
        common->bad_lun_okay = 1;
        memset(buf, 0, 36);
        buf[0] = 0x7f;      /* Unsupported, no device-type */
        buf[4] = 31;        /* Additional length */
        return 36;
    }

    buf[0] = TYPE_DISK;
    buf[1] = curlun->removable ? 0x80 : 0;
    buf[2] = 2;     /* ANSI SCSI level 2 */
    buf[3] = 2;     /* SCSI-2 INQUIRY data format */
    buf[4] = 31;        /* Additional length */
    buf[5] = 0;     /* No special options */
    buf[6] = 0;
    buf[7] = 0;
    memcpy(buf + 8, inquiry_string, sizeof inquiry_string);
    return 36;
}

static int do_request_sense(struct fsg_common *common)
{
    struct fsg_lun  *curlun = common->curlun;
    u8      *buf = (u8 *) masstorage_buffer;
    u32     sd, sdinfo;
    int     valid;

    dolog(">> do_request_sense\n");
    /*
     * From the SCSI-2 spec., section 7.9 (Unit attention condition):
     *
     * If a REQUEST SENSE command is received from an initiator
     * with a pending unit attention condition (before the target
     * generates the contingent allegiance condition), then the
     * target shall either:
     *   a) report any pending sense data and preserve the unit
     *  attention condition on the logical unit, or,
     *   b) report the unit attention condition, may discard any
     *  pending sense data, and clear the unit attention
     *  condition on the logical unit for that initiator.
     *
     * FSG normally uses option a); enable this code to use option b).
     */
#if 0
    if (curlun && curlun->unit_attention_data != SS_NO_SENSE) {
        curlun->sense_data = curlun->unit_attention_data;
        curlun->unit_attention_data = SS_NO_SENSE;
    }
#endif

    if (!curlun) {      /* Unsupported LUNs are okay */
        common->bad_lun_okay = 1;
        sd = SS_LOGICAL_UNIT_NOT_SUPPORTED;
        sdinfo = 0;
        valid = 0;
    } else {
        sd = curlun->sense_data;
        sdinfo = curlun->sense_data_info;
        valid = curlun->info_valid << 7;
        curlun->sense_data = SS_NO_SENSE;
        curlun->sense_data_info = 0;
        curlun->info_valid = 0;
    }

    memset(buf, 0, 18);
    buf[0] = valid | 0x70;          /* Valid, current error */
    buf[2] = SK(sd);
    put_unaligned_be32(sdinfo, &buf[3]);    /* Sense information */
    buf[7] = 18 - 8;            /* Additional sense length */
    buf[12] = ASC(sd);
    buf[13] = ASCQ(sd);
    return 18;
}

static int do_read_capacity(struct fsg_common *common)
{
    struct fsg_lun  *curlun = common->curlun;
    u32     lba = get_unaligned_be32(&cur_cmd.CBWCB[2]);
    int     pmi = cur_cmd.CBWCB[8];
    u8      *buf = (u8 *)masstorage_buffer;

    dolog(">> do_read_capacity\n");
    /* Check the PMI and LBA fields */
    if (pmi > 1 || (pmi == 0 && lba != 0)) {
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        return -ERR_EINVAL;
    }

    put_unaligned_be32(curlun->sectorCount - 1, &buf[0]);
                        /* Max logical block */
    put_unaligned_be32(512, &buf[4]);   /* Block length */
    return 8;
}

static int do_mode_sense(struct fsg_common *common)
{
    struct fsg_lun  *curlun = common->curlun;
    int     mscmnd = cur_cmd.CBWCB[0];
    u8      *buf = (u8 *) masstorage_buffer;
    u8      *buf0 = buf;
    int     pc, page_code;
    int     changeable_values, all_pages;
    int     valid_page = 0;
    int     len, limit;

    dolog(">> do_mode_sense\n");
    if ((cur_cmd.CBWCB[1] & ~0x08) != 0) {  /* Mask away DBD */
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        return -ERR_EINVAL;
    }
    pc = cur_cmd.CBWCB[2] >> 6;
    page_code = cur_cmd.CBWCB[2] & 0x3f;
    if (pc == 3) {
        curlun->sense_data = SS_SAVING_PARAMETERS_NOT_SUPPORTED;
        return -ERR_EINVAL;
    }
    changeable_values = (pc == 1);
    all_pages = (page_code == 0x3f);

    /*
     * Write the mode parameter header.  Fixed values are: default
     * medium type, no cache control (DPOFUA), and no block descriptors.
     * The only variable value is the WriteProtect bit.  We will fill in
     * the mode data length later.
     */
    memset(buf, 0, 8);
    if (mscmnd == MODE_SENSE) {
        buf[2] = (curlun->ro ? 0x80 : 0x00);        /* WP, DPOFUA */
        buf += 4;
        limit = 255;
    } else {            /* MODE_SENSE_10 */
        buf[3] = (curlun->ro ? 0x80 : 0x00);        /* WP, DPOFUA */
        buf += 8;
        limit = 65535;      /* Should really be FSG_BUFLEN */
    }

    /* No block descriptors */

    /*
     * The mode pages, in numerical order.  The only page we support
     * is the Caching page.
     */
    if (page_code == 0x08 || all_pages) {
        valid_page = 1;
        buf[0] = 0x08;      /* Page code */
        buf[1] = 10;        /* Page length */
        memset(buf+2, 0, 10);   /* None of the fields are changeable */

        if (!changeable_values) {
            buf[2] = 0x04;  /* Write cache enable, */
                    /* Read cache not disabled */
                    /* No cache retention priorities */
            put_unaligned_be16(0xffff, &buf[4]);
                    /* Don't disable prefetch */
                    /* Minimum prefetch = 0 */
            put_unaligned_be16(0xffff, &buf[8]);
                    /* Maximum prefetch */
            put_unaligned_be16(0xffff, &buf[10]);
                    /* Maximum prefetch ceiling */
        }
        buf += 12;
    }

    /*
     * Check that a valid page was requested and the mode data length
     * isn't too long.
     */
    len = buf - buf0;
    if (!valid_page || len > limit) {
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        return -ERR_EINVAL;
    }

    /*  Store the mode data length */
    if (mscmnd == MODE_SENSE)
        buf0[0] = len - 1;
    else
        put_unaligned_be16(len - 2, buf0);
    return len;
}

static int do_start_stop(struct fsg_common *common)
{
    struct fsg_lun  *curlun = common->curlun;
    int     loej, start;

    dolog(">> do_start_stop\n");
    if (!curlun) {
        return -ERR_EINVAL;
    } else if (!curlun->removable) {
        curlun->sense_data = SS_INVALID_COMMAND;
        return -ERR_EINVAL;
    } else if ((cur_cmd.CBWCB[1] & ~0x01) != 0 || /* Mask away Immed */
           (cur_cmd.CBWCB[4] & ~0x03) != 0) { /* Mask LoEj, Start */
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        return -ERR_EINVAL;
    }

    loej  = cur_cmd.CBWCB[4] & 0x02;
    start = cur_cmd.CBWCB[4] & 0x01;

    /*
     * Our emulation doesn't support mounting; the medium is
     * available for use as soon as it is loaded.
     */
    if (start) {
        if (!curlun->is_open) {
            curlun->sense_data = SS_MEDIUM_NOT_PRESENT;
            return -ERR_EINVAL;
        }
        return 0;
    }

    /* Are we allowed to unload the media? */
    if (curlun->prevent_medium_removal) {
        LDBG(curlun, "unload attempt prevented\n");
        curlun->sense_data = SS_MEDIUM_REMOVAL_PREVENTED;
        return -ERR_EINVAL;
    }

    if (!loej)
        return 0;
    fsg_lun_close(curlun);
    return 0;
}

static int do_prevent_allow(struct fsg_common *common)
{
    struct fsg_lun  *curlun = common->curlun;
    int     prevent;

    dolog(">> do_prevent_allow: prevent=%d\n", cur_cmd.CBWCB[4] & 0x01);
    if (!common->curlun) {
        return -ERR_EINVAL;
    } else if (!common->curlun->removable) {
        common->curlun->sense_data = SS_INVALID_COMMAND;
        return -ERR_EINVAL;
    }

    prevent = cur_cmd.CBWCB[4] & 0x01;
    if ((cur_cmd.CBWCB[4] & ~0x01) != 0) {  /* Mask away Prevent */
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        return -ERR_EINVAL;
    }

    if (curlun->prevent_medium_removal && !prevent)
        fsg_lun_fsync_sub(curlun);
    curlun->prevent_medium_removal = prevent;
    return 0;
}

static int do_read_format_capacities(struct fsg_common *common)
{
    struct fsg_lun  *curlun = common->curlun;
    u8      *buf = (u8 *) masstorage_buffer;

    dolog(">> do_read_format_capacities\n");
    buf[0] = buf[1] = buf[2] = 0;
    buf[3] = 8; /* Only the Current/Maximum Capacity Descriptor */
    buf += 4;

    put_unaligned_be32(curlun->sectorCount, &buf[0]);
                        /* Number of blocks */
    put_unaligned_be32(512, &buf[4]);   /* Block length */
    buf[4] = 0x02;              /* Current capacity */
    return 12;
}

static int do_mode_select(struct fsg_common *common)
{
    struct fsg_lun  *curlun = common->curlun;

    dolog(">> do_mode_select\n");
    /* We don't support MODE SELECT */
    if (curlun)
        curlun->sense_data = SS_INVALID_COMMAND;
    return -ERR_EINVAL;
}


static void finish_reply(struct fsg_common *common)
{
    if( cur_cmd.dCBWDataTransferLength != 0 &&
            (cur_cmd.bmCBWFlags & USB_BULK_IN_FLAG) )
    {
        masstorage_tx(masstorage_buffer, cur_cmd.dCBWDataTransferLength);
    }
}

static void send_status(struct fsg_common *common)
{
    struct fsg_lun      *curlun = common->curlun;
    struct SCSICommandStatusWrapper csw;
    u8          status = USB_STATUS_PASS;
    u32         sd, sdinfo = 0;

    if (curlun) {
        sd = curlun->sense_data;
        sdinfo = curlun->sense_data_info;
    } else if (common->bad_lun_okay)
        sd = SS_NO_SENSE;
    else
        sd = SS_LOGICAL_UNIT_NOT_SUPPORTED;

    if (common->phase_error) {
        DBG(common, "sending phase-error status\n");
        status = USB_STATUS_PHASE_ERROR;
        sd = SS_INVALID_COMMAND;
    } else if (sd != SS_NO_SENSE) {
        DBG(common, "sending command-failure status\n");
        status = USB_STATUS_FAIL;
        VDBG(common, "  sense data: SK x%02x, ASC x%02x, ASCQ x%02x;"
                "  info x%x\n",
                SK(sd), ASC(sd), ASCQ(sd), sdinfo);
    }

    dolog("         send_status: %d, sd=%d\n", status, sd);
    csw.dCSWSignature = cpu_to_le32(USB_BULK_CS_SIG);
    csw.dCSWTag = cur_cmd.dCBWTag;
    csw.dCSWDataResidue = cpu_to_le32(common->residue);
    csw.bCSWStatus = status;
    masstorage_tx(&csw, USB_BULK_CS_WRAP_LEN);
}


/*-------------------------------------------------------------------------*/

/*
 * Check whether the command is properly formed and whether its data size
 * and direction agree with the values we already have.
 */
static int check_command(struct fsg_common *common, int cmnd_size,
             enum data_direction data_dir, unsigned int mask,
             int needs_medium, const char *name)
{
    int         i;
    int         lun = cur_cmd.CBWCB[1] >> 5;
    static const char   dirletter[4] = {'u', 'o', 'i', 'n'};
    char            hdlen[20];
    struct fsg_lun      *curlun;
    enum data_direction cbw_data_dir = cur_cmd.dCBWDataTransferLength == 0 ?
        DATA_DIR_NONE : (cur_cmd.bmCBWFlags & USB_BULK_IN_FLAG ?
                DATA_DIR_TO_HOST : DATA_DIR_FROM_HOST);

    hdlen[0] = 0;
    sprintf(hdlen, ", H%c=%u", dirletter[(int) cbw_data_dir],
            cur_cmd.dCBWDataTransferLength);
    VDBG(common, "SCSI command: %s;  Dc=%d, D%c=%u;  Hc=%d%s\n",
         name, cmnd_size, dirletter[(int) data_dir],
         common->data_size_from_cmnd, common->cmnd_size, hdlen);

    /*
     * We can't reply at all until we know the correct data direction
     * and size.
     */
    if (common->data_size_from_cmnd == 0)
        data_dir = DATA_DIR_NONE;
    if (cur_cmd.dCBWDataTransferLength < common->data_size_from_cmnd) {
        /*
         * Host data size < Device data size is a phase error.
         * Carry out the command, but only transfer as much as
         * we are allowed.
         */
        common->data_size_from_cmnd = cur_cmd.dCBWDataTransferLength;
        common->phase_error = 1;
    }
    common->residue = cur_cmd.dCBWDataTransferLength;

    /* Conflicting data directions is a phase error */
    if (cbw_data_dir != data_dir && common->data_size_from_cmnd > 0) {
        common->phase_error = 1;
        return -ERR_EINVAL;
    }

    /* Verify the length of the command itself */
    if (cmnd_size != cur_cmd.bCBWCBLength) {

        /*
         * Special case workaround: There are plenty of buggy SCSI
         * implementations. Many have issues with cbw->Length
         * field passing a wrong command size. For those cases we
         * always try to work around the problem by using the length
         * sent by the host side provided it is at least as large
         * as the correct command length.
         * Examples of such cases would be MS-Windows, which issues
         * REQUEST SENSE with cbw->Length == 12 where it should
         * be 6, and xbox360 issuing INQUIRY, TEST UNIT READY and
         * REQUEST SENSE with cbw->Length == 10 where it should
         * be 6 as well.
         */
        if (cmnd_size <= cur_cmd.bCBWCBLength) {
            DBG(common, "%s is buggy! Expected length %d "
                "but we got %d\n", name,
                cmnd_size, common->cmnd_size);
            cmnd_size = cur_cmd.bCBWCBLength;
        } else {
            common->phase_error = 1;
            return -ERR_EINVAL;
        }
    }

    /* Check that the LUN values are consistent */
    if (common->lun != lun)
        DBG(common, "using LUN %d from CBW, not LUN %d from CDB\n",
            common->lun, lun);

    /* Check the LUN */
    if (common->lun < common->nluns) {
        curlun = &common->luns[common->lun];
        common->curlun = curlun;
        if (cur_cmd.CBWCB[0] != REQUEST_SENSE) {
            curlun->sense_data = SS_NO_SENSE;
            curlun->sense_data_info = 0;
            curlun->info_valid = 0;
        }
    } else {
        common->curlun = NULL;
        curlun = NULL;
        common->bad_lun_okay = 0;

        /*
         * INQUIRY and REQUEST SENSE commands are explicitly allowed
         * to use unsupported LUNs; all others may not.
         */
        if (cur_cmd.CBWCB[0] != INQUIRY &&
            cur_cmd.CBWCB[0] != REQUEST_SENSE) {
            DBG(common, "unsupported LUN %d\n", common->lun);
            return -ERR_EINVAL;
        }
    }

    /*
     * If a unit attention condition exists, only INQUIRY and
     * REQUEST SENSE commands are allowed; anything else must fail.
     */
    if (curlun && curlun->unit_attention_data != SS_NO_SENSE &&
        cur_cmd.CBWCB[0] != INQUIRY &&
        cur_cmd.CBWCB[0] != REQUEST_SENSE) {
        curlun->sense_data = curlun->unit_attention_data;
        curlun->unit_attention_data = SS_NO_SENSE;
        return -ERR_EINVAL;
    }

    /* Check that only command bytes listed in the mask are non-zero */
    cur_cmd.CBWCB[1] &= 0x1f;           /* Mask away the LUN */
    for (i = 1; i < cmnd_size; ++i) {
        if (cur_cmd.CBWCB[i] && !(mask & (1 << i))) {
            if (curlun)
                curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
            return -ERR_EINVAL;
        }
    }

    /* If the medium isn't mounted and the command needs to access
     * it, return an error. */
    if (curlun && !curlun->is_open && needs_medium) {
        curlun->sense_data = SS_MEDIUM_NOT_PRESENT;
        return -ERR_EINVAL;
    }

    return 0;
}

static void do_scsi_command(struct fsg_common *common)
{
    int         reply = -ERR_EINVAL;
    int         i;
    static char     unknown[16];

    /* Wait for the next buffer to become available for data or status */
    common->phase_error = 0;

    switch (cur_cmd.CBWCB[0]) {

    case INQUIRY:
        common->data_size_from_cmnd = cur_cmd.CBWCB[4];
        reply = check_command(common, 6, DATA_DIR_TO_HOST,
                      (1<<4), 0,
                      "INQUIRY");
        if (reply == 0)
            reply = do_inquiry(common);
        break;

    case MODE_SELECT:
        common->data_size_from_cmnd = cur_cmd.CBWCB[4];
        reply = check_command(common, 6, DATA_DIR_FROM_HOST,
                      (1<<1) | (1<<4), 0,
                      "MODE SELECT(6)");
        if (reply == 0)
            reply = do_mode_select(common);
        break;

    case MODE_SELECT_10:
        common->data_size_from_cmnd =
            get_unaligned_be16(&cur_cmd.CBWCB[7]);
        reply = check_command(common, 10, DATA_DIR_FROM_HOST,
                      (1<<1) | (3<<7), 0,
                      "MODE SELECT(10)");
        if (reply == 0)
            reply = do_mode_select(common);
        break;

    case MODE_SENSE:
        common->data_size_from_cmnd = cur_cmd.CBWCB[4];
        reply = check_command(common, 6, DATA_DIR_TO_HOST,
                      (1<<1) | (1<<2) | (1<<4), 0,
                      "MODE SENSE(6)");
        if (reply == 0)
            reply = do_mode_sense(common);
        break;

    case MODE_SENSE_10:
        common->data_size_from_cmnd =
            get_unaligned_be16(&cur_cmd.CBWCB[7]);
        reply = check_command(common, 10, DATA_DIR_TO_HOST,
                      (1<<1) | (1<<2) | (3<<7), 0,
                      "MODE SENSE(10)");
        if (reply == 0)
            reply = do_mode_sense(common);
        break;

    case ALLOW_MEDIUM_REMOVAL:
        common->data_size_from_cmnd = 0;
        reply = check_command(common, 6, DATA_DIR_NONE,
                      (1<<4), 0,
                      "PREVENT-ALLOW MEDIUM REMOVAL");
        if (reply == 0)
            reply = do_prevent_allow(common);
        break;

    case READ_6:
        i = cur_cmd.CBWCB[4];
        common->data_size_from_cmnd = (i == 0 ? 256 : i) << 9;
        reply = check_command(common, 6, DATA_DIR_TO_HOST,
                      (7<<1) | (1<<4), 1,
                      "READ(6)");
        if (reply == 0)
            reply = do_read(common);
        break;

    case READ_10:
        common->data_size_from_cmnd =
                get_unaligned_be16(&cur_cmd.CBWCB[7]) << 9;
        reply = check_command(common, 10, DATA_DIR_TO_HOST,
                      (1<<1) | (0xf<<2) | (3<<7), 1,
                      "READ(10)");
        if (reply == 0)
            reply = do_read(common);
        break;

    case READ_12:
        common->data_size_from_cmnd =
                get_unaligned_be32(&cur_cmd.CBWCB[6]) << 9;
        reply = check_command(common, 12, DATA_DIR_TO_HOST,
                      (1<<1) | (0xf<<2) | (0xf<<6), 1,
                      "READ(12)");
        if (reply == 0)
            reply = do_read(common);
        break;

    case READ_CAPACITY:
        common->data_size_from_cmnd = 8;
        reply = check_command(common, 10, DATA_DIR_TO_HOST,
                      (0xf<<2) | (1<<8), 1,
                      "READ CAPACITY");
        if (reply == 0)
            reply = do_read_capacity(common);
        break;

    case READ_FORMAT_CAPACITIES:
        common->data_size_from_cmnd =
            get_unaligned_be16(&cur_cmd.CBWCB[7]);
        reply = check_command(common, 10, DATA_DIR_TO_HOST,
                      (3<<7), 1,
                      "READ FORMAT CAPACITIES");
        if (reply == 0)
            reply = do_read_format_capacities(common);
        break;

    case REQUEST_SENSE:
        common->data_size_from_cmnd = cur_cmd.CBWCB[4];
        reply = check_command(common, 6, DATA_DIR_TO_HOST,
                      (1<<4), 0,
                      "REQUEST SENSE");
        if (reply == 0)
            reply = do_request_sense(common);
        break;

    case START_STOP:
        common->data_size_from_cmnd = 0;
        reply = check_command(common, 6, DATA_DIR_NONE,
                      (1<<1) | (1<<4), 0,
                      "START-STOP UNIT");
        if (reply == 0)
            reply = do_start_stop(common);
        break;

    case SYNCHRONIZE_CACHE:
        common->data_size_from_cmnd = 0;
        reply = check_command(common, 10, DATA_DIR_NONE,
                      (0xf<<2) | (3<<7), 1,
                      "SYNCHRONIZE CACHE");
        if (reply == 0)
            reply = do_synchronize_cache(common);
        break;

    case TEST_UNIT_READY:
        dolog(">> TEST_UNIT_READY\n");
        common->data_size_from_cmnd = 0;
        reply = check_command(common, 6, DATA_DIR_NONE,
                0, 1,
                "TEST UNIT READY");
        break;

    /*
     * Although optional, this command is used by MS-Windows.  We
     * support a minimal version: BytChk must be 0.
     */
    case VERIFY:
        common->data_size_from_cmnd = 0;
        reply = check_command(common, 10, DATA_DIR_NONE,
                      (1<<1) | (0xf<<2) | (3<<7), 1,
                      "VERIFY");
        if (reply == 0)
            reply = do_verify(common);
        break;

    case WRITE_6:
        i = cur_cmd.CBWCB[4];
        common->data_size_from_cmnd = (i == 0 ? 256 : i) << 9;
        reply = check_command(common, 6, DATA_DIR_FROM_HOST,
                      (7<<1) | (1<<4), 1,
                      "WRITE(6)");
        if (reply == 0)
            reply = do_write(common);
        break;

    case WRITE_10:
        common->data_size_from_cmnd =
                get_unaligned_be16(&cur_cmd.CBWCB[7]) << 9;
        reply = check_command(common, 10, DATA_DIR_FROM_HOST,
                      (1<<1) | (0xf<<2) | (3<<7), 1,
                      "WRITE(10)");
        if (reply == 0)
            reply = do_write(common);
        break;

    case WRITE_12:
        common->data_size_from_cmnd =
                get_unaligned_be32(&cur_cmd.CBWCB[6]) << 9;
        reply = check_command(common, 12, DATA_DIR_FROM_HOST,
                      (1<<1) | (0xf<<2) | (0xf<<6), 1,
                      "WRITE(12)");
        if (reply == 0)
            reply = do_write(common);
        break;

    default:
        dolog(">> do_scsi_command: unknown cmd=0x%x\n", cur_cmd.CBWCB[0]);
        common->data_size_from_cmnd = 0;
        sprintf(unknown, "Unknown x%02x", cur_cmd.CBWCB[0]);
        reply = check_command(common, cur_cmd.bCBWCBLength,
                      DATA_DIR_UNKNOWN, ~0, 0, unknown);
        if (reply == 0) {
            common->curlun->sense_data = SS_INVALID_COMMAND;
            reply = -ERR_EINVAL;
        }
        break;
    }

    /* Set up the single reply buffer for finish_reply() */
    if (reply == -ERR_EINVAL)
        reply = 0;      /* Error reply length */
    if (reply >= 0 && cur_cmd.dCBWDataTransferLength != 0 &&
            (cur_cmd.bmCBWFlags & USB_BULK_IN_FLAG))
    {
        if( reply > common->data_size_from_cmnd )
            reply = common->data_size_from_cmnd;
        common->residue -= reply;
    }               /* Otherwise it's already set */
}

static struct fsg_lun lun = {
    .is_open = 0,
    .ro = 1,
    .removable = 1,
    .firstSector = 0,
    .sectorCount = 0
};

static struct fsg_common common = {
    .nluns = 1,
    .lun   = 0,
    .luns = &lun
};

int masstorage_rx_handler(const char *buffer, int buffer_size)
{
    if( cur_cmd.dCBWDataTransferLength == 0 || (cur_cmd.bmCBWFlags & 0x80) ||
            cur_cmd.dCBWDataTransferLength == cur_cmd_bytes_written )
    {
        /* a new command */
        if( buffer_size != sizeof(cur_cmd) ) {
            dolog("masstorage_rx_handler FATAL: wrong data size on input=%d\n",
                    buffer_size);
            return 0;
        }
        memcpy(&cur_cmd, buffer, sizeof(cur_cmd));
        if( cur_cmd.dCBWSignature != 0x43425355 ) {
            dolog("masstorage_rx_handler FATAL: wrong magic on cmd=%x\n",
                    cur_cmd.dCBWSignature);
            return 0;
        }
        if( cur_cmd.dCBWDataTransferLength > 0x1000000 ) {    /* max 16 MB */
            dolog("masstorage_rx_handler FATAL: too big data size on cmd=0x%x\n",
                    cur_cmd.dCBWDataTransferLength);
            return 0;
        }
        cur_cmd_bytes_written = 0;
    }else{
        /* command data */
        int to_write = cur_cmd.dCBWDataTransferLength - cur_cmd_bytes_written;
        if( buffer_size <= to_write )
            to_write = buffer_size;
        else
            dolog("masstorage_rx_handler: ignore extra %d bytes on input\n",
                    buffer_size - to_write);
        memcpy(masstorage_buffer + cur_cmd_bytes_written, buffer, to_write);
        cur_cmd_bytes_written += to_write;
    }
    if( cur_cmd.dCBWDataTransferLength == 0 || (cur_cmd.bmCBWFlags & 0x80) ||
            cur_cmd.dCBWDataTransferLength == cur_cmd_bytes_written )
    {
        do_scsi_command(&common);
        finish_reply(&common);
        send_status(&common);
    }
    return 0;
}

int masstorage_mount(unsigned firstSector, unsigned sectorCount, int read_write)
{
    int diskSectCount = NAND_GetDiskSize();
    dolog("masstorage_mount: first=%d, count=%d\n", firstSector, sectorCount);
    if( lun.is_open ) {
        dolog("masstorage_mount: please eject disk first\n");
        return -1;
    }
    if( firstSector >= diskSectCount || firstSector + sectorCount > diskSectCount )
    {
        dolog("masstorage_mount: requested parameters beyond disk size\n");
        return -1;
    }
    lun.firstSector = firstSector;
    lun.sectorCount = sectorCount;
    lun.ro = ! read_write;
    lun.is_open = 1;
    return 0;
}

