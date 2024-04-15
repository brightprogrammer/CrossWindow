/**
 * @file Window.h
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

#ifndef CROSSWINDOW_WINDOW_H
#define CROSSWINDOW_WINDOW_H

#include <Types.h>

typedef struct XwWindowSize {
    Uint32 width;
    Uint32 height;
} XwWindowSize;

typedef struct XwWindowPos {
    Uint32 x;
    Uint32 y;
} XwWindowPos;

/**
 * @b Window state made up of @c XwWindowStateMask
 *
 * @sa XwWindowStateMask
 * */
typedef Uint16 XwWindowState;

/**
 * @b Closely follows this : https://specifications.freedesktop.org/wm-spec/latest/ar01s05.html#idm46025198457920
 * */
typedef enum XwWindowStateMask : XwWindowState {
    XW_WINDOW_STATE_MASK_CLEAR          = 0,            /* state is empty, nothing is set */
    XW_WINDOW_STATE_MASK_MODAL          = (1 << 0),     /* make modal */
    XW_WINDOW_STATE_MASK_STICKY         = (1 << 1),     /* make it stick to top */
    XW_WINDOW_STATE_MASK_MAXIMIZED_VERT = (1 << 2),     /* maximize it vertically */
    XW_WINDOW_STATE_MASK_MAXIMIZED_HORZ = (1 << 3),     /* maximize it horizontally */
    XW_WINDOW_STATE_MASK_MAXIMIZED =
        XW_WINDOW_STATE_MASK_MAXIMIZED_VERT | XW_WINDOW_STATE_MASK_MAXIMIZED_HORZ,
    XW_WINDOW_STATE_MASK_SHADED            = (1 << 4),  /* shade (just show titlebar) */
    XW_WINDOW_STATE_MASK_SKIP_TASKBAR      = (1 << 5),  /* don't show on tasbar? */
    XW_WINDOW_STATE_MASK_SKIP_PAGER        = (1 << 6),  /* don't show in ALT+TAB? */
    XW_WINDOW_STATE_MASK_HIDDEN            = (1 << 7),  /* set window in hidden state */
    XW_WINDOW_STATE_MASK_FULLSCREEN        = (1 << 8),  /* make fullscreen */
    XW_WINDOW_STATE_MASK_ABOVE             = (1 << 9),  /* reorder to above */
    XW_WINDOW_STATE_MASK_BELOW             = (1 << 10), /* reorder to below */
    XW_WINDOW_STATE_MASK_DEMANDS_ATTENTION = (1 << 11), /* needs attention (blinking window) */
    XW_WINDOW_STATE_MASK_FOCUSED           = (1 << 12)  /* set focused */
} XwWindowStateMask;

/**
 * @b Actual window information but platform dependent.
 * 
 * Window creation process is different on each platform. This struct 
 * is defined by platform dependent code, and is completely opaque.
 * */
typedef struct XwWindowPlatformData XwWindowPlatformData;

/**
 * @b Made from bitwise OR of following action permission masks.
 *
 * This indicates what actions are allowed on a window. If a flag is set,
 * then that action is allowed, if a flag is unset then it isn't allowed
 * */
typedef Uint16 XwWindowActionPermissions;

/**
 * @b Each flag denotes an allowed action permission to window.
 * */
typedef enum XwWindowActionPermissionMask : XwWindowActionPermissions {
    XW_WINDOW_ACTION_PERMISSION_MASK_CLEAR         = 0,          /* no actions allowed! */
    XW_WINDOW_ACTION_PERMISSION_MASK_MOVE          = (1 << 0),   /* is movable */
    XW_WINDOW_ACTION_PERMISSION_MASK_RESIZE        = (1 << 1),   /* is resizable */
    XW_WINDOW_ACTION_PERMISSION_MASK_MINIMIZE      = (1 << 2),   /* is minimizable */
    XW_WINDOW_ACTION_PERMISSION_MASK_SHADE         = (1 << 3),   /* can be shaded */
    XW_WINDOW_ACTION_PERMISSION_MASK_STICK         = (1 << 4),   /* can be sticky to top */
    XW_WINDOW_ACTION_PERMISSION_MASK_MAXIMIZE_HORZ = (1 << 5),   /* can be maximized horizontally */
    XW_WINDOW_ACTION_PERMISSION_MASK_MAXIMIZE_VERT = (1 << 6),   /* can be maximized vertically */
    XW_WINDOW_ACTION_PERMISSION_MASK_MAXIMIZE = XW_WINDOW_ACTION_PERMISSION_MASK_MAXIMIZE_VERT |
                                                XW_WINDOW_ACTION_PERMISSION_MASK_MAXIMIZE_HORZ,
    XW_WINDOW_ACTION_PERMISSION_MASK_FULLSCREEN     = (1 << 7),  /* can go fullscreen */
    XW_WINDOW_ACTION_PERMISSION_MASK_CHANGE_DESKTOP = (1 << 8),  /* can change desktop */
    XW_WINDOW_ACTION_PERMISSION_MASK_CLOSE          = (1 << 9),  /* is closable */
    XW_WINDOW_ACTION_PERMISSION_MASK_ABOVE          = (1 << 10), /* can reorder to above */
    XW_WINDOW_ACTION_PERMISSION_MASK_BELOW          = (1 << 11), /* can reorder to below */
} XwWindowActionMask;

/**
 * @b Platform dependent opaque window.
 *
 * The struct is opaque because access to it's members must be controlled.
 * One cannot just alter the window data, as this will result in loss of 
 * data synchronicity between the platform window data and data stored inside
 * this struct.
 *
 * The complete definition of struct is present in Private/Window.h but is not
 * recommended to be accessed by user-code.
 * */
typedef struct XwWindow XwWindow;

XwWindow *xw_window_create (CString title, Uint32 width, Uint32 height, Uint32 xpos, Uint32 ypos);
XwWindow *xw_window_init (
    XwWindow *self,
    CString   title,
    Uint32    width,
    Uint32    height,
    Uint32    xpos,
    Uint32    ypos
);
XwWindow *xw_window_deinit (XwWindow *self);
void      xw_window_destroy (XwWindow *self);

XwWindow *xw_window_show (XwWindow *self);
XwWindow *xw_window_hide (XwWindow *self);

CString                   xw_window_get_title (XwWindow *self);
XwWindowSize              xw_window_get_size (XwWindow *self);
XwWindowSize              xw_window_get_max_size (XwWindow *self);
XwWindowSize              xw_window_get_min_size (XwWindow *self);
XwWindowPos               xw_window_get_pos (XwWindow *self);
XwWindowState             xw_window_get_state (XwWindow *self);
XwWindowActionPermissions xw_window_get_action_permissions (XwWindow *self);

CString      xw_window_set_title (XwWindow *self, CString title);
XwWindowSize xw_window_set_size (XwWindow *self, XwWindowSize size);
XwWindowSize xw_window_set_max_size (XwWindow *self, XwWindowSize size);
XwWindowSize xw_window_set_min_size (XwWindow *self, XwWindowSize size);
XwWindowPos  xw_window_set_pos (XwWindow *self, XwWindowPos pos);
/* TODO : Does not work for XCB */
XwWindowState xw_window_set_state (XwWindow *self, XwWindowState state);
XwWindowActionPermissions
          xw_window_set_action_permissions (XwWindow *self, XwWindowActionPermissions permissions);
XwWindow *xw_window_set_bordered (XwWindow *self, Bool border);

#endif // CROSSWINDOW_WINDOW_H
