#include "script.h"
#include "picture.h"
#include "wlibc.h"
#include "syscalls.h"
#include "bootos.h"
#include "bootpara.h"
#include "string.h"


enum LineType {
    LTYPE_ERROR     = -1,
    LTYPE_COMMENT,
    LTYPE_SECTION,
    LTYPE_PARAM
};

static int GetScriptLine(const char *script, enum LineType *lineType,
        int scriptSize)
{
    int lineLen;
    char c;

    c = *script++;
    --scriptSize;
    switch( c ) {
    case ';':
    case '\r':
        *lineType = LTYPE_COMMENT;
        break;
    case '[':
        *lineType = LTYPE_SECTION;
        break;
    default:
        *lineType = LTYPE_PARAM;
    }
    lineLen = 1;
    c = *script++;
    while(c != '\n' && lineLen < scriptSize) {
        c = *script++;
        if(++lineLen >= 512) {
            *lineType = LTYPE_ERROR;
            return 0;
        }
    }
    return lineLen + 1;
}

static int GetSectionName(const char *line, char *nameBuf)
{
    char c;
    int nameLen = 0;

    ++line;
    c = *line++;
    while(c != ']') {
        nameBuf[nameLen++] = c;
        c = *line++;
        if(nameLen >= 32) {
            nameBuf[nameLen-1] = 0;
            break;
        }
    }
    return 0;
}

static int GetParam(const char *script, char *paramName, char *paramVal)
{
    int var7 = 0;

    while(*script != '=') {
        if(*script != ' ' && *script != '\t') {
            *paramName++ = *script;
            ++var7;
        }
        ++script;
        if(var7 >= 31) {
            *paramName = 0;
            break;
        }
    }
    *paramName = 0;
    while(*script != '=') {
        ++script;
    }
    ++script;
    var7 = 0;
    while(*script != '\r') {
        if(*script != ' ' && *script != '\t') {
            *paramVal++ = *script;
            var7 = var7 + 1;
        }
        ++script;
        if(var7 >= 127) {
            return -1;
        }
    }
    *paramVal = 0;
    return 0;
}

static int ParseParamVal(const char *buf, int *var1)
{
    int type = 0;   /* 0 - string, 1 - decimal, 2 - hex */
    unsigned val = 0;
    int len = 2;
    int sign = 1;

    if(*buf == '-') {
        sign = -1;
        if(buf[1] == '0') {
            if(buf[2] == 'x' || buf[2] == 'X') {
                buf += 3;
                type = 2;
            }
        } else if(buf[1] >= '0' && buf[1] <= '9') {
            ++buf;
            type = 1;
        }
    } else if(*buf == '0' && (buf[1] == 'x' || buf[1] == 'X')) {
        buf += 2;
        type = 2;
    } else if(*buf >= '0' && *buf <= '9') {
        type = 1;
    }
    if(type == 0) {
        len = 0;
        while(buf[len] != 0) {
            ++len;
            if(len > 127) {
                break;
            }
        }
        if(len & 3) {
            len = (len & ~3) + 4;
        }
        var1[0] = len >> 2;
        return 2;
    }
    if(type == 1) {
        while(*buf != 0) {
            if(*buf >= '0' && *buf <= '9') {;
                val = (*buf - '0') + 10 * val;
                ++buf;
            } else {
                if(*buf == '+' || *buf == '-')
                    break;
                return -1;
            }
        }
        var1[0] = val;
        var1[1] = sign;
        var1[2] = 1;
        if(*buf == '-') {
            var1[2] = -1;
        }
        return 1;
    }
    if(type == 2) {
        while(*buf != 0) {
            if(*buf >= '0' && *buf <= '9') {
                val = (*buf - '0') + (val << 4);
                ++buf;
            } else if(*buf >= 'A' && *buf <= 'F') {
                val = (*buf + 10 - 'A') + (val << 4);
                ++buf;
            } else if(*buf >= 'a' && *buf <= 'f') {
                val = (*buf + 10 - 'a') + (val << 4);
                ++buf;
            } else { 
                if(*buf == '+' || *buf == '-')
                    break;
                return -1;
            }
        }
        var1[0] = val;
        var1[1] = sign;
        var1[2] = 1;
        if(*buf == '-') {
            var1[2] = -1;
        }
        return 1;
    }
    return -1;
}

