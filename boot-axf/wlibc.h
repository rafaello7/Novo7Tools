#ifndef WLIBC_H
#define WLIBC_H

void wlibc_uprintf(const char *fmt, ...);
void wlibc_ntprintf(const char *fmt, ...);
void wlibc_int_disable(void);
void wlibc_int_enable(void);

void sdelay(unsigned loops);

#endif /* WLIBC_H */
