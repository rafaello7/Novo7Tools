#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "cmdline.h"

static void Usage(void)
{
    static const char usageStr[] = "\n"
    "usage: virtual-keyboard [-h] [-x [-]<num>[%%]] [-y [-]<num>[%%]]\n"
    "                        [-xalt [-]<num>[%%]] [-yalt [-]<num>[%%]]\n"
    "                        [-width [-]<num>[%%]] [-height <num>[%%]]\n"
    "                        [-decorated] [-resizable] [-taskbar]\n"
    "                        [-notop]\n"
    "\n"
    "    -h             - print this help\n"
    "\n"
    "    -x <num>       - x offset\n"
    "    -xalt <num>    - x offset of alternate position\n"
    "                     '-' prefix -- of right edge from end of screen\n"
    "                     '%%' suffix -- percentage of screen width\n"
    "\n"
    "    -y <num>       - y offset\n"
    "    -yalt <num>    - y offset of alternate position\n"
    "                     '-' prefix -- of bottom edge from end of screen\n"
    "                     '%%' suffix -- percentage of screen height\n"
    "\n"
    "    -width <num>   - keyboard width\n"
    "                     '-' prefix -- subtract from screen width\n"
    "                     '%%' suffix -- percentage of screen width\n"
    "\n"
    "    -height <num>  - keyboard height\n"
    "                     '%%' suffix -- percentage of keyboard width\n"
    "\n"
    "    -decorated     - with title bar and border\n"
    "    -resizeGrip    - with resize grip\n"
    "    -taskbar       - appears on taskbar\n"
    "    -notop         - not on top\n"
    "\n";
    g_print(usageStr);
}

static gboolean ParseNum(const char *option, const char *arg,
        gboolean *isNeg, guint *val, gboolean *isPercent)
{
    char *argEnd;

    if( arg == NULL ) {
        g_print("option %s requires an argument\n", option);
        return FALSE;
    }
    if( isNeg && (*isNeg = *arg == '-') )
        ++arg;
    *val = strtoul(arg, &argEnd, 10);
    if( isPercent != NULL )
        *isPercent = *argEnd == '%';
    return TRUE;
}

gboolean ParseCmdLine(int argc, char *argv[], struct CmdLineOptions *opts)
{
    int argNo = 1;
    gboolean isXAltSet = FALSE, isYAltSet = FALSE, isWidthSet = FALSE;

    /* place on the left bottom */
    opts->x = 0;
    opts->isXNeg = FALSE;
    opts->isXPercent = FALSE;
    opts->y = 0;
    opts->isYNeg = TRUE;
    opts->isYPercent = FALSE;

    /* height as 25% of keyboard width */
    opts->height = 25;
    opts->isHeightPercent = TRUE;

    opts->isDecorated = FALSE;
    opts->hasResizeGrip = FALSE;
    opts->isOnTaskBar = FALSE;
    opts->isOnTop = TRUE;

    while( argNo < argc ) {
        if( !strcmp(argv[argNo], "-h") ) {
            Usage();
            return FALSE;
        }else if( !strcmp(argv[argNo], "-x") ) {
            if( ! ParseNum(argv[argNo], argv[argNo+1],
                        &opts->isXNeg, &opts->x, &opts->isXPercent) )
                return FALSE;
            argNo += 2;
        }else if( !strcmp(argv[argNo], "-y") ) {
            if( ! ParseNum(argv[argNo], argv[argNo+1],
                        &opts->isYNeg, &opts->y, &opts->isYPercent) )
                return FALSE;
            argNo += 2;
        }else if( !strcmp(argv[argNo], "-xalt") ) {
            if( ! ParseNum(argv[argNo], argv[argNo+1],
                        &opts->isXAltNeg, &opts->xalt, &opts->isXAltPercent) )
                return FALSE;
            isXAltSet = TRUE;
            argNo += 2;
        }else if( !strcmp(argv[argNo], "-yalt") ) {
            if( ! ParseNum(argv[argNo], argv[argNo+1],
                        &opts->isYAltNeg, &opts->yalt, &opts->isYAltPercent) )
                return FALSE;
            isYAltSet = TRUE;
            argNo += 2;
        }else if( !strcmp(argv[argNo], "-width") ) {
            if( ! ParseNum(argv[argNo], argv[argNo+1], &opts->isWidthNeg,
                        &opts->width, &opts->isWidthPercent) )
                return FALSE;
            isWidthSet = TRUE;
            argNo += 2;
        }else if( !strcmp(argv[argNo], "-height") ) {
            if( ! ParseNum(argv[argNo], argv[argNo+1], NULL,
                        &opts->height, &opts->isHeightPercent) )
                return FALSE;
            argNo += 2;
        }else if( !strcmp(argv[argNo], "-decorated") ) {
            opts->isDecorated = TRUE;
            ++argNo;
        }else if( !strcmp(argv[argNo], "-resizeGrip") ) {
            opts->hasResizeGrip = TRUE;
            ++argNo;
        }else if( !strcmp(argv[argNo], "-taskbar") ) {
            opts->isOnTaskBar = TRUE;
            ++argNo;
        }else if( !strcmp(argv[argNo], "-notop") ) {
            opts->isOnTop = FALSE;
            ++argNo;
        }else{
            g_print("unknown option -- %s\n", argv[argNo]);
            g_print("type -h option for usage\n");
            return FALSE;
        }
    }
    if( ! isXAltSet ) {
        opts->xalt = opts->x;
        opts->isXAltNeg = opts->isXNeg;
        opts->isXAltPercent = opts->isXPercent;
    }
    if( ! isYAltSet ) {
        opts->yalt = 0;
        opts->isYAltNeg = ! opts->isYNeg;
        opts->isYAltPercent = FALSE;
    }
    if( ! isWidthSet ) {
        opts->width = opts->x;
        opts->isWidthNeg = TRUE;
        opts->isWidthPercent = opts->isXPercent;
    }
    return TRUE;
}
