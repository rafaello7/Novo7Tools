#include <gtk/gtk.h>
#include "keyboard_layout.h"
#include <string.h>


enum {
    VKL_ROWCOUNT = 5,
    VKL_GRID_COLCOUNT = 30 /* number of columns occupied by keyboard on grid */
};

enum VirtKeyTypeSpec {
    VKT_SPACER = -1,
    VKT_END    = -2
};

struct VirtKey {
    int vkt;                /* VirtKeyType or VirtKeyTypeSpec value */
    const char *image;      /* image displayed on key */
    const char *disp;       /* text displayed on the key when image is NULL */
    guint kcode;            /* the key code */
    guint hsize;            /* number of cells occupied on grid */
};

struct VirtKeyboardLayout {
    const struct VirtKey* rows[VKL_ROWCOUNT];
};

/* Main keyboard layout
 */
static const struct VirtKeyboardLayout gKeyboardMain = { .rows = {
    (const struct VirtKey[]){
        { .vkt = VKT_NORMAL, .disp = "~\n`",  .kcode =  49, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "!\n1",  .kcode =  10, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "@\n2",  .kcode =  11, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "#\n3",  .kcode =  12, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "$\n4",  .kcode =  13, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "%\n5",  .kcode =  14, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "^\n6",  .kcode =  15, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "&\n7",  .kcode =  16, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "*\n8",  .kcode =  17, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "(\n9",  .kcode =  18, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = ")\n0",  .kcode =  19, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "_\n-",  .kcode =  20, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "+\n=",  .kcode =  21, .hsize = 2 },
        { .vkt = VKT_DISMISS, .image = GTK_STOCK_CLOSE, .disp = "Di",
                                              .kcode =   0, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "<-",    .kcode =  22, .hsize = 2 },
        { .vkt = VKT_END }
    },
    (const struct VirtKey[]){
        { .vkt = VKT_NORMAL, .disp = "Tab",   .kcode =  23, .hsize = 3 },
        { .vkt = VKT_NORMAL, .disp = "q",     .kcode =  24, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "w",     .kcode =  25, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "e",     .kcode =  26, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "r",     .kcode =  27, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "t",     .kcode =  28, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "y",     .kcode =  29, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "u",     .kcode =  30, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "i",     .kcode =  31, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "o",     .kcode =  32, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "p",     .kcode =  33, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "{\n[",  .kcode =  34, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "}\n]",  .kcode =  35, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "|\n\\", .kcode =  51, .hsize = 3 },
        { .vkt = VKT_END }
    },
    (const struct VirtKey[]){
        { .vkt = VKT_NORMAL, .disp = "Esc",   .kcode =   9, .hsize = 4 },
        { .vkt = VKT_NORMAL, .disp = "a",     .kcode =  38, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "s",     .kcode =  39, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "d",     .kcode =  40, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "f",     .kcode =  41, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "g",     .kcode =  42, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "h",     .kcode =  43, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "j",     .kcode =  44, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "k",     .kcode =  45, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "l",     .kcode =  46, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = ":\n;",  .kcode =  47, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "\"\n'", .kcode =  48, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "Enter", .kcode =  36, .hsize = 4 },
        { .vkt = VKT_END }
    },
    (const struct VirtKey[]){
        { .vkt = VKT_SHIFT, .disp = "Shift",  .kcode =  50, .hsize = 5 },
        { .vkt = VKT_NORMAL, .disp = "z",     .kcode =  52, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "x",     .kcode =  53, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "c",     .kcode =  54, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "v",     .kcode =  55, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "b",     .kcode =  56, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "n",     .kcode =  57, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "m",     .kcode =  58, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "<\n,",  .kcode =  59, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = ">\n.",  .kcode =  60, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "?\n/",  .kcode =  61, .hsize = 2 },
        { .vkt = VKT_SPACER, .disp = NULL,    .kcode =   0, .hsize = 1 },
        { .vkt = VKT_NORMAL, .disp = "\xe2\x86\x91", .kcode = 111, .hsize = 2},
        { .vkt = VKT_ALTPOS, .image = "go-jump",
                             .disp = "\xe2\x86\x91\xe2\x86\x93",
                             .kcode = 0, .hsize = 2 },
        { .vkt = VKT_END }
    },
    (const struct VirtKey[]){
        { .vkt = VKT_CTRL,   .disp = "Ctrl",  .kcode =  37, .hsize = 4 },
        { .vkt = VKT_ALT,    .disp = "Alt",   .kcode =  64, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = " ",     .kcode =  65, .hsize = 14 },
        { .vkt = VKT_RIGHTALT,.disp = "Alt",  .kcode =   0, .hsize = 2 },
        { .vkt = VKT_FNMAIN, .disp = "Fn",    .kcode =   0, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "\xe2\x86\x90", .kcode = 113, .hsize = 2},
        { .vkt = VKT_NORMAL, .disp = "\xe2\x86\x93", .kcode = 116, .hsize = 2},
        { .vkt = VKT_NORMAL, .disp = "\xe2\x86\x92", .kcode = 114, .hsize = 2},
        { .vkt = VKT_END }
    }
} };

