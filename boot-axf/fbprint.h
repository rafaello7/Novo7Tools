#ifndef FBPRINT_H
#define FBPRINT_H

struct ColorPair {
    unsigned fg, bg;
};

void fbclear(void);
void fbprint_line(int row, int col, const struct ColorPair*,
        int size, const char *line);
void fbprint_cons(const char *buf, int size);

#endif /* FBPRINT_H */
