/**
 * @file State.c 
 * @time 04/04/2024 20:31:26
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) 2024 Siddharth Mishra
 * @copyright Copyright (c) 2024 Anvie Labs
 *
 * Copyright 2024 Siddharth Mishra, Anvie Labs
 * 
 * Redistribution and use in source and binary forms, with or without modification, are permitted 
 * provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions
 *    and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 *    and the following disclaimer in the documentation and/or other materials provided with the
 *    distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse
 *    or promote products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * */

#include <Common.h>
#include <CrossWindow/Event.h>
#include <CrossWindow/Window.h>
#include <Types.h>

/* local includes */
#include "State.h"

/* libc includes */
#include <stdlib.h>
#include <string.h>

/* xcb/x11 includes */
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xproto.h>

/* for xk-definitions */
#define XK_MISCELLANY
#define XK_XKB_KEYS
#define XK_LATIN1
#define XK_LATIN2
#define XK_LATIN3
#define XK_LATIN4
#define XK_CYRILLIC
#define XK_GREEK
#define XK_ARMENIAN
#include <X11/keysymdef.h>

/* global state */
XwState xw_state = {0};

/* locally used errors */
#define CONNECTION_FAILED "Failed to create XCB connection\n"
#define SETUP_FAILED      "Failed to get XCB setup\n"
#define KEYBOARD_FAILED   "Failed to initialize keyboard\n"
#define REPLY_FAILED      "Failed to get reply from X11\n"

/* atom names */
#define WM_PROTOCOLS_ATOM_NAME     "WM_PROTOCOLS"
#define WM_DELETE_WINDOW_ATOM_NAME "WM_DELETE_WINDOW"
#define WM_STATE_ATOM_NAME         "WM_STATE"

#define _NET_WM_STATE_ATOM_NAME                   "_NET_WM_STATE"
#define _NET_WM_STATE_MODAL_ATOM_NAME             "_NET_WM_STATE_MODAL"
#define _NET_WM_STATE_STICKY_ATOM_NAME            "_NET_WM_STATE_STICKY"
#define _NET_WM_STATE_MAXIMIZED_VERT_ATOM_NAME    "_NET_WM_STATE_MAXIMIZED_VERT"
#define _NET_WM_STATE_MAXIMIZED_HORZ_ATOM_NAME    "_NET_WM_STATE_MAXIMIZED_HORZ"
#define _NET_WM_STATE_SHADED_ATOM_NAME            "_NET_WM_STATE_SHADED"
#define _NET_WM_STATE_SKIP_TASKBAR_ATOM_NAME      "_NET_WM_STATE_SKIP_TASKBAR"
#define _NET_WM_STATE_SKIP_PAGER_ATOM_NAME        "_NET_WM_STATE_SKIP_PAGER"
#define _NET_WM_STATE_HIDDEN_ATOM_NAME            "_NET_WM_STATE_HIDDEN"
#define _NET_WM_STATE_FULLSCREEN_ATOM_NAME        "_NET_WM_STATE_FULLSCREEN"
#define _NET_WM_STATE_ABOVE_ATOM_NAME             "_NET_WM_STATE_ABOVE"
#define _NET_WM_STATE_BELOW_ATOM_NAME             "_NET_WM_STATE_BELOW"
#define _NET_WM_STATE_DEMANDS_ATTENTION_ATOM_NAME "_NET_WM_STATE_DEMANDS_ATTENTION"
#define _NET_WM_STATE_FOCUSED_ATOM_NAME           "_NET_WM_STATE_FOCUSED"