/* Layout of "Fn" keyboard
 */
static const struct VirtKeyboardLayout gKeyboardFn = { .rows = {
    (const struct VirtKey[]){
        { .vkt = VKT_SPACER, .disp = NULL,    .kcode =   0, .hsize = 3 },
        { .vkt = VKT_NORMAL, .image = "audio-volume-low",
                             .disp = "Volume\nDown", .kcode = 122, .hsize = 3},
        { .vkt = VKT_NORMAL, .image = "audio-volume-high",
                             .disp = "Volume\nUp", .kcode = 123, .hsize = 3 },
        { .vkt = VKT_NORMAL, .image = "audio-volume-muted",
                             .disp = "Mute",  .kcode = 121, .hsize = 2 },
        { .vkt = VKT_SPACER, .disp = NULL,    .kcode =   0, .hsize = 1 },
        { .vkt = VKT_NORMAL, .disp = "F1",    .kcode =  67, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "F2",    .kcode =  68, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "F3",    .kcode =  69, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "F4",    .kcode =  70, .hsize = 2 },
        { .vkt = VKT_SPACER, .disp = NULL,    .kcode =   0, .hsize = 1 },
        { .vkt = VKT_NORMAL, .disp = "Print\nScreen", .kcode = 107, .hsize = 3},
        { .vkt = VKT_NORMAL, .disp = "Scroll\nLock", .kcode = 78, .hsize = 3 },
        { .vkt = VKT_NORMAL, .disp = "Pause", .kcode =  127, .hsize = 3 },
        { .vkt = VKT_END }
    },
    (const struct VirtKey[]){
        { .vkt = VKT_SPACER, .disp = NULL,    .kcode =   0, .hsize = 3 },
        { .vkt = VKT_NORMAL, .disp = "Screen\nDimmer",
            .kcode =  232, .hsize = 3 },
        { .vkt = VKT_NORMAL, .disp = "Screen\nBrighter",
            .kcode =  233, .hsize = 3 },
        { .vkt = VKT_SPACER, .disp = NULL,    .kcode =   0, .hsize = 3 },
        { .vkt = VKT_NORMAL, .disp = "F5",    .kcode =  71, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "F6",    .kcode =  72, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "F7",    .kcode =  73, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "F8",    .kcode =  74, .hsize = 2 },
        { .vkt = VKT_SPACER, .disp = NULL,    .kcode =   0, .hsize = 1 },
        { .vkt = VKT_NORMAL, .disp = "Insert",.kcode = 118, .hsize = 3 },
        { .vkt = VKT_NORMAL, .disp = "Home",  .kcode = 110, .hsize = 3 },
        { .vkt = VKT_NORMAL, .disp = "Page\nUp",  .kcode =  112, .hsize = 3 },
        { .vkt = VKT_END }
    },
    (const struct VirtKey[]){
        { .vkt = VKT_SPACER, .disp = NULL,    .kcode =   0, .hsize = 12 },
        { .vkt = VKT_NORMAL, .disp = "F9",    .kcode =  75, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "F10",   .kcode =  76, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "F11",   .kcode =  95, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "F12",   .kcode =  96, .hsize = 2 },
        { .vkt = VKT_SPACER, .disp = NULL,    .kcode =   0, .hsize = 1 },
        { .vkt = VKT_NORMAL, .disp = "Delete",.kcode = 119, .hsize = 3 },
        { .vkt = VKT_NORMAL, .disp = "End",   .kcode = 115, .hsize = 3 },
        { .vkt = VKT_NORMAL, .disp = "Page\nDown", .kcode = 117, .hsize = 3 },
        { .vkt = VKT_END }
    },
    (const struct VirtKey[]){
        { .vkt = VKT_SHIFT, .disp = "Shift",  .kcode =  50, .hsize = 5 },
        { .vkt = VKT_SPACER, .disp = NULL,    .kcode =   0, .hsize = 21 },
        { .vkt = VKT_NORMAL, .disp = "\xe2\x86\x91", .kcode = 111, .hsize = 2 },
        { .vkt = VKT_ALTPOS, .image = "go-jump",
                             .disp = "\xe2\x86\x91\xe2\x86\x93",
                             .kcode =   0, .hsize = 2 },
        { .vkt = VKT_END }
    },
    (const struct VirtKey[]){
        { .vkt = VKT_CTRL,   .disp = "Ctrl",  .kcode =  37, .hsize = 4 },
        { .vkt = VKT_ALT,    .disp = "Alt",   .kcode =  64, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = " ",     .kcode =  65, .hsize = 14 },
        { .vkt = VKT_RIGHTALT,.disp = "Alt",  .kcode =   0, .hsize = 2 },
        { .vkt = VKT_FNFN,   .disp = "Fn",    .kcode =   0, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "\xe2\x86\x90", .kcode = 113, .hsize = 2},
        { .vkt = VKT_NORMAL, .disp = "\xe2\x86\x93", .kcode = 116, .hsize = 2},
        { .vkt = VKT_NORMAL, .disp = "\xe2\x86\x92", .kcode = 114, .hsize = 2},
        { .vkt = VKT_END }
    }
} };

