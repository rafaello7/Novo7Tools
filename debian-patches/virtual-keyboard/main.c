#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/extensions/XTest.h>
#include "cmdline.h"

enum VirtKeyType {
    VKT_NORMAL,
    VKT_SHIFT,
    VKT_CTRL,
    VKT_ALT,
    VKT_ALTPOS,
    VKT_DISMISS,
    VKT_SPACER,
    VKT_END
};

struct VirtKey {
    enum VirtKeyType vkt;
    const char *disp;
    guint kcode;
    guint hsize;
};

static const struct VirtKey *gKeyboard[] = {
    (const struct VirtKey[]){
        {
            .vkt = VKT_NORMAL, .disp = "~\n`",
            .kcode =  49, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "!\n1",
            .kcode =  10, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "@\n2",
            .kcode =  11, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "#\n3",
            .kcode =  12, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "$\n4",
            .kcode =  13, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "%\n5",
            .kcode =  14, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "^\n6",
            .kcode =  15, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "&\n7",
                .kcode =  16, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "*\n8",
                .kcode =  17, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "(\n9",
                .kcode =  18, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = ")\n0",
                .kcode =  19, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "_\n-",
                .kcode =  20, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "+\n=",
                .kcode =  21, .hsize = 2
        },{
            .vkt = VKT_DISMISS, .disp = "Di",
                .kcode =   0, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "<-",
                .kcode =  22, .hsize = 2
        },{
            .vkt = VKT_END
        }
    },
    (const struct VirtKey[]){
        {
            .vkt = VKT_NORMAL, .disp = "Tab",
            .kcode =  23, .hsize = 3
        },{
            .vkt = VKT_NORMAL, .disp = "q",
            .kcode =  24, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "w",
            .kcode =  25, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "e",
            .kcode =  26, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "r",
            .kcode =  27, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "t",
            .kcode =  28, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "y",
            .kcode =  29, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "u",
                .kcode =  30, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "i",
                .kcode =  31, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "o",
                .kcode =  32, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "p",
                .kcode =  33, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "{\n[",
                .kcode =  34, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "}\n]",
                .kcode =  35, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "|\n\\",
                .kcode =  51, .hsize = 3
        },{
            .vkt = VKT_END
        }
    },
    (const struct VirtKey[]){
        {
            .vkt = VKT_NORMAL, .disp = "Esc",
            .kcode =   9, .hsize = 4
        },{
            .vkt = VKT_NORMAL, .disp = "a",
            .kcode =  38, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "s",
            .kcode =  39, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "d",
            .kcode =  40, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "f",
            .kcode =  41, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "g",
            .kcode =  42, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "h",
            .kcode =  43, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "j",
                .kcode =  44, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "k",
                .kcode =  45, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "l",
                .kcode =  46, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = ":\n;",
                .kcode =  47, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "\"\n'",
                .kcode =  48, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "Enter",
                .kcode =  36, .hsize = 4
        },{
            .vkt = VKT_END
        }
    },
    (const struct VirtKey[]){
        {
            .vkt = VKT_SHIFT, .disp = "Shift",
            .kcode =  50, .hsize = 5
        },{
            .vkt = VKT_NORMAL, .disp = "z",
            .kcode =  52, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "x",
            .kcode =  53, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "c",
            .kcode =  54, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "v",
            .kcode =  55, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "b",
            .kcode =  56, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "n",
            .kcode =  57, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "m",
            .kcode =  58, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "<\n,",
            .kcode =  59, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = ">\n.",
            .kcode =  60, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "?\n/",
            .kcode =  61, .hsize = 2
        },{
            .vkt = VKT_SPACER, .disp = NULL,
            .kcode =   0, .hsize = 1
        },{
            .vkt = VKT_NORMAL, .disp = "^",
            .kcode = 111, .hsize = 2
        },{
            .vkt = VKT_SPACER, .disp = NULL,
            .kcode =   0, .hsize = 2
        },{
            .vkt = VKT_END
        }
    },
    (const struct VirtKey[]){
        {
            .vkt = VKT_CTRL, .disp = "Ctrl",
            .kcode =  37, .hsize = 4
        },{
            .vkt = VKT_ALT, .disp = "Alt",
            .kcode =  64, .hsize = 4
        },{
            .vkt = VKT_NORMAL, .disp = " ",
            .kcode =  65, .hsize = 14
        },{
            .vkt = VKT_ALTPOS, .disp = "^v",
            .kcode =   0, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "<",
            .kcode = 113, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = "v",
            .kcode = 116, .hsize = 2
        },{
            .vkt = VKT_NORMAL, .disp = ">",
            .kcode = 114, .hsize = 2
        },{
            .vkt = VKT_END
        }
    },
    NULL
};

static guint gWinX, gWinY, gWinXAlt, gWinYAlt;
static GtkWidget *gButtonShift, *gButtonCtrl, *gButtonAlt;

