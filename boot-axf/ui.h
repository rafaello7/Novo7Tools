#ifndef UI_H
#define UI_H

#include "display_interface.h"

struct BatteryChargeImg {   /* 24 bytes */
    void *imgdata;
    int imgsiz;
    int depth;
    int width;
    int height;
    int lineSize;
};

struct MultiLayerPara {
    char *dispbuf;
    unsigned width;
    int height;
};

struct LayerPara *ui_AllocLayerPara(struct BatteryChargeImg *img);
struct LayerPara *ui_multi_AllocLayerPara(struct MultiLayerPara*, void*);
int ui_FreeLayerPara(struct LayerPara *layerPara);


#endif /* UI_H */
