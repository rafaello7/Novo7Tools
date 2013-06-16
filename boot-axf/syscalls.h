#ifndef SYSCALLS_H
#define SYSCALLS_H


/* para:
 *      0 - buffer size 172, zeroes and sets only one member,
 *          at offset 32 - value in boot 1 at offset 0x30 + 0x81A0 + 32
 *      1 - gets data embedded in boot1 at offset 0x30, 33440 (0x82A0) bytes
 */
unsigned svc_getpara(int para, void *buf);


void svc_gofel(void);
int svc_05(void);
int svc_interrupt_sethandler(unsigned, void(*)(void), unsigned);
int svc_interrupt_enable(unsigned);
int svc_interrupt_disable(int);
int svc_bootos(int, int, void*, void*);
int svc_2a(int*);
int svc_2b(int*);
void *svc_alloc(unsigned size);
void svc_free(void *ptr);
void svc_38(int *p1, int *p2);
int svc_3a(void);
int svc_3b(void);
void svc_poweroff(void);
int svc_3e(void);
int svc_40(void);
int svc_41(int);
int svc_42(int);
void svc_49(const char *str);
void svc_4a(const char *str);
unsigned svc_read_uart(unsigned timeout);
int svc_50(const char *fname);
int svc_51(int);
int svc_52(int, int);
int svc_56(int p0, int p1, int p2, int *p3);
int svc_diskread(unsigned firstSector, int sectorCount, void *buf);
int svc_diskwrite(unsigned firstSector, int sectorCount, const void *data);

int svc_open(const char *fname, const char *mode);
int svc_read(void *buf, int p1, int size, int hdle);
int svc_write(void *buf, int p1, int size, int hdle);
int svc_63(int hdle);
int svc_filesize(int hdle);
void svc_65(int hdle, int p1, int p2);
void svc_close(int hdle);
void svc_deletefile(const char *fname);

int svc_6c(const char *p1, const char *p2, int*);
int svc_6e(int);
int svc_6f(int);
int svc_partcount(int);
int svc_80(void(*)(void), unsigned);
int svc_81(unsigned);
int svc_82(unsigned, int, int);
int svc_83(unsigned);
void svc_delay(unsigned ms);
int svc_90(int p1);
int svc_93(int p1);
int svc_96(int p1);
int svc_keypressed(void);
int svc_keyval(void);
int svc_b0(const char *target, const char *start, int *buf, int p);

#endif /* SYSCALLS_H */
