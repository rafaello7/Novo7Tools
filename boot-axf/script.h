#ifndef SCRIPT_H
#define SCRIPT_H

#include "picture.h"

struct BootIni { /* 1368 bytes */
    int osCount;
    char osNames[8][32];
    char startOSName[32];
    int startOSNum;
    int timeout;
    int displayDevice;
    int displayMode;
    struct MultiPictureParam osPictures[8];
};

int script_patch(const char *, void*, int);


#endif /* SCRIPT_H */
