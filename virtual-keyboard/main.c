#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/extensions/XTest.h>
#include "cmdline.h"

enum VirtKeyType {
    VKT_NORMAL,
    VKT_SHIFT,
    VKT_CTRL,
    VKT_ALT,
    VKT_FNMAIN,     /* "Fn" key on main keyboard */
    VKT_FNFN,       /* "Fn" key on "Fn" keyboard */
    VKT_ALTPOS,     /* "Move to alternate position" key */
    VKT_DISMISS,
    VKT_SPACER,
    VKT_END
};

struct VirtKey {
    enum VirtKeyType vkt;
    const char *disp;       /* text displayed on the key */
    guint kcode;            /* the key code */
    guint hsize;            /* number of cells occupied on grid */
};

/* Main keyboard layout
 */
static const struct VirtKey *gKeyboardMain[] = {
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
        { .vkt = VKT_DISMISS, .disp = "Di",   .kcode =   0, .hsize = 2 },
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
        { .vkt = VKT_NORMAL, .disp = "^",     .kcode = 111, .hsize = 2 },
        { .vkt = VKT_ALTPOS, .disp = "^v",    .kcode =   0, .hsize = 2 },
        { .vkt = VKT_END }
    },
    (const struct VirtKey[]){
        { .vkt = VKT_CTRL,   .disp = "Ctrl",  .kcode =  37, .hsize = 4 },
        { .vkt = VKT_ALT,    .disp = "Alt",   .kcode =  64, .hsize = 4 },
        { .vkt = VKT_NORMAL, .disp = " ",     .kcode =  65, .hsize = 14 },
        { .vkt = VKT_FNMAIN, .disp = "Fn",    .kcode =   0, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "<",     .kcode = 113, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "v",     .kcode = 116, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = ">",     .kcode = 114, .hsize = 2 },
        { .vkt = VKT_END }
    },
    NULL
};

/* Layout of "Fn" keyboard
 */
static const struct VirtKey *gKeyboardFn[] = {
    (const struct VirtKey[]){
        { .vkt = VKT_SPACER, .disp = NULL,    .kcode =   0, .hsize = 3 },
        { .vkt = VKT_NORMAL, .disp = "Volume\nDown", .kcode = 122, .hsize = 3},
        { .vkt = VKT_NORMAL, .disp = "Volume\nUp", .kcode = 123, .hsize = 3 },
        { .vkt = VKT_NORMAL, .disp = "Mute",  .kcode = 121, .hsize = 2 },
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
        { .vkt = VKT_NORMAL, .disp = "Screen\nBrighter",
            .kcode =  233, .hsize = 3 },
        { .vkt = VKT_NORMAL, .disp = "Screen\nDimmer",
            .kcode =  232, .hsize = 3 },
        { .vkt = VKT_SPACER, .disp = NULL,    .kcode =   0, .hsize = 3 },
        { .vkt = VKT_NORMAL, .disp = "F5",    .kcode =  71, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "F6",    .kcode =  72, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "F7",    .kcode =  73, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "F8",    .kcode =  74, .hsize = 2 },
        { .vkt = VKT_SPACER, .disp = NULL,    .kcode =   0, .hsize = 1 },
        { .vkt = VKT_NORMAL, .disp = "Insert",   .kcode =  118, .hsize = 3 },
        { .vkt = VKT_NORMAL, .disp = "Home",  .kcode =  110, .hsize = 3 },
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
        { .vkt = VKT_NORMAL, .disp = "Delete",   .kcode =  119, .hsize = 3 },
        { .vkt = VKT_NORMAL, .disp = "End",   .kcode =  115, .hsize = 3 },
        { .vkt = VKT_NORMAL, .disp = "Page\nDown", .kcode =  117, .hsize = 3 },
        { .vkt = VKT_END }
    },
    (const struct VirtKey[]){
        { .vkt = VKT_SHIFT, .disp = "Shift",  .kcode =  50, .hsize = 5 },
        { .vkt = VKT_SPACER, .disp = NULL,    .kcode =   0, .hsize = 21 },
        { .vkt = VKT_NORMAL, .disp = "^",     .kcode = 111, .hsize = 2 },
        { .vkt = VKT_ALTPOS, .disp = "^v",    .kcode =   0, .hsize = 2 },
        { .vkt = VKT_END }
    },
    (const struct VirtKey[]){
        { .vkt = VKT_CTRL,   .disp = "Ctrl",  .kcode =  37, .hsize = 4 },
        { .vkt = VKT_ALT,    .disp = "Alt",   .kcode =  64, .hsize = 4 },
        { .vkt = VKT_NORMAL, .disp = " ",     .kcode =  65, .hsize = 14 },
        { .vkt = VKT_FNFN,   .disp = "Fn",    .kcode =   0, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "<",     .kcode = 113, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = "v",     .kcode = 116, .hsize = 2 },
        { .vkt = VKT_NORMAL, .disp = ">",     .kcode = 114, .hsize = 2 },
        { .vkt = VKT_END }
    },
    NULL
};

