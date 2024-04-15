/**
 * @file Event.c 
 * @time 04/04/2024 20:31:26
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) 2024 Siddharth Mishra
 * @copyright Copyright (c) 2024 Anvie Labs
 *
 * @brief Contains XwEvent data structure for event handling in CrossWindow. 
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

/* local headers */
#include "State.h"
#include "Window.h"

/* libc headers */
#include <string.h>

/* x11/xcb headers */
#include <xcb/xcb_keysyms.h>
#include <xcb/xproto.h>

#define ERR_WINDOW_SEARCH_FAILED "Failed to find window associated with event\n"

extern XwState xw_state;

static XwKey    xw_key_from_xcb_keycode (xcb_keycode_t detail);
static XwEvent *xw_fill_event (XwEvent *eq, const xcb_generic_event_t *event);
static Size get_xcb_atom_property_atom (xcb_atom_t atom, xcb_window_t window, xcb_atom_t **values);

/* defined in Window.c */
extern XwWindow *xw_get_window_by_xcb_id (xcb_window_t xcb_win_id);

XwEvent *xw_event_poll (XwEvent *e) {
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_ARGUMENTS);

    /* make sure all pending operations are done */
    xcb_flush (xw_state.connection);

    /* poll event and fill the given event object */
    xcb_generic_event_t *xcb_event = xcb_poll_for_event (xw_state.connection);
    if (!xcb_event) {
        return Null;
    } else {
        xw_fill_event (e, xcb_event);
        FREE (xcb_event);
        return e;
    }
}

XwEvent *xw_event_wait (XwEvent *e) {
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_ARGUMENTS);

    /* make sure all pending operations are done */
    xcb_flush (xw_state.connection);

    /* poll event and fill the given event object */
    xcb_generic_event_t *xcb_event = xcb_wait_for_event (xw_state.connection);
    xw_fill_event (e, xcb_event);

    return e;
}

/**
 * @b Fill given @c XwEvent object by converting data in a @c xcb_generic_event_t to
 *    equivalent data.
 *
 * REF : https://tronche.com/gui/x/xlib/events/types.html
 * */
