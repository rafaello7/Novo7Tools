/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef IO_H
#define IO_H

/*
 * Generic virtual read/write.  Note that we don't support half-word
 * read/writes.  We define __arch_*[bl] here, and leave __arch_*w
 * to the architecture specific code.
 */
#define __arch_getb(a)			(*(volatile unsigned char *)(a))
#define __arch_getw(a)			(*(volatile unsigned short *)(a))
#define __arch_getl(a)			(*(volatile unsigned int *)(a))

#define __arch_putb(v,a)		(*(volatile unsigned char *)(a) = (v))
#define __arch_putw(v,a)		(*(volatile unsigned short *)(a) = (v))
#define __arch_putl(v,a)		(*(volatile unsigned int *)(a) = (v))

#define dmb()		__asm__ __volatile__ ("" : : : "memory")
#define writeb(v,c)	({ unsigned char  __v = v; dmb(); __arch_putb(__v,c); __v; })
#define writew(v,c)	({ unsigned short __v = v; dmb(); __arch_putw(__v,c); __v; })
#define writel(v,c)	({ unsigned __v = v; dmb(); __arch_putl(__v,c); __v; })

#define readb(c)	({ unsigned char  __v = __arch_getb(c); dmb(); __v; })
#define readw(c)	({ unsigned short __v = __arch_getw(c); dmb(); __v; })
#define readl(c)	({ unsigned __v = __arch_getl(c); dmb(); __v; })

#endif	/* IO_H */
