#ifndef BOOTPARA_H
#define BOOTPARA_H

struct BootPara {
    unsigned bpparam0[8192];
    unsigned bpparam32768[168];
};

extern struct BootPara gBootPara;

#endif /* BOOTPARA_H */
