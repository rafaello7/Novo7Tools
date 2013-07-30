#include <stdio.h>
#include <fcntl.h>

enum {
	DISP_CMD_VERSION = 0x00,
    DISP_CMD_LCD_SET_BRIGHTNESS = 0x142,
    DISP_CMD_LCD_GET_BRIGHTNESS = 0x143
};

static int disp_announceversion(int disp_fd)
{
    unsigned long 	arg[4];

    arg[0] = 0x10000;
    return ioctl(disp_fd, DISP_CMD_VERSION, (unsigned long)arg);
}

static int disp_getbrightness(int disp_fd)
{
    unsigned long 	arg[4];

    arg[0] = 0;
    return ioctl(disp_fd, DISP_CMD_LCD_GET_BRIGHTNESS, (unsigned long)arg);
}
      
static int  disp_setbrightness(int disp_fd, int brightness)
{
    unsigned long 	arg[4];

    arg[0] = 0;
    arg[1] = brightness;
    return ioctl(disp_fd, DISP_CMD_LCD_SET_BRIGHTNESS, (unsigned long)arg);
}

int main(int argc, char *argv[])
{
    int disp_fd, cur_brightness, new_brightness;
    char *arg;

    if( argc > 1 && !strcmp(argv[1], "-h") ) {
        printf("usage: sunxi-disp-brightness [[+|-]<number>]\n");
        return 0;
    }
    if( (disp_fd = open("/dev/disp", O_RDWR)) < 0 ) {
        printf("/dev/disp open fail\n");
        return 1;
    }
    disp_announceversion(disp_fd);
    cur_brightness = disp_getbrightness(disp_fd);
    if(argc == 1 ) {
        printf("brightness: %d\n", cur_brightness);
    }else{
        arg = argv[1];
        if( *arg == '+' || *arg == '-' )
            ++arg;
        new_brightness = strtoul(arg, NULL, 10);
        if( *argv[1] == '+' )
            new_brightness += cur_brightness;
        else if( *argv[1] == '-' )
            new_brightness = cur_brightness - new_brightness;
        if( new_brightness < 0 )
            new_brightness = 0;
        if( new_brightness > 255 )
            new_brightness = 255;
        if( new_brightness != cur_brightness )
            disp_setbrightness(disp_fd, new_brightness);
    }

    return 0;
}

