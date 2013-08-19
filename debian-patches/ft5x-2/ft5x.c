/*
 * (c) 2012 rafaello7 <fatwildcat@gmail.com>
 *
 * derived from the xf86-input-tslib driver
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is  hereby granted without fee, provided that
 * the  above copyright   notice appear  in   all  copies and  that both  that
 * copyright  notice   and   this  permission   notice  appear  in  supporting
 * documentation, and that   the  name of  Frederic   Lepied not  be  used  in
 * advertising or publicity pertaining to distribution of the software without
 * specific,  written      prior  permission.     Frederic  Lepied   makes  no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * FREDERIC  LEPIED DISCLAIMS ALL   WARRANTIES WITH REGARD  TO  THIS SOFTWARE,
 * INCLUDING ALL IMPLIED   WARRANTIES OF MERCHANTABILITY  AND   FITNESS, IN NO
 * EVENT  SHALL FREDERIC  LEPIED BE   LIABLE   FOR ANY  SPECIAL, INDIRECT   OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA  OR PROFITS, WHETHER  IN  AN ACTION OF  CONTRACT,  NEGLIGENCE OR OTHER
 * TORTIOUS  ACTION, ARISING    OUT OF OR   IN  CONNECTION  WITH THE USE    OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

/* ft5x input driver */

#include "xorg-server.h"

#ifndef XFree86LOADER
#include <unistd.h>
#include <errno.h>
#endif

#include <misc.h>
#include <xf86.h>
#if !defined(DGUX)
#include <xisb.h>
#endif
#include <xf86_OSproc.h>
#include <xf86Xinput.h>
#include <exevents.h>		/* Needed for InitValuator/Proximity stuff */
#include <X11/keysym.h>
#include <mipointer.h>
#include <randrstr.h>

#include <sys/time.h>
#include <time.h>
#include <linux/input.h>
#include <fcntl.h>


#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
#include <xserver-properties.h>
#endif
#ifdef XFree86LOADER
#include <xf86Module.h>
#endif

#define MAXBUTTONS 3
#define TIME23RDBUTTON 0.5
#define MOVEMENT23RDBUTTON 4

#define DEFAULT_HEIGHT		240
#define DEFAULT_WIDTH		320

#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) < 12
#define COLLECT_INPUT_OPTIONS(pInfo, options) xf86CollectInputOptions((pInfo), (options), NULL)
#else
#define COLLECT_INPUT_OPTIONS(pInfo, options) xf86CollectInputOptions((pInfo), (options))
#endif

#if 0
#define FT5XDBG ErrorF
#else
#define FT5XDBG(...)
#endif

enum {
    FT5X_ROTATE_NONE=0,
    FT5X_ROTATE_CW=270,
    FT5X_ROTATE_UD=180,
    FT5X_ROTATE_CCW=90
};

enum btn_down {
    BTN_DOWN_NONE,
    BTN_DOWN_LEFT,
    BTN_DOWN_RIGHT,
    BTN_IS_SCROLL
};

enum { SCROLL_DIST = 50, PRESS_DIST = 20 };

struct ft5x_point {
    int x, y;
};

struct ft5x_priv {
	XISBuffer *buffer;
    int input_fd;
	int screen_num;
	int rotate;
	int height;
	int width;
    enum btn_down btn_down;
    int isRockchip;

	int lastcnt, motion, isScrollMotion;
    struct ft5x_point pressxy, scrollxy, motionxy;
    int count;      /* number of fingers touching screen, in range 0..2 */
    struct ft5x_point xy1;     /* position of touch with first finger */
    struct ft5x_point xy2;     /* position of touch with second finger */
	struct timeval ev_tv; /* touch event time */

    /* data collected for touch report */
    int current_id, press1, press2;
    struct ft5x_point currentxy;
};

static void
PointerControlProc(DeviceIntPtr dev, PtrCtrl * ctrl)
{
}

static Bool
ConvertProc( InputInfoPtr local,
			 int first,
			 int num,
			 int v0,
			 int v1,
			 int v2,
			 int v3,
			 int v4,
			 int v5,
			 int *x,
			 int *y )
{
	*x = v0;
	*y = v1;
	return TRUE;
}

