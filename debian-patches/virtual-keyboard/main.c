#include <gtk/gtk.h>
#include <gdk/gdkx.h>

enum VirtKeyType {
    VKT_NORMAL,
    VKT_SHIFT,
    VKT_CTRL,
    VKT_ALT,
    VKT_MOVE,
    VKT_DISMISS,
    VKT_SPACER,
    VKT_END
};

struct VirtKey {
    enum VirtKeyType vkt;
    const char *disp, *dispShift;
    guint kcode;
    guint hsize;
};

static const struct VirtKey *gKeyboard[] = {
    (const struct VirtKey[]){
        { .vkt = VKT_NORMAL,  .disp = "`",     .dispShift = "~",  .kcode =  49, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "1",     .dispShift = "!",  .kcode =  10, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "2",     .dispShift = "@",  .kcode =  11, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "3",     .dispShift = "#",  .kcode =  12, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "4",     .dispShift = "$",  .kcode =  13, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "5",     .dispShift = "%",  .kcode =  14, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "6",     .dispShift = "^",  .kcode =  15, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "7",     .dispShift = "&",  .kcode =  16, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "8",     .dispShift = "*",  .kcode =  17, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "9",     .dispShift = "(",  .kcode =  18, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "0",     .dispShift = ")",  .kcode =  19, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "-",     .dispShift = "_",  .kcode =  20, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "=",     .dispShift = "+",  .kcode =  21, .hsize = 2 },
        { .vkt = VKT_DISMISS, .disp = "Di",    .dispShift = NULL, .kcode =   0, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "<-",    .dispShift = NULL, .kcode =  22, .hsize = 2 },
        { .vkt = VKT_END }
    },
    (const struct VirtKey[]){
        { .vkt = VKT_NORMAL,  .disp = "Tab",   .dispShift = NULL, .kcode =  23, .hsize = 3 },
        { .vkt = VKT_NORMAL,  .disp = "q",     .dispShift = "Q",  .kcode =  24, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "w",     .dispShift = "W",  .kcode =  25, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "e",     .dispShift = "E",  .kcode =  26, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "r",     .dispShift = "R",  .kcode =  27, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "t",     .dispShift = "T",  .kcode =  28, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "y",     .dispShift = "Y",  .kcode =  29, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "u",     .dispShift = "U",  .kcode =  30, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "i",     .dispShift = "I",  .kcode =  31, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "o",     .dispShift = "O",  .kcode =  32, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "p",     .dispShift = "P",  .kcode =  33, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "[",     .dispShift = "{",  .kcode =  34, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "]",     .dispShift = "}",  .kcode =  35, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "\\",    .dispShift = "|",  .kcode =  51, .hsize = 3 },
        { .vkt = VKT_END }
    },
    (const struct VirtKey[]){
        { .vkt = VKT_NORMAL,  .disp = "Esc",   .dispShift = NULL, .kcode =   9, .hsize = 4 },
        { .vkt = VKT_NORMAL,  .disp = "a",     .dispShift = "A",  .kcode =  38, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "s",     .dispShift = "S",  .kcode =  39, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "d",     .dispShift = "D",  .kcode =  40, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "f",     .dispShift = "F",  .kcode =  41, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "g",     .dispShift = "G",  .kcode =  42, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "h",     .dispShift = "H",  .kcode =  43, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "j",     .dispShift = "J",  .kcode =  44, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "k",     .dispShift = "K",  .kcode =  45, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "l",     .dispShift = "L",  .kcode =  46, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = ";",     .dispShift = ":",  .kcode =  47, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "'",     .dispShift = "\"", .kcode =  48, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "Enter", .dispShift = NULL, .kcode =  36, .hsize = 4 },
        { .vkt = VKT_END }
    },
    (const struct VirtKey[]){
        { .vkt = VKT_SHIFT,   .disp = "Shift", .dispShift = NULL, .kcode =  50, .hsize = 5 },
        { .vkt = VKT_NORMAL,  .disp = "z",     .dispShift = "Z",  .kcode =  52, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "x",     .dispShift = "X",  .kcode =  53, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "c",     .dispShift = "C",  .kcode =  54, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "v",     .dispShift = "V",  .kcode =  55, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "b",     .dispShift = "B",  .kcode =  56, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "n",     .dispShift = "N",  .kcode =  57, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "m",     .dispShift = "M",  .kcode =  58, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = ",",     .dispShift = "<",  .kcode =  59, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = ".",     .dispShift = ">",  .kcode =  60, .hsize = 2 },
        { .vkt = VKT_NORMAL,  .disp = "/",     .dispShift = "?",  .kcode =  61, .hsize = 2 },
        { .vkt = VKT_SPACER,  .disp = NULL,    .dispShift = NULL, .kcode =   0, .hsize = 1 },
        { .vkt = VKT_NORMAL,  .disp = "^",     .dispShift = NULL, .kcode = 111, .hsize = 2 },
        { .vkt = VKT_MOVE,    .disp = "^v",    .dispShift = NULL, .kcode =   0, .hsize = 2 },
        { .vkt = VKT_END }
    },
    (const struct VirtKey[]){
        { .vkt = VKT_CTRL,     .disp = "Ctrl", .dispShift = NULL, .kcode =  37, .hsize = 4 },
        { .vkt = VKT_ALT,      .disp = "Alt",  .dispShift = NULL, .kcode =  64, .hsize = 4 },
        { .vkt = VKT_NORMAL,   .disp = " ",    .dispShift = NULL, .kcode =  65, .hsize = 14 },
        { .vkt = VKT_SPACER,   .disp = NULL,   .dispShift = NULL, .kcode =   0, .hsize = 2 },
        { .vkt = VKT_NORMAL,   .disp = "<",    .dispShift = NULL, .kcode = 113, .hsize = 2 },
        { .vkt = VKT_NORMAL,   .disp = "v",    .dispShift = NULL, .kcode = 116, .hsize = 2 },
        { .vkt = VKT_NORMAL,   .disp = ">",    .dispShift = NULL, .kcode = 114, .hsize = 2 },
        { .vkt = VKT_END }
    },
    NULL
};

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