enum ImgSection {
    SECT_SEGMENT = 1,
    SECT_SCRIPT_INFO,
    SECT_LOGO_INFO,
    SECT_GLOBAL
};


int parser_script_os_img(const char *script, int var5,
        struct OSImageScript *imgScript)
{
    int var7 = -1;
    int adjustBaseAddr;
    enum ImgSection section;
    int segmentNo;
    int lineLen;
    int sp8[8];
    char paramVal[128];
    char paramName[128];
    enum LineType lineType;

    segmentNo = -1;
    while(var5 != 0) {
        lineLen = GetScriptLine(script, &lineType, var5);
        var5 = var5 - lineLen;
        switch( lineType ) {
        case LTYPE_COMMENT:
            break;
        case LTYPE_SECTION:
            memset(paramName, 0, sizeof(paramName));
            if(GetSectionName(script, paramName) != 0) {
                goto L42802C84;
            }
            if(strcmp(paramName, "segment") == 0) {
                section = SECT_SEGMENT;
                ++segmentNo;
            } else if(strcmp(paramName, "script_info") == 0) {
                section = SECT_SCRIPT_INFO;
            } else if(strcmp(paramName, "logo_info") == 0) {
                section = SECT_LOGO_INFO;
            } else if(strcmp(paramName, "global") == 0) {
                section = SECT_GLOBAL;
            }
            adjustBaseAddr = 0;
            break;
        case LTYPE_PARAM:
            if(GetParam(script, paramName, paramVal) != 0) {
                goto L42802C84;
            }
            var7 = ParseParamVal(paramVal, sp8);
            if(var7 == -1) {
                goto L42802C84;
            }
            switch( section ) {
            case SECT_SEGMENT:
                if(strcmp(paramName, "img_name") == 0) {
                    strncpy(imgScript->images[segmentNo].name, paramVal,
                            4 * sp8[0]);
                } else { 
                    if(strcmp(paramName, "img_size") == 0) {
                        imgScript->images[segmentNo].maxSize = sp8[0];
                        if(adjustBaseAddr != 0) {
                            imgScript->images[segmentNo].baseAddr =
                                (char*)imgScript->images[segmentNo].baseAddr -
                                sp8[0];
                            adjustBaseAddr = 0;
                        }
                    } else { 
                        if(strcmp(paramName, "img_base") == 0) {
                            if(sp8[1] < 0) {
                                sp8[0] = (gBootPara.bpparam0[7] +
                                        (gBootPara.bpparam0[17] << 20))
                                    - sp8[0];
                            }
                            if(sp8[2] < 0) {
                                if( imgScript->images[segmentNo].maxSize  == 0) {
                                    adjustBaseAddr = 1;
                                } else { 
                                    sp8[0] -= imgScript->images[segmentNo].maxSize;
                                }
                            }
                            imgScript->images[segmentNo].baseAddr = (void*)sp8[0];
                        }
                    }
                }
                break;
            case SECT_SCRIPT_INFO:
                if(strcmp(paramName, "script_base") == 0) {
                    if(sp8[1] < 0) {
                        sp8[0] = (gBootPara.bpparam0[7] +
                                (gBootPara.bpparam0[17] << 20)) - sp8[0];
                    }
                    if(sp8[2] < 0) {
                        if(imgScript->maxSize == 0) {
                            adjustBaseAddr = 1;
                        } else { 
                            sp8[0] -= imgScript->maxSize;
                        }
                    }
                    imgScript->baseAddr = (void*)sp8[0];
                } else { 
                    if(strcmp(paramName, "script_size") == 0) {
                        imgScript->maxSize = sp8[0];
                        if(adjustBaseAddr != 0) {
                            imgScript->baseAddr -= imgScript->maxSize;
                            adjustBaseAddr = 0;
                        }
                    }
                }
                break;
            case SECT_LOGO_INFO:
                if(strcmp(paramName, "logo_name") == 0) {
                    strncpy(imgScript->logoFileName, paramVal, sp8[0] << 2);
                } else if(strcmp(paramName, "logo_show") == 0) {
                    imgScript->logoShow = sp8[0];
                } else if(strcmp(paramName, "logo_address") == 0) {
                    if(sp8[1] < 0) {
                        sp8[0] = (gBootPara.bpparam0[7] +
                                (gBootPara.bpparam0[17] << 20)) - sp8[0];
                    }
                    imgScript->logoAddress = (void*)sp8[0];
                } else if(strcmp(paramName, "logo_off") == 0) {
                    imgScript->logoOff = sp8[0];
                }
                break;
            case SECT_GLOBAL:
                if(strcmp(paramName, "start_mode") == 0) {
                    imgScript->startMode = sp8[0];
                }
                break;
            }
            break;
        default:
            goto L42802C84;
        }
        script = script + lineLen;
    }
    imgScript->segmentSectionCount = segmentNo + 1;
L42802C84:
    var7 = var7 >> 31;
    return var7;
}

