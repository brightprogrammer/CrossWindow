/**
 * @file Window.h 
 * @time 04/04/2024 19:47:26
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) 2024 Siddharth Mishra
 * @copyright Copyright (c) 2024 Anvie Labs
 *
 * @brief Contains defintions of private api members of Window.h
 * */

#ifndef CROSSWINDOW_PRIVATE_WINDOW_H
#define CROSSWINDOW_PRIVATE_WINDOW_H

#include <CrossWindow/Window.h>
#include <xcb/xcb.h>

typedef struct XwWindow {
    /* platform specific data */
    xcb_window_t  xcb_window_id;
    xcb_screen_t *screen;

    /* platform independent data */
    Size          xw_id; /**< @b This is cross window id. Different from platform window id. */
    Uint32        border_width;
    XwWindowSize  size;
    XwWindowSize  min_size;
    XwWindowSize  max_size;
    XwWindowPos   pos;
    CString       title;
    CString       icon_path;
    XwWindowState state;

    Uint32 last_cursor_pos_x;
    Uint32 last_cursor_pos_y;
} XwWindow;

#endif // CROSSWINDOW_PRIVATE_WINDOW_H