static void on_toggle_shift(GtkWidget *widget, gpointer data)
{
    GtkGrid *grid;
    GtkWidget *button;
    const struct VirtKey *const*row, *key;
    int rowNo, colNo;
    gboolean isShift;

    grid = GTK_GRID(gtk_bin_get_child(GTK_BIN(widget)));
    isShift = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gButtonShift));
    rowNo = 0;
    for(row = gKeyboard; *row != NULL; ++row) {
        colNo = 0;
        for(key = *row; key->vkt != VKT_END; ++key) {
            if( key->dispShift != NULL ) {
                button = gtk_grid_get_child_at(grid, colNo, rowNo);
                gtk_button_set_label(GTK_BUTTON(button),
                        isShift ? key->dispShift : key->disp);
            }
            colNo += key->hsize;
        }
        ++rowNo;
    }
}

static void on_move(GtkWidget *widget, gpointer data)
{
    GdkScreen *screen;
    gint x, y, width, height, screenHeight;

    screen = gtk_widget_get_screen(widget);
    gtk_window_get_position(GTK_WINDOW(widget), &x, &y);
    gtk_window_get_size(GTK_WINDOW(widget), &width, &height);
    screenHeight = gdk_screen_get_height(screen);
    gtk_window_move(GTK_WINDOW(widget), 0,
            y > screenHeight / 4 ? 0 : screenHeight - height);
}

int main (int argc, char *argv[])
{
    GtkWidget *window;
    GtkGrid *grid;
    GtkWidget *button;
    GdkScreen *screen;
    const struct VirtKey *const*row, *key;
    int rowNo, colNo, winHeight;

    gtk_init (&argc, &argv);
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    screen = gtk_widget_get_screen(window);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    winHeight = gdk_screen_get_height(screen) * 4 / 10;
    gtk_window_set_default_size(GTK_WINDOW(window), 
            gdk_screen_get_width(screen), winHeight);
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
    gtk_window_set_accept_focus(GTK_WINDOW(window), FALSE);
    gtk_window_move(GTK_WINDOW(window), 0,
            gdk_screen_get_height(screen) - winHeight);
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

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
                button = gtk_button_new_with_label (key->disp);
                g_signal_connect(button, "clicked", G_CALLBACK (on_click),
                        (void*)key);
                break;
            case VKT_SHIFT:
                gButtonShift = button =
                    gtk_toggle_button_new_with_label(key->disp);
                g_signal_connect_swapped(button, "toggled",
                        G_CALLBACK (on_toggle_shift), window);
                break;
            case VKT_CTRL:
                gButtonCtrl = button =
                    gtk_toggle_button_new_with_label(key->disp);
                break;
            case VKT_ALT:
                gButtonAlt = button =
                    gtk_toggle_button_new_with_label(key->disp);
                break;
            case VKT_DISMISS:
                button = gtk_button_new();
                gtk_button_set_image(GTK_BUTTON(button),
                        gtk_image_new_from_stock(GTK_STOCK_QUIT,
                        GTK_ICON_SIZE_BUTTON));
                g_signal_connect_swapped(button, "clicked",
                        G_CALLBACK(gtk_widget_destroy), window);
                break;
            case VKT_MOVE:
                button = gtk_button_new_with_label(key->disp);
                g_signal_connect_swapped(button, "clicked",
                        G_CALLBACK(on_move), window);
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
    gtk_widget_show(window);
    gtk_main ();
    return 0;
}