/* Read input event from touchscreen file on /dev
 */
static int novo7_read(struct ft5x_priv *priv)
{
	struct input_event ev;
	int rd;

    while( (rd = read(priv->input_fd, &ev, sizeof(struct input_event))) ==
         sizeof(struct input_event))
    {
        switch (ev.type) {
        case EV_ABS:
            switch (ev.code) {
            case ABS_MT_TOUCH_MAJOR:
                break;
            case ABS_MT_TRACKING_ID:
                priv->current_id = ev.value;
                break;
            case ABS_MT_WIDTH_MAJOR:
                break;
            case ABS_MT_POSITION_X:
                priv->currentxy.x = ev.value;
                break;
            case ABS_MT_POSITION_Y:
                priv->currentxy.y = ev.value;
                break;
            }
            break;
        case EV_SYN:
            switch( ev.code ) {
            case SYN_MT_REPORT:
                /* we are interested with touches of first two fingers only */
                if( priv->current_id == 0 ) {
                    priv->xy1 = priv->currentxy;
                    ++priv->count;
                }else if( priv->current_id == 1 ) {
                    priv->xy2 = priv->currentxy;
                    ++priv->count;
                }
                break;
            case SYN_REPORT:
                priv->ev_tv = ev.time;
                priv->current_id = 0;
                FT5XDBG("novo7_read: SYN_REPORT: xyp1=(%d,%d,%d) "
                        "xyp2=(%d,%d,%d) count=%d\n",
                        priv->xy1.x, priv->xy1.y, priv->press1,
                        priv->xy2.x, priv->xy2.y, priv->press2, priv->count);
                return 1;
            }
        }
    }
    return 0;
}

/* Read input event from touchscreen file on /dev
 */
static int rk_read(struct ft5x_priv *priv)
{
	struct input_event ev;
	int rd;

    while( (rd = read(priv->input_fd, &ev, sizeof(struct input_event))) ==
         sizeof(struct input_event))
    {
        switch (ev.type) {
        case EV_ABS:
            switch (ev.code) {
            case ABS_MT_POSITION_X:
                FT5XDBG("rk_read: ABS_MT_POSITION_X: value=%d\n", ev.value);
                if( priv->current_id == 0 ) {
                    priv->xy1.x = ev.value;
                }else{
                    priv->xy2.x = ev.value;
                }
                break;
            case ABS_MT_POSITION_Y:
                FT5XDBG("rk_read: ABS_MT_POSITION_Y: value=%d\n", ev.value);
                if( priv->current_id == 0 ) {
                    priv->xy1.y = ev.value;
                }else{
                    priv->xy2.y = ev.value;
                }
                break;
            case ABS_MT_PRESSURE:
                FT5XDBG("rk_read: ABS_MT_PRESSURE: value=%d\n", ev.value);
                if( priv->current_id == 0 ) {
                    priv->press1 = ev.value;
                }else{
                    priv->press2 = ev.value;
                }
                break;
            case ABS_MT_SLOT:
                FT5XDBG("rk_read: ABS_MT_SLOT: value=%d\n", ev.value);
                priv->current_id = ev.value;
                break;
            case ABS_MT_TOUCH_MAJOR:
                FT5XDBG("rk_read: ABS_MT_TOUCH_MAJOR: value=%d\n", ev.value);
                break;
            case ABS_MT_TRACKING_ID:
                FT5XDBG("rk_read: ABS_MT_TRACKING_ID: value=%d\n", ev.value);
                /* there are two different implementations of ft5x driver
                 * on RockChip kernels. In stock kernel, the
                 * ABS_MT_TRACKING_ID appears with ev.value >= 0 on
                 * a finger press and with -1 on finger release. The
                 * ABS_MT_PRESSURE message does not appear at all.
                 * In "custom" kernels the ABS_MT_TRACKING_ID appears only
                 * at very begining, 10 times with ev.value equal to 0..9 */
                if( priv->current_id == 0 ) {
                    priv->press1 = ev.value >= 0 ? 200 : 0;
                }else if( priv->current_id == 1 ) {
                    priv->press2 = ev.value >= 0 ? 200 : 0;
                }else if( priv->current_id >= 9 ) {
                    /* custom kernel: the message doesn't mean pressure */
                    priv->press1 = priv->press2 = 0;
                }
                break;
            default:
                FT5XDBG("rk_read: EV_ABS: code=0x%x value=%d\n",
                        ev.code, ev.value);
                break;
            }
            break;
        case EV_SYN:
            switch( ev.code ) {
            case SYN_REPORT:
                priv->ev_tv = ev.time;
                priv->count = priv->press1 ? priv->press2 ? 2 : 1 : 0;
                FT5XDBG("rk_read: SYN_REPORT: xyp1=(%d,%d,%d) xyp2=(%d,%d,%d) "
                        "count=%d\n",
                        priv->xy1.x, priv->xy1.y, priv->press1,
                        priv->xy2.x, priv->xy2.y, priv->press2, priv->count);
                return 1;
            default:
                FT5XDBG("rk_read: EV_SYN: value=%d\n", ev.value);
                break;
            }
        }
    }
    return 0;
}