/* Toggle buttons of modifier keys
 */
static struct ModifierButtons {
    GtkWidget *shift, *ctrl, *alt;
} modifiersMain, modifiersFn, *modifiers = &modifiersMain;


/* The main window position on screen
 */
static guint gWinX, gWinY, gWinXAlt, gWinYAlt;

static gboolean gIsAltPos;


/* "clicked" event handler of VKT_NORMAL button
 */
static void on_click(GtkWidget *widget, gpointer data)
{
    struct VirtKey *vk = data;
    Display *dp;

    dp = gdk_x11_display_get_xdisplay(gtk_widget_get_display(widget));
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(modifiers->shift)) ) {
        XTestFakeKeyEvent(dp, 50, TRUE, CurrentTime);
    }
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(modifiers->ctrl)) ) {
        XTestFakeKeyEvent(dp, 37, TRUE, CurrentTime);
    }
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(modifiers->alt)) ) {
        XTestFakeKeyEvent(dp, 64, TRUE, CurrentTime);
    }
    XTestFakeKeyEvent(dp, vk->kcode, TRUE, CurrentTime);
    XTestFakeKeyEvent(dp, vk->kcode, FALSE, CurrentTime);
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(modifiers->alt)) ) {
        XTestFakeKeyEvent(dp, 64, FALSE, CurrentTime);
    }
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(modifiers->ctrl)) ) {
        XTestFakeKeyEvent(dp, 37, FALSE, CurrentTime);
    }
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(modifiers->shift)) ) {
        XTestFakeKeyEvent(dp, 50, FALSE, CurrentTime);
    }
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(modifiers->shift), FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(modifiers->ctrl), FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(modifiers->alt), FALSE);
}

/* "clicked" event handler of VKT_ALTPOS button
 */
static void on_click_altpos(GtkWidget *button, gpointer data)
{
    GtkWidget *window = data;
    gint x, y;

    gtk_window_get_position(GTK_WINDOW(window), &x, &y);
    if( (gIsAltPos = !gIsAltPos) ) {
        gWinX = x;
        gWinY = y;
        x = gWinXAlt;
        y = gWinYAlt;
    }else{
        gWinXAlt = x;
        gWinYAlt = y;
        x = gWinX;
        y = gWinY;
    }
    gtk_window_move(GTK_WINDOW(window), x, y);
}


/* "clicked" event handler of VKT_DISMISS button
 */
static void on_click_dismiss(GtkWidget *window)
{
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(modifiers->alt)) ) {
        gtk_widget_destroy(window);
    }else{
        gtk_window_iconify(GTK_WINDOW(window));
    }
}

static void ReplicateModifiers(const struct ModifierButtons *to,
        const struct ModifierButtons *from)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(to->shift),
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(from->shift)));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(to->ctrl),
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(from->ctrl)));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(to->alt),
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(from->alt)));
}

static void on_click_fnmain(GtkWidget *button, gpointer data)
{
    GtkNotebook *notebook = data;

    gtk_notebook_set_current_page(notebook, 1);
    ReplicateModifiers(&modifiersFn, &modifiersMain);
    modifiers = &modifiersFn;
}

