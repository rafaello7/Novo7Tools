#ifndef BOOTOS_H
#define BOOTOS_H

#include "script.h"

struct OSImage {  /* 44 bytes */
    char name[32];
    int maxSize;
    int fileLen;
    void *baseAddr;
};

struct OSImageScript {    /* 832 bytes */
    int param0;
    int param4;
    int param8;
    int param12;
    int param16;
    int param20;
    int param24;
    int param28;
    int startMode;
    struct OSImage images[16];
    int param740;
    int param744;
    int param748;
    int param752;
    int param756;
    int param760;
    int param764;
    int param768;
    int maxSize;
    int param776;
    char *baseAddr;
    int segmentSectionCount;
    char logoFileName[32];
    int logoShow;
    void *logoAddress;
    int logoOff;
};

int BootOS_detect_os_type(void**, void**, struct BootIni*, int*);
void BootOS(void*, void*);

#endif /* BOOTOS_H */