guint vkl_GetKeyCodeFromUserData(gpointer user_data)
{
    struct VirtKey *vk = user_data;

    return vk->kcode;
}

struct KCodeCompose {
    char disp;
    guint kcodes[2];
};

static struct KCodeCompose gComposeKeys[256] = {
    { '\0'  }, { '\0'  }, { '\0'  }, { '\0'  }, { '\0'  }, { '\0'  },
    { '\0'  }, { '\0'  }, { '\0'  }, { '\033'}, { '1'   }, { '2'   },
    { '3'   }, { '4'   }, { '5'   }, { '6'   }, { '7'   }, { '8'   },
    { '9'   }, { '0'   }, { '-'   }, { '='   }, { '\b'  }, { '\t'  },
    { 'q'   }, { 'w'   }, { 'e'   }, { 'r'   }, { 't'   }, { 'y'   },
    { 'u'   }, { 'i'   }, { 'o'   }, { 'p'   }, { '['   }, { ']'   },
    { '\n'  }, { '\0'  }, { 'a'   }, { 's'   }, { 'd'   }, { 'f'   },
    { 'g'   }, { 'h'   }, { 'j'   }, { 'k'   }, { 'l'   }, { ';'   },
    { '\''  }, { '`'   }, { '\0'  }, { '\\'  }, { 'z'   }, { 'x'   },
    { 'c'   }, { 'v'   }, { 'b'   }, { 'n'   }, { 'm'   }, { ','   },
    { '.'   }, { '/'   },
};

static int getKCodeForChar(char c)
{
    int i;

    for(i = 0; i < 256 && gComposeKeys[i].disp != c; ++i) {
    }
    return i >= 0 ? i : -1;
}

/* cmdLineParam format:
 *  "aa,cc'ee,..."
 * consists of 3-character sequences. First character - the key, which
 * the compose sequence should be assigned to; two subsequent characters
 * - the compose sequence
 */
