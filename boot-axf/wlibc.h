#ifndef WLIBC_H
#define WLIBC_H

#include <stdarg.h>

void wlibc_uprintf(const char *fmt, ...);
void wlibc_ntprintf(const char *fmt, ...);
int wlibc_vsprintf(char *buf, const char *fmt, va_list);
int wlibc_sprintf(char *buf, const char *fmt, ...);
void wlibc_int_disable(void);
void wlibc_int_enable(void);

void sdelay(unsigned loops);

#endif /* WLIBC_H */
