#ifndef PICTURE_H
#define PICTURE_H

#include "ui.h"

struct ShowBatteryChangeHandle {    /* 296 bytes */
    struct LayerPara *layerPara;
    struct BatteryChargeImg images[11];
    struct BatteryChargeImg lp;
    int imgsize;
};

struct MultiPictureParam {  /* 132 bytes */
    int filecount;
    char fnames[4][32];
};


struct LayerPara *ShowPictureEx(const char*, void*);
int ShutPictureEx(struct LayerPara*);
struct ShowBatteryChangeHandle *ShowBatteryCharge_init(void);
int ShowBatteryCharge_rate(struct ShowBatteryChangeHandle*, int rate);
int ShowBatteryCharge_degrade(struct ShowBatteryChangeHandle*);
int ShowBatteryCharge_exit(struct ShowBatteryChangeHandle*);

#endif /* PICTURE_H */
