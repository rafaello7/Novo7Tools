/*
 * Copyright (C) 2012  Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libusb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <libgen.h>

struct  aw_usb_request {
    char signature[8];
    uint32_t length;
    uint32_t unknown1;	/* 0x0c000000 */
    uint16_t request;
    uint32_t length2;	/* Same as length */
    char	pad[10];
}  __attribute__((packed));

static libusb_device_handle *usb;

static void usb_bulk_send(const void *data, int length)
{
    int rc, sent;
    while (length > 0) {
        rc = libusb_bulk_transfer(usb, 1 | LIBUSB_ENDPOINT_OUT,
                (void*)data, length, &sent, 0);
        if (rc != 0) {
            fprintf(stderr, "usb_bulk_send error %d\n", rc);
            exit(2);
        }
        length -= sent;
        data += sent;
    }
}

static void usb_bulk_recv(void *data, int length)
{
    int rc, recv;
    while (length > 0) {
        rc = libusb_bulk_transfer(usb, 2 | LIBUSB_ENDPOINT_IN,
                data, length, &recv, 0);
        if (rc != 0) {
            fprintf(stderr, "usb_bulk_recv error %d\n", rc);
            exit(2);
        }
        length -= recv;
        data += recv;
    }
}

/*
 * send: AWUC 0 length 0x0c000000 type (2B) length pad
 */
static void aw_send_usb_request(int type, int length)
{
    struct aw_usb_request req;
    memset(&req, 0, sizeof(req));
    strcpy(req.signature, "AWUC");
    req.length = req.length2 = htole32(length);
    req.request = htole16(type);
    req.unknown1 = htole32(0x0c000000);
    usb_bulk_send(&req, sizeof(req));
}

/*
 * recv: AWUS + 9byte pad
 */
static void aw_read_usb_response(void)
{
    char buf[13];
    usb_bulk_recv(&buf, sizeof(buf));
    if( strcmp(buf, "AWUS") ) {
        printf("warn: bad response from device\n");
    }
}

/*
 * send: AWUC 0 length 0x0c000000 0x12 length pad
 * send: data (len bytes)
 * recv: AWUS + 9byte pad
 */
static void aw_usb_write(const void *data, size_t len)
{
    aw_send_usb_request(0x12, len);
    usb_bulk_send(data, len);
    aw_read_usb_response();
}

/*
 * send: AWUC 0 length 0x0c000000 0x11 length pad
 * recv: data (len bytes)
 * recv: AWUS + 9byte pad
 */
static void aw_usb_read(void *data, size_t len)
{
    aw_send_usb_request(0x11, len);
    usb_bulk_recv(data, len);
    aw_read_usb_response();
}

struct aw_fel_request {
    uint32_t request;
    uint32_t address;
    uint32_t length;
    uint32_t flags;
};

enum aw_fel_request_id {
    AW_FEL_VERSION = 0x001,
    AW_FEL_1_WRITE = 0x101,
    AW_FEL_1_EXEC  = 0x102,
    AW_FEL_1_READ  = 0x103
};

/*
 * send: AWUC 0 0x10 0x0c000000 0x12 0x10 0000000000
 * send: type addr length 0x0
 * recv: AWUS + 9byte pad
 */
static void aw_send_fel_request(int type, uint32_t addr,
        uint32_t length)
{
    struct aw_fel_request req;
    memset(&req, 0, sizeof(req));
    req.request = htole32(type);
    req.address = htole32(addr);
    req.length = htole32(length);
    req.flags = 0;
    aw_usb_write(&req, sizeof(req));
}

/*
 * send: AWUC 0 length 0x0c000000 0x11 length pad
 * recv: 8 bytes
 * recv: AWUS + 9byte pad
 */
static void aw_read_fel_status(void)
{
    char buf[8];
    aw_usb_read(&buf, sizeof(buf));
}

/* sends request which reads a data from device
 */
static void send_read_request(
        enum aw_fel_request_id reqno,   /* request number */
        unsigned address,               /* address to send */
        unsigned length,                /* length to send */
        void *buf,                      /* buffer for receive data */
        size_t to_read)                 /* buffer size */
{
    aw_send_fel_request(reqno, address, length);
    aw_usb_read(buf, to_read);
    aw_read_fel_status();
}

