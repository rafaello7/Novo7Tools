#ifndef CMDLINE_H
#define CMDLINE_H

enum CmdLine_WinStateSet {
    CL_WSTATE_ICONIFY,
    CL_WSTATE_PRESENT,
    CL_WSTATE_TOGGLE    /* iconify or present, depend on current state */
};

struct CmdLineOptions {
    guint x, y, xalt, yalt, width, height;
    gboolean isXNeg, isYNeg, isXAltNeg, isYAltNeg, isWidthNeg;
    gboolean isXPercent, isYPercent, isXAltPercent, isYAltPercent;
    gboolean isWidthPercent, isHeightPercent;

    gboolean isDecorated, hasResizeGrip, isOnTaskBar, isOnTop;
    enum CmdLine_WinStateSet winStateToSet;
    const char *composeKeys;
};

gboolean ParseCmdLine(int argc, char *argv[], struct CmdLineOptions*);

#endif /* CMDLINE_H */
