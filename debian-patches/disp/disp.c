#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/fb.h>
#include "drv_display_sun4i.h"


/* returns 0/1
 */
static int disp_hdmistatus(int disp_fd)
{
    unsigned long args[4];
    args[0] = 0;
    
    return ioctl(disp_fd, DISP_CMD_HDMI_GET_HPD_STATUS, args);
}

static const char *modenames[] = {
    "480I",
    "576I",
    "480P",
    "576P",
    "720P_50HZ",
    "720P_60HZ",
    "1080I_50HZ",
    "1080I_60HZ",
    "1080P_24HZ",
    "1080P_50HZ",
    "1080P_60HZ"
};

static int display_getmodebyname(const char *name)
{
    int i = 0;
    while( modenames[i] && strcmp(name, modenames[i]) )
        ++i;
    return modenames[i] ? i : -1;
}

static int disp_hdmibestmode(int disp_fd, int sel)
{
    unsigned long args[4];
    static int modes[] = {
        DISP_TV_MOD_1080P_60HZ, DISP_TV_MOD_1080P_50HZ,
        DISP_TV_MOD_1080I_60HZ, DISP_TV_MOD_1080I_50HZ,
        DISP_TV_MOD_720P_60HZ
    };
    int i;
    
    for(i = 0; i < sizeof(modes) / sizeof(modes[0]); ++i) {
        args[0] = sel;
        args[1] = modes[i];
        if( ioctl(disp_fd, DISP_CMD_HDMI_SUPPORT_MODE, args) > 0 )
            return modes[i];
    }
    return DISP_TV_MOD_720P_50HZ;
}      

static int disp_getoutputtype(int disp_fd, int sel)
{
    unsigned long args[4];
    int ret;

    args[0] = sel;
    return ioctl(disp_fd, DISP_CMD_GET_OUTPUT_TYPE, args);
}

static const char *disp_getoutputtypename(int type)
{
    switch( type ) {
    case DISP_OUTPUT_TYPE_NONE:     return "none";
    case DISP_OUTPUT_TYPE_LCD:      return "LCD";
    case DISP_OUTPUT_TYPE_TV:       return "TV";
    case DISP_OUTPUT_TYPE_HDMI:     return "HDMI";
    case DISP_OUTPUT_TYPE_VGA:      return "VGA";
    }
    return "unknown";
}

static const char *disp_getmodename(int mode)
{
    switch( mode ) {
    case DISP_LAYER_WORK_MODE_NORMAL:       return "normal";
    case DISP_LAYER_WORK_MODE_PALETTE:      return "palette";
    case DISP_LAYER_WORK_MODE_INTER_BUF:    return "inter_buf";
    case DISP_LAYER_WORK_MODE_GAMMA:        return "gamma";
    case DISP_LAYER_WORK_MODE_SCALER:       return "scaler";
    }
    return "unknown";
}

static const char *disp_getfbmodename(int fb_mode)
{
    switch( fb_mode ) {
    case FB_MODE_SCREEN0: return "screen 0";
    case FB_MODE_SCREEN1: return "screen 1";
    case FB_MODE_DUAL_SAME_SCREEN_TB: return "pan";
    case FB_MODE_DUAL_DIFF_SCREEN_SAME_CONTENTS: return "dub";
    }
    return "unknown";
}

static const char *disp_gettvmodename(int tvmode)
{
    return tvmode >= 0 && tvmode < sizeof(modenames)/sizeof(modenames[0]) ?
        modenames[tvmode] : "unknown";
}

static void disp_str(const char *name, const char *aux, const char *val)
{
    char buf[80];

    strcpy(buf, name);
    if( aux )
        sprintf(buf + strlen(buf), " (%s)", aux);
    strcat(buf, ":");
    printf("  %-25s %s\n", buf, val);
}

static void disp_num(const char *name, const char *aux, int val)
{
    char buf[20];

    sprintf(buf, "%d", val);
    disp_str(name, aux, buf);
}