static void ApplyRotateOnInput(struct ft5x_priv *priv)
{
    int tmp_x;
    ScrnInfoPtr pScrn = xf86Screens[priv->screen_num];
    Rotation rotation = rrGetScrPriv (pScrn->pScreen) ?
        RRGetRotation(pScrn->pScreen) : RR_Rotate_0;

    switch(priv->rotate) {
    case FT5X_ROTATE_CW:
        tmp_x = priv->xy1.x;
        priv->xy1.x = priv->xy1.y;
        priv->xy1.y = priv->width - tmp_x;
        tmp_x = priv->xy2.x;
        priv->xy2.x = priv->xy2.y;
        priv->xy2.y = priv->width - tmp_x;
        break;
    case FT5X_ROTATE_UD:
        priv->xy1.x = priv->width - priv->xy1.x;
        priv->xy1.y = priv->height - priv->xy1.y;
        priv->xy2.x = priv->width - priv->xy2.x;
        priv->xy2.y = priv->height - priv->xy2.y;
        break;
    case FT5X_ROTATE_CCW:
        tmp_x = priv->xy1.x;
        priv->xy1.x = priv->height - priv->xy1.y;
        priv->xy1.y = tmp_x;
        tmp_x = priv->xy2.x;
        priv->xy2.x = priv->height - priv->xy2.y;
        priv->xy2.y = tmp_x;
        break;
    default:
        break;
    }


    switch(rotation) {
    case RR_Rotate_90:
        tmp_x = priv->xy1.x;
        priv->xy1.x = (priv->height - priv->xy1.y - 1) *
            priv->width / priv->height;
        priv->xy1.y = tmp_x * priv->height / priv->width;
        tmp_x = priv->xy2.x;
        priv->xy2.x = (priv->height - priv->xy2.y - 1) *
            priv->width / priv->height;
        priv->xy2.y = tmp_x * priv->height / priv->width;
        break;
    case RR_Rotate_180:
        priv->xy1.x = priv->width - priv->xy1.x - 1;
        priv->xy1.y = priv->height - priv->xy1.y - 1;
        priv->xy2.x = priv->width - priv->xy2.x - 1;
        priv->xy2.y = priv->height - priv->xy2.y - 1;
        break;
    case RR_Rotate_270:
        tmp_x = priv->xy1.x;
        priv->xy1.x = priv->xy1.y * priv->width / priv->height;
        priv->xy1.y = (priv->width - tmp_x - 1) *
            priv->height / priv->width;
        tmp_x = priv->xy2.x;
        priv->xy2.x = priv->xy2.y * priv->width / priv->height;
        priv->xy2.y = (priv->width - tmp_x - 1) *
            priv->height / priv->width;
        break;
    }
}

static int xydist(const struct ft5x_point *p1, const struct ft5x_point *p2)
{
    return max(abs(p1->x - p2->x), abs(p1->y - p2->y));
}