int parser_script_os_sel(const char *script, int scriptSize,
        struct BootIni *bootIni)
{
    int var7 = -1;
    int var8;
    int var9 = -1;
    int vasl = -1;
    int lineLen;
    char sp4[12];
    int sp24[8];
    char paramVal[128];
    char paramName[128];
    int lineType;

    while(scriptSize != 0) {
        lineLen = GetScriptLine(script, &lineType, scriptSize);
        scriptSize -= lineLen;
        switch( lineType ) {
        case LTYPE_COMMENT:
            break;
        case LTYPE_SECTION:
            memset(paramName, 0, sizeof(paramName));
            if(GetSectionName(script, paramName) != 0) {
                goto L42803010;
            }
            var9 = -1;
            if(strcmp(paramName, "system") == 0) {
                var9 = 1;
            } else { 
                var9 = 0;
                ++vasl;
                strcpy(bootIni->param4[vasl], paramName);
                if(vasl >= 8) {
                    return -1;
                }
            }
            break;
        case LTYPE_PARAM:
            if(GetParam(script, paramName, paramVal) != 0) {
                goto L42803010;
            }
            var7 = ParseParamVal(paramVal, sp24);
            if(var7 == -1) {
                goto L42803010;

            }
            if(var9 == 1) {
                if(strcmp(paramName, "start_os_name") == 0) {
                    strncpy(bootIni->startOSName, paramVal, sp24[0] << 2);
                } else if(strcmp(paramName, "timeout") == 0) {
                    bootIni->timeout = sp24[0] * sp24[1];
                } else if(strcmp(paramName, "display_device") == 0) {
                    bootIni->displayDevice = sp24[0] * sp24[1];
                } else if(strcmp(paramName, "display_mode") == 0) {
                    bootIni->displayMode = sp24[0] * sp24[1];
                }
            } else { 
                if(var9 == 0) {
                    strcpy(sp4, "os_show[0]");
                    for(var8 = 0; var8 < 4; ++var8) {
                        if(strcmp(paramName, sp4) == 0) {
                            if(sp24 != 0) {
                                strncpy(bootIni->param312[vasl].fnames[var8],
                                        paramVal, sp24[0] << 2);
                                ++bootIni->param312[vasl].filecount;
                                break;
                            }
                        }
                        ++sp4[8];
                    }
                }
            }
            break;
        default:
            goto L42803010;
        }
        script = script + lineLen;
    }
    bootIni->param0 = vasl + 1;
    for(var8 = 0; var8 < 8; ++var8) {
        if(strcmp(bootIni->startOSName, bootIni->param4[var8]) == 0) {
            bootIni->param292 = var8;
            break;
        }
    }
L42803010:
    var7 = var7 >> 31;
    return var7;
}

int script_patch(const char *fname, void *var5, int is_osimg)
{
    int hdle;
    int res;
    char *buf;
    int filesize;

    res = -1;
    buf = 0;
    if(fname == 0) {
        wlibc_uprintf("the input script is empty\n");
        return -1;
    }
    hdle = svc_open(fname, "rb");
    if(hdle == 0) {
        wlibc_uprintf("unable to open script file %s\n", fname);
        return -1;

    }
    filesize = svc_filesize(hdle);
    buf = svc_alloc(filesize);
    if(buf == 0) {
        wlibc_uprintf("unable to malloc memory for script\n");
    } else { 
        svc_read(buf, 1, filesize, hdle);
        svc_close(hdle);
        hdle = 0;
        if(is_osimg == 0) {
            res = parser_script_os_sel(buf, filesize, var5);
        } else { 
            res = parser_script_os_img(buf, filesize, var5);
        }
        if(res != 0) {
            wlibc_uprintf("script is invalid\n");
        }
    }
    if(buf != 0) {
        svc_free(buf);
    }
    if(hdle != 0) {
        svc_close(hdle);
        hdle = 0;
    }
    return res;
}