#define _NET_WM_ALLOWED_ACTIONS_ATOM_NAME       "_NET_WM_ALLOWED_ACTIONS"
#define _NET_WM_ACTION_MOVE_ATOM_NAME           "_NET_WM_ACTION_MOVE"
#define _NET_WM_ACTION_RESIZE_ATOM_NAME         "_NET_WM_ACTION_RESIZE"
#define _NET_WM_ACTION_MINIMIZE_ATOM_NAME       "_NET_WM_ACTION_MINIMIZE"
#define _NET_WM_ACTION_SHADE_ATOM_NAME          "_NET_WM_ACTION_SHADE"
#define _NET_WM_ACTION_STICK_ATOM_NAME          "_NET_WM_ACTION_STICK"
#define _NET_WM_ACTION_MAXIMIZE_HORZ_ATOM_NAME  "_NET_WM_ACTION_MAXIMIZE_HORZ"
#define _NET_WM_ACTION_MAXIMIZE_VERT_ATOM_NAME  "_NET_WM_ACTION_MAXIMIZE_VERT"
#define _NET_WM_ACTION_FULLSCREEN_ATOM_NAME     "_NET_WM_ACTION_FULLSCREEN"
#define _NET_WM_ACTION_CHANGE_DESKTOP_ATOM_NAME "_NET_WM_ACTION_CHANGE_DESKTOP"
#define _NET_WM_ACTION_CLOSE_ATOM_NAME          "_NET_WM_ACTION_CLOSE"
#define _NET_WM_ACTION_ABOVE_ATOM_NAME          "_NET_WM_ACTION_ABOVE"
#define _NET_WM_ACTION_BELOW_ATOM_NAME          "_NET_WM_ACTION_BELOW"

#define _MOTIF_WM_HINTS_ATOM_NAME "_MOTIF_WM_HINTS"

#define _NET_WM_WINDOW_TYPE_ATOM_NAME         "_NET_WM_WINDOW_TYPE"
#define _NET_WM_WINDOW_TYPE_DESKTOP_ATOM_NAME "_NET_WM_WINDOW_TYPE_DESKTOP"
#define _NET_WM_WINDOW_TYPE_DOCK_ATOM_NAME    "_NET_WM_WINDOW_TYPE_DOCK"
#define _NET_WM_WINDOW_TYPE_TOOLBAR_ATOM_NAME "_NET_WM_WINDOW_TYPE_TOOLBAR"
#define _NET_WM_WINDOW_TYPE_MENU_ATOM_NAME    "_NET_WM_WINDOW_TYPE_MENU"
#define _NET_WM_WINDOW_TYPE_UTILITY_ATOM_NAME "_NET_WM_WINDOW_TYPE_UTILITY"
#define _NET_WM_WINDOW_TYPE_SPLASH_ATOM_NAME  "_NET_WM_WINDOW_TYPE_SPLASH"
#define _NET_WM_WINDOW_TYPE_DIALOG_ATOM_NAME  "_NET_WM_WINDOW_TYPE_DIALOG"
#define _NET_WM_WINDOW_TYPE_NORMAL_ATOM_NAME  "_NET_WM_WINDOW_TYPE_NORMAL"

static xcb_atom_t xw_get_xcb_atom (CString atom_name);

/**
 * @b Initialize CrossWindow.
 *
 * @return True if initialization is successful.
 * @return False otherwise.
 * */
