/*
 * This program is Free Software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 */


void raise (int signum)
{
    // jump to FEL
    asm("ldr r0, =0xffff0020; bx r0");
}

void __aeabi_unwind_cpp_pr0(void)
{
}

void flush_cache(unsigned saddr, unsigned bytes)
{
    /* processor cache is turned off */
}
