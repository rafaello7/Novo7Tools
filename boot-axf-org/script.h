#ifndef SCRIPT_H
#define SCRIPT_H

#include "picture.h"

struct BootIni { /* 1368 bytes */
    int param0;
    char param4[8][32];
    char startOSName[32];
    int param292;
    int param296;
    int timeout;
    int displayDevice;
    int displayMode;
    struct MultiPictureParam param312[8];
};

int script_patch(const char *, void*, int);


#endif /* SCRIPT_H */
