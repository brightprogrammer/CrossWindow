/**
 * @file State.h
 * @time 04/04/2024 19:49:49
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) 2024 Siddharth Mishra
 * @copyright Copyright (c) 2024 Anvie Labs
 *
 * @brief Contains platform dependent global state of our connection with 
 *        X11 server.
 * */

#ifndef CROSSWINDOW_PLATFORM_XCB_STATE_H
#define CROSSWINDOW_PLATFORM_XCB_STATE_H

#include <CrossWindow/Event.h>

/* xcb related headers */
#include <xcb/xcb.h>

#define ERR_XW_STATE_NOT_INITIALIZED                                                               \
    "It looks like CrossWindow is not yet initialized. Please call xw_init() before using "        \
    "CrossWindow\n"

typedef struct XwState {
    xcb_connection_t     *connection;
    Size                  keyboard_size; /**< @b Number of entries in @c keyboard array */
    XwKey                *keyboard;      /**< @b Mapping of keysyms to @c XwKey */
    xcb_screen_iterator_t screen_iterator;

    /** @b Atom to be used to set other created atoms. */
    xcb_atom_t WM_PROTOCOLS;
    /** @b Atom we receive in client message events to recognize for close window events. */
    xcb_atom_t WM_DELETE_WINDOW;

    /**< @b WM_STATE : https://x.org/releases/X11R7.6/doc/xorg-docs/specs/ICCCM/icccm.html */
    xcb_atom_t WM_STATE;

    /**< @b _NET_WM_STATE : https://specifications.freedesktop.org/wm-spec/latest/ar01s05.html#idm46025198457920 */
    xcb_atom_t _NET_WM_STATE;
    xcb_atom_t _NET_WM_STATE_MODAL;          /**< @b One of the possible values of _NET_WM_STATE */
    xcb_atom_t _NET_WM_STATE_STICKY;         /**< @b One of the possible values of _NET_WM_STATE */
    xcb_atom_t _NET_WM_STATE_MAXIMIZED_VERT; /**< @b One of the possible values of _NET_WM_STATE */
    xcb_atom_t _NET_WM_STATE_MAXIMIZED_HORZ; /**< @b One of the possible values of _NET_WM_STATE */
    xcb_atom_t _NET_WM_STATE_SHADED;         /**< @b One of the possible values of _NET_WM_STATE */
    xcb_atom_t _NET_WM_STATE_SKIP_TASKBAR;   /**< @b One of the possible values of _NET_WM_STATE */
    xcb_atom_t _NET_WM_STATE_SKIP_PAGER;     /**< @b One of the possible values of _NET_WM_STATE */
    xcb_atom_t _NET_WM_STATE_HIDDEN;         /**< @b One of the possible values of _NET_WM_STATE */
    xcb_atom_t _NET_WM_STATE_FULLSCREEN;     /**< @b One of the possible values of _NET_WM_STATE */
    xcb_atom_t _NET_WM_STATE_ABOVE;          /**< @b One of the possible values of _NET_WM_STATE */
    xcb_atom_t _NET_WM_STATE_BELOW;          /**< @b One of the possible values of _NET_WM_STATE */
    xcb_atom_t
        _NET_WM_STATE_DEMANDS_ATTENTION;     /**< @b One of the possible values of _NET_WM_STATE */
    xcb_atom_t _NET_WM_STATE_FOCUSED;        /**< @b One of the possible values of _NET_WM_STATE */

#define _NET_WM_STATE_REMOVE 0               /* remove/unset property */
#define _NET_WM_STATE_ADD    1               /* add/set property */
#define _NET_WM_STATE_TOGGLE 2               /* toggle property  */

    /** 
     * @b A mapping from CrossWindow Id to the window itself.
     * This is used by event poll and wait methods to get the window for which
     * the xcb event was generated.
     * */
    struct XwWindow *windows[64];
    Size             window_count;
} XwState;

Bool xw_init_keyboard (void);
Size xw_create_new_window_id (struct XwWindow *win);
void xw_remove_window_id (Size window_id);

#endif // CROSSWINDOW_PLATFORM_XCB_STATE_H