void vkl_SetComposeKeys(const char *cmdLineParam)
{
    int ckey, comp1, comp2;

    while(cmdLineParam[0] && cmdLineParam[1] && cmdLineParam[2]) {
        if( (ckey = getKCodeForChar(cmdLineParam[0])) >= 0 &&
                (comp1 = getKCodeForChar(cmdLineParam[1])) >= 0 &&
                (comp2 = getKCodeForChar(cmdLineParam[2])) >= 0)
        {
            gComposeKeys[ckey].kcodes[0] = comp1;
            gComposeKeys[ckey].kcodes[1] = comp2;
        }
        cmdLineParam += 3;
    }
}

guint *vkl_GetComposeFromUserData(gpointer user_data)
{
    struct VirtKey *vk = user_data;

    return gComposeKeys[vk->kcode].kcodes;
}

static GtkWidget *CreateKeyboard(
        const struct VirtKeyboardLayout *keyboardLayout,
        const struct VirtKeyHandler *handlers,
        struct ModifierButtons *mod)
{
    GtkGrid *grid;
    const struct VirtKey *const*row, *key;
    gint rowNo, colNo;

    grid = GTK_GRID(gtk_grid_new());
    for(rowNo = 0; rowNo < VKL_ROWCOUNT; ++rowNo)
        gtk_grid_insert_row(grid, rowNo);
    for(colNo = 0; colNo < VKL_GRID_COLCOUNT; ++colNo)
        gtk_grid_insert_column(grid, colNo);
    gtk_grid_set_row_homogeneous(grid, TRUE);
    gtk_grid_set_column_homogeneous(grid, TRUE);

    for(rowNo = 0, row = keyboardLayout->rows; rowNo < VKL_ROWCOUNT;
            ++rowNo, ++row)
    {
        colNo = 0;
        for(key = *row; key->vkt != VKT_END; ++key) {
            if( key->vkt != VKT_SPACER ) {
                const struct VirtKeyHandler *hdl = handlers + key->vkt;
                GtkWidget *button = hdl->isToggleButton ?
                    gtk_toggle_button_new() : gtk_button_new();
                if( key->image != NULL ) {
                    gtk_button_set_image(GTK_BUTTON(button),
                           gtk_image_new_from_icon_name(key->image,
                            GTK_ICON_SIZE_BUTTON));
                }else if( strchr(key->disp, '\n') == NULL ) {
                    gtk_button_set_label(GTK_BUTTON(button), key->disp);
                }else{
                    GtkWidget *label = gtk_label_new(key->disp);
                    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
                    gtk_button_set_image(GTK_BUTTON(button), label);
                }
                if( hdl->clickCallback != NULL ) {
                    g_signal_connect(button,
                            hdl->isToggleButton ? "toggled" : "clicked",
                            hdl->clickCallback,
                            hdl->userData == NULL ? (void*)key : hdl->userData);
                }
                if( hdl->btnPressCallback != NULL ) {
                    g_signal_connect(button, "button-press-event",
                            hdl->btnPressCallback,
                            hdl->userData == NULL ? (void*)key : hdl->userData);
                }
                if( hdl->btnReleaseCallback != NULL ) {
                    g_signal_connect(button, "button-release-event",
                            hdl->btnReleaseCallback,
                            hdl->userData == NULL ? (void*)key : hdl->userData);
                }
                if( button != NULL ) {
                    gtk_grid_attach(grid, button, colNo, rowNo, key->hsize, 1);
                    gtk_widget_show(button);
                }
                switch( key->vkt ) {
                case VKT_SHIFT:
                    mod->shift = button;
                    break;
                case VKT_CTRL:
                    mod->ctrl = button;
                    break;
                case VKT_ALT:
                    mod->alt = button;
                    break;
                case VKT_RIGHTALT:
                    mod->rightalt = button;
                    break;
                default:
                    break;
                }
            }
            colNo += key->hsize;
        }
    }
    gtk_widget_show(GTK_WIDGET(grid));
    return GTK_WIDGET(grid);
}

GtkWidget *vkl_CreateMainKeyboard(
        const struct VirtKeyHandler *handlers,
        struct ModifierButtons *mod)
{
    return CreateKeyboard(&gKeyboardMain, handlers, mod);
}

GtkWidget *vkl_CreateFnKeyboard(
        const struct VirtKeyHandler *handlers,
        struct ModifierButtons *mod)
{
    return CreateKeyboard(&gKeyboardFn, handlers, mod);
}

