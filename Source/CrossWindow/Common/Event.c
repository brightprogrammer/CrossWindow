/**
 * @file Event.c
 * @time 04/04/2024 20:31:53
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) 2024 Siddharth Mishra
 * @copyright Copyright (c) 2024 Anvie Labs
 *
 * @brief Defines XwEvent methods. 
 * */

#include <Common.h>
#include <CrossWindow/Event.h>

/* headers from libc */
#include <string.h>

static XwEvent *xw_event_init (XwEvent *e, XwEventType type, XwWindow *win);
extern CString  xwkey_to_cstr_map[XWK_MAX];

/**
 * @b Convert given @c XwKey to corresponding CString.
 *
 * The caps_case parameter only works correctly for key-press events.
 * On key-release events, not all modifiers are active.
 * IT'S NOT A BUG, IT'S A FEATURE!!
 *
 * @param key @c XwKey enum to be converted to CString.
 * @param caps_case True if caps, False otherwise.
 *
 * @return CString on success.
 * @return Null otherwise.
 * */
CString xw_key_cstr (XwKey key, Bool caps_case) {
    RETURN_VALUE_IF (key >= XWK_MAX, Null, ERR_INVALID_ARGUMENTS);

    if (caps_case && XWK_a <= key && key <= XWK_z) {
        key = XWK_A + (key - XWK_a);
    } else if (!caps_case && XWK_A <= key && key <= XWK_Z) {
        key = XWK_a + (key - XWK_A);
    }

    return xwkey_to_cstr_map[key];
}

/**
 * @b Create a new event of type @c XW_EVENT_TYPE_STATE_CHANGE
 *
 * @param e Event
 * @param new_state New window state.
 * @param win Window Window
 *
 * @return XwEvent* on success
 * @return Null otherwise
 * */
XwEvent *xw_event_state_change (XwEvent *e, XwWindowState new_state, XwWindow *win) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_STATE_CHANGE, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    e->state_change.new_state = new_state;

    return e;
}

/**
 * @b Create a new event of type @c XW_EVENT_TYPE_CLOSE_WINDOW
 *
 * @param e Event
 * @param win Window 
 *
 * @return XwEvent* on success
 * @return Null otherwise
 * */
XwEvent *xw_event_close_window (XwEvent *e, XwWindow *win) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_CLOSE_WINDOW, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    return e;
}

/**
 * @b Create a new event of type @c XW_EVENT_TYPE_VISIBILITY
 *
 * @param e Event
 * @param visible True if window is visible after this event. False otherwise.
 * @param win Window 
 *
 * @return XwEvent* on success
 * @return Null otherwise
 * */
XwEvent *xw_event_visibility (XwEvent *e, Bool visible, XwWindow *win) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_VISIBILITY, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    e->visibility.visible = visible;

    return e;
}

/** 
 * @b Create a new event of type @c XW_EVENT_TYPE_ENTER
 *
 * @param e Event
 * @param Uint32 x X-coordinate from which cursor enter the window
 * @param Uint32 y Y-coordinate from which cursor enter the window
 * @param win Window
 *
 * @return XwEvent* on successs,
 * @return Null otherwise
 * */
XwEvent *xw_event_enter (XwEvent *e, Uint32 x, Uint32 y, XwWindow *win) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_ENTER, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    e->enter.x = x;
    e->enter.y = y;

    return e;
}

/** 
 * @b Create a new event of type @c XW_EVENT_TYPE_LEAVE
 *
 * @param e Event
 * @param Uint32 x X-coordinate from which cursor leave the window
 * @param Uint32 y Y-coordinate from which cursor leave the window
 * @param win Window
 *
 * @return XwEvent* on successs,
 * @return Null otherwise
 * */
XwEvent *xw_event_leave (XwEvent *e, Uint32 x, Uint32 y, XwWindow *win) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_LEAVE, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    e->leave.x = x;
    e->leave.y = y;

    return e;
}

