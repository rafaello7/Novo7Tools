#ifndef KEYBOARD_LAYOUT_H
#define KEYBOARD_LAYOUT_H

enum VirtKeyType {
    VKT_NORMAL,
    VKT_SHIFT,
    VKT_CTRL,
    VKT_ALT,
    VKT_RIGHTALT,
    VKT_FNMAIN,     /* "Fn" key on main keyboard */
    VKT_FNFN,       /* "Fn" key on "Fn" keyboard */
    VKT_ALTPOS,     /* "Move to alternate position" key */
    VKT_DISMISS,
};

struct VirtKeyHandler {
    gboolean isToggleButton;
    GCallback btnPressCallback;
    GCallback btnReleaseCallback;
    GCallback clickCallback;
    gpointer userData;
};

struct ModifierButtons {
    GtkWidget *shift, *ctrl, *alt, *rightalt;
};

GtkWidget *vkl_CreateMainKeyboard(
        const struct VirtKeyHandler*handlers, struct ModifierButtons*);
GtkWidget *vkl_CreateFnKeyboard(
        const struct VirtKeyHandler*handlers, struct ModifierButtons*);

guint vkl_GetKeyCodeFromUserData(gpointer user_data);
guint *vkl_GetComposeFromUserData(gpointer user_data);
void vkl_SetComposeKeys(const char *cmdLineParam);

#endif /* KEYBOARD_LAYOUT_H */
