#include "bootmain.h"

const struct egon2_magic {
    char magic[8];
    unsigned flags;
    unsigned f2;
    int (*main)(int, char*[]);
    unsigned res[8];
} egon2_magic __attribute__((section("EGON2_MAGIC"))) = {
    .magic = "eGon2app",
    .flags = 0x1000000,
    .main  = BootMain
};
