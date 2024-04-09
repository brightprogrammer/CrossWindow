/**
 * @file Window.c 
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

/* local headers */
#include "State.h"
#include "Types.h"
#include "Window.h"

/* libc headers */
#include <string.h>

/* xcb related headers */
#include <xcb/xcb.h>
#include <xcb/xproto.h>


extern XwState xw_state;

/**
 * @b Create a new @x XwWindow object.
 *
 * @param title
 * @param width
 * @param height
 * @param xpos
 * @param ypos
 *
 * @return XwWindow* on success.
 * @return Null otherwise.
 * */
XwWindow *xw_window_create (CString title, Uint32 width, Uint32 height, Uint32 xpos, Uint32 ypos) {
    RETURN_VALUE_IF (!width || !height, Null, ERR_INVALID_ARGUMENTS);

    XwWindow *self = NEW (XwWindow);
    RETURN_VALUE_IF (!self, Null, ERR_OUT_OF_MEMORY);

    XwWindow *iself = xw_window_init (self, title, width, height, xpos, ypos);
    GOTO_HANDLER_IF (!iself, INIT_FAILED, ERR_OBJECT_INITIALIZATION_FAILED);

    return iself;

    /* error catchers/handlers */
INIT_FAILED: {
    xw_window_destroy (self);
    return Null;
}
}

/**
 * @b Create a new @x XwWindow object.
 *
 * @param self XwWindow object to be initialized.
 * @param title Cannot be @c Null.
 * @param width Cannot be 0.
 * @param height Cannot be 0.
 * @param xpos
 * @param ypos
 *
 * @return XwWindow* on success.
 * @return Null otherwise.
 * */
XwWindow *xw_window_init (
    XwWindow *self,
    CString   title,
    Uint32    width,
    Uint32    height,
    Uint32    xpos,
    Uint32    ypos
) {
    RETURN_VALUE_IF (!self || !width || !height, Null, ERR_INVALID_ARGUMENTS);

    xcb_connection_t *conn   = xw_state.connection;
    xcb_screen_t     *screen = xw_state.screen_iterator.data;
    RETURN_VALUE_IF (!conn || !screen, Null, ERR_XW_STATE_NOT_INITIALIZED);

    /* create platform data */
    self->xcb_window_id = -1;

    /* generate id for new window. */
    xcb_window_t win_id = xcb_generate_id (conn);
    RETURN_VALUE_IF (
        win_id == (xcb_window_t)-1, /* -1 is returned on failure */
        Null,
        "Failed to generate new window ID\n"
    );

    self->xcb_window_id = win_id;
    self->border_width  = 0;
    self->min_size      = (XwWindowSize) {0, 0};
    self->max_size      = (XwWindowSize) {screen->width_in_pixels, screen->height_in_pixels};
    self->size.width    = CLAMP (width, 0, self->max_size.width);
    self->size.height   = CLAMP (height, 0, self->max_size.height);
    self->pos.x         = xpos;
    self->pos.y         = ypos;
    self->title         = title ? strdup (title) : Null;
    self->icon_path     = Null;

    Uint32 win_mask     = XCB_CW_EVENT_MASK;
    Uint32 win_values[] = {
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_ENTER_WINDOW |
        XCB_EVENT_MASK_LEAVE_WINDOW | XCB_EVENT_MASK_FOCUS_CHANGE |
        XCB_EVENT_MASK_VISIBILITY_CHANGE | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
        XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_POINTER_MOTION |
        XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
        XCB_EVENT_MASK_RESIZE_REDIRECT
    };

    /* create window corresponding to window id */
    xcb_create_window (
        conn,                          /* connection */
        XCB_COPY_FROM_PARENT,          /* depth: Copy depth info from parent */
        win_id,                        /* wid: id of window object to be created */
        screen->root,                  /* parent: parent window of this window */
        self->pos.x,                   /* x */
        self->pos.y,                   /* y */
        self->size.width,              /* width */
        self->size.height,             /* height */
        self->border_width,            /* border width */
        XCB_WINDOW_CLASS_INPUT_OUTPUT, /* window class */
        screen->root_visual,           /* visual data */
        win_mask,                      /* value mask */
        win_values                     /* value mask array */
    );

    /* create new atom to help us detect close window event messages */
    xcb_change_property (
        conn,                      /* xcb connection */
        XCB_PROP_MODE_REPLACE,     /* replace the property with new value */
        self->xcb_window_id,       /* xcb id of window object */
        xw_state.WM_PROTOCOLS,     /* change something in protocl */
        XCB_ATOM_ATOM,             /* change an atom */
        32,                        /* process data in chunks of 32 bits */
        1,                         /* length of data */
        &xw_state.WM_DELETE_WINDOW /* data */
    );

    if (title) {
        xw_window_set_title (self, title);
    }

    /* register this window to global state. */
    self->xw_id = xw_create_new_window_id (self);

    xcb_map_window (conn, win_id);
    xcb_flush (conn);
    return self;
}