static void on_click_fnfn(GtkWidget *button, gpointer data)
{
    GtkNotebook *notebook = data;

    gtk_notebook_set_current_page(notebook, 0);
    ReplicateModifiers(&modifiersMain, &modifiersFn);
    modifiers = &modifiersMain;
}

static guint CalcOptionValue(guint val, gboolean isNeg, gboolean isPercentage,
        guint val100Percent, guint valToSubIfNeg)
{
    if( isPercentage )
        val = val100Percent * val / 100;
    if( isNeg ) {
        val += valToSubIfNeg;
        val = val > val100Percent ? 0 : val100Percent - val;
    }
    return val;
}

static GtkWidget *CreateButton(const char *text, gboolean isToggle)
{
    GtkWidget *label, *button;
   
    label = gtk_label_new(text);
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
    button = isToggle ? gtk_toggle_button_new() : gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(button), label);
    return button;
}

static GtkWidget *CreateKeyboard(const struct VirtKey *const *keyboardLayout,
        GtkWidget *window, GtkWidget *notebook,
        struct ModifierButtons *mod)
{
    GtkGrid *grid;
    GtkWidget *button;
    const struct VirtKey *const*row, *key;
    gint rowNo, colNo;

    grid = GTK_GRID(gtk_grid_new());
    for(rowNo = 0; rowNo < 5; ++rowNo)
        gtk_grid_insert_row(grid, rowNo);
    for(colNo = 0; colNo < 30; ++colNo)
        gtk_grid_insert_column(grid, colNo);
    gtk_grid_set_row_homogeneous(grid, TRUE);
    gtk_grid_set_column_homogeneous(grid, TRUE);

    rowNo = 0;
    for(row = keyboardLayout; *row != NULL; ++row) {
        colNo = 0;
        for(key = *row; key->vkt != VKT_END; ++key) {
            switch( key->vkt ) {
            case VKT_NORMAL:
                button = CreateButton(key->disp, FALSE);
                g_signal_connect(button, "clicked", G_CALLBACK (on_click),
                        (void*)key);
                break;
            case VKT_SHIFT:
                mod->shift = button = CreateButton(key->disp, TRUE);
                break;
            case VKT_CTRL:
                mod->ctrl = button = CreateButton(key->disp, TRUE);
                break;
            case VKT_ALT:
                mod->alt = button = CreateButton(key->disp, TRUE);
                break;
            case VKT_DISMISS:
                button = gtk_button_new();
                gtk_button_set_image(GTK_BUTTON(button),
                        gtk_image_new_from_stock(GTK_STOCK_CLOSE,
                        GTK_ICON_SIZE_BUTTON));
                g_signal_connect_swapped(button, "clicked",
                        G_CALLBACK(on_click_dismiss), window);
                break;
            case VKT_FNMAIN:
                button = CreateButton(key->disp, FALSE);
                g_signal_connect(button, "clicked",
                        G_CALLBACK(on_click_fnmain), GTK_NOTEBOOK(notebook));
                break;
            case VKT_FNFN:
                button = CreateButton(key->disp, FALSE);
                g_signal_connect(button, "clicked",
                        G_CALLBACK(on_click_fnfn), GTK_NOTEBOOK(notebook));
                break;
            case VKT_ALTPOS:
                button = CreateButton(key->disp, FALSE);
                g_signal_connect(button, "clicked",
                        G_CALLBACK(on_click_altpos), window);
                break;
            default:
                button = NULL;
            }
            if( button != NULL ) {
                gtk_grid_attach(grid, button, colNo, rowNo, key->hsize, 1);
                gtk_widget_show(button);
            }
            colNo += key->hsize;
        }
        ++rowNo;
    }
    gtk_widget_show(GTK_WIDGET(grid));
    return GTK_WIDGET(grid);
}