CONSTRUCTOR Bool xw_init (void) {
    memset (&xw_state, 0, sizeof (xw_state));

    /* open a new connection to xcb */
    xcb_connection_t *conn = xcb_connect (
        Null /* displayname : Use the one provided in environment variable */,
        Null /* screenp     : Screen pointer */
    );
    RETURN_VALUE_IF (!conn, EXIT_FAILURE, CONNECTION_FAILED);

    /* get xcb setup to help us get screen iterator */
    const xcb_setup_t *setup = xcb_get_setup (conn);
    GOTO_HANDLER_IF (!setup, GET_SETUP_FAILED, SETUP_FAILED);

    /* get screen iterator to help us get screen */
    xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator (setup);

    /* update xw_state */
    xw_state.screen_iterator = screen_iter;
    xw_state.connection      = conn;

    /* get protocol atom */
    xw_state.WM_PROTOCOLS     = xw_get_xcb_atom (WM_PROTOCOLS_ATOM_NAME);
    xw_state.WM_DELETE_WINDOW = xw_get_xcb_atom (WM_DELETE_WINDOW_ATOM_NAME);

    /* get WM_STATE */
    xw_state.WM_STATE = xw_get_xcb_atom (WM_STATE_ATOM_NAME);

    /* get _NET_WM_STATE and it's possible values */
    xw_state._NET_WM_STATE        = xw_get_xcb_atom (_NET_WM_STATE_ATOM_NAME);
    xw_state._NET_WM_STATE_MODAL  = xw_get_xcb_atom (_NET_WM_STATE_MODAL_ATOM_NAME);
    xw_state._NET_WM_STATE_HIDDEN = xw_get_xcb_atom (_NET_WM_STATE_HIDDEN_ATOM_NAME);
    xw_state._NET_WM_STATE_MAXIMIZED_VERT =
        xw_get_xcb_atom (_NET_WM_STATE_MAXIMIZED_VERT_ATOM_NAME);
    xw_state._NET_WM_STATE_MAXIMIZED_HORZ =
        xw_get_xcb_atom (_NET_WM_STATE_MAXIMIZED_HORZ_ATOM_NAME);
    xw_state._NET_WM_STATE_SHADED       = xw_get_xcb_atom (_NET_WM_STATE_SHADED_ATOM_NAME);
    xw_state._NET_WM_STATE_SKIP_TASKBAR = xw_get_xcb_atom (_NET_WM_STATE_SKIP_TASKBAR_ATOM_NAME);
    xw_state._NET_WM_STATE_SKIP_PAGER   = xw_get_xcb_atom (_NET_WM_STATE_SKIP_PAGER_ATOM_NAME);
    xw_state._NET_WM_STATE_FULLSCREEN   = xw_get_xcb_atom (_NET_WM_STATE_FULLSCREEN_ATOM_NAME);
    xw_state._NET_WM_STATE_ABOVE        = xw_get_xcb_atom (_NET_WM_STATE_ABOVE_ATOM_NAME);
    xw_state._NET_WM_STATE_BELOW        = xw_get_xcb_atom (_NET_WM_STATE_BELOW_ATOM_NAME);
    xw_state._NET_WM_STATE_DEMANDS_ATTENTION =
        xw_get_xcb_atom (_NET_WM_STATE_DEMANDS_ATTENTION_ATOM_NAME);
    xw_state._NET_WM_STATE_FOCUSED = xw_get_xcb_atom (_NET_WM_STATE_FOCUSED_ATOM_NAME);

    /* get allowed action atoms */
    xw_state._NET_WM_ALLOWED_ACTIONS = xw_get_xcb_atom (_NET_WM_ALLOWED_ACTIONS_ATOM_NAME);
    xw_state._NET_WM_ACTION_MOVE     = xw_get_xcb_atom (_NET_WM_ACTION_MOVE_ATOM_NAME);
    xw_state._NET_WM_ACTION_RESIZE   = xw_get_xcb_atom (_NET_WM_ACTION_RESIZE_ATOM_NAME);
    xw_state._NET_WM_ACTION_MINIMIZE = xw_get_xcb_atom (_NET_WM_ACTION_MINIMIZE_ATOM_NAME);
    xw_state._NET_WM_ACTION_SHADE    = xw_get_xcb_atom (_NET_WM_ACTION_SHADE_ATOM_NAME);
    xw_state._NET_WM_ACTION_STICK    = xw_get_xcb_atom (_NET_WM_ACTION_STICK_ATOM_NAME);
    xw_state._NET_WM_ACTION_MAXIMIZE_HORZ =
        xw_get_xcb_atom (_NET_WM_ACTION_MAXIMIZE_HORZ_ATOM_NAME);
    xw_state._NET_WM_ACTION_MAXIMIZE_VERT =
        xw_get_xcb_atom (_NET_WM_ACTION_MAXIMIZE_VERT_ATOM_NAME);
    xw_state._NET_WM_ACTION_FULLSCREEN = xw_get_xcb_atom (_NET_WM_ACTION_FULLSCREEN_ATOM_NAME);
    xw_state._NET_WM_ACTION_CHANGE_DESKTOP =
        xw_get_xcb_atom (_NET_WM_ACTION_CHANGE_DESKTOP_ATOM_NAME);
    xw_state._NET_WM_ACTION_CLOSE = xw_get_xcb_atom (_NET_WM_ACTION_CLOSE_ATOM_NAME);
    xw_state._NET_WM_ACTION_ABOVE = xw_get_xcb_atom (_NET_WM_ACTION_ABOVE_ATOM_NAME);
    xw_state._NET_WM_ACTION_BELOW = xw_get_xcb_atom (_NET_WM_ACTION_BELOW_ATOM_NAME);

    xw_state._MOTIF_WM_HINTS = xw_get_xcb_atom (_MOTIF_WM_HINTS_ATOM_NAME);

    /* get atoms for setting windows state */
    xw_state._NET_WM_WINDOW_TYPE         = xw_get_xcb_atom (_NET_WM_WINDOW_TYPE_ATOM_NAME);
    xw_state._NET_WM_WINDOW_TYPE_DESKTOP = xw_get_xcb_atom (_NET_WM_WINDOW_TYPE_DESKTOP_ATOM_NAME);
    xw_state._NET_WM_WINDOW_TYPE_DOCK    = xw_get_xcb_atom (_NET_WM_WINDOW_TYPE_DOCK_ATOM_NAME);
    xw_state._NET_WM_WINDOW_TYPE_TOOLBAR = xw_get_xcb_atom (_NET_WM_WINDOW_TYPE_TOOLBAR_ATOM_NAME);
    xw_state._NET_WM_WINDOW_TYPE_MENU    = xw_get_xcb_atom (_NET_WM_WINDOW_TYPE_MENU_ATOM_NAME);
    xw_state._NET_WM_WINDOW_TYPE_UTILITY = xw_get_xcb_atom (_NET_WM_WINDOW_TYPE_UTILITY_ATOM_NAME);
    xw_state._NET_WM_WINDOW_TYPE_SPLASH  = xw_get_xcb_atom (_NET_WM_WINDOW_TYPE_SPLASH_ATOM_NAME);
    xw_state._NET_WM_WINDOW_TYPE_DIALOG  = xw_get_xcb_atom (_NET_WM_WINDOW_TYPE_DIALOG_ATOM_NAME);
    xw_state._NET_WM_WINDOW_TYPE_NORMAL  = xw_get_xcb_atom (_NET_WM_WINDOW_TYPE_NORMAL_ATOM_NAME);

    return True;

GET_SETUP_FAILED: {
    xcb_disconnect (conn);
    return False;
}
}

