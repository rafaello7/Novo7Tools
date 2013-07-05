
unsigned long long __aeabi_uidivmod(unsigned num, unsigned den)
{
    unsigned quot = 0, d = sizeof(unsigned) * 8;

    while( d-- ) {
        if( num >> d >= den ) {
            quot += 1 << d;
            num -= den << d;
        }
    }
    /* remainder (num) into r1 register, quotient in r0 */
    return (unsigned long long)num << 32 | quot;
}

long long __aeabi_idivmod(int num, int den)
{
    long long quotrem = __aeabi_uidivmod(
            num >= 0 ? num : -num, den >= 0 ? den : -den);
    int quot = quotrem & 0xffffffff;
    int rem = quotrem >> 32;
    return (long long)(num >= 0 ? rem : -rem) << 32 |
        ((num >= 0) == (den >= 0) ? quot : -quot);
}

unsigned __aeabi_uidiv(unsigned num, unsigned den)
{
    return __aeabi_uidivmod(num, den);
}

int __aeabi_idiv(int num, int den)
{
    int res = __aeabi_uidiv(num >= 0 ? num : -num, den >= 0 ? den : -den);

    return (num >= 0) == (den >= 0) ? res : -res;
}