/**
 * @b Deinitialize given @c XwWindow object.
 *
 * @param self
 *
 * @return @c self on success.
 * @return Null otherwise.
 * */
XwWindow *xw_window_deinit (XwWindow *self) {
    RETURN_VALUE_IF (!self, Null, ERR_INVALID_ARGUMENTS);

    /* unregister this window from xw_state */
    xw_remove_window_id (self->xw_id);

    /* if window was created then destroy it */
    if (self->xcb_window_id != (xcb_window_t)-1) {
        xcb_connection_t *conn = xw_state.connection;
        RETURN_VALUE_IF (!conn, Null, ERR_XW_STATE_NOT_INITIALIZED);

        xcb_destroy_window (conn, self->xcb_window_id);
        self->screen = Null;
    }

    /* destroy strdup-ed string */
    if (self->title) {
        FREE (self->title);
        self->title = Null;
    }

    /* destroy strdup-ed string */
    if (self->icon_path) {
        FREE (self->icon_path);
        self->icon_path = Null;
    }

    return self;
}

/**
 * @b Destroy window.
 *
 * This will also free the given @c XwWindow object.
 *
 * @param self @c XwWindow object to be destroyed.
 * */
void xw_window_destroy (XwWindow *self) {
    RETURN_IF (!self, ERR_INVALID_ARGUMENTS);

    xw_window_deinit (self);
    FREE (self);
}

/**
 * @b Change window's visibility to visible.
 *
 * @param self
 *
 * @return @c self on success.
 * @return Null otherwise.
 * */
XwWindow *xw_window_show (XwWindow *self) {
    RETURN_VALUE_IF (!self, Null, ERR_INVALID_ARGUMENTS);

    xcb_map_window (xw_state.connection, self->xcb_window_id);
    xcb_flush (xw_state.connection);

    return self;
}

/**
 * @b Change window's visibility to invisible.
 *
 * @param self
 *
 * @return @c self on success.
 * @return Null otherwise.
 * */
XwWindow *xw_window_hide (XwWindow *self) {
    RETURN_VALUE_IF (!self, Null, ERR_INVALID_ARGUMENTS);

    xcb_unmap_window (xw_state.connection, self->xcb_window_id);
    xcb_flush (xw_state.connection);

    return self;
}

/**
 * @b Get title string of given window.
 *
 * User must free the returend pointer to title string after use.
 * The returned memory is strduped out of actual window title.
 *
 * @param self Window object to get title of.
 *
 * @return CString if title is set and we can successfully
 *         strdup it.
 * @return Null if title does not exist or some error occured.
 * */
CString xw_window_get_title (XwWindow *self) {
    RETURN_VALUE_IF (!self, Null, ERR_INVALID_ARGUMENTS);
    CString title = self->title ? strdup (self->title) : Null;
    RETURN_VALUE_IF (!title, Null, ERR_OUT_OF_MEMORY);
    return title;
}

/**
 * @b Get window size.
 *
 * @param self Window object.
 *
 * @return XwWindowSize containing width and height of window.
 * */
XwWindowSize xw_window_get_size (XwWindow *self) {
    RETURN_VALUE_IF (!self, ((XwWindowSize) {0, 0}), ERR_INVALID_ARGUMENTS);
    return self->size;
}

/**
 * @b Get minimum window size.
 *
 * @param self Window object.
 *
 * @return XwWindowSize containing min width and min height of window.
 * */
XwWindowSize xw_window_get_min_size (XwWindow *self) {
    RETURN_VALUE_IF (!self, ((XwWindowSize) {0, 0}), ERR_INVALID_ARGUMENTS);
    return self->min_size;
}

/**
 * @b Get maximum window size.
 *
 * @param self Window object.
 *
 * @return XwWindowSize containing max width and max height of window.
 * */
XwWindowSize xw_window_get_max_size (XwWindow *self) {
    RETURN_VALUE_IF (!self, ((XwWindowSize) {0, 0}), ERR_INVALID_ARGUMENTS);
    return self->max_size;
}