/* sends request which writes a data from device
 */
static void send_write_request(
        enum aw_fel_request_id reqno,   /* request number */
        unsigned address,               /* address to send */
        unsigned length,                /* length to send */
        const void *data,               /* data to send */
        uint32_t datasize)
{
    aw_send_fel_request(reqno, address, length);
    aw_usb_write(data, datasize);
    aw_read_fel_status();
}

static void send_exec_request(
        enum aw_fel_request_id reqno,
        unsigned address)
{
    aw_send_fel_request(reqno, address, 0);
    aw_read_fel_status();
}

static void aw_fel_get_version(libusb_device_handle *usb)
{
    struct aw_fel_version {
        char signature[8];
        uint32_t unknown_08;	/* 0x00162300 */
        uint32_t unknown_0a;	/* 1 */
        uint16_t protocol;	/* 1 */
        uint8_t  unknown_12;	/* 0x44 */
        uint8_t  unknown_13;	/* 0x08 */
        uint32_t scratchpad;	/* 0x7e00 */
        uint32_t pad[2];	/* unused */
    } __attribute__((packed)) buf;

    send_read_request(AW_FEL_VERSION, 0, 0, &buf, sizeof(buf));

    buf.unknown_08 = le32toh(buf.unknown_08);
    buf.unknown_0a = le32toh(buf.unknown_0a);
    buf.protocol = le32toh(buf.protocol);
    buf.scratchpad = le16toh(buf.scratchpad);
    buf.pad[0] = le32toh(buf.pad[0]);
    buf.pad[1] = le32toh(buf.pad[1]);

    printf("%.8s %08x %08x ver=%04x %02x %02x scratchpad=%08x %08x %08x\n",
            buf.signature, buf.unknown_08, buf.unknown_0a, buf.protocol,
            buf.unknown_12, buf.unknown_13, buf.scratchpad, buf.pad[0],
            buf.pad[1]);
}

static void aw_fel_read(uint32_t offset,
        void *buf, size_t len)
{
    send_read_request(AW_FEL_1_READ, offset, len, buf, len);
}

static int send_bootfile(const char *fname, unsigned load_addr)
{
    int fd = -1, rd;
    char buf[0x80000];
    char filepath[1000];

    /* check for freshly compiled version first */
    if( (rd = readlink("/proc/self/exe", buf, sizeof(buf))) > 0 )
    {
        buf[rd] = '\0';
        char *dname = dirname(buf);
        sprintf(filepath, "%s/arm/%s", dname, fname);
        fd = open(filepath, O_RDONLY);
        if( fd == -1 && errno != ENOENT ) {
            fprintf(stderr, "unable to open FEL bootfile %s: %s\n",
                    filepath, strerror(errno));
            return -1;
        }
    }
    if( fd == -1 ) {
        sprintf(filepath, "/usr/share/allwindisk/%s", fname);
        if( (fd = open(filepath, O_RDONLY)) == -1 ) {
            fprintf(stderr, "unable to open FEL bootfile %s\n", filepath);
            return -1;
        }
    }
    while( (rd = read(fd, buf, sizeof(buf))) > 0 ) {
        send_write_request(AW_FEL_1_WRITE, load_addr, rd, buf, rd);
        load_addr += rd;
    }
    close(fd);
    return 0;
}

int fel_ainol(void)
{
    int res;

    usb = libusb_open_device_with_vid_pid(NULL, 0x1f3a, 0xefe8);
    if (!usb)
        return -1;
    libusb_claim_interface(usb, 0);
    if( (res = send_bootfile("draminit.bin", 0x2000)) == 0 ) {
        send_exec_request(AW_FEL_1_EXEC, 0x2000);
    }
    if( (res = send_bootfile("diskboot.bin", 0x4A000000)) == 0 ) {
        send_exec_request(AW_FEL_1_EXEC, 0x4A000000);
    }
    libusb_release_interface(usb, 0);
    libusb_close(usb);
    return res;
}