static void InitMainWindow(struct CmdLineOptions *opts, GtkApplication *app)
{
    GtkWidget *window, *notebook;
    GdkScreen *screen;
    guint screenWidth, screenHeight;
    gint winWidth, winHeight;

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    screen = gtk_widget_get_screen(window);
    gtk_window_set_decorated(GTK_WINDOW(window), opts->isDecorated);
    gtk_window_set_has_resize_grip(GTK_WINDOW(window), opts->hasResizeGrip);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), !opts->isOnTaskBar);
    gtk_window_set_keep_above(GTK_WINDOW(window), opts->isOnTop);
    gtk_window_set_accept_focus(GTK_WINDOW(window), FALSE);
    if( opts->winStateToSet == CL_WSTATE_ICONIFY )
        gtk_window_iconify(GTK_WINDOW(window));

    screenWidth = gdk_screen_get_width(screen);
    screenHeight = gdk_screen_get_height(screen);

    winWidth = CalcOptionValue(opts->width, opts->isWidthNeg,
            opts->isWidthPercent, screenWidth, 0);
    winHeight = CalcOptionValue(opts->height, FALSE, opts->isHeightPercent,
            winWidth, 0);
    gWinX = CalcOptionValue(opts->x, opts->isXNeg, opts->isXPercent,
            screenWidth, winWidth);
    gWinY = CalcOptionValue(opts->y, opts->isYNeg, opts->isYPercent,
            screenHeight, winHeight);
    gWinXAlt = CalcOptionValue(opts->xalt, opts->isXAltNeg, opts->isXAltPercent,
            screenWidth, winWidth);
    gWinYAlt = CalcOptionValue(opts->yalt, opts->isYAltNeg, opts->isYAltPercent,
            screenHeight, winHeight);
    gtk_window_set_default_size(GTK_WINDOW(window), 
            winWidth, winHeight);
    gtk_window_move(GTK_WINDOW(window), gWinX, gWinY);

    notebook = gtk_notebook_new();
    gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
            CreateKeyboard(gKeyboardMain, window, notebook, &modifiersMain),
            NULL);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
            CreateKeyboard(gKeyboardFn, window, notebook, &modifiersFn), NULL);
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(notebook));
    gtk_widget_show(GTK_WIDGET(notebook));
    gtk_window_set_application(GTK_WINDOW (window), app);
    gtk_widget_show(window);
}


static void command_line(GApplication *app, GApplicationCommandLine *cmdLine,
        gpointer user_data)
{
    GList *list;
    struct CmdLineOptions *opts = user_data;

    list = gtk_application_get_windows(GTK_APPLICATION(app));
    if (list) {
        int argc;
        gchar **argv;
        enum CmdLine_WinStateSet winState;
       
        argv = g_application_command_line_get_arguments(cmdLine, &argc);
        winState = argc < 2 ? CL_WSTATE_TOGGLE : argv[1][0] - 'A';

        if( winState == CL_WSTATE_TOGGLE ) {
            GdkWindow *win = gtk_widget_get_window(list->data);
            winState = (win == NULL ||
                    gdk_window_get_state(win) & GDK_WINDOW_STATE_ICONIFIED) ?
                    CL_WSTATE_PRESENT : CL_WSTATE_ICONIFY;
        }
        if( winState == CL_WSTATE_PRESENT ) {
            gtk_window_present(GTK_WINDOW(list->data));
        }else{
            on_click_dismiss(list->data);
        }
    }else{
        InitMainWindow(opts, GTK_APPLICATION(app));
    }
}

int main(int argc, char *argv[])
{
    struct CmdLineOptions opts;
    gint status = 0;
    char cmdopt[2];
    char *cmdopts[] = { argv[0], cmdopt, NULL };

    if( ParseCmdLine(argc, argv, &opts) ) {
        GtkApplication *app;
        app = gtk_application_new("org.novo7tools.virtual-keyboard",
                G_APPLICATION_HANDLES_COMMAND_LINE);
        g_signal_connect(app, "command-line", G_CALLBACK(command_line), &opts);
        cmdopt[0] = opts.winStateToSet + 'A';
        cmdopt[1] = '\0';
        status = g_application_run(G_APPLICATION(app), 2, cmdopts);
        g_object_unref(app);
    }
    return status;
}