static void StartMotion(struct ft5x_priv *priv)
{
    if( ! priv->motion ) {
        FT5XDBG(">> ft5x: StartMotion()\n");
        priv->motionxy = priv->xy1;
        priv->isScrollMotion = TRUE;
        priv->motion = TRUE;
    }
}

static void StopMotion(struct ft5x_priv *priv)
{
    if( priv->motion ) {
        FT5XDBG(">> ft5x: StopMotion()\n");
        priv->motion = FALSE;
    }
    if( priv->count == 0 )
        priv->isScrollMotion = FALSE;
}

static void EmulateClickAbs(InputInfoPtr local, struct ft5x_priv *priv)
{
    FT5XDBG(">> ft5x: EmulateClickAbs()\n");
    /* XXX: is it possible to move back after click ? */
    xf86PostMotionEvent(local->dev, Absolute, 0, 2, priv->pressxy.x,
            priv->pressxy.y);
    /* left button down */
    xf86PostButtonEvent(local->dev, Relative, 1, TRUE, 0, 0);
    /* button up */
    xf86PostButtonEvent(local->dev, Relative, 1, FALSE, 0, 0);
}

static void StartScroll(InputInfoPtr local, struct ft5x_priv *priv)
{
    if( priv->btn_down == BTN_DOWN_NONE ) {
        if( ! priv->isScrollMotion ) {
            FT5XDBG(">> ft5x: StartScroll: move abs(%d, %d)\n",
                    priv->pressxy.x, priv->pressxy.y);
            xf86PostMotionEvent(local->dev, Absolute, 0, 2, priv->pressxy.x,
                    priv->pressxy.y);
        }
        priv->btn_down = BTN_IS_SCROLL;
    }
}

static int EmulateScroll(InputInfoPtr local, struct ft5x_priv *priv)
{
    int isScrolled = FALSE;

    while( priv->scrollxy.y + SCROLL_DIST < priv->xy2.y ) {
        StartScroll(local, priv);
        FT5XDBG(">> ft5x: EmulateScroll(DOWN)\n");
        xf86PostButtonEvent(local->dev, Relative, 4, TRUE, 0, 0);
        xf86PostButtonEvent(local->dev, Relative, 4, FALSE, 0, 0);
        priv->scrollxy.y += SCROLL_DIST;
        isScrolled = TRUE;
    }
    while( priv->scrollxy.y - SCROLL_DIST > priv->xy2.y ) {
        StartScroll(local, priv);
        FT5XDBG(">> ft5x: EmulateScroll(UP)\n");
        xf86PostButtonEvent(local->dev, Relative, 5, TRUE, 0, 0);
        xf86PostButtonEvent(local->dev, Relative, 5, FALSE, 0, 0);
        priv->scrollxy.y -= SCROLL_DIST;
        isScrolled = TRUE;
    }
    while( priv->scrollxy.x + SCROLL_DIST < priv->xy2.x ) {
        StartScroll(local, priv);
        FT5XDBG(">> ft5x: EmulateScroll(RIGHT)\n");
        xf86PostButtonEvent(local->dev, Relative, 6, TRUE, 0, 0);
        xf86PostButtonEvent(local->dev, Relative, 6, FALSE, 0, 0);
        priv->scrollxy.x += SCROLL_DIST;
        isScrolled = TRUE;
    }
    while( priv->scrollxy.x - SCROLL_DIST > priv->xy2.x ) {
        StartScroll(local, priv);
        FT5XDBG(">> ft5x: EmulateScroll(LEFT)\n");
        xf86PostButtonEvent(local->dev, Relative, 7, TRUE, 0, 0);
        xf86PostButtonEvent(local->dev, Relative, 7, FALSE, 0, 0);
        priv->scrollxy.x -= SCROLL_DIST;
        isScrolled = TRUE;
    }
    return isScrolled;
}

static void LeftButtonDown(InputInfoPtr local, struct ft5x_priv *priv)
{
    FT5XDBG(">> ft5x: LeftButtonDown()\n");
    xf86PostButtonEvent(local->dev, Relative, 1, TRUE, 0, 0);
    priv->btn_down = BTN_DOWN_LEFT;
}

