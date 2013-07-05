#ifndef DISPLAY_INTERFACE_H
#define DISPLAY_INTERFACE_H

struct LayerPara {
    char lpparam0;
    char lpparam1;
    char lpparam2;
    char lpparam3;
    char lpparam4;
    char lpparam5;
    short lpparam6;
    char lpparam8;
    char lpparam9;
    short lpparam10;
    int lpparam12;
    int lpparam16;
    int width2;
    int height2;
    int horizShift;
    int vertShift;
    int width;
    int height;
    char *imgdata;
    int lpparam48;
    int lpparam52;
    int dispWidth;
    int lpparam60;
    char lpparam64;
    char lpparam65;
    char lpparam66;
    char lpparam67;
    int lpparam68;
    int lpparam72;
    int lpparam76;
    int lpparam80;
    int lpparam84;
};

int De_IsLCDOpen(void);
int De_IsLCDClose(void);
int De_RequestLayer(int);
int De_ReleaseLayer(int);
int De_OpenLayer(int);
int De_CloseLayer(int);
int De_GetSceenWidth(void);
int De_GetSceenHeight(void);
int De_GetLayerPara(int, int);
int De_SetLayerPara(int, struct LayerPara*);
int De_SetBlkColor(int var5);
int De_OpenDevice(int var4);
int De_CloseDevice(int var4);

#endif /* DISPLAY_INTERFACE_H */
