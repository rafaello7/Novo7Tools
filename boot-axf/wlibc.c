#include "syscalls.h"
#include "wlibc.h"
#include "fbprint.h"


int int_to_string_dec(int num, char *var5)
{
    int var6;
    int var7;
    int var8;
    char sp0[12];

    var6 = 0;
    if(num == 0) {
        *var5 = '0';
        var5[1] = 0;
        return 0;
    }
    if(num < 0) {
        var6 = 1;
        num = -num;
    }
    var8 = 0;
    while(num > 0) {
        sp0[var8++] = num % 10 + '0';
        num = num / 10;
    }
    var7 = 0;
    if(var6 == 1) {
        var5[var7++] = '-';
    }
    --var8;
    while(var8 >= 0) {
        var5[var7++] = sp0[var8--];
    }
    var5[var7] = '\0';
    return 0;
}

int Uint_to_string_dec(unsigned num, char *var5)
{
    int var6;
    int var7;
    int var8;
    char sp0[12];

    var6 = 0;
    if(num == 0) {
        *var5 = '0';
        var5[1] = 0;
        return 0;
    }
    if(num < 0) {
        var6 = 1;
        num = -num;
    }
    var8 = 0;
    while(num > 0) {
        sp0[var8++] = num % 10 + '0';
        num = num / 10;
    }
    var7 = 0;
    if(var6 == 1) {
        var5[var7++] = '-';
    }
    --var8;
    while(var8 >= 0) {
        var5[var7++] = sp0[var8--];
    }
    var5[var7] = '\0';
    return 0;
}

void int_to_string_hex(unsigned var0, char *var1, char var2)
{
    int var3;
    int var4;
    char *var6;
    char sp0[8];

    if(var2 == 'X') {
        var6 = "0123456789ABCDEF";
    } else { 
        var6 = "0123456789abcdef";
    }
    for(var3 = 0; var3 < 8; ++var3) {
        sp0[var3] = var6[var0 & 0xf];
        var0 >>= 4;
    }
    var1[0] = '0';
    var1[1] = var2;
    var4 = 2;
    for(var3 = 7; var3 >= 0; --var3) {
        var1[var4] = sp0[var3];
        ++var4;
    }
    var1[var4] = 0;
}


static int cpy_unix2dos(char *var2, char *var1)
{
    int var0 = 0;

    while(*var2 != '\0') {
        if(*var2 == '\n') {
            *var1++ = '\r';
            ++var0;
        }
        *var1++ = *var2++;
        ++var0;
    }
    return var0;
}

int wlibc_vsprintf(char *buf, const char *fmt, va_list args)
{
    char *bufpos;
    int hexspec;
    int numhi;
    int numlo;
    char numbuf[48];

    bufpos = buf;

    while(*fmt != 0) {
        if(*fmt == '%') {
            ++fmt;
            hexspec = 'X';
            switch( *fmt ) {
            case 'd':
                int_to_string_dec(va_arg(args, int), numbuf);
                bufpos += cpy_unix2dos(numbuf, bufpos);
                ++fmt;
                break;
            case 'x':
                hexspec = 'x';
            case 'p':
            case 'X':
                int_to_string_hex(va_arg(args, int), numbuf, hexspec);
                bufpos += cpy_unix2dos(numbuf, bufpos);
                ++fmt;
                break;
            case 'u':
                Uint_to_string_dec(va_arg(args, unsigned), numbuf);
                bufpos += cpy_unix2dos(numbuf, bufpos);
                ++fmt;
                break;
            case 'c':
                *bufpos++ = va_arg(args, int);
                ++fmt;
                break;
            case 's':
                bufpos += cpy_unix2dos(va_arg(args, char*), bufpos);
                ++fmt;
                break;
            case 'l':
                if(fmt[1] == 'l' && (fmt[2] == 'x' || fmt[2] == 'X')) {
                    numlo = va_arg(args, unsigned);
                    numhi = va_arg(args, unsigned);
                    if(fmt[2] == 'x') {
                        hexspec = 'x';
                    } else { 
                        hexspec = 'X';
                    }
                    int_to_string_hex(numhi, numbuf, hexspec);
                    bufpos += cpy_unix2dos(numbuf, bufpos);
                    int_to_string_hex(numlo, numbuf, hexspec);
                    bufpos += cpy_unix2dos(numbuf + 2, bufpos);
                    fmt += 3;
                    break;
                }
                int_to_string_dec(va_arg(args, int), numbuf);
                bufpos += cpy_unix2dos(numbuf, bufpos);
                ++fmt;
                break;
            default:
                *bufpos++ = '%';
                *bufpos++ = *fmt++;
            }
        } else { 
            if(*fmt == '\n') {
                *bufpos++ = '\r';
            }
            *bufpos++ = *fmt++;

        }
    }
    *bufpos = '\0';
    return bufpos - buf;
}

/* printf, prepended with timer value
 */
void wlibc_uprintf(const char *fmt, ...)
{
    char buf[256];
    va_list args;
    int len;

    va_start(args, fmt);
    len = wlibc_vsprintf(buf, fmt, args);
    va_end(args);
    svc_4a(buf);
    fbprint_cons(buf, len);
}

/* "nt" aka "no timer" printf */
void wlibc_ntprintf(const char *fmt, ...)
{
    char buf[256];
    va_list args;
    int len;

    va_start(args, fmt);
    len = wlibc_vsprintf(buf, fmt, args);
    va_end(args);
    svc_49(buf);
    fbprint_cons(buf, len);
}

int wlibc_sprintf(char *buf, const char *fmt, ...)
{
    va_list args;
    int len;

    va_start(args, fmt);
    len = wlibc_vsprintf(buf, fmt, args);
    va_end(args);
    return len;
}

void wlibc_int_enable(void)
{
    asm("mrs r0, CPSR; bic r0, r0, #0xc0; msr CPSR_c, r0" ::: "r0");
}

void wlibc_int_disable(void)
{
    asm("mrs r0, CPSR; orr r0, r0, #0xc0; msr CPSR_c, r0" ::: "r0");
}

void sdelay(unsigned loops)
{
    loops <<= 1;
	__asm__ volatile ("1:   subs %0, %1, #1; bne 1b"
            : "=r" (loops) : "0"(loops));
}