static void RightButtonDown(InputInfoPtr local, struct ft5x_priv *priv)
{
    FT5XDBG(">> ft5x: RightButtonDown()\n");
    xf86PostButtonEvent(local->dev, Relative, 3, TRUE, 0, 0);
    priv->btn_down = BTN_DOWN_RIGHT;
}

static void ButtonUp(InputInfoPtr local, struct ft5x_priv *priv)
{
    switch( priv->btn_down ) {
    case BTN_DOWN_NONE:
        break;
    case BTN_DOWN_LEFT:
        FT5XDBG(">> ft5x: ButtonUp(left)\n");
        xf86PostButtonEvent(local->dev, Relative, 1, FALSE, 0, 0);
        break;
    case BTN_DOWN_RIGHT:
        FT5XDBG(">> ft5x: ButtonUp(right)\n");
        xf86PostButtonEvent(local->dev, Relative, 3, FALSE, 0, 0);
        break;
    default: /* BTN_IS_SCROLL */
        FT5XDBG(">> ft5x: ButtonUp(scroll)\n");
        break;
    }
    priv->btn_down = BTN_DOWN_NONE;
}

static void ReadInput (InputInfoPtr local)
{
    struct ft5x_priv *priv = (struct ft5x_priv *) (local->private);

    while( priv->isRockchip ? rk_read(priv) : novo7_read(priv) == 1 )
    {
        if( priv->count > 0 ) {
            ApplyRotateOnInput(priv);

            /*
               xf86XInputSetScreen(local, priv->screen_num,
               priv->x,
               priv->y);*/

            if( priv->lastcnt == 0 ) {
                priv->pressxy = priv->xy1;
            }
            if( priv->count > 1 && priv->lastcnt <= 1 ) {
                priv->scrollxy = priv->xy2;
            }
        }

        switch( priv->btn_down ) {
        case BTN_DOWN_NONE:
            switch( priv->lastcnt ) {
            case 0:
                switch( priv->count ) {
                case 0:
                    break;
                case 1:
                    break;
                default: /* 2 */
                    StartMotion(priv);
                    break;
                }
                break;
            case 1:
                switch( priv->count ) {
                case 0:
                    if( ! priv->motion && ! priv->isScrollMotion ) {
                        EmulateClickAbs(local, priv);
                    }
                    StopMotion(priv);
                    break;
                case 1:
                    if( xydist(&priv->pressxy, &priv->xy1) > PRESS_DIST ) {
                        StartMotion(priv);
                    }
                    break;
                default: /* 2 */
                    StopMotion(priv);
                    /* adjusting pressxy up to calculate distance
                     * from current xy1 for motion start */
                    priv->pressxy = priv->xy1;
                    break;
                }
                break;
            default: /* 2 */
                switch( priv->count ) {
                case 0:
                    StopMotion(priv);
                    break;
                case 1:
                    StartMotion(priv);
                    if( priv->scrollxy.x <= priv->pressxy.x ) {
                        LeftButtonDown(local, priv);
                    }else{
                        RightButtonDown(local, priv);
                    }
                    ButtonUp(local, priv);
                    break;
                default: /* 2 */
                    if( EmulateScroll(local, priv) ) {
                        StopMotion(priv);
                    }else if( xydist(&priv->pressxy, &priv->xy1) > PRESS_DIST )
                    {
                        StartMotion(priv);
                        if( priv->scrollxy.x <= priv->pressxy.x ) {
                            LeftButtonDown(local, priv);
                        }else{
                            RightButtonDown(local, priv);
                        }
                    }
                    break;
                }
                break;
            }
            break;
        case BTN_DOWN_LEFT:
        case BTN_DOWN_RIGHT:
            switch( priv->lastcnt ) {
            case 0:
                ErrorF("ft5x error: BAD BUTTON STATE=%d, lastcnt=%d\n",
                        priv->btn_down, priv->lastcnt);
                break;
            case 1:
                switch( priv->count ) {
                case 0:
                    /* button up */
                    ButtonUp(local, priv);
                    StopMotion(priv);
                    break;
                case 1:
                    break;
                default: /* 2 */
                    break;
                }
                break;
            default: /* 2 */
                switch( priv->count ) {
                case 0:
                    ButtonUp(local, priv);
                    StopMotion(priv);
                    break;
                case 1:
                    ButtonUp(local, priv);
                    break;
                default: /* 2 */
                    break;
                }
                break;
            }
            break;
        default:    /* BTN_IS_SCROLL */
            switch( priv->lastcnt ) {
            case 0:
                ErrorF("ft5x error: BAD BUTTON STATE=%d, lastcnt=%d\n",
                        priv->btn_down, priv->lastcnt);
                break;
            case 1:
                switch( priv->count ) {
                case 0:
                    ButtonUp(local, priv);
                    StopMotion(priv);
                    break;
                case 1:
                    break;
                default: /* 2 */
                    break;
                }
                break;
            default:
                switch( priv->count ) {
                case 0:
                    ButtonUp(local, priv);
                    break;
                case 1:
                    if( priv->isScrollMotion )
                        ButtonUp(local, priv);
                    break;
                default: /* 2 */
                    EmulateScroll(local, priv);
                    break;
                }
                break;
            }
            break;
        }

        if( priv->motion && (priv->xy1.x != priv->motionxy.x ||
                    priv->xy1.y != priv->motionxy.y))
        {
            xf86PostMotionEvent(local->dev, FALSE, 0, 2,
                    priv->xy1.x - priv->motionxy.x,
                    priv->xy1.y - priv->motionxy.y);
            priv->motionxy = priv->xy1;
        }
        priv->lastcnt = priv->count;
        if( ! priv->isRockchip )
            priv->count = 0;
    }
}