static void on_click(GtkWidget *widget, gpointer data)
{
    struct VirtKey *vk = data;
    Display *dp;

    dp = gdk_x11_display_get_xdisplay(gtk_widget_get_display(widget));
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gButtonShift)) ) {
        XTestFakeKeyEvent(dp, 50, TRUE, CurrentTime);
    }
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gButtonCtrl)) ) {
        XTestFakeKeyEvent(dp, 37, TRUE, CurrentTime);
    }
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gButtonAlt)) ) {
        XTestFakeKeyEvent(dp, 64, TRUE, CurrentTime);
    }
    XTestFakeKeyEvent(dp, vk->kcode, TRUE, CurrentTime);
    XTestFakeKeyEvent(dp, vk->kcode, FALSE, CurrentTime);
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gButtonAlt)) ) {
        XTestFakeKeyEvent(dp, 64, FALSE, CurrentTime);
    }
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gButtonCtrl)) ) {
        XTestFakeKeyEvent(dp, 37, FALSE, CurrentTime);
    }
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gButtonShift)) ) {
        XTestFakeKeyEvent(dp, 50, FALSE, CurrentTime);
    }
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gButtonShift), FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gButtonCtrl), FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gButtonAlt), FALSE);
}

static void on_clicked_altpos(GtkWidget *button, gpointer data)
{
    GtkWidget *window = data;
    gboolean isAltPos = gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON(button));
    gint x, y;

    gtk_window_get_position(GTK_WINDOW(window), &x, &y);
    if( isAltPos ) {
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

static void on_clicked_dismiss(GtkWidget *window)
{
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gButtonAlt)) ) {
        gtk_widget_destroy(window);
    }else{
        gtk_window_iconify(GTK_WINDOW(window));
    }
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

static void InitMainWindow(struct CmdLineOptions *opts, GtkApplication *app)
{
    GtkWidget *window;
    GtkGrid *grid;
    GtkWidget *button;
    GdkScreen *screen;
    const struct VirtKey *const*row, *key;
    guint screenWidth, screenHeight;
    gint rowNo, colNo, winWidth, winHeight;

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    screen = gtk_widget_get_screen(window);
    gtk_window_set_decorated(GTK_WINDOW(window), opts->isDecorated);
    gtk_window_set_has_resize_grip(GTK_WINDOW(window), opts->hasResizeGrip);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), !opts->isOnTaskBar);
    gtk_window_set_keep_above(GTK_WINDOW(window), opts->isOnTop);
    gtk_window_set_accept_focus(GTK_WINDOW(window), FALSE);

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

    grid = GTK_GRID(gtk_grid_new());
    for(rowNo = 0; rowNo < 5; ++rowNo)
        gtk_grid_insert_row(grid, rowNo);
    for(colNo = 0; colNo < 30; ++colNo)
        gtk_grid_insert_column(grid, colNo);
    gtk_grid_set_row_homogeneous(grid, TRUE);
    gtk_grid_set_column_homogeneous(grid, TRUE);
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(grid));

    rowNo = 0;
    for(row = gKeyboard; *row != NULL; ++row) {
        colNo = 0;
        for(key = *row; key->vkt != VKT_END; ++key) {
            switch( key->vkt ) {
            case VKT_NORMAL:
                button = CreateButton(key->disp, FALSE);
                g_signal_connect(button, "clicked", G_CALLBACK (on_click),
                        (void*)key);
                break;
            case VKT_SHIFT:
                gButtonShift = button = CreateButton(key->disp, TRUE);
                break;
            case VKT_CTRL:
                gButtonCtrl = button = CreateButton(key->disp, TRUE);
                break;
            case VKT_ALT:
                gButtonAlt = button = CreateButton(key->disp, TRUE);
                break;
            case VKT_DISMISS:
                button = gtk_button_new();
                gtk_button_set_image(GTK_BUTTON(button),
                        gtk_image_new_from_stock(GTK_STOCK_CLOSE,
                        GTK_ICON_SIZE_BUTTON));
                g_signal_connect_swapped(button, "clicked",
                        G_CALLBACK(on_clicked_dismiss), window);
                break;
            case VKT_ALTPOS:
                button = CreateButton(key->disp, TRUE);
                g_signal_connect(button, "clicked",
                        G_CALLBACK(on_clicked_altpos), window);
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
    gtk_window_set_application(GTK_WINDOW (window), app);
    gtk_widget_show(window);
}


static void activate(GtkApplication *app, gpointer user_data)
{
    GList *list;
    struct CmdLineOptions *opts = user_data;

    list = gtk_application_get_windows(app);
    if (list) {
        gtk_window_present(GTK_WINDOW(list->data));
    }else{
        InitMainWindow(opts, app);
    }
}

int main(int argc, char *argv[])
{
    struct CmdLineOptions opts;
    gint status = 0;

    if( ParseCmdLine(argc, argv, &opts) ) {
        GtkApplication *app;
        app = gtk_application_new("org.novo7tools.virtual-keyboard",
                G_APPLICATION_FLAGS_NONE);
        g_signal_connect(app, "activate", G_CALLBACK(activate), &opts);
        status = g_application_run(G_APPLICATION(app), 0, NULL);
        g_object_unref(app);
    }
    return status;
}