/** 
 * @b Create a new event of type @c XW_EVENT_TYPE_FOCUS
 *
 * @param e Event
 * @param focused
 * @param win Window
 *
 * @return XwEvent* on successs,
 * @return Null otherwise
 * */
XwEvent *xw_event_focus (XwEvent *e, Bool focused, XwWindow *win) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_FOCUS, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    e->focus.focused = focused;

    return e;
}

/** 
 * @b Create a new event of type @c XW_EVENT_TYPE_PAINT
 *
 * @param e Event
 * @param focused
 * @param win Window
 *
 * @return XwEvent* on successs,
 * @return Null otherwise
 * */
XwEvent *xw_event_paint (XwEvent *e, XwWindow *win) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_PAINT, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    return e;
}

/** 
 * @b Create a new event of type @c XW_EVENT_TYPE_REPOSITION
 *
 * @param e Event to be filled with data
 * @param x New x position of window
 * @param y New y position of window
 * @param win Window
 *
 * @return XwEvent* on successs,
 * @return Null otherwise
 * */
XwEvent *xw_event_reposition (XwEvent *e, Uint32 x, Uint32 y, XwWindow *win) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_REPOSITION, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    e->reposition.x = x;
    e->reposition.y = y;

    return e;
}

/** 
 * @b Create a new event of type @c XW_EVENT_TYPE_BORDER_WIDTH_CHANGE
 *
 * @param e Event to be filled with data
 * @param border_width New border width of window
 * @param win Window
 *
 * @return XwEvent* on successs,
 * @return Null otherwise
 * */
XwEvent *xw_event_border_width_change (XwEvent *e, Uint32 border_width, XwWindow *win) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_BORDER_WIDTH_CHANGE, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    e->border_width_change.border_width = border_width;

    return e;
}

/** 
 * @b Create a new event of type @c XW_EVENT_TYPE_RESTACK
 *
 * @param e Event to be filled with data
 * @param above @c XwWindow* above which this window is not rendered to.
 * @param win Window
 *
 * @return XwEvent* on successs,
 * @return Null otherwise
 * */
XwEvent *xw_event_restack (XwEvent *e, XwWindow *above_window, XwWindow *win) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_RESTACK, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    e->restack.above = above_window;

    return e;
}

/** 
 * @b Create a new event of type @c XW_EVENT_TYPE_RESIZE
 *
 * @param e Event to be filled with data
 * @param width New width of window
 * @param height New height of window
 * @param win Window
 *
 * @return XwEvent* on successs,
 * @return Null otherwise
 * */
XwEvent *xw_event_resize (XwEvent *e, Uint32 width, Uint32 height, XwWindow *win) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_RESIZE, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    e->resize.width  = width;
    e->resize.height = height;

    return e;
}

/** 
 * @b Create a new event of type @c XW_EVENT_TYPE_DPI_CHANGE
 *
 * @param e Event
 * @param scale 
 * @param win Window
 *
 * @return XwEvent* on successs,
 * @return Null otherwise
 * */
XwEvent *xw_event_dpi_change (XwEvent *e, Float32 scale, XwWindow *win) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_DPI_CHANGE, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    e->dpi_change.scale = scale;

    return e;
}

/** 
 * @b Create a new event of type @c XW_EVENT_TYPE_KEYBOARD_INPUT
 *
 * @param e Event
 * @param key
 * @param state
 * @param mod
 * @param win Window
 *
 * @return XwEvent* on successs,
 * @return Null otherwise
 * */
XwEvent *xw_event_keyboard_input (
    XwEvent        *e,
    XwKey           key,
    XwButtonState   state,
    XwModifierState mod,
    XwWindow       *win
) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_KEYBOARD_INPUT, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    e->keyboard_input.key   = key;
    e->keyboard_input.state = state;
    e->keyboard_input.mod   = mod;

    return e;
}

