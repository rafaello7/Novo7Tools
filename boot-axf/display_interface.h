#ifndef DISPLAY_INTERFACE_H
#define DISPLAY_INTERFACE_H

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


int De_IsInit(void);
int De_IsLCDOpen(void);
int De_IsLCDClose(void);
int De_RequestLayer(int);
int De_ReleaseLayer(int);
int De_GetSceenWidth(void);
int De_GetSceenHeight(void);
int De_SetBlkColor(int var5);
int De_OpenDevice(int var4);
int De_CloseDevice(int var4);
int De_DegradeBrightness(void);
void De_GetConsoleBuf(struct MultiLayerPara*);
void De_GetPictBuf(struct MultiLayerPara*);

int BoardInit_Display(int, int);
int BoardExit(int, int);

#endif /* DISPLAY_INTERFACE_H */