/**
 * @b Get window position.
 *
 * @param self Window object.
 *
 * @return XwWindowPos containing x and y coordinates of window from left corner of screen.
 * */
XwWindowPos xw_window_get_pos (XwWindow *self) {
    RETURN_VALUE_IF (!self, ((XwWindowPos) {0, 0}), ERR_INVALID_ARGUMENTS);
    return self->pos;
}

/**
 * @b Get window state (minimized, fullscreen, normal, etc...)
 *
 * @param self @c XwWindow object to get state of.
 * 
 * @return @c XW_WINDOW_STATE_UNKNOWN on failure or if state is not known,
 * @return Any other value less than @x XW_WINDOW_STATE_MAX on success.
 * */
XwWindowState xw_window_get_state (XwWindow *self) {
    RETURN_VALUE_IF (!self, XW_WINDOW_STATE_MASK_UNKNOWN, ERR_INVALID_ARGUMENTS);
    return self->state;
}

/**
 * @b Set window title.
 *
 * @param self
 * @param title
 *
 * @return The set window title on success.
 * @return Null otherwise.
 * */
CString xw_window_set_title (XwWindow *self, CString title) {
    RETURN_VALUE_IF (!self || !title, Null, ERR_INVALID_ARGUMENTS);

    CString set_title = strdup (title);
    RETURN_VALUE_IF (!set_title, Null, ERR_OUT_OF_MEMORY);

    if (self->title) {
        FREE (self->title);
    }
    self->title = set_title;
    /* set title */
    xcb_change_property (
        xw_state.connection,   /* xcb connection */
        XCB_PROP_MODE_REPLACE, /* replace the property with new value */
        self->xcb_window_id,   /* id of object */
        XCB_ATOM_WM_NAME,      /* property is window name */
        XCB_ATOM_STRING,       /* atom type is string */
        8,                     /* process data in chunks of 8 bits */
        strlen (self->title),  /* length of data */
        self->title            /* data */
    );

    xcb_flush (xw_state.connection);

    return title;
}

/**
 * @b Set window size.
 *
 * If provided window size exceeds limit then size is not changed.
 *
 * @param self
 * @param size
 *
 * @return Set window size on success. This may or may not be same as provided size
 *         due to window size bounds.
 * @return {0, 0} on failure. 
 * */
XwWindowSize xw_window_set_size (XwWindow *self, XwWindowSize size) {
    RETURN_VALUE_IF (!self, ((XwWindowSize) {0, 0}), ERR_INVALID_ARGUMENTS);

    /* make sure the size is in bounds */
    if (size.width < self->min_size.width || size.width > self->max_size.width ||
        size.height < self->min_size.height || size.height > self->max_size.height) {
        return self->size;
    }

    /* store new size */
    self->size = size;

    /* set new size */
    xcb_configure_window (
        xw_state.connection,
        self->xcb_window_id,
        XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
        &size
    );

    /* commit changes */
    xcb_flush (xw_state.connection);

    return size;
}

/**
 * @b Set minimum window size.
 *
 * @param self Window object.
 * @param size Min size of window.
 *
 * @return @c size on success.
 * @return @c {0, 0} on failure 
 * */
XwWindowSize xw_window_set_min_size (XwWindow *self, XwWindowSize size) {
    RETURN_VALUE_IF (!self, ((XwWindowSize) {0, 0}), ERR_INVALID_ARGUMENTS);
    RETURN_VALUE_IF (
        size.width > self->max_size.width || size.height > self->max_size.height,
        ((XwWindowSize) {0, 0}),
        "Min size bound cannot be greater than max size bound of window\n"
    );
    return (self->min_size = size);
}

/**
 * @b Get maximum window size.
 *
 * @param self Window object.
 *
 * @return XwWindowSize containing max width and max height of window.
 * */
XwWindowSize xw_window_set_max_size (XwWindow *self, XwWindowSize size) {
    RETURN_VALUE_IF (!self, ((XwWindowSize) {0, 0}), ERR_INVALID_ARGUMENTS);
    RETURN_VALUE_IF (
        size.width < self->min_size.width || size.height < self->min_size.height,
        ((XwWindowSize) {0, 0}),
        "Max size bound cannot be less than min size bound of window\n"
    );
    return (self->max_size = size);
}

/**
 * @b Set window position.
 *
 * @param self
 * @param pos
 *
 * @return The window position that was set.
 * */