#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
static void xf86Ft5xInitButtonLabels(Atom *labels, int nlabels)
{
	memset(labels, 0, nlabels * sizeof(Atom));
	switch(nlabels)
	{
		default:
		case 7:
			labels[6] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_HWHEEL_RIGHT);
		case 6:
			labels[5] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_HWHEEL_LEFT);
		case 5:
			labels[4] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_WHEEL_DOWN);
		case 4:
			labels[3] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_WHEEL_UP);
		case 3:
			labels[2] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_RIGHT);
		case 2:
			labels[1] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_MIDDLE);
		case 1:
			labels[0] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_LEFT);
			break;
	}
}
#endif

/*
 * xf86Ft5xControlProc --
 *
 * called to change the state of a device.
 */
static int
xf86Ft5xControlProc(DeviceIntPtr device, int what)
{
	InputInfoPtr pInfo;
	unsigned char map[MAXBUTTONS + 1];
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
	Atom labels[MAXBUTTONS];
#endif
	int i, axiswidth, axisheight;
	struct ft5x_priv *priv;

	pInfo = device->public.devicePrivate;
	priv = pInfo->private;

	switch (what) {
	case DEVICE_INIT:
		device->public.on = FALSE;

		for (i = 0; i < MAXBUTTONS; i++) {
			map[i + 1] = i + 1;
		}
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
		xf86Ft5xInitButtonLabels(labels, MAXBUTTONS);
#endif

		if (InitButtonClassDeviceStruct(device, MAXBUTTONS,
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
						labels,
#endif
						map) == FALSE) {
			ErrorF("unable to allocate Button class device\n");
			return !Success;
		}

		if (InitValuatorClassDeviceStruct(device,
						  2,
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
						labels,
#endif
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) < 3
						  xf86GetMotionEvents,
#endif
						  0, Absolute) == FALSE) {
			ErrorF("unable to allocate Valuator class device\n");
			return !Success;
		}

		switch(priv->rotate) {
		case FT5X_ROTATE_CW:
		case FT5X_ROTATE_CCW:
			axiswidth = priv->height;
			axisheight = priv->width;
			break;
		default:
			axiswidth = priv->width;
			axisheight = priv->height;
			break;
		}

		InitValuatorAxisStruct(device, 0,
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
					       XIGetKnownProperty(AXIS_LABEL_PROP_ABS_X),
#endif
					       0,		/* min val */
					       axiswidth - 1,	/* max val */
					       axiswidth,	/* resolution */
					       0,		/* min_res */
					       axiswidth	/* max_res */
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 12
					       ,Absolute
#endif
					       );

		InitValuatorAxisStruct(device, 1,
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
					       XIGetKnownProperty(AXIS_LABEL_PROP_ABS_Y),
#endif
					       0,		/* min val */
					       axisheight - 1,	/* max val */
					       axisheight,	/* resolution */
					       0,		/* min_res */
					       axisheight	/* max_res */
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 12
					       ,Absolute
#endif
					       );

		if (InitProximityClassDeviceStruct (device) == FALSE) {
			ErrorF ("Unable to allocate EVTouch touchscreen ProximityClassDeviceStruct\n");
			return !Success;
		}

		/* allocate the motion history buffer if needed */
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) == 0
		xf86MotionHistoryAllocate(pInfo);