static void disp_showinfo(__disp_fb_create_para_t *para,
        int outputtype0, int outputtype1, int tvmode)
{
    const char *outputs[2];

    outputs[0] = disp_getoutputtypename(outputtype0);
    outputs[1] = disp_getoutputtypename(outputtype1);
    disp_str("mode",        NULL, disp_getmodename(para->mode));
    disp_str("fb_mode",     NULL, disp_getfbmodename(para->fb_mode));
    disp_num("buffer_num",  NULL, para->buffer_num);
    disp_num("width",       NULL, para->width);
    disp_num("height",      NULL, para->height);
    if( para->mode == DISP_LAYER_WORK_MODE_SCALER ) {
        disp_num("output width", outputs[para->primary_screen_id],
                para->output_width);
        disp_num("output height", outputs[para->primary_screen_id],
                para->output_height);
    }
    if( para->fb_mode == FB_MODE_DUAL_DIFF_SCREEN_SAME_CONTENTS ) {
        disp_num("aux output width", outputs[1 - para->primary_screen_id],
                para->aux_output_width);
        disp_num("aux output height", outputs[1 - para->primary_screen_id],
                para->aux_output_height);
    }
    disp_str("output 0",    NULL, outputs[0]);
    disp_str("output 1",    NULL, outputs[1]);
    disp_num("primary screen", NULL, para->primary_screen_id);
    if( tvmode >= 0 )
        disp_str("hdmi mode", NULL, disp_gettvmodename(tvmode));
}

static void disp_getpara(int disp_fd, int fb_id,
        __disp_fb_create_para_t *para)
{
    unsigned long arg[4];

    arg[0] = fb_id;
    arg[1] = (unsigned long)para;
    ioctl(disp_fd, DISP_CMD_FB_GET_PARA, (unsigned long)arg);
}

static void display_set_src_window(int disp_fd, int fb_id,
        int x, int y, int width, int height)
{
    unsigned long arg[4];
    __disp_rect_t rect;

    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;
    arg[0] = fb_id;
    arg[1] = (unsigned long)&rect;
    ioctl(disp_fd, DISP_CMD_LAYER_SET_SRC_WINDOW, (unsigned long)arg);
}

static void display_set_scn_window(int disp_fd, int mode,
        int x, int y, int width, int height)
{
    unsigned long arg[4];
    __disp_rect_t rect;
    unsigned long 				fb_layer_hdl;

    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;
    int fb_fd = open(mode == 1 ? "/dev/fb1" : "/dev/fb0", O_RDWR);


    ioctl(fb_fd, mode == 1 ? FBIOGET_LAYER_HDL_1 : FBIOGET_LAYER_HDL_0,
            &fb_layer_hdl);
    arg[0] 				= mode != 0;    /* screen number */
    arg[1] 				= fb_layer_hdl;
    arg[2] = (unsigned long)&rect;
    ioctl(disp_fd, DISP_CMD_LAYER_SET_SCN_WINDOW, (unsigned long)arg);
    close(fb_fd);
}

static int disp_getwidth(int format)
{
    switch (format) 
    {
	    case DISP_TV_MOD_480I:       	return 720;           
	    case DISP_TV_MOD_480P:     		return 720;    	    
	    case DISP_TV_MOD_576I:     		return 720;     	    
	    case DISP_TV_MOD_576P:  		return 720;  		    
	    case DISP_TV_MOD_720P_50HZ:  	return 1280;      
	    case DISP_TV_MOD_720P_60HZ:     return 1280;      
	    case DISP_TV_MOD_1080I_50HZ:    return 1920;     
	    case DISP_TV_MOD_1080I_60HZ:    return 1920;     
	    case DISP_TV_MOD_1080P_50HZ:    return 1920;     
	    case DISP_TV_MOD_1080P_60HZ:    return 1920;     
	    case DISP_TV_MOD_1080P_24HZ:  	return 1920;   
		default:						break;  
	 
    }
    return -1;
}

static int disp_getheight(int format)
{
    switch (format) {
	    case DISP_TV_MOD_480I:       	return 480;           
	    case DISP_TV_MOD_480P:     		return 480;    	    
	    case DISP_TV_MOD_576I:     		return 576;     	    
	    case DISP_TV_MOD_576P:  		return 576;  		    
	    case DISP_TV_MOD_720P_50HZ:  	return 720;      
	    case DISP_TV_MOD_720P_60HZ:    	return 720;      
	    case DISP_TV_MOD_1080I_50HZ:   	return 1080;     
	    case DISP_TV_MOD_1080I_60HZ:   	return 1080;     
	    case DISP_TV_MOD_1080P_50HZ:   	return 1080;     
	    case DISP_TV_MOD_1080P_60HZ:   	return 1080;     
	    case DISP_TV_MOD_1080P_24HZ:  	return 1080;   
		default:						break;  
	 
    }  
    return -1;
}