/**
 * @b Deinitialize globally initialized XwState object.
 *
 * @return True on success.
 * @return False otherwise.
 * */
DESTRUCTOR Bool xw_deinit (void) {
    if (xw_state.connection) {
        xcb_disconnect (xw_state.connection);
        xw_state.connection = Null;
    }

    if (xw_state.keyboard) {
        FREE (xw_state.keyboard);
        xw_state.keyboard      = Null;
        xw_state.keyboard_size = 0;
    }

    return True;
}

static struct {
    xcb_keysym_t keysym;
    XwKey        key;
} xwkeymap[] = {
    {          XK_a,             XWK_a},
    {          XK_A,             XWK_A},
    {          XK_b,             XWK_b},
    {          XK_B,             XWK_B},
    {          XK_c,             XWK_c},
    {          XK_C,             XWK_C},
    {          XK_d,             XWK_d},
    {          XK_D,             XWK_D},
    {          XK_e,             XWK_e},
    {          XK_E,             XWK_E},
    {          XK_f,             XWK_f},
    {          XK_F,             XWK_F},
    {          XK_g,             XWK_g},
    {          XK_G,             XWK_G},
    {          XK_h,             XWK_h},
    {          XK_H,             XWK_H},
    {          XK_i,             XWK_i},
    {          XK_I,             XWK_I},
    {          XK_j,             XWK_j},
    {          XK_J,             XWK_J},
    {          XK_k,             XWK_k},
    {          XK_K,             XWK_K},
    {          XK_l,             XWK_l},
    {          XK_L,             XWK_L},
    {          XK_m,             XWK_m},
    {          XK_M,             XWK_M},
    {          XK_n,             XWK_n},
    {          XK_N,             XWK_N},
    {          XK_o,             XWK_o},
    {          XK_O,             XWK_O},
    {          XK_p,             XWK_p},
    {          XK_P,             XWK_P},
    {          XK_q,             XWK_q},
    {          XK_Q,             XWK_Q},
    {          XK_r,             XWK_r},
    {          XK_R,             XWK_R},
    {          XK_s,             XWK_s},
    {          XK_S,             XWK_S},
    {          XK_t,             XWK_t},
    {          XK_T,             XWK_T},
    {          XK_u,             XWK_u},
    {          XK_U,             XWK_U},
    {          XK_v,             XWK_v},
    {          XK_V,             XWK_V},
    {          XK_w,             XWK_w},
    {          XK_W,             XWK_W},
    {          XK_x,             XWK_x},
    {          XK_X,             XWK_X},
    {          XK_y,             XWK_y},
    {          XK_Y,             XWK_Y},
    {          XK_z,             XWK_z},
    {          XK_Z,             XWK_Z},
    {          XK_0,             XWK_0},
    {          XK_1,             XWK_1},
    {          XK_2,             XWK_2},
    {          XK_3,             XWK_3},
    {          XK_4,             XWK_4},
    {          XK_5,             XWK_5},
    {          XK_6,             XWK_6},
    {          XK_7,             XWK_7},
    {          XK_8,             XWK_8},
    {          XK_9,             XWK_9},
    {       XK_KP_0,          XWK_NUM0},
    {       XK_KP_1,          XWK_NUM1},
    {       XK_KP_2,          XWK_NUM2},
    {       XK_KP_3,          XWK_NUM3},
    {       XK_KP_4,          XWK_NUM4},
    {       XK_KP_5,          XWK_NUM5},
    {       XK_KP_6,          XWK_NUM6},
    {       XK_KP_7,          XWK_NUM7},
    {       XK_KP_8,          XWK_NUM8},
    {       XK_KP_9,          XWK_NUM9},

    {         XK_F1,            XWK_F1},
    {         XK_F2,            XWK_F2},
    {         XK_F3,            XWK_F3},
    {         XK_F4,            XWK_F4},
    {         XK_F5,            XWK_F5},
    {         XK_F6,            XWK_F6},
    {         XK_F7,            XWK_F7},
    {         XK_F8,            XWK_F8},
    {         XK_F9,            XWK_F9},
    {        XK_F10,           XWK_F10},
    {        XK_F11,           XWK_F11},
    {        XK_F12,           XWK_F12},

    {     XK_Escape,        XWK_ESCAPE},
    {      XK_space,         XWK_SPACE},
    {     XK_exclam,   XWK_EXCLAMATION},
    {   XK_quotedbl, XWK_DOUBLE_QUOTES},
    { XK_apostrophe,  XWK_SINGLE_QUOTE},
    { XK_numbersign,          XWK_HASH},
    {     XK_dollar,      XWK_CURRENCY},
    {    XK_percent,       XWK_PERCENT},
    {  XK_ampersand,           XWK_AND},
    {   XK_asterisk,          XWK_STAR},
    {  XK_parenleft,        XWK_LPAREN},
    { XK_parenright,        XWK_RPAREN},
    {       XK_plus,           XWK_ADD},
    {      XK_comma,         XWK_COMMA},
    {      XK_minus,        XWK_HYPHEN},
    {     XK_period,        XWK_PERIOD},
    {      XK_slash,     XWK_FWD_SLASH},
    {  XK_backslash,    XWK_BACK_SLASH},

    {  XK_Control_L,      XWK_LCONTROL},
    {  XK_Control_R,      XWK_RCONTROL},
    {    XK_Shift_L,        XWK_LSHIFT},
    {    XK_Shift_R,        XWK_RSHIFT},
    {      XK_Alt_L,          XWK_LALT},
    {      XK_Alt_R,          XWK_RALT},
    {  XK_Caps_Lock,     XWK_CAPS_LOCK},
    {   XK_Num_Lock,      XWK_NUM_LOCK},
    {XK_Scroll_Lock,   XWK_SCROLL_LOCK},

    {         XK_Up,            XWK_UP},
    {       XK_Down,          XWK_DOWN},
    {       XK_Left,          XWK_LEFT},
    {      XK_Right,         XWK_RIGHT},
};