#endif

		if (!InitPtrFeedbackClassDeviceStruct(device, PointerControlProc))
			return !Success;
		break;

	case DEVICE_ON:
		AddEnabledDevice(pInfo->fd);

		device->public.on = TRUE;
		break;

	case DEVICE_OFF:
	case DEVICE_CLOSE:
		device->public.on = FALSE;
		break;
	}
	return Success;
}

/*
 * xf86Ft5xUninit --
 *
 * called when the driver is unloaded.
 */
static void
xf86Ft5xUninit(InputDriverPtr drv, InputInfoPtr pInfo, int flags)
{
	struct ft5x_priv *priv = (struct ft5x_priv *)(pInfo->private);
	ErrorF("%s\n", __FUNCTION__);
	xf86Ft5xControlProc(pInfo->dev, DEVICE_OFF);
    close(priv->input_fd);
	free(pInfo->private);
	pInfo->private = NULL;
	xf86DeleteInput(pInfo, 0);
}

/*
 * xf86Ft5xInit --
 *
 * called when the module subsection is found in XF86Config
 */
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 12
static int 
xf86Ft5xInit(InputDriverPtr drv, InputInfoPtr pInfo, int flags)
#else
static InputInfoPtr
xf86Ft5xInit(InputDriverPtr drv, IDevPtr dev, int flags)
#endif
{
	struct ft5x_priv *priv;
	char *s;
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) < 12
	InputInfoPtr pInfo;
#endif

	priv = calloc (1, sizeof (struct ft5x_priv));
        if (!priv)
                return BadValue;

#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) < 12
	if (!(pInfo = xf86AllocateInput(drv, 0))) {
		free(priv);
		return BadValue;
	}

	/* Initialise the InputInfoRec. */
	pInfo->name = dev->identifier;
	pInfo->flags =
	    XI86_KEYBOARD_CAPABLE | XI86_POINTER_CAPABLE |
	    XI86_SEND_DRAG_EVENTS;
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) == 0
	pInfo->motion_history_proc = xf86GetMotionEvents;
	pInfo->history_size = 0;
#endif
	pInfo->conf_idev = dev;
	pInfo->close_proc = NULL;
	pInfo->conversion_proc = ConvertProc;
	pInfo->reverse_conversion_proc = NULL;
	pInfo->private_flags = 0;
	pInfo->always_core_feedback = 0;
#endif

	pInfo->type_name = XI_TOUCHSCREEN;
	pInfo->control_proc = NULL;
	pInfo->read_input = ReadInput;
	pInfo->device_control = xf86Ft5xControlProc;
	pInfo->switch_mode = NULL;
	pInfo->private = priv;
	pInfo->dev = NULL;

	/* Collect the options, and process the common options. */
	COLLECT_INPUT_OPTIONS(pInfo, NULL);
	xf86ProcessCommonOptions(pInfo, pInfo->options);

	priv->screen_num = xf86SetIntOption(pInfo->options, "ScreenNumber", 0 );

	priv->width = xf86SetIntOption(pInfo->options, "Width", 0);
	if (priv->width <= 0)	priv->width = screenInfo.screens[0]->width;

	priv->height = xf86SetIntOption(pInfo->options, "Height", 0);
	if (priv->height <= 0)	priv->height = screenInfo.screens[0]->height;

	s = xf86SetStrOption(pInfo->options, "Rotate", 0);
	if (s > 0) {
		if (strcmp(s, "CW") == 0) {
			priv->rotate = FT5X_ROTATE_CW;
		} else if (strcmp(s, "UD") == 0) {
			priv->rotate = FT5X_ROTATE_UD;
		} else if (strcmp(s, "CCW") == 0) {
			priv->rotate = FT5X_ROTATE_CCW;
		} else {
			priv->rotate = FT5X_ROTATE_NONE;
		}
	} else {
		priv->rotate = FT5X_ROTATE_NONE;
	}