static void disp_setpan(int fb_id, int width, int height)
{
    struct fb_var_screeninfo var;
    int fb_fd = open(fb_id == 0 ? "/dev/fb0" : "/dev/fb1", O_RDWR);

    if( fb_fd  < 0 ) {
        fprintf(stderr, "open fb failed\n");
        return;
	}
    if( ioctl(fb_fd, FBIOGET_VSCREENINFO, (unsigned long)&var) < 0 ) {
        fprintf(stderr, "ioctl(FBIOGET_VSCREENINFO) failed\n");
        return;
    }
    var.xres = width;
    var.yres = height;
    var.xres_virtual = var.xres;
    var.yres_virtual = var.yres;
    var.xoffset = 0;
    var.yoffset = 0;
    if( ioctl(fb_fd, FBIOPAN_DISPLAY, (unsigned long)&var) < 0 ) {
        fprintf(stderr, "ioctl(FBIOPAN_DISPLAY) failed\n");
        return;
    }
    close(fb_fd);
}

static int disp_on(int disp_fd, int sel, int outputtype)
{
	unsigned long args[4];
	
	args[0]  	= sel;
	args[1]		= 0;
	args[2]		= 0;
	args[3]		= 0;
    switch( outputtype ) {
    case DISP_OUTPUT_TYPE_LCD:
        return ioctl(disp_fd, DISP_CMD_LCD_ON, (unsigned long)args);
    case DISP_OUTPUT_TYPE_HDMI:
        return ioctl(disp_fd, DISP_CMD_HDMI_ON, (unsigned long)args);
    }
    return 0;
}
      
static void disp_off(int disp_fd, int dispNo)
{
	unsigned long 	args[4];
	
    args[0] = dispNo;
	args[1]		= 0;
	args[2]		= 0;
	args[3]		= 0;
    switch( ioctl(disp_fd, DISP_CMD_GET_OUTPUT_TYPE, args) ) {
    case DISP_OUTPUT_TYPE_LCD:
		ioctl(disp_fd, DISP_CMD_LCD_OFF, (unsigned long)args);
        break;
    case DISP_OUTPUT_TYPE_HDMI:
		ioctl(disp_fd, DISP_CMD_HDMI_OFF, (unsigned long)args);
        break;
    }
}

static int disp_hdmi_getmode(int disp_fd)
{
    unsigned long 	arg[4];
    int sel;

    for(sel = 0; sel < 2; ++sel) {
        if( disp_getoutputtype(disp_fd, sel) == DISP_OUTPUT_TYPE_HDMI ) {
            arg[0] = sel;
            return ioctl(disp_fd, DISP_CMD_HDMI_GET_MODE, (unsigned long)arg);
        }
    }
    return -1;
}
      
static int  disp_hdmi_setmode(int disp_fd, int sel, int mode)
{
    unsigned long 	arg[4];

    arg[0] = sel;
    arg[1] = (__disp_tv_mode_t)mode;
    return ioctl(disp_fd, DISP_CMD_HDMI_SET_MODE, (unsigned long)arg);
}