/** 
 * @b Create a new event of type @c XW_EVENT_TYPE_MOUSE_MOVE
 *
 * @param e Event
 * @param x
 * @param y
 * @param screenx
 * @param screeny
 * @param deltax
 * @param deltay
 * @param win Window
 *
 * @return XwEvent* on successs,
 * @return Null otherwise
 * */
XwEvent *xw_event_mouse_move (XwEvent *e, Uint32 x, Uint32 y, Int32 dx, Int32 dy, XwWindow *win) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_MOUSE_MOVE, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    e->mouse_move.x  = x;
    e->mouse_move.y  = y;
    e->mouse_move.dx = dx;
    e->mouse_move.dy = dy;

    return e;
}

/** 
 * @b Create a new event of type @c XW_EVENT_TYPE_MOUSE_INPUT
 *
 * @param e Event
 * @param button
 * @param state
 * @param mod
 * @param win Window
 *
 * @return XwEvent* on successs,
 * @return Null otherwise
 * */
XwEvent *xw_event_mouse_input (
    XwEvent           *e,
    XwMouseButtonState state,
    Uint32             posx,
    Uint32             posy,
    XwModifierState    mod,
    XwWindow          *win
) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_MOUSE_INPUT, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    e->mouse_input.button_state = state;
    e->mouse_input.x            = posx;
    e->mouse_input.y            = posy;
    e->mouse_input.mod          = mod;

    return e;
}

/** 
 * @b Create a new event of type @c XW_EVENT_TYPE_MOUSE_WHEEL
 *
 * @param e Event
 * @param delta
 * @param mod
 * @param win Window
 *
 * @return XwEvent* on successs,
 * @return Null otherwise
 * */
XwEvent *xw_event_mouse_wheel (
    XwEvent        *e,
    Uint32          x,
    Uint32          y,
    Bool            direction,
    XwModifierState mod,
    XwWindow       *win
) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_MOUSE_WHEEL, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    e->mouse_wheel.x         = x;
    e->mouse_wheel.y         = y;
    e->mouse_wheel.direction = direction;
    e->mouse_wheel.mod       = mod;

    return e;
}

/** 
 * @b Create a new event of type @c XW_EVENT_TYPE_TOUCH
 
 * @param e Event
 * @param delta
 * @param mod
 * @param win Window
 *
 * @return XwEvent* on successs,
 * @return Null otherwise
 * */
XwEvent *xw_event_touch (XwEvent *e, Size touch_count, XwTouchPoint *points, XwWindow *win) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_TOUCH, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    RETURN_VALUE_IF (
        touch_count > XW_TOUCH_COUNT_MAX,
        Null,
        "Touch count cannot be greater than %zu\n",
        XW_GAMEPAD_AXES_COUNT_MAX
    );

    e->touch.touch_count = touch_count;
    memcpy (e->touch.touches, points, sizeof (points[0]) * touch_count);

    return e;
}

/** 
 * @b Create a new event of type @c XW_EVENT_TYPE_GAMEPAD
 *
 * @param e Event
 * @param delta
 * @param mod
 * @param win Window
 *
 * @return XwEvent* on successs,
 * @return Null otherwise
 * */
XwEvent *xw_event_gamepad (
    XwEvent  *e,
    Bool      connected,
    Size      index,
    CString   id,
    CString   mapping,
    Uint32    axes_count,
    Float64  *axis,
    Uint32    button_count,
    Float64  *analog_button,
    Bool     *digital_button,
    XwWindow *win
) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_GAMEPAD, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    RETURN_VALUE_IF (
        axes_count > XW_GAMEPAD_AXES_COUNT_MAX,
        Null,
        "Axis count cannot be greater than %zu\n",
        XW_GAMEPAD_AXES_COUNT_MAX
    );

    RETURN_VALUE_IF (
        button_count > XW_GAMEPAD_BUTTON_COUNT_MAX,
        Null,
        "Button count cannot be greater than %zu\n",
        XW_GAMEPAD_BUTTON_COUNT_MAX
    );

    e->gamepad.connected = connected;
    e->gamepad.index     = index;
    e->gamepad.id        = id;
    e->gamepad.mapping   = mapping;

    if (axis && axes_count) {
        e->gamepad.axes_count = axes_count;
        memcpy (e->gamepad.axis, axis, sizeof (axis[0]) * axes_count);
    }

    e->gamepad.button_count = button_count;
    if (button_count && analog_button) {
        memcpy (e->gamepad.analog_button, analog_button, sizeof (analog_button[0]) * button_count);
    }
    if (button_count && digital_button) {
        memcpy (
            e->gamepad.digital_button,
            digital_button,
            sizeof (digital_button[0]) * button_count
        );
    }

    return e;
}