/**
 * @b Initialize keyboard in global XwState object.
 * */
Bool xw_init_keyboard (void) {
    const xcb_setup_t *setup = xcb_get_setup (xw_state.connection);
    GOTO_HANDLER_IF (!setup, GET_SETUP_FAILED, SETUP_FAILED);

    xcb_key_symbols_t *syms = xcb_key_symbols_alloc (xw_state.connection);
    RETURN_VALUE_IF (!syms, False, "Failed to allocate symbols\n");

    xw_state.keyboard_size = setup->max_keycode;
    xw_state.keyboard      = ALLOCATE (XwKey, setup->max_keycode);
    GOTO_HANDLER_IF (!xw_state.keyboard, ALLOC_FAILED, ERR_OUT_OF_MEMORY);

    Size num_keymap = ARRAY_SIZE (xwkeymap);

    for (xcb_keycode_t xk = setup->min_keycode; xk < setup->max_keycode; xk++) {
        xcb_keysym_t keysym = xcb_key_symbols_get_keysym (syms, xk, 0);

        XwKey key = XWK_UNKNOWN;
        for (Size s = 0; s < num_keymap; s++) {
            if (xwkeymap[s].keysym == keysym) {
                key = xwkeymap[s].key;
                break;
            }
        }

        xw_state.keyboard[xk] = key;
    }


    xcb_key_symbols_free (syms);

    return True;

ALLOC_FAILED:
GET_SETUP_FAILED: {
    xcb_disconnect (xw_state.connection);
    xw_state.connection = Null;
    return False;
}
}