int main(int argc, char *argv[])
{
    int fb_id = 0;

    if(argc < 2) {
        int mode;
        printf("usage:\n");
        printf("    disp [-s] hdmi|<mode>           - HDMI output only\n");
        printf("    disp [-s] lcd|dub hdmi|<mode>   - two-display mode, LCD primary\n");
        printf("    disp [-s] hdmi|<mode> lcd|dub   - two-display mode, HDMI primary\n");
        printf("    disp lcd                        - LCD output only\n");
        printf("    disp off                        - no output\n");
        printf("    disp pan <width> <height>       - set mode\n");
        printf("    disp info                       - show info\n");
        printf("\n  <mode>:");
        for(mode = 0; mode < sizeof(modenames) / sizeof(modenames[0]); ++mode) {
            if( mode % 6 == 0 )
                printf("\n    ");
            printf(" %-11s", modenames[mode]);
        }
        printf("\n\n");
        return 0;
    }
    if( ! strcmp(argv[1], "info") ) {
        __disp_fb_create_para_t para;
        unsigned long arg[4];
        int disp_fd, fb_fd;
        struct fb_var_screeninfo var;

        if( (disp_fd = open("/dev/disp", O_RDWR)) < 0 ) {
            printf("/dev/disp open fail\n");
            return 1;
        }
        arg[0] = 0; /* fb id */
        arg[1] = (unsigned long)&para;
        ioctl(disp_fd, DISP_CMD_FB_GET_PARA, (unsigned long)arg);
        disp_showinfo(&para,
                disp_getoutputtype(disp_fd, 0),
                disp_getoutputtype(disp_fd, 1),
                disp_hdmi_getmode(disp_fd));
        disp_str("hdmi status", NULL,
                disp_hdmistatus(disp_fd) ?  "connected" : "disconnected");
        if( (fb_fd = open(fb_id == 0 ? "/dev/fb0" : "/dev/fb1", O_RDWR)) < 0 ) {
            fprintf(stderr, "fb open fail\n");
            return 1;
        }
        if( ioctl(fb_fd, FBIOGET_VSCREENINFO, (unsigned long)&var) < 0 ) {
            fprintf(stderr, "ioctl(FBIOGET_VSCREENINFO) failed\n");
            return 1;
        }
        close(fb_fd);
        disp_num("fb xres", NULL, var.xres);
        disp_num("fb yres", NULL, var.yres);
        disp_num("fb xres_virtual", NULL, var.xres_virtual);
        disp_num("fb yres_virtual", NULL, var.yres_virtual);
        disp_num("fb xoffset", NULL, var.xoffset);
        disp_num("fb yoffset", NULL, var.yoffset);
    }else if( ! strcmp(argv[1], "pan") ) {
        int width, height;
        if( argc < 4 ) {
            fprintf(stderr, "too few parameters\n");
            return 1;
        }
        width = atoi(argv[2]);
        height = atoi(argv[3]);
        disp_setpan(fb_id, width, height);
    }else if( ! strcmp(argv[1], "off") ) {
        int disp_fd;
        unsigned long 	arg[4];

        if( (disp_fd = open("/dev/disp", O_RDWR)) < 0 ) {
            printf("/dev/disp open fail\n");
            return;
        }
        disp_off(disp_fd, 0);
        disp_off(disp_fd, 1);
        close(disp_fd);
    }else{
        __disp_fb_create_para_t para, para_old;
        int disp_fd, sel, tvmode = -1, argn = 1, fb_fd;
        int disp_hdmi = 0, disp_lcd = 0;
        unsigned long 	arg[4];
        int outputtype[2];
        struct fb_var_screeninfo var;
        char *disp_membuf = NULL;
        int disp_memsize = 0;

        if( (disp_fd = open("/dev/disp", O_RDWR)) < 0 ) {
            printf("/dev/disp open fail\n");
            return 1;
        }
        para.buffer_num = 1;
        para.fb_mode = FB_MODE_SCREEN0;
        para.mode = DISP_LAYER_WORK_MODE_NORMAL;
        para.primary_screen_id = 0;
        if( !strcmp(argv[argn], "-s") ) {
            para.mode = DISP_LAYER_WORK_MODE_SCALER;
            ++argn;
        }
        for(sel = 0; sel < 2 && argn < argc; ++sel, ++argn) {
            if( !strcmp(argv[argn], "dub") || !strcmp(argv[argn], "lcd") ) {
                if( disp_lcd ) {
                    fprintf(stderr, "error: lcd display specified twice\n");
                    return 1;
                }
                if( !strcmp(argv[argn], "dub") )
                    para.fb_mode = FB_MODE_DUAL_DIFF_SCREEN_SAME_CONTENTS;
                disp_lcd = 1;
                para.primary_screen_id = sel;
            }else{
                if( disp_hdmi ) {
                    fprintf(stderr, "error: hdmi display specified twice\n");
                    return 1;
                }
                if( !strcmp(argv[argn], "hdmi") ) {
                    tvmode = disp_hdmibestmode(disp_fd, sel);
                }else{
                    int i;
                    for(i = 0; i < sizeof(modenames)/sizeof(modenames[0]); ++i)
                    {
                        if(!strcmp(argv[argn], modenames[i]) ) {
                            tvmode = i;
                            break;
                        }
                    }
                    if( tvmode < 0 ) {
                        fprintf(stderr, "unrecognized mode: %s\n",
                                argv[argn]);
                        return 1;
                    }
                }
                disp_hdmi = 1;
            }
            if( sel == 1 && para.fb_mode == FB_MODE_SCREEN0 ) {
                para.buffer_num = 2;
                para.fb_mode = FB_MODE_DUAL_SAME_SCREEN_TB;
            }
        }
        if( disp_lcd ) {
            outputtype[0] = DISP_OUTPUT_TYPE_LCD;
            outputtype[1] = disp_hdmi ? DISP_OUTPUT_TYPE_HDMI :
                DISP_OUTPUT_TYPE_NONE;
        }else{
            outputtype[0] = disp_hdmi ? DISP_OUTPUT_TYPE_HDMI :
                DISP_OUTPUT_TYPE_NONE;
            outputtype[1] = DISP_OUTPUT_TYPE_NONE;
        }
        if( disp_lcd && para.primary_screen_id == 0 ) {
            para.width = 1024;
            para.height = 600;
            para.aux_output_width = disp_getwidth(tvmode);
            para.aux_output_height = disp_getheight(tvmode);
        }else{
            para.width = disp_getwidth(tvmode);
            para.height = disp_getheight(tvmode);
            para.aux_output_width = 1024;
            para.aux_output_height = 600;
        }
        if( para.mode == DISP_LAYER_WORK_MODE_SCALER ) {
            para.output_width = para.width;
            para.output_height = para.height;
            para.width = 1024;
            para.height = 600;
        }else{
            /* unused */
            para.output_width = para.output_height = 0;
        }

        if(para.fb_mode == FB_MODE_DUAL_SAME_SCREEN_TB) {
            para.height *= 2;
            para.output_height *= 2;
        }
        printf("request parameters:\n");
        disp_showinfo(&para, outputtype[0], outputtype[1], tvmode);

        disp_off(disp_fd, 0);
        disp_off(disp_fd, 1);
        arg[0] = fb_id;
        arg[1] = (unsigned long)&para_old;
        ioctl(disp_fd, DISP_CMD_FB_GET_PARA, (unsigned long)arg);
        if( para.buffer_num != para_old.buffer_num ||
                para.fb_mode != para_old.fb_mode ||
                para.mode != para_old.mode ||
                para.primary_screen_id != para_old.primary_screen_id ||
                para.width != para_old.width ||
                para.height != para_old.height ||
                para.mode == DISP_LAYER_WORK_MODE_SCALER &&
                    (para.output_width != para_old.output_width ||
                    para.output_height != para_old.output_height) ||
                para.fb_mode == FB_MODE_DUAL_DIFF_SCREEN_SAME_CONTENTS &&
                    (para.aux_output_width != para_old.aux_output_width ||
                    para.aux_output_height != para_old.aux_output_height) )
        {
            printf("releasing & requesting framebuffer\n");
            fb_fd = open(fb_id == 0 ? "/dev/fb0" : "/dev/fb1", O_RDWR);
            if( fb_fd >= 0 ) {
                if(ioctl(fb_fd, FBIOGET_VSCREENINFO, (unsigned long)&var) < 0) {
                    fprintf(stderr, "ioctl(FBIOGET_VSCREENINFO) failed\n");
                    return 1;
                }
                if( var.xres == para.width ) {
                    disp_memsize = var.bits_per_pixel / 8 * var.xres * 
                        (var.yres < para.height ? var.yres : para.height);
                    disp_membuf = malloc(disp_memsize);
                    if(read(fb_fd, disp_membuf, disp_memsize) != disp_memsize) {
                        free(disp_membuf);
                        disp_membuf = NULL;
                    }
                }
                arg[0] = fb_id;
                ioctl(disp_fd, DISP_CMD_FB_RELEASE, (unsigned long)arg);
            }
            arg[0] = fb_id;
            arg[1] = (unsigned long)&para;
            if( ioctl(disp_fd, DISP_CMD_FB_REQUEST, (unsigned long)arg) < 0 ) {
                fprintf(stderr, "fb_request fail\n");
                return 1;
            }
        }else{
            printf("keep current framebuffer\n");
        }
        if( tvmode >= 0 )
            disp_hdmi_setmode(disp_fd, outputtype[0] != DISP_OUTPUT_TYPE_HDMI,
                    tvmode);
        if( disp_membuf != NULL ) {
            lseek(fb_fd, 0, SEEK_SET);
            write(fb_fd, disp_membuf, disp_memsize);
        }
        if( fb_fd >= 0 )
            close(fb_fd);
        disp_on(disp_fd, 0, outputtype[0]);
        disp_on(disp_fd, 1, outputtype[1]);
    }
    return 0;
}