/** 
 * @b Create a new event of type @c XW_EVENT_TYPE_DROP_FILE
 *
 * @param e Event
 * @param win Window
 *
 * @return XwEvent* on successs,
 * @return Null otherwise
 * */
XwEvent *xw_event_drop_file (XwEvent *e, XwWindow *win) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_DROP_FILE, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    return e;
}

/** 
 * @b Create a new event of type @c XW_EVENT_TYPE_HOVER_FILE
 *
 * @param e Event
 * @param win Window
 *
 * @return XwEvent* on successs,
 * @return Null otherwise
 * */
XwEvent *xw_event_hover_file (XwEvent *e, XwWindow *win) {
    RETURN_VALUE_IF (!e || !win, Null, ERR_INVALID_ARGUMENTS);

    e = xw_event_init (e, XW_EVENT_TYPE_HOVER_FILE, win);
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_OBJECT_REF);

    return e;
}

/************************************* XwEventQueue ***************************************/

#if 0

typedef struct XwEventQueue {
    Size      count;
    Size      capacity;
    XwEvent **events;
} XwEventQueue;

/**
 * @b Create a new XwEventQueue object.
 *
 * @return XwEventQueue* on success.
 * @return Null otherwise.
 * */
XwEventQueue *xw_event_queue_create (void) {
    XwEventQueue *eq = NEW (XwEventQueue);
    RETURN_VALUE_IF (!eq, Null, ERR_OUT_OF_MEMORY);

    eq->capacity = 32;
    eq->events   = ALLOCATE (XwEvent *, eq->capacity);
    GOTO_HANDLER_IF (!eq->events, ALLOC_FAILED, ERR_OUT_OF_MEMORY);

    return eq;

ALLOC_FAILED: {
    FREE (eq);
    return Null;
}
}

/* Definition of xw_event_queue_update is platform dependent. */

/** 
 * @b Get reference to front element of queue.
 *
 * @param eq 
 *
 * @return XwEvent* on success.
 * @return Null otherwise.
 * */
const XwEvent *xw_event_queue_front (const XwEventQueue *eq) {
    RETURN_VALUE_IF (!eq, Null, ERR_INVALID_ARGUMENTS);

    return eq->events[0];
}

/** 
 * @b Pop an event from the front of queue.
 *
 * Popping results in permanent removal of event from queue.
 *
 * @param eq 
 *
 * @return XwEvent* on success.
 * @return Null otherwise.
 * */
XwEvent *xw_event_queue_pop (XwEventQueue *eq) {
    RETURN_VALUE_IF (!eq, Null, ERR_INVALID_ARGUMENTS);

    XwEvent *popped = eq->events[0];
    for (Size s = 0; s < eq->count - 1; s++) {
        eq->events[s] = eq->events[s + 1];
    }
    eq->events[eq->count--] = Null;
    return popped;
}

/** 
 * @b Push an event into the event queue. 
 *
 * Ownership of @c XwEvent object is transferred to provided @c XwEventQueue 
 * on success. This means, given @c XwEventQueue would store the same pointer
 * as provided without making another copy of it, and will free it when the
 * queue is being destroyed!
 *
 * @param eq 
 *
 * @return XwEvent* on success.
 * @return Null otherwise.
 * */
