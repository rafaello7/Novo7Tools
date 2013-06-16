#include "syscalls.h"

unsigned svc_getpara(int para, void *buf)
{
    unsigned res;
    asm("mov r0, %[para]; mov r1, %[buf]; svc 0x02; mov %[res], r0"
            : [res] "=r" (res)
            : [para] "r" (para), [buf] "r" (buf):
            "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

void svc_gofel(void)
{
    asm("svc 0x04");
}

int svc_05(void)
{
    int res;
    asm("svc 0x05; mov %[res], r0" : [res] "=r" (res)
            :: "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_bootos(int p1, int p2, void *p3, void *bootAddr)
{
    int res;
    asm("mov r0, %[p1]; mov r1, %[p2]; mov r2, %[p3];"
            " mov r3, %[bootAddr]; svc 0x06; mov %[res], r0"
        : [res] "=r" (res)
        : [p1] "r" (p1), [p2] "r" (p2),
          [p3] "r" (p3), [bootAddr] "r" (bootAddr)
        : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_interrupt_sethandler(unsigned interruptNo, void(*handler)(void),
        unsigned p2)
{
    int res;
    asm("mov r0, %[interruptNo]; mov r1, %[handler]; mov r2, %[p2]; svc 0x20;"
            " mov %[res], r0"
            : [res] "=r" (res)
            : [interruptNo] "r" (interruptNo), [handler] "r" (handler),
            [p2] "r" (p2)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_interrupt_enable(unsigned interruptNo)
{
    int res;
    asm("mov r0, %[interruptNo]; svc 0x22; mov %[res], r0"
            : [res] "=r" (res)
            : [interruptNo] "r" (interruptNo)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_interrupt_disable(int p1)
{
    int res;
    asm("mov r0, %[p1]; svc 0x23; mov %[res], r0"
            : [res] "=r" (res)
            : [p1] "r" (p1): "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_2a(int *p1)
{
    int res;
    asm("mov r0, %[p1]; svc 0x2a; mov %[res], r0"
            : [res] "=r" (res)
            :[p1] "r" (p1)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_2b(int *p1)
{
    int res;
    asm("mov r0, %[p1]; svc 0x2b; mov %[res], r0"
            : [res] "=r" (res)
            :[p1] "r" (p1)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

void *svc_alloc(unsigned size)
{
    void *res;
    asm("mov r0, %[size]; svc 0x30; mov %[res], r0" : [res] "=r" (res) :
            [size] "r" (size): "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

void svc_free(void *ptr)
{
    asm("mov r0, %[ptr]; svc 0x33" ::
            [ptr] "r" (ptr): "r0", "r1", "r2", "r3", "lr", "cc");
}

void svc_38(int *p1, int *p2)
{
    asm("mov r0, %[p1]; mov r1, %[p2]; svc 0x38"
            :: [p1] "r" (p1), [p2] "r" (p2)
            : "r0", "r1", "r2", "r3", "lr", "cc");
}

int svc_3a(void)
{
    int res;
    asm("svc 0x3a; mov %[res], r0"
            : [res] "=r" (res)
            :: "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_3b(void)
{
    int res;
    asm("svc 0x3b; mov %[res], r0" : [res] "=r" (res)
            :: "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

void svc_poweroff(void)
{
    asm("svc 0x3d");
}

int svc_3e(void)
{
    int res;
    asm("svc 0x3e; mov %[res], r0" : [res] "=r" (res)
            :: "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_40(void)
{
    int res;
    asm("svc 0x40; mov %[res], r0"
            : [res] "=r" (res)
            :: "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_41(int p1)
{
    int res;
    asm("mov r0, %[p1]; svc 0x41; mov %[res], r0"
            : [res] "=r" (res)
            : [p1] "r" (p1)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_42(int p1)
{
    int res;
    asm("mov r0, %[p1]; svc 0x42; mov %[res], r0"
            : [res] "=r" (res)
            : [p1] "r" (p1)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

void svc_49(const char *str)
{
    asm("mov r0, %[str]; svc 0x49"
            :: [str] "r" (str)
            : "r0", "r1", "r2", "r3", "lr", "cc");
}

void svc_4a(const char *str)
{
    asm("mov r0, %[str]; svc 0x4a"
            :: [str] "r" (str)
            : "r0", "r1", "r2", "r3", "lr", "cc");
}

/* Reads from serial0 (uart0) interface.
 * timeout param:
 *      0   - wait forever
 *      >0  - number of read attempts
 * returns:
 *      0   - timeout
 *      >0  - character value
 */
unsigned svc_read_uart(unsigned timeout)
{
    unsigned res;
    asm("mov r0, %[timeout]; svc 0x4f; mov %[res], r0" : [res] "=r" (res) :
            [timeout] "r" (timeout): "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_50(const char *fname)
{
    int res;
    asm("mov r0, %[fname]; svc 0x50; mov %[res], r0"
            : [res] "=r" (res)
            : [fname] "r" (fname)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_51(int val)
{
    int res;
    asm("mov r0, %[val]; svc 0x51; mov %[res], r0"
            : [res] "=r" (res)
            : [val] "r" (val)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_52(int val1, int val2)
{
    int res;
    asm("mov r0, %[val1]; mov r1, %[val2]; svc 0x52; mov %[res], r0"
            : [res] "=r" (res) :
            [val1] "r" (val1), [val2] "r" (val2)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_56(int p0, int p1, int p2, int *p3)
{
    int res;
    asm("mov r0, %[p0]; mov r1, %[p1]; mov r2, %[p2]; mov r3, %[p3];"
            " svc 0x56; mov %[res], r0"
            : [res] "=r" (res)
            : [p0] "r" (p0), [p1] "r" (p1), [p2] "r" (p2), [p3] "r" (p3)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_diskread(unsigned firstSector, int sectorCount, void *buf)
{
    int res;
    asm("mov r0, %[firstSector]; mov r1, %[sectorCount]; mov r2, %[buf];"
            " svc 0x5a; mov %[res], r0"
            : [res] "=r" (res)
            : [firstSector] "r" (firstSector), [sectorCount] "r" (sectorCount),
            [buf] "r" (buf)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_diskwrite(unsigned firstSector, int sectorCount, const void *data)
{
    int res;
    asm("mov r0, %[firstSector]; mov r1, %[sectorCount]; mov r2, %[data];"
            " svc 0x5b; mov %[res], r0"
            : [res] "=r" (res)
            : [firstSector] "r" (firstSector), [sectorCount] "r" (sectorCount),
            [data] "r" (data)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_open(const char *fname, const char *mode)
{
    int res;
    asm("mov r0, %[fname]; mov r1, %[mode]; svc 0x60; mov %[res], r0"
            : [res] "=r" (res)
            : [fname] "r" (fname), [mode] "r" (mode)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_read(void *buf, int p1, int size, int hdle)
{
    int res;
    asm("mov r0, %[buf]; mov r1, %[p1]; mov r2, %[size]; mov r3, %[hdle];"
           " svc 0x61; mov %[res], r0"
            : [res] "=r" (res)
            : [buf] "r" (buf), [p1] "r" (p1),
               [size] "r" (size), [hdle] "r" (hdle)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_write(void *buf, int p1, int size, int hdle)
{
    int res;
    asm("mov r0, %[buf]; mov r1, %[p1]; mov r2, %[size]; mov r3, %[hdle];"
           " svc 0x62; mov %[res], r0"
            : [res] "=r" (res)
            : [buf] "r" (buf), [p1] "r" (p1),
               [size] "r" (size), [hdle] "r" (hdle)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_63(int hdle)
{
    int res;

    asm("mov r0, %[hdle]; svc 0x63; mov %[res], r0"
            : [res] "=r" (res)
            : [hdle] "r" (hdle)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_filesize(int hdle)
{
    int res;

    asm("mov r0, %[hdle]; svc 0x64; mov %[res], r0"
            : [res] "=r" (res)
            : [hdle] "r" (hdle)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

void svc_65(int hdle, int p1, int p2)
{
    asm("mov r0, %[hdle]; mov r1, %[p1]; mov r2, %[p2]; svc 0x65"
            :: [hdle] "r" (hdle), [p1] "r" (p1), [p2] "r" (p2)
            : "r0", "r1", "r2", "r3", "lr", "cc");
}

void svc_close(int hdle)
{
    asm("mov r0, %[hdle]; svc 0x68"
            :: [hdle] "r" (hdle)
            : "r0", "r1", "r2", "r3", "lr", "cc");
}

void svc_deletefile(const char *fname)
{
    asm("mov r0, %[fname]; svc 0x69"
            :: [fname] "r" (fname)
            : "r0", "r1", "r2", "r3", "lr", "cc");
}

int svc_6c(const char *p1, const char *p2, int *p3)
{
    int res;
    asm("mov r0, %[p1]; mov r1, %[p2]; mov r2, %[p3];"
            " svc 0x6c; mov %[res], r0"
        : [res] "=r" (res)
        : [p1] "r" (p1), [p2] "r" (p2), [p3] "r" (p3)
        : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_partcount(int val)
{
    unsigned res;
    asm("mov r0, %[val]; svc 0x6d; mov %[res], r0"
            : [res] "=r" (res)
            : [val] "r" (val)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_6e(int p1)
{
    int res;

    asm("mov r0, %[p1]; svc 0x6e; mov %[res], r0"
            : [res] "=r" (res)
            : [p1] "r" (p1)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_6f(int p1)
{
    int res;

    asm("mov r0, %[p1]; svc 0x6f; mov %[res], r0"
            : [res] "=r" (res)
            : [p1] "r" (p1)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_80(void(*handler)(void), unsigned p2)
{
    int res;
    asm("mov r0, %[handler]; mov r1, %[p2]; svc 0x80; mov %[res], r0"
            : [res] "=r" (res)
            : [handler] "r" (handler), [p2] "r" (p2):
            "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_81(unsigned p1)
{
    int res;
    asm("mov r0, %[p1]; svc 0x81; mov %[res], r0"
            : [res] "=r" (res)
            : [p1] "r" (p1)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_82(unsigned p1, int p2, int p3)
{
    int res;
    asm("mov r0, %[p1]; mov r1, %[p2]; mov r2, %[p3]; svc 0x82; mov %[res], r0"
            : [res] "=r" (res)
            : [p1] "r" (p1), [p2] "r" (p2), [p3] "r" (p3)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_83(unsigned p1)
{
    int res;
    asm("mov r0, %[p1]; svc 0x83; mov %[res], r0"
            : [res] "=r" (res)
            : [p1] "r" (p1)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

void svc_delay(unsigned ms)
{
    asm("mov r0, %[ms]; svc 0x85" ::
            [ms] "r" (ms): "r0", "r1", "r2", "r3", "lr", "cc");
}

int svc_90(int p1)
{
    int res;
    asm("mov r0, %[p1]; svc 0x90; mov %[res], r0"
            : [res] "=r" (res)
            : [p1] "r" (p1)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_93(int p1)
{
    int res;
    asm("mov r0, %[p1]; svc 0x93; mov %[res], r0"
            : [res] "=r" (res)
            : [p1] "r" (p1)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_96(int p1)
{
    int res;
    asm("mov r0, %[p1]; svc 0x96; mov %[res], r0"
            : [res] "=r" (res)
            : [p1] "r" (p1)
            : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

/* returns:
 *  1   - a key was pressed
 *  0   - a key is held
 *  <0  - no key is held
 */
int svc_keypressed(void)
{
    int res;
    asm("svc 0xa8; mov %[res], r0" : [res] "=r" (res)
            :: "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_keyval(void)
{
    int res;
    asm("svc 0xa9; mov %[res], r0" : [res] "=r" (res)
            :: "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}

int svc_b0(const char *target, const char *start, int *buf, int p)
{
    int res;
    asm("mov r0, %[target]; mov r1, %[start]; mov r2, %[buf];"
            " mov r3, %[p]; svc 0xb0; mov %[res], r0"
        : [res] "=r" (res)
        : [target] "r" (target), [start] "r" (start),
           [buf] "r" (buf), [p] "r" (p)
        : "r0", "r1", "r2", "r3", "lr", "cc");
    return res;
}
