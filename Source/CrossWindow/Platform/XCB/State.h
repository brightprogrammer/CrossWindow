/**
 * @file State.h
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

#ifndef ANVIE_CROSSWINDOW_PLATFORM_XCB_STATE_H
#define ANVIE_CROSSWINDOW_PLATFORM_XCB_STATE_H

#include <Anvie/CrossWindow/Event.h>

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
    xcb_atom_t _NET_WM_STATE_MODAL;
    xcb_atom_t _NET_WM_STATE_STICKY;
    xcb_atom_t _NET_WM_STATE_MAXIMIZED_VERT;
    xcb_atom_t _NET_WM_STATE_MAXIMIZED_HORZ;
    xcb_atom_t _NET_WM_STATE_SHADED;
    xcb_atom_t _NET_WM_STATE_SKIP_TASKBAR;
    xcb_atom_t _NET_WM_STATE_SKIP_PAGER;
    xcb_atom_t _NET_WM_STATE_HIDDEN;
    xcb_atom_t _NET_WM_STATE_FULLSCREEN;
    xcb_atom_t _NET_WM_STATE_ABOVE;
    xcb_atom_t _NET_WM_STATE_BELOW;
    xcb_atom_t _NET_WM_STATE_DEMANDS_ATTENTION;
    xcb_atom_t _NET_WM_STATE_FOCUSED;

#define _NET_WM_STATE_REMOVE 0 /* remove/unset property */
#define _NET_WM_STATE_ADD    1 /* add/set property */
#define _NET_WM_STATE_TOGGLE 2 /* toggle property  */

    /**< @b _NET_WM_ALLOWED_ACTIONS : https://specifications.freedesktop.org/wm-spec/1.4/ar01s05.html */
    xcb_atom_t _NET_WM_ALLOWED_ACTIONS;
    xcb_atom_t _NET_WM_ACTION_MOVE;
    xcb_atom_t _NET_WM_ACTION_RESIZE;
    xcb_atom_t _NET_WM_ACTION_MINIMIZE;
    xcb_atom_t _NET_WM_ACTION_SHADE;
    xcb_atom_t _NET_WM_ACTION_STICK;
    xcb_atom_t _NET_WM_ACTION_MAXIMIZE_HORZ;
    xcb_atom_t _NET_WM_ACTION_MAXIMIZE_VERT;
    xcb_atom_t _NET_WM_ACTION_FULLSCREEN;
    xcb_atom_t _NET_WM_ACTION_CHANGE_DESKTOP;
    xcb_atom_t _NET_WM_ACTION_CLOSE;
    xcb_atom_t _NET_WM_ACTION_ABOVE;
    xcb_atom_t _NET_WM_ACTION_BELOW;

    /* to remove window borders */
    xcb_atom_t _MOTIF_WM_HINTS;

    /* atoms to manage window type 
     * https://specifications.freedesktop.org/wm-spec/wm-spec-1.3.html#idm45821695774192 */
    xcb_atom_t _NET_WM_WINDOW_TYPE;
    xcb_atom_t _NET_WM_WINDOW_TYPE_DESKTOP;
    xcb_atom_t _NET_WM_WINDOW_TYPE_DOCK;
    xcb_atom_t _NET_WM_WINDOW_TYPE_TOOLBAR;
    xcb_atom_t _NET_WM_WINDOW_TYPE_MENU;
    xcb_atom_t _NET_WM_WINDOW_TYPE_UTILITY;
    xcb_atom_t _NET_WM_WINDOW_TYPE_SPLASH;
    xcb_atom_t _NET_WM_WINDOW_TYPE_DIALOG;
    xcb_atom_t _NET_WM_WINDOW_TYPE_NORMAL;

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

#endif // ANVIE_CROSSWINDOW_PLATFORM_XCB_STATE_H