const XwEvent *xw_event_queue_push (XwEventQueue *eq, XwEvent *event) {
    RETURN_VALUE_IF (!eq && event, Null, ERR_INVALID_ARGUMENTS);

    for (Size s = eq->count - 1; s; s--) {
        eq->events[s + 1] = eq->events[s];
    }
    eq->events[0] = event;
    eq->count++;

    return event;
}

/** 
 * @b Check if given event queue is empty or not.
 *
 * @param eq
 *
 * @return True if empty or if @c eq is @c Null
 * @return False otherwise.
 * */
Bool xw_event_queue_is_empty (const XwEventQueue *eq) {
    RETURN_VALUE_IF (!eq, True, ERR_INVALID_ARGUMENTS);

    return !eq->count;
}

#endif

/******************************************* PRIVATE METHODS *****************************************/

/**
 * @b Set type and window to given @c XwEvent object.
 *
 * @param type 
 * @param win Window
 *
 * @return XwEvent on success,
 * @return Null on failure.
 * */
static XwEvent *xw_event_init (XwEvent *e, XwEventType type, XwWindow *win) {
    RETURN_VALUE_IF (!e, Null, ERR_INVALID_ARGUMENTS);

    e->type   = type;
    e->window = win;

    return e;
}

CString xwkey_to_cstr_map[XWK_MAX] = {
    [XWK_UNKNOWN] = "UNKNOWN",

    /* digits */
    [XWK_1] = "1",
    [XWK_2] = "2",
    [XWK_3] = "3",
    [XWK_4] = "4",
    [XWK_5] = "5",
    [XWK_6] = "6",
    [XWK_7] = "7",
    [XWK_8] = "8",
    [XWK_9] = "9",
    [XWK_0] = "0",

    /* alphabets */
    [XWK_Q] = "Q",
    [XWK_W] = "W",
    [XWK_E] = "E",
    [XWK_R] = "R",
    [XWK_T] = "T",
    [XWK_Y] = "Y",
    [XWK_U] = "U",
    [XWK_I] = "I",
    [XWK_O] = "O",
    [XWK_P] = "P",
    [XWK_A] = "A",
    [XWK_S] = "S",
    [XWK_D] = "D",
    [XWK_F] = "F",
    [XWK_G] = "G",
    [XWK_H] = "H",
    [XWK_J] = "J",
    [XWK_K] = "K",
    [XWK_L] = "L",
    [XWK_Z] = "Z",
    [XWK_X] = "X",
    [XWK_C] = "C",
    [XWK_V] = "V",
    [XWK_B] = "B",
    [XWK_N] = "N",
    [XWK_M] = "M",

    [XWK_q] = "q",
    [XWK_w] = "w",
    [XWK_e] = "e",
    [XWK_r] = "r",
    [XWK_t] = "t",
    [XWK_y] = "y",
    [XWK_u] = "u",
    [XWK_i] = "i",
    [XWK_o] = "o",
    [XWK_p] = "p",
    [XWK_a] = "a",
    [XWK_s] = "s",
    [XWK_d] = "d",
    [XWK_f] = "f",
    [XWK_g] = "g",
    [XWK_h] = "h",
    [XWK_j] = "j",
    [XWK_k] = "k",
    [XWK_l] = "l",
    [XWK_z] = "z",
    [XWK_x] = "x",
    [XWK_c] = "c",
    [XWK_v] = "v",
    [XWK_b] = "b",
    [XWK_n] = "n",
    [XWK_m] = "m",

    /* function keys */
    [XWK_F1]  = "F1",
    [XWK_F2]  = "F2",
    [XWK_F3]  = "F3",
    [XWK_F4]  = "F4",
    [XWK_F5]  = "F5",
    [XWK_F6]  = "F6",
    [XWK_F7]  = "F7",
    [XWK_F8]  = "F8",
    [XWK_F9]  = "F9",
    [XWK_F10] = "F10",
    [XWK_F11] = "F11",
    [XWK_F12] = "F12",

    [XWK_ESCAPE]    = "ESCAPE",
    [XWK_BACKSPACE] = "BACKSPACE",
    [XWK_TAB]       = "TABSPACE",
    [XWK_ENTER]     = "ENTER",
    [XWK_DECIMAL]   = "DECIMAL",

    [XWK_LCONTROL] = "LCONTROL",
    [XWK_RCONTROL] = "RCONTROL",
    [XWK_LSHIFT]   = "LSHIFT",
    [XWK_RSHIFT]   = "RSHIFT",
    [XWK_LALT]     = "LALT",
    [XWK_RALT]     = "RALT",

    [XWK_EQUALS]   = "=",
    [XWK_ADD]      = "+",
    [XWK_SUBTRACT] = "-",
    [XWK_MULTIPLY] = "*",
    [XWK_DIVIDE]   = "/",

    [XWK_TILDE]        = "`",
    [XWK_GRAVE_ACCENT] = "~",
    [XWK_EXCLAMATION]  = "!",
    [XWK_AT]           = "@",
    [XWK_HASH]         = "#",
    [XWK_CURRENCY]     = "$",
    [XWK_PERCENT]      = "%",
    [XWK_HAT]          = "^", /* ^ character */
    [XWK_AND]          = "&",
    [XWK_STAR]         = "*",
    [XWK_LPAREN]       = "(",
    [XWK_RPAREN]       = ")",

    [XWK_HYPHEN]        = "-",
    [XWK_SEMICOLON]     = ";",
    [XWK_COLON]         = ":",
    [XWK_SINGLE_QUOTE]  = "'",
    [XWK_DOUBLE_QUOTES] = "\"",
    [XWK_BACK_SLASH]    = "\\",
    [XWK_FWD_SLASH]     = "/",
    [XWK_PIPE]          = "|",
    [XWK_COMMA]         = ",",
    [XWK_PERIOD]        = ".",
    [XWK_SPACE]         = " ",
    [XWK_LBRACKET]      = "[",
    [XWK_RBRACKET]      = "]",
    [XWK_LBRACE]        = "{",
    [XWK_RBRACE]        = "}",
    [XWK_LANGLE]        = "<",
    [XWK_RANGLE]        = ">",
    [XWK_QUESTION]      = "?",

    [XWK_UP]    = "UP",
    [XWK_DOWN]  = "DOWN",
    [XWK_LEFT]  = "LEFT",
    [XWK_RIGHT] = "RIGHT",

    [XWK_CAPS_LOCK]   = "CAPS_LOCK",
    [XWK_NUM_LOCK]    = "NUM_LOCK",
    [XWK_SCROLL_LOCK] = "SCROLL_LOCK",
    [XWK_PRINTSCREEN] = "PRINTSCREEN",
    [XWK_SYSREQ]      = "SYSREQ",
    [XWK_PAUSE]       = "PAUSE",
    [XWK_INSERT]      = "INSERT",
    [XWK_DEL]         = "DEL",
    [XWK_HOME]        = "HOME",
    [XWK_END]         = "END",
    [XWK_PGUP]        = "PGUP",
    [XWK_PGDN]        = "PGDN",
    [XWK_LWIN]        = "LWIN",
    [XWK_RWIN]        = "RWIN",
    [XWK_APPS]        = "APPS",

    [XWK_NUM7]         = "NUM7",
    [XWK_NUM8]         = "NUM8",
    [XWK_NUM9]         = "NUM9",
    [XWK_NUM4]         = "NUM4",
    [XWK_NUM5]         = "NUM5",
    [XWK_NUM6]         = "NUM6",
    [XWK_NUM1]         = "NUM1",
    [XWK_NUM2]         = "NUM2",
    [XWK_NUM3]         = "NUM3",
    [XWK_NUM0]         = "NUM0",
    [XWK_NUMPAD_ENTER] = "NUMPAD_ENTER"
};
