#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/extensions/XTest.h>
#include "cmdline.h"
#include "keyboard_layout.h"


/* Toggle buttons of modifier keys
 */
static struct ModifierButtons modifiersMain, modifiersFn;
static struct ModifierButtons *modifiers = &modifiersMain;
static gboolean gIsShiftLocked, gIsCtrlLocked;

/* Saved alternate position of main window on screen.
 */
static guint gWinAltX, gWinAltY;


/* "clicked" event handler of VKT_NORMAL button
 */
static void on_click(GtkWidget *widget, gpointer user_data)
{
    Display *dp;
    guint kcode;

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
    kcode = vkl_GetKeyCodeFromUserData(user_data);
    XTestFakeKeyEvent(dp, kcode, TRUE, CurrentTime);
    XTestFakeKeyEvent(dp, kcode, FALSE, CurrentTime);
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(modifiers->alt)) ) {
        XTestFakeKeyEvent(dp, 64, FALSE, CurrentTime);
    }
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(modifiers->ctrl)) ) {
        XTestFakeKeyEvent(dp, 37, FALSE, CurrentTime);
    }
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(modifiers->shift)) ) {
        XTestFakeKeyEvent(dp, 50, FALSE, CurrentTime);
    }
    if( ! gIsShiftLocked ) {
        gtk_toggle_button_set_active(
                GTK_TOGGLE_BUTTON(modifiers->shift), FALSE);
    }
    if( ! gIsCtrlLocked ) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(modifiers->ctrl), FALSE);
    }
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(modifiers->alt), FALSE);
}

static void ReplicateModifiers(const struct ModifierButtons *to,
        const struct ModifierButtons *from)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(to->shift),
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(from->shift)));
    gtk_button_set_label(GTK_BUTTON(to->shift),
            gtk_button_get_label(GTK_BUTTON(from->shift)));
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

/* "clicked" event handler of VKT_ALTPOS button
 */
static void on_click_altpos(GtkWidget *button, gpointer data)
{
    GtkWidget *window = data;
    gint x, y;

    gtk_window_get_position(GTK_WINDOW(window), &x, &y);
    gtk_window_move(GTK_WINDOW(window), gWinAltX, gWinAltY);
    gWinAltX = x;
    gWinAltY = y;
}

/* "clicked" event handler of VKT_DISMISS button
 */
static void on_click_dismiss(GtkWidget *button, gpointer user_data)
{
    GtkWidget *window = user_data;

    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(modifiers->alt)) ) {
        gtk_widget_destroy(window);
    }else{
        gtk_window_iconify(GTK_WINDOW(window));
    }
}

static gboolean on_release_shift(GtkWidget *button, GdkEvent *event,
        gpointer user_data)
{
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)) ) {
        const char *label = "Shift";
        if( (gIsShiftLocked = !gIsShiftLocked) ) {
            label = "Shift Lock";
        }
        gtk_button_set_label(GTK_BUTTON(button), label);
    }
    return gIsShiftLocked;
}

static gboolean on_release_ctrl(GtkWidget *button, GdkEvent *event,
        gpointer user_data)
{
    if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)) ) {
        const char *label = "Ctrl";
        if( (gIsCtrlLocked = !gIsCtrlLocked) ) {
            label = "Ctrl Lock";
        }
        gtk_button_set_label(GTK_BUTTON(button), label);
    }
    return gIsCtrlLocked;
}

static void CreateKeyboardOnNotebook(GtkWidget *window, GtkWidget *notebook)
{
    struct VirtKeyHandler handlers[] = {
        [VKT_NORMAL] =  { .isToggleButton = FALSE,
                          .callbackFunction = G_CALLBACK(on_click) },
        [VKT_SHIFT] =   { .isToggleButton = TRUE },
        [VKT_CTRL] =    { .isToggleButton = TRUE },
        [VKT_ALT] =     { .isToggleButton = TRUE },
        [VKT_FNMAIN] = {
            .isToggleButton = FALSE,
            .callbackFunction = G_CALLBACK(on_click_fnmain),
            .userData = notebook
        },
        [VKT_FNFN] = {
            .isToggleButton = FALSE,
            .callbackFunction = G_CALLBACK(on_click_fnfn),
            .userData = notebook
        },
        [VKT_ALTPOS] = {
            .isToggleButton = FALSE,
            .callbackFunction = G_CALLBACK(on_click_altpos),
            .userData = window
        },
        [VKT_DISMISS] = {
            .isToggleButton = FALSE,
            .callbackFunction = G_CALLBACK(on_click_dismiss),
            .userData = window
        }
    };

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
            vkl_CreateMainKeyboard(handlers, &modifiersMain), NULL);
    g_signal_connect(modifiersMain.shift, "button-release-event",
            G_CALLBACK(on_release_shift), NULL);
    g_signal_connect(modifiersMain.ctrl, "button-release-event",
            G_CALLBACK(on_release_ctrl), NULL);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
            vkl_CreateFnKeyboard(handlers, &modifiersFn), NULL);
    g_signal_connect(modifiersFn.shift, "button-release-event",
            G_CALLBACK(on_release_shift), NULL);
    g_signal_connect(modifiersFn.ctrl, "button-release-event",
            G_CALLBACK(on_release_ctrl), NULL);
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

static void InitMainWindow(struct CmdLineOptions *opts, GtkApplication *app)
{
    GtkWidget *window, *notebook;
    GdkScreen *screen;
    guint screenWidth, screenHeight;
    gint winX, winY, winWidth, winHeight;

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
    winX = CalcOptionValue(opts->x, opts->isXNeg, opts->isXPercent,
            screenWidth, winWidth);
    winY = CalcOptionValue(opts->y, opts->isYNeg, opts->isYPercent,
            screenHeight, winHeight);
    gWinAltX = CalcOptionValue(opts->xalt, opts->isXAltNeg, opts->isXAltPercent,
            screenWidth, winWidth);
    gWinAltY = CalcOptionValue(opts->yalt, opts->isYAltNeg, opts->isYAltPercent,
            screenHeight, winHeight);
    gtk_window_set_default_size(GTK_WINDOW(window), 
            winWidth, winHeight);
    gtk_window_move(GTK_WINDOW(window), winX, winY);

    notebook = gtk_notebook_new();
    gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
    CreateKeyboardOnNotebook(window, notebook);
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
            on_click_dismiss(NULL, list->data);
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