static XwEvent *xw_fill_event (XwEvent *e, const xcb_generic_event_t *xcb_event) {
    RETURN_VALUE_IF (!e || !xcb_event, Null, ERR_INVALID_ARGUMENTS);

    Uint8 event_code = xcb_event->response_type & 0x7f;

    /* set default even type in case the event goes un-detected */
    e->type = XW_EVENT_TYPE_NONE;

    switch (event_code) {
        /* Generated when a window is mapped onto screen. 
         * REF : https://tronche.com/gui/x/xlib/events/window-state-change/map.html
         * */
        case XCB_MAP_NOTIFY : {
            xcb_map_notify_event_t *notify = (xcb_map_notify_event_t *)xcb_event;

            /* find window associated with given event */
            XwWindow *window = xw_get_window_by_xcb_id (notify->window);
            GOTO_HANDLER_IF (!window, WINDOW_SEARCH_FAILED, ERR_WINDOW_SEARCH_FAILED);

            e = xw_event_visibility (e, True, window);

            break;
        }

        /* Generated when a window is unmapped onto screen. 
         * REF : https://tronche.com/gui/x/xlib/events/window-state-change/map.html
         * */
        case XCB_UNMAP_NOTIFY : {
            xcb_unmap_notify_event_t *notify = (xcb_unmap_notify_event_t *)xcb_event;

            /* find window associated with given event */
            XwWindow *window = xw_get_window_by_xcb_id (notify->window);
            GOTO_HANDLER_IF (!window, WINDOW_SEARCH_FAILED, ERR_WINDOW_SEARCH_FAILED);

            e = xw_event_visibility (e, False, window);

            break;
        }

        /* REF : https://tronche.com/gui/x/xlib/events/input-focus/ */
        case XCB_FOCUS_IN : {
            xcb_focus_in_event_t *fin = (xcb_focus_in_event_t *)xcb_event;

            /* find window associated with given event */
            XwWindow *window = xw_get_window_by_xcb_id (fin->event);
            GOTO_HANDLER_IF (!window, WINDOW_SEARCH_FAILED, ERR_WINDOW_SEARCH_FAILED);

            e = xw_event_focus (e, True, window);

            break;
        }

        /* REF : https://tronche.com/gui/x/xlib/events/input-focus/ */
        case XCB_FOCUS_OUT : {
            xcb_focus_out_event_t *fout = (xcb_focus_out_event_t *)xcb_event;

            /* find window associated with given event */
            XwWindow *window = xw_get_window_by_xcb_id (fout->event);
            GOTO_HANDLER_IF (!window, WINDOW_SEARCH_FAILED, ERR_WINDOW_SEARCH_FAILED);

            e = xw_event_focus (e, False, window);

            break;
        }

        /* Accounts for window state changes like window size, position, stack order, 
         * or border width.
         *
         * REF : https://tronche.com/gui/x/xlib/events/window-state-change/configure.html
         * */
        case XCB_CONFIGURE_NOTIFY : {
            xcb_configure_notify_event_t *notify = (xcb_configure_notify_event_t *)xcb_event;

            /* find window associated with given event */
            XwWindow *window = xw_get_window_by_xcb_id (notify->window);
            GOTO_HANDLER_IF (!window, WINDOW_SEARCH_FAILED, ERR_WINDOW_SEARCH_FAILED);

            /* I'm assuming here that not all are possible at once! */
            if (notify->width != window->size.width || notify->height != window->size.height) {
                window->size = (XwWindowSize) {notify->width, notify->height};
                e            = xw_event_resize (e, notify->width, notify->height, window);
            } else if ((Uint32)notify->x != window->pos.x || (Uint32)notify->y != window->pos.y) {
                window->pos = (XwWindowPos) {notify->x, notify->y};
                e           = xw_event_reposition (e, notify->x, notify->y, window);
            } else if (notify->border_width != window->border_width) {
                window->border_width = notify->border_width;
                e = xw_event_border_width_change (e, notify->border_width, window);
            } else if (notify->above_sibling != XCB_WINDOW_NONE) {
                /* search for sibling window */
                XwWindow *above_sibling = xw_get_window_by_xcb_id (notify->above_sibling);
                GOTO_HANDLER_IF (!above_sibling, WINDOW_SEARCH_FAILED, ERR_WINDOW_SEARCH_FAILED);

                e = xw_event_restack (e, above_sibling, window);
            }

            break;
        }

        /* Generated when some part/region of window got damaged. The event contains info about
         * the (x, y) and (width, height) of region of window that got damaged and just got exposed,
         * that we need to redraw.
         *
         * CrossWindow will treat this as if whole window needs to be redrawn.
         * REF : https://tronche.com/gui/x/xlib/events/exposure/expose.html
         * */
        case XCB_EXPOSE : {
            xcb_expose_event_t *expose = (xcb_expose_event_t *)xcb_event;

            /* find window associated with this event */
            XwWindow *window = xw_get_window_by_xcb_id (expose->window);
            GOTO_HANDLER_IF (!window, WINDOW_SEARCH_FAILED, ERR_WINDOW_SEARCH_FAILED);

            e = xw_event_paint (e, window);
            break;
        }

        /* Someone attempted to resize a window by calling @c xcb_configure_window or some other
         * related methods.
         *
         * REF : https://tronche.com/gui/x/xlib/events/structure-control/resize.html
         * */
        case XCB_RESIZE_REQUEST : {
            xcb_resize_request_event_t *resize = (xcb_resize_request_event_t *)xcb_event;

            /* find window associated with this event */
            XwWindow *window = xw_get_window_by_xcb_id (resize->window);
            GOTO_HANDLER_IF (!window, WINDOW_SEARCH_FAILED, ERR_WINDOW_SEARCH_FAILED);

            e = xw_event_resize (e, resize->width, resize->height, window);
            break;
        }

        /* When pointer motion begins in some other window but ends up getting into another window.
         *
         * REF : https://tronche.com/gui/x/xlib/events/window-entry-exit/
         * */
        case XCB_ENTER_NOTIFY : {
            xcb_enter_notify_event_t *enter = (xcb_enter_notify_event_t *)xcb_event;

            /* find window associated with this event */
            XwWindow *window = xw_get_window_by_xcb_id (enter->event);
            GOTO_HANDLER_IF (!window, WINDOW_SEARCH_FAILED, ERR_WINDOW_SEARCH_FAILED);

            e = xw_event_enter (e, enter->event_x, enter->event_y, window);
            break;
        }

        /* When pointer motion begins in this window but ends up getting into another window.
         *
         * REF : https://tronche.com/gui/x/xlib/events/window-entry-exit/
         * */
        case XCB_LEAVE_NOTIFY : {
            xcb_leave_notify_event_t *leave = (xcb_leave_notify_event_t *)xcb_event;

            /* find window associated with this event */
            XwWindow *window = xw_get_window_by_xcb_id (leave->event);
            GOTO_HANDLER_IF (!window, WINDOW_SEARCH_FAILED, ERR_WINDOW_SEARCH_FAILED);

            e = xw_event_leave (e, leave->event_x, leave->event_y, window);
            break;
        }

        /* Some X11 window client issued a @c xcb_send_event or equivalent.
         *
         * REF : https://tronche.com/gui/x/xlib/events/client-communication/client-message.html 
         * */
        case XCB_CLIENT_MESSAGE : {
            xcb_client_message_event_t *msg = (xcb_client_message_event_t *)xcb_event;

            /* find window associated with this event */
            XwWindow *window = xw_get_window_by_xcb_id (msg->window);
            GOTO_HANDLER_IF (!window, WINDOW_SEARCH_FAILED, ERR_WINDOW_SEARCH_FAILED);

            if (msg->type == xw_state.WM_PROTOCOLS && msg->format == 32) {
                xcb_atom_t protocol = msg->data.data32[0];
                if (protocol == xw_state.WM_DELETE_WINDOW) {
                    e = xw_event_close_window (e, window);
                }
            }
            break;
        }

        /* Some property of window changed.
         *
         * REF : https://tronche.com/gui/x/xlib/events/client-communication/property.html
         * */
        case XCB_PROPERTY_NOTIFY : {
            xcb_property_notify_event_t *notify = (xcb_property_notify_event_t *)xcb_event;

            /* find window associated with this event */
            XwWindow *window = xw_get_window_by_xcb_id (notify->window);
            GOTO_HANDLER_IF (!window, WINDOW_SEARCH_FAILED, ERR_WINDOW_SEARCH_FAILED);

            if (notify->atom == xw_state._NET_WM_STATE) {
                xcb_atom_t *values      = Null;
                Size        value_count = get_xcb_atom_property_atom (
                    xw_state._NET_WM_STATE,
                    window->xcb_window_id,
                    &values
                );

                XwWindowState new_state = window->state;
                Bool          add       = notify->state;
                for (Size s = 0; s < value_count; s++) {
                    if (values[s] == xw_state._NET_WM_STATE_MODAL) {
                        new_state = add ? new_state | XW_WINDOW_STATE_MASK_MODAL :
                                          new_state & ~XW_WINDOW_STATE_MASK_MODAL;
                    } else if (values[s] == xw_state._NET_WM_STATE_STICKY) {
                        new_state = add ? new_state | XW_WINDOW_STATE_MASK_STICKY :
                                          new_state & ~XW_WINDOW_STATE_MASK_STICKY;
                    } else if (values[s] == xw_state._NET_WM_STATE_MAXIMIZED_VERT) {
                        new_state = add ? new_state | XW_WINDOW_STATE_MASK_MAXIMIZED_VERT :
                                          new_state & ~XW_WINDOW_STATE_MASK_MAXIMIZED_VERT;
                    } else if (values[s] == xw_state._NET_WM_STATE_MAXIMIZED_HORZ) {
                        new_state = add ? new_state | XW_WINDOW_STATE_MASK_MAXIMIZED_HORZ :
                                          new_state & ~XW_WINDOW_STATE_MASK_MAXIMIZED_HORZ;
                    } else if (values[s] == xw_state._NET_WM_STATE_SHADED) {
                        new_state = add ? new_state | XW_WINDOW_STATE_MASK_SHADED :
                                          new_state & ~XW_WINDOW_STATE_MASK_SHADED;
                    } else if (values[s] == xw_state._NET_WM_STATE_SKIP_TASKBAR) {
                        new_state = add ? new_state | XW_WINDOW_STATE_MASK_SKIP_TASKBAR :
                                          new_state & ~XW_WINDOW_STATE_MASK_SKIP_TASKBAR;
                    } else if (values[s] == xw_state._NET_WM_STATE_SKIP_PAGER) {
                        new_state = add ? new_state | XW_WINDOW_STATE_MASK_SKIP_PAGER :
                                          new_state & ~XW_WINDOW_STATE_MASK_SKIP_PAGER;
                    } else if (values[s] == xw_state._NET_WM_STATE_HIDDEN) {
                        new_state = add ? new_state | XW_WINDOW_STATE_MASK_HIDDEN :
                                          new_state & ~XW_WINDOW_STATE_MASK_HIDDEN;
                    } else if (values[s] == xw_state._NET_WM_STATE_FULLSCREEN) {
                        new_state = add ? new_state | XW_WINDOW_STATE_MASK_FULLSCREEN :
                                          new_state & ~XW_WINDOW_STATE_MASK_FULLSCREEN;
                    } else if (values[s] == xw_state._NET_WM_STATE_ABOVE) {
                        new_state = add ? new_state | XW_WINDOW_STATE_MASK_ABOVE :
                                          new_state & ~XW_WINDOW_STATE_MASK_ABOVE;
                    } else if (values[s] == xw_state._NET_WM_STATE_BELOW) {
                        new_state = add ? new_state | XW_WINDOW_STATE_MASK_BELOW :
                                          new_state & ~XW_WINDOW_STATE_MASK_BELOW;
                    } else if (values[s] == xw_state._NET_WM_STATE_DEMANDS_ATTENTION) {
                        new_state = add ? new_state | XW_WINDOW_STATE_MASK_DEMANDS_ATTENTION :
                                          new_state & ~XW_WINDOW_STATE_MASK_DEMANDS_ATTENTION;
                    } else if (values[s] == xw_state._NET_WM_STATE_FOCUSED) {
                        new_state = add ? new_state | XW_WINDOW_STATE_MASK_FOCUSED :
                                          new_state & ~XW_WINDOW_STATE_MASK_FOCUSED;
                    }
                }

                /* set new state */
                window->state = new_state;
                e             = xw_event_state_change (e, new_state, window);
            }

            break;
        }

        /* REF : https://tronche.com/gui/x/xlib/events/keyboard-pointer/keyboard-pointer.html */
        case XCB_BUTTON_PRESS : {
            xcb_button_press_event_t *bp = (xcb_button_press_event_t *)xcb_event;

            /* find window associated with this event */
            XwWindow *window = xw_get_window_by_xcb_id (bp->event);
            GOTO_HANDLER_IF (!window, WINDOW_SEARCH_FAILED, ERR_WINDOW_SEARCH_FAILED);

            // REF : https://stackoverflow.com/questions/35885572/get-status-of-currently-active-modifiers-in-x11
            XwModifierState mod;
            mod.ctrl      = !!(bp->state & XCB_MOD_MASK_CONTROL);
            mod.shift     = !!(bp->state & XCB_MOD_MASK_SHIFT);
            mod.caps_lock = !!(bp->state & XCB_MOD_MASK_LOCK);
            mod.num_lock  = !!(bp->state & XCB_MOD_MASK_2);
            mod.alt       = !!(bp->state & XCB_MOD_MASK_1);
            mod.caps_lock = !!(bp->state & XCB_MOD_MASK_LOCK);
            mod.meta      = !!(bp->state & XCB_MOD_MASK_4);

            /* update the button states */
            XwMouseButtonState new_state = XW_MOUSE_BUTTON_MASK_UNKNOWN;
            if (bp->state & XCB_BUTTON_MASK_1) {
                new_state |= XW_MOUSE_BUTTON_MASK_LEFT;
            }
            if (bp->state & XCB_BUTTON_MASK_2) {
                new_state |= XW_MOUSE_BUTTON_MASK_RIGHT;
            }
            if (bp->state & XCB_BUTTON_MASK_3) {
                new_state |= XW_MOUSE_BUTTON_MASK_MIDDLE;
            }
            if (bp->state & XCB_BUTTON_MASK_4) {
                new_state |= XW_MOUSE_BUTTON_MASK_BUTTON4;
            }
            if (bp->state & XCB_BUTTON_MASK_5) {
                new_state |= XW_MOUSE_BUTTON_MASK_BUTTON5;
            }

            xw_event_mouse_input (e, new_state, bp->event_x, bp->event_y, mod, window);

            break;
        }

        /* REF : https://tronche.com/gui/x/xlib/events/keyboard-pointer/keyboard-pointer.html */
        case XCB_BUTTON_RELEASE : {
            xcb_button_release_event_t *br = (xcb_button_release_event_t *)xcb_event;

            /* find window associated with this event */
            XwWindow *window = xw_get_window_by_xcb_id (br->event);
            GOTO_HANDLER_IF (!window, WINDOW_SEARCH_FAILED, ERR_WINDOW_SEARCH_FAILED);

            // REF : https://stackoverflow.com/questions/35885572/get-status-of-currently-active-modifiers-in-x11
            XwModifierState mod;
            mod.ctrl      = !!(br->state & XCB_MOD_MASK_CONTROL);
            mod.shift     = !!(br->state & XCB_MOD_MASK_SHIFT);
            mod.caps_lock = !!(br->state & XCB_MOD_MASK_LOCK);
            mod.num_lock  = !!(br->state & XCB_MOD_MASK_2);
            mod.alt       = !!(br->state & XCB_MOD_MASK_1);
            mod.caps_lock = !!(br->state & XCB_MOD_MASK_LOCK);
            mod.meta      = !!(br->state & XCB_MOD_MASK_4);

            /* update the button states */
            XwMouseButtonState new_state = XW_MOUSE_BUTTON_MASK_UNKNOWN;
            if (br->state & XCB_BUTTON_MASK_1) {
                new_state |= XW_MOUSE_BUTTON_MASK_LEFT;
            }
            if (br->state & XCB_BUTTON_MASK_2) {
                new_state |= XW_MOUSE_BUTTON_MASK_RIGHT;
            }
            if (br->state & XCB_BUTTON_MASK_3) {
                new_state |= XW_MOUSE_BUTTON_MASK_MIDDLE;
            }
            if (br->state & XCB_BUTTON_MASK_4) {
                new_state |= XW_MOUSE_BUTTON_MASK_BUTTON4;
                /* this is scroll up in XCB */
            }
            if (br->state & XCB_BUTTON_MASK_5) {
                new_state |= XW_MOUSE_BUTTON_MASK_BUTTON5;
                /* this is scroll down in XCB */
            }

            if (br->state & (XCB_BUTTON_MASK_4 | XCB_BUTTON_MASK_5)) {
                xw_event_mouse_wheel (
                    e,
                    br->event_x,
                    br->event_y,
                    !!(br->state & XCB_BUTTON_MASK_4),
                    mod,
                    window
                );
            } else {
                xw_event_mouse_input (e, new_state, br->event_x, br->event_y, mod, window);
            }

            break;
        }

        /* REF : https://tronche.com/gui/x/xlib/events/keyboard-pointer/keyboard-pointer.html */
        case XCB_MOTION_NOTIFY : {
            xcb_motion_notify_event_t *motion = (xcb_motion_notify_event_t *)xcb_event;

            /* find window associated with this event */
            XwWindow *window = xw_get_window_by_xcb_id (motion->event);
            GOTO_HANDLER_IF (!window, WINDOW_SEARCH_FAILED, ERR_WINDOW_SEARCH_FAILED);

            /* compute new displacement */
            Int32 dx = motion->root_x - window->last_cursor_pos_x;
            Int32 dy = motion->root_y - window->last_cursor_pos_y;

            /* set event data */
            e = xw_event_mouse_move (e, motion->root_x, motion->root_y, dx, dy, window);

            /* update last cursor position */
            window->last_cursor_pos_x = motion->root_x;
            window->last_cursor_pos_y = motion->root_y;

            break;
        }

        /* REF : https://tronche.com/gui/x/xlib/events/keyboard-pointer/keyboard-pointer.html */
        case XCB_KEY_PRESS : {
            const xcb_key_press_event_t *key = (xcb_key_press_event_t *)xcb_event;

            /* find window associated with this event */
            XwWindow *window = xw_get_window_by_xcb_id (key->event);
            GOTO_HANDLER_IF (!window, WINDOW_SEARCH_FAILED, ERR_WINDOW_SEARCH_FAILED);

            // REF : https://stackoverflow.com/questions/35885572/get-status-of-currently-active-modifiers-in-x11
            XwModifierState mod;
            mod.ctrl      = !!(key->state & XCB_MOD_MASK_CONTROL);
            mod.shift     = !!(key->state & XCB_MOD_MASK_SHIFT);
            mod.caps_lock = !!(key->state & XCB_MOD_MASK_LOCK);
            mod.num_lock  = !!(key->state & XCB_MOD_MASK_2);
            mod.alt       = !!(key->state & XCB_MOD_MASK_1);
            mod.caps_lock = !!(key->state & XCB_MOD_MASK_LOCK);
            mod.meta      = !!(key->state & XCB_MOD_MASK_4);

            e = xw_event_keyboard_input (
                e,
                xw_key_from_xcb_keycode (key->detail),
                XW_BUTTON_STATE_PRESSED,
                mod,
                window
            );

            break;
        }

        /* REF : https://tronche.com/gui/x/xlib/events/keyboard-pointer/keyboard-pointer.html */
        case XCB_KEY_RELEASE : {
            const xcb_key_release_event_t *key = (const xcb_key_release_event_t *)xcb_event;

            /* find window associated with this event */
            XwWindow *window = xw_get_window_by_xcb_id (key->event);
            GOTO_HANDLER_IF (!window, WINDOW_SEARCH_FAILED, ERR_WINDOW_SEARCH_FAILED);

            // REF : https://stackoverflow.com/questions/35885572/get-status-of-currently-active-modifiers-in-x11
            XwModifierState mod;
            mod.ctrl      = !!(key->state & XCB_MOD_MASK_CONTROL);
            mod.shift     = !!(key->state & XCB_MOD_MASK_SHIFT);
            mod.caps_lock = !!(key->state & XCB_MOD_MASK_LOCK);
            mod.num_lock  = !!(key->state & XCB_MOD_MASK_2);
            mod.alt       = !!(key->state & XCB_MOD_MASK_1);
            mod.caps_lock = !!(key->state & XCB_MOD_MASK_LOCK);
            mod.meta      = !!(key->state & XCB_MOD_MASK_4);

            e = xw_event_keyboard_input (
                e,
                xw_key_from_xcb_keycode (key->detail),
                XW_BUTTON_STATE_RELEASED,
                mod,
                window
            );

            break;
        }

        default :
            break;
    }

    return e;

WINDOW_SEARCH_FAILED:
    e->type = XW_EVENT_TYPE_NONE;
    return e;
}

/**
 * @b Convert given @c xcb_keycode_t to @c XwKey
 * */
static XwKey xw_key_from_xcb_keycode (xcb_keycode_t keycode) {
    if (!xw_state.keyboard) {
        xw_init_keyboard();
    }
    return xw_state.keyboard[keycode];
}

static Size get_xcb_atom_property_atom (xcb_atom_t atom, xcb_window_t window, xcb_atom_t **values) {
    xcb_get_property_cookie_t cookie = xcb_get_property (
        xw_state.connection, // xcb_connection_t *c,
        False,               // uint8_t           _delete,
        window,              // xcb_window_t      window,
        atom,                // xcb_atom_t        property,
        XCB_ATOM_ATOM,       // xcb_atom_t        type,
        0,                   // uint32_t          long_offset,
        UINT32_MAX           // uint32_t          long_length
    );
    xcb_get_property_reply_t *reply = xcb_get_property_reply (xw_state.connection, cookie, Null);
    *values                         = (xcb_atom_t *)xcb_get_property_value (reply);
    Size length                     = xcb_get_property_value_length (reply);
    FREE (reply);

    return length;
}
