#ifndef KEYBOARD_LAYOUT_H
#define KEYBOARD_LAYOUT_H

enum VirtKeyType {
    VKT_NORMAL,
    VKT_SHIFT,
    VKT_CTRL,
    VKT_ALT,
    VKT_FNMAIN,     /* "Fn" key on main keyboard */
    VKT_FNFN,       /* "Fn" key on "Fn" keyboard */
    VKT_ALTPOS,     /* "Move to alternate position" key */
    VKT_DISMISS,
};

struct VirtKeyHandler {
    gboolean isToggleButton;
    GCallback callbackFunction;
    gpointer userData;
};

struct ModifierButtons {
    GtkWidget *shift, *ctrl, *alt;
};

GtkWidget *vkl_CreateMainKeyboard(
        const struct VirtKeyHandler*handlers, struct ModifierButtons*);
GtkWidget *vkl_CreateFnKeyboard(
        const struct VirtKeyHandler*handlers, struct ModifierButtons*);

guint vkl_GetKeyCodeFromUserData(gpointer user_data);

#endif /* KEYBOARD_LAYOUT_H */
