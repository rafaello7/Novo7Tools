#ifndef CMDLINE_H
#define CMDLINE_H

struct CmdLineOptions {
    guint x, y, xalt, yalt, width, height;
    gboolean isXNeg, isYNeg, isXAltNeg, isYAltNeg, isWidthNeg;
    gboolean isXPercent, isYPercent, isXAltPercent, isYAltPercent;
    gboolean isWidthPercent, isHeightPercent;

    gboolean isDecorated, hasResizeGrip, isOnTaskBar, isOnTop;
};

gboolean ParseCmdLine(int argc, char *argv[], struct CmdLineOptions*);

#endif /* CMDLINE_H */
