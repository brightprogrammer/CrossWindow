/**
 * @file Window.h 
 * @time 04/04/2024 19:49:49
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) 2024 Siddharth Mishra
 * @copyright Copyright (c) 2024 Anvie Labs
 *
 * @brief Contains defintions of public api members of Window.h
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
    XW_WINDOW_STATE_MASK_UNKNOWN        = 0,
    XW_WINDOW_STATE_MASK_MODAL          = (1 << 0),
    XW_WINDOW_STATE_MASK_STICKY         = (1 << 1),
    XW_WINDOW_STATE_MASK_MAXIMIZED_VERT = (1 << 2),
    XW_WINDOW_STATE_MASK_MAXIMIZED_HORZ = (1 << 3),
    XW_WINDOW_STATE_MASK_MAXIMIZED =
        XW_WINDOW_STATE_MASK_MAXIMIZED_VERT | XW_WINDOW_STATE_MASK_MAXIMIZED_HORZ,
    XW_WINDOW_STATE_MASK_SHADED            = (1 << 4),
    XW_WINDOW_STATE_MASK_SKIP_TASKBAR      = (1 << 5),
    XW_WINDOW_STATE_MASK_SKIP_PAGER        = (1 << 6),
    XW_WINDOW_STATE_MASK_HIDDEN            = (1 << 7),
    XW_WINDOW_STATE_MASK_FULLSCREEN        = (1 << 8),
    XW_WINDOW_STATE_MASK_ABOVE             = (1 << 9),
    XW_WINDOW_STATE_MASK_BELOW             = (1 << 10),
    XW_WINDOW_STATE_MASK_DEMANDS_ATTENTION = (1 << 11),
    XW_WINDOW_STATE_MASK_FOCUSED           = (1 << 12)
} XwWindowStateMask;

/**
 * @b Actual window information but platform dependent.
 * 
 * Window creation process is different on each platform. This struct 
 * is defined by platform dependent code, and is completely opaque.
 * */
typedef struct XwWindowPlatformData XwWindowPlatformData;

/**
 * @b This struct derives from @c XwWindowPlatformData. Contains window 
 *    attributes like size, position, etc...
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

CString       xw_window_get_title (XwWindow *self);
XwWindowSize  xw_window_get_size (XwWindow *self);
XwWindowSize  xw_window_get_max_size (XwWindow *self);
XwWindowSize  xw_window_get_min_size (XwWindow *self);
XwWindowPos   xw_window_get_pos (XwWindow *self);
XwWindowState xw_window_get_state (XwWindow *self);

CString      xw_window_set_title (XwWindow *self, CString title);
XwWindowSize xw_window_set_size (XwWindow *self, XwWindowSize size);
XwWindowSize xw_window_set_max_size (XwWindow *self, XwWindowSize size);
XwWindowSize xw_window_set_min_size (XwWindow *self, XwWindowSize size);
XwWindowPos  xw_window_set_pos (XwWindow *self, XwWindowPos pos);
/* TODO : Does not work for XCB */
XwWindowState xw_window_set_state (XwWindow *self, XwWindowState state);

#endif // CROSSWINDOW_WINDOW_H