#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) < 12
 	s = xf86CheckStrOption(dev->commonOptions, "path", NULL);
#else
	s = xf86CheckStrOption(pInfo->options, "path", NULL);
#endif
  	if (!s)
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) < 12
		s = xf86CheckStrOption(dev->commonOptions, "Device", NULL);
#else
		s = xf86CheckStrOption(pInfo->options, "Device", NULL);
#endif
 
    priv->input_fd = open(s, O_RDONLY | O_NONBLOCK);

	if (priv->input_fd < 0) {
		ErrorF("ts_open failed (device=%s)\n",s);
        free(s);
		xf86DeleteInput(pInfo, 0);
		return BadValue;
	}
	free(s);

	int version;
	if ( ioctl(priv->input_fd, EVIOCGVERSION, &version) < 0) {
        ErrorF("ft5x: ioctl(EVIOCGVERSION) fail!");
    }else if( version != EV_VERSION && version != 0x010000) {
		ErrorF("ft5x: invalid protocol version !\n");
		xf86DeleteInput(pInfo, 0);
		return BadValue;
	}

	pInfo->fd = priv->input_fd;

/*	priv->state = BUTTON_NOT_PRESSED;
	if (xf86SetIntOption(pInfo->options, "EmulateRightButton", 0) == 0) {
		priv->state = BUTTON_EMULATION_OFF;
	}*/
    priv->motionxy.x = priv->motionxy.y = -1;

    /* Assume "ft5x_ts" driver name is for Allwinner,
     * ft5x0x_ts for Rockchip (rk3066) */
    if( ! strcmp(pInfo->name, "ft5x0x_ts") ) {
        priv->isRockchip = TRUE;
    }

#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) < 12
	/* Mark the device configured */
	pInfo->flags |= XI86_CONFIGURED;
#endif

	/* Return the configured device */
	return Success;
}

_X_EXPORT InputDriverRec FT5X = {
	1,			/* driver version */
	"ft5x",		/* driver name */
	NULL,			/* identify */
	xf86Ft5xInit,		/* pre-init */
	xf86Ft5xUninit,	/* un-init */
	NULL,			/* module */
	0			/* ref count */
};

/*
 ***************************************************************************
 *
 * Dynamic loading functions
 *
 ***************************************************************************
 */
#ifdef XFree86LOADER

/*
 * xf86Ft5xUnplug --
 *
 * called when the module subsection is found in XF86Config
 */
static void xf86Ft5xUnplug(pointer p)
{
}

/*
 * xf86Ft5xPlug --
 *
 * called when the module subsection is found in XF86Config
 */
static pointer xf86Ft5xPlug(pointer module, pointer options, int *errmaj, int *errmin)
{
	static Bool Initialised = FALSE;

	xf86AddInputDriver(&FT5X, module, 0);

	return module;
}

static XF86ModuleVersionInfo xf86Ft5xVersionRec = {
	"ft5x",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XORG_VERSION_CURRENT,
	0, 0, 1,
	ABI_CLASS_XINPUT,
	ABI_XINPUT_VERSION,
	MOD_CLASS_XINPUT,
	{0, 0, 0, 0}		/* signature, to be patched into the file by */
	/* a tool */
};

_X_EXPORT XF86ModuleData ft5xModuleData = {
	&xf86Ft5xVersionRec,
	xf86Ft5xPlug,
	xf86Ft5xUnplug
};

#endif				/* XFree86LOADER */

/*
 * Local variables:
 * change-log-default-name: "~/xinput.log"
 * c-file-style: "bsd"
 * End:
 */
/* end of ft5x.c */