/**
 * @b Goes through the windows array and finds an entry that is Null.
 *
 * CrossWindow ID helps in pairing events with the window they correspond to.
 * Once the window is destroyed, they set the corresponding entry in XwState to Null.
 * This way a new ID can be generated for the same position which was Nulled out before.
 *
 * @param win Window to generate new ID for.
 *
 * @return New window ID on success.
 * @return SIZE_MAX on failure.
 * */
Size xw_create_new_window_id (XwWindow *win) {
    RETURN_VALUE_IF (!win, SIZE_MAX, ERR_INVALID_ARGUMENTS);

    for (Size s = 0; s < ARRAY_SIZE (xw_state.windows); s++) {
        if (xw_state.windows[s] == Null) {
            xw_state.windows[s] = win;
            return s;
        }
    }

    return SIZE_MAX;
}

/**
 * @b Mark a window to be free for use for new windows.
 *
 * This is used only by Window.c when it's destroying or de-initing the window.
 *
 * @param window_id CrossWindow ID of window.
 * */
void xw_remove_window_id (Size window_id) {
    RETURN_IF (window_id >= ARRAY_SIZE (xw_state.windows), ERR_INVALID_ARGUMENTS);
    xw_state.windows[window_id] = Null;
}

/****************************** PRIVATE METHODS ************************************/

/**
 * @b Get atom with given name. If atom is not present then it will be created.
 * 
 * This method will call exit if returned atom is @c XCB_ATOM_NONE.
 *  
 * @param atom_name.
 *
 * @return xcb_atom_t
 * */
static xcb_atom_t xw_get_xcb_atom (CString atom_name) {
    /* make a request to create a new atom with given name */
    xcb_intern_atom_cookie_t cookie =
        xcb_intern_atom_unchecked (xw_state.connection, False, strlen (atom_name), atom_name);

    /* wait for reply */
    xcb_intern_atom_reply_t *reply = Null;
    reply                          = xcb_intern_atom_reply (xw_state.connection, cookie, Null);
    GOTO_HANDLER_IF (!reply, ATOM_REPLY_FAILED, "Atom reply failed (got Null)\n");

    /* set the atom and free reply */
    xcb_atom_t atom = reply->atom;
    GOTO_HANDLER_IF (atom == XCB_ATOM_NONE, ATOM_IS_NONE, "Retrieved atom is XCB_ATOM_NONE\n");

    /* free reply before exit */
    FREE (reply);
    return atom;

/* error handlers */
ATOM_IS_NONE:
    FREE (reply);
ATOM_REPLY_FAILED:
    exit (EXIT_FAILURE);
}