XwWindowPos xw_window_set_pos (XwWindow *self, XwWindowPos pos) {
    RETURN_VALUE_IF (!self, ((XwWindowPos) {0, 0}), ERR_INVALID_ARGUMENTS);

    self->pos = pos;

    xcb_configure_window (
        xw_state.connection,
        self->xcb_window_id,
        XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
        &pos
    );

    xcb_flush (xw_state.connection);

    return pos;
}

/**
 * @b Mask of window states to be set.
 *
 * @param self
 * @param state
 *
 * @return @c XwWindowStateMask as mask of all states that were set on success.
 *         Mostly this is exactly same as provided state.
 * @return @c XW_WINDOW_STATE_MASK_UNKNOWN on failure.
 * */
XwWindowState xw_window_set_state (XwWindow *self, XwWindowState state) {
    RETURN_VALUE_IF (!self || !state, XW_WINDOW_STATE_MASK_UNKNOWN, ERR_INVALID_ARGUMENTS);

    /* create pairing of atom with it's mask */
    struct {
        xcb_atom_t        atom;
        XwWindowStateMask mask;
    } atoms[] = {
        {            xw_state._NET_WM_STATE_MODAL,             XW_WINDOW_STATE_MASK_MODAL},
        {           xw_state._NET_WM_STATE_STICKY,            XW_WINDOW_STATE_MASK_STICKY},
        {   xw_state._NET_WM_STATE_MAXIMIZED_VERT,    XW_WINDOW_STATE_MASK_MAXIMIZED_VERT},
        {   xw_state._NET_WM_STATE_MAXIMIZED_HORZ,    XW_WINDOW_STATE_MASK_MAXIMIZED_HORZ},
        {           xw_state._NET_WM_STATE_SHADED,            XW_WINDOW_STATE_MASK_SHADED},
        {     xw_state._NET_WM_STATE_SKIP_TASKBAR,      XW_WINDOW_STATE_MASK_SKIP_TASKBAR},
        {       xw_state._NET_WM_STATE_SKIP_PAGER,        XW_WINDOW_STATE_MASK_SKIP_PAGER},
        {           xw_state._NET_WM_STATE_HIDDEN,            XW_WINDOW_STATE_MASK_HIDDEN},
        {       xw_state._NET_WM_STATE_FULLSCREEN,        XW_WINDOW_STATE_MASK_FULLSCREEN},
        {            xw_state._NET_WM_STATE_ABOVE,             XW_WINDOW_STATE_MASK_ABOVE},
        {            xw_state._NET_WM_STATE_BELOW,             XW_WINDOW_STATE_MASK_BELOW},
        {xw_state._NET_WM_STATE_DEMANDS_ATTENTION, XW_WINDOW_STATE_MASK_DEMANDS_ATTENTION},
        {          xw_state._NET_WM_STATE_FOCUSED,           XW_WINDOW_STATE_MASK_FOCUSED},
    };

    /* fill data array */
    xcb_client_message_data_t data[ARRAY_SIZE (atoms)];
    for (Size i = 0; i < ARRAY_SIZE (atoms); i++) {
        data[i].data32[0] = (state & atoms[i].mask) ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
        data[i].data32[1] = atoms[i].atom;
        data[i].data32[2] = XCB_ATOM_NONE; /* source indicator */
    }

    /* send event for each mask */
    for (Size s = 0; s < ARRAY_SIZE (data); s++) {
        xcb_client_message_event_t payload = {
            .response_type = XCB_CLIENT_MESSAGE,
            .type          = xw_state._NET_WM_STATE,
            .format        = 32,
            .window        = self->xcb_window_id,
            .data          = data[s]
        };

        xcb_send_event (
            xw_state.connection,
            False,               /* whether to propagate the event or not */
            self->xcb_window_id, /* destination window */
            XCB_EVENT_MASK_STRUCTURE_NOTIFY,
            (CString)&payload
        );
    }

    xcb_flush (xw_state.connection);
    self->state = state;

    return state;
}

/************************************** PRIVATE METHODS **************************************/

/**
 * @b Get XwWindow object by providing platform-dependent xcb window id.
 *
 * Defined here but is used in Event.c
 *
 * @param xcb_win_id @c xcb_window_t for window to be retrieved.
 *
 * @return @c XwWindow* on success.
 * @return Null otherwise.
 * */
XwWindow *xw_get_window_by_xcb_id (xcb_window_t xcb_win_id) {
    for (Size s = 0; s < ARRAY_SIZE (xw_state.windows); s++) {
        if (xw_state.windows[s] && xw_state.windows[s]->xcb_window_id == xcb_win_id) {
            return xw_state.windows[s];
        }
    }

    return Null;
}
