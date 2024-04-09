/**
 * @file Event.h 
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

#ifndef CROSSWINDOW_EVENT_H
#define CROSSWINDOW_EVENT_H

#include <CrossWindow/Window.h>
#include <Types.h>

/**
 * @b Various types of events in CrossWindow.
 * */
typedef enum XwEventType {
    XW_EVENT_TYPE_NONE = 0,
    XW_EVENT_TYPE_STATE_CHANGE, /**< @b State like hidden, maximized etc... @sa XwWindowStateMask */
    XW_EVENT_TYPE_CLOSE_WINDOW, /**< @b Closing a window. */
    XW_EVENT_TYPE_VISIBILITY,   /**< @b Window is visible or invisible suddenly */
    XW_EVENT_TYPE_ENTER,        /**< @b Mouse enter the window. Focus is not gained yet. */
    XW_EVENT_TYPE_LEAVE,        /**< @b Mouse leaves the window. Focus may not be lost yet. */
    XW_EVENT_TYPE_FOCUS,        /**< @b Focus/Unfocus on a window. */
    XW_EVENT_TYPE_PAINT,        /**< @b Repaint window (some region of window just got damaged). */
    XW_EVENT_TYPE_BORDER_WIDTH_CHANGE, /**< @b Border width just changed. */
    XW_EVENT_TYPE_REPOSITION,          /**< @b Change position of a window. */
    XW_EVENT_TYPE_RESIZE,              /**< @b Resizing a window. */
    XW_EVENT_TYPE_RESTACK,             /**< @b Window stack ordering changed. */

    /**
     * (NOT TESTED)
     * @b Change in the screen DPI scaling (such as moving a window to a monitor
     * with a larger DPI.
     * */
    XW_EVENT_TYPE_DPI_CHANGE,

    XW_EVENT_TYPE_KEYBOARD_INPUT, /**< @b Keyboard input such as press/release events. */
    XW_EVENT_TYPE_MOUSE_MOVE,     /**< @b Mouse moving events. */
    XW_EVENT_TYPE_MOUSE_WHEEL,    /**< @b Mouse scrolling events. */
    XW_EVENT_TYPE_MOUSE_INPUT,    /**< @b Mouse button press events. */
    XW_EVENT_TYPE_TOUCH,          /**< @b (NOT TESTED) Touch events. */
    XW_EVENT_TYPE_GAMEPAD, /**< @b (NOT TESTED) Gamepad Input Events such as analog sticks, button presses. */
    XW_EVENT_TYPE_DROP_FILE,  /**< @b TODO: Dropping a file on the window. */
    XW_EVENT_TYPE_HOVER_FILE, /**< @b TODO: Hovering a file over a window. */

    XW_EVENT_TYPE_MAX
} XwEventType;

/**
 * @b Window state change.
 *
 * @sa XwWindowStateMask
 * @sa XwWindowState
 * */
typedef struct XwStateChangeEvent {
    XwWindowState new_state; /**< @b New window state. */
} XwStateChangeEvent;

/**
 * @b Window's visibility just changed.
 * */
typedef struct XwVisibilityEvent {
    Bool visible; /**< @b True if window is visible, @c False otherwise. */
} XwVisibilityEvent;

/**
 * @b Mouse either enters or leaves window boundaries.
 * */
typedef struct XwEnterEvent {
    Uint32 x; /**< @b X-coordinate of entering/leaving window. */
    Uint32 y; /**< @b Y-coordinate of entering/leaving window. */
} XwEnterEvent, XwLeaveEvent;

/**
 * @b Focus data passed with Focus events
 * */
typedef struct XwFocusEvent {
    Bool focused; /**< @b @b True if focused, @b False otherwise. */
} XwFocusEvent;

/**
 * @b Restacking of windows.
 *
 * @c above is pointer to window above which the window for which this event was generated
 * is stacked to. This means the @c above window might be hidden by the window for which 
 * the event was generated.
 * */
typedef struct XwRestackEvent {
    XwWindow *above; /**< Sibling window above which the event window is restacked. */
} XwRestackEvent;

/**
 * @b Window position was changed.
 * */
typedef struct XwRepositionEvent {
    Uint32 x; /**< @b New x position */
    Uint32 y; /**< @b New y position */
} XwRepositionEvent;

/**
 * @b Border width of window changed.
 * */
typedef struct XwBorderWidthChangeEvent {
    Uint32 border_width; /**< @b New border width. */
} XwBorderWidthChangeEvent;

/**
 * @b Resize data passed with Resize events
 * */
typedef struct XwResizeEvent {
    Uint32 width;  /**< @b New width of window */
    Uint32 height; /**< @b New height of window */
} XwResizeEvent;

/**
 * @b DPI data passed with DPI events
 */
typedef struct XwDpiChangeEvent {
    Float32 scale; /**< @b DPI Scaling info. */
} XwDpiChangeEvent;

/**
 * @b The state of a button press, be it keyboard, mouse, etc.
 * */
typedef enum XwButtonState : Bool {
    XW_BUTTON_STATE_UNKOWN = 0,
    XW_BUTTON_STATE_RELEASED,
    XW_BUTTON_STATE_PRESSED,
} XwButtonState;

/**
 * @b Modifier key states.
 *
 * The state of modifier keys such as ctrl, alt, shift, and the windows/command
 * buttons. Pressed is true, released is false;
 */
typedef struct XwModifierState {
    XwButtonState ctrl;
    XwButtonState alt;
    XwButtonState caps_lock;
    XwButtonState num_lock;
    XwButtonState shift;
    XwButtonState meta; /**< @b Meta buttons such as the Windows button or Mac's Command button. */
} XwModifierState;

/**
 * @b Key event enum
 * */
typedef enum XwKey {
    XWK_UNKNOWN = 0,
    /* digits */
    XWK_1,
    XWK_2,
    XWK_3,
    XWK_4,
    XWK_5,
    XWK_6,
    XWK_7,
    XWK_8,
    XWK_9,
    XWK_0,

    /* alphabets */
    XWK_A,
    XWK_B,
    XWK_C,
    XWK_D,
    XWK_E,
    XWK_F,
    XWK_G,
    XWK_H,
    XWK_I,
    XWK_J,
    XWK_K,
    XWK_L,
    XWK_M,
    XWK_N,
    XWK_O,
    XWK_P,
    XWK_Q,
    XWK_R,
    XWK_S,
    XWK_T,
    XWK_U,
    XWK_V,
    XWK_W,
    XWK_X,
    XWK_Y,
    XWK_Z,

    XWK_a,
    XWK_b,
    XWK_c,
    XWK_d,
    XWK_e,
    XWK_f,
    XWK_g,
    XWK_h,
    XWK_i,
    XWK_j,
    XWK_k,
    XWK_l,
    XWK_m,
    XWK_n,
    XWK_o,
    XWK_p,
    XWK_q,
    XWK_r,
    XWK_s,
    XWK_t,
    XWK_u,
    XWK_v,
    XWK_w,
    XWK_x,
    XWK_y,
    XWK_z,

    /* function keys */
    XWK_F1,
    XWK_F2,
    XWK_F3,
    XWK_F4,
    XWK_F5,
    XWK_F6,
    XWK_F7,
    XWK_F8,
    XWK_F9,
    XWK_F10,
    XWK_F11,
    XWK_F12,

    XWK_ESCAPE,
    XWK_BACKSPACE,
    XWK_TAB,
    XWK_ENTER,
    XWK_DECIMAL,

    XWK_LCONTROL,
    XWK_RCONTROL,
    XWK_LSHIFT,
    XWK_RSHIFT,
    XWK_LALT,
    XWK_RALT,

    XWK_EQUALS,
    XWK_ADD,
    XWK_SUBTRACT,
    XWK_MULTIPLY,
    XWK_DIVIDE,

    XWK_TILDE,
    XWK_GRAVE_ACCENT,
    XWK_EXCLAMATION,
    XWK_AT,
    XWK_HASH,
    XWK_CURRENCY,
    XWK_PERCENT,
    XWK_HAT, /* ^ character */
    XWK_AND,
    XWK_STAR,
    XWK_LPAREN,
    XWK_RPAREN,

    XWK_HYPHEN,
    XWK_SEMICOLON,
    XWK_COLON,
    XWK_SINGLE_QUOTE,
    XWK_DOUBLE_QUOTES,
    XWK_BACK_SLASH,
    XWK_FWD_SLASH,
    XWK_PIPE,
    XWK_COMMA,
    XWK_PERIOD,
    XWK_SPACE,
    XWK_LBRACKET,
    XWK_RBRACKET,
    XWK_LBRACE,
    XWK_RBRACE,
    XWK_LANGLE,
    XWK_RANGLE,
    XWK_QUESTION,

    XWK_UP,
    XWK_DOWN,
    XWK_LEFT,
    XWK_RIGHT,

    XWK_CAPS_LOCK,
    XWK_NUM_LOCK,
    XWK_SCROLL_LOCK,
    XWK_PRINTSCREEN,
    XWK_SYSREQ,
    XWK_PAUSE,
    XWK_INSERT,
    XWK_DEL,
    XWK_HOME,
    XWK_END,
    XWK_PGUP,
    XWK_PGDN,
    XWK_LWIN,
    XWK_RWIN,
    XWK_APPS,

    XWK_NUM7,
    XWK_NUM8,
    XWK_NUM9,
    XWK_NUM4,
    XWK_NUM5,
    XWK_NUM6,
    XWK_NUM1,
    XWK_NUM2,
    XWK_NUM3,
    XWK_NUM0,
    XWK_NUMPAD_ENTER,

    XWK_MAX
} XwKey;

CString xw_key_cstr (XwKey key, Bool caps_case);

/**
 * @b Data sent during keyboard events
 * */
typedef struct XwKeyboardInputEvent {
    XwKey           key;   /**< @b Which key? */
    XwButtonState   state; /**< @b Input state. */
    XwModifierState mod;   /**< @b Input modifiers. */
} XwKeyboardInputEvent;

/**
 * The event data passed with mouse events click, mouse moving events
 */
typedef struct XwMouseMoveEvent {
    Uint32 x;  /**< @b Current x position relative to active window. */
    Uint32 y;  /**< @b Current y position relative to active window. */
    Int32  dx; /**< @b Change in x relative to previous event, used for FPS motion. */
    Int32  dy; /**< @b Change in y relative to previous event, used for FPS motion. */
} XwMouseMoveEvent;

/**
 * @b To be used as index into the @c XwMouseInputEvent::state
 * */
typedef enum XwMouseButtonMask {
    XW_MOUSE_BUTTON_MASK_UNKNOWN = 0,
    XW_MOUSE_BUTTON_MASK_LEFT    = (1 << 0),
    XW_MOUSE_BUTTON_MASK_RIGHT   = (1 << 1),
    XW_MOUSE_BUTTON_MASK_MIDDLE  = (1 << 2),
    XW_MOUSE_BUTTON_MASK_BUTTON4 = (1 << 3),
    XW_MOUSE_BUTTON_MASK_BUTTON5 = (1 << 4),
    XW_MOUSE_BUTTON_MASK_MAX
} XwMouseButtonMask;

typedef Uint8 XwMouseButtonState;

/**
 * @b Data passed with mouse input events
 * */
typedef struct XwMouseInputEvent {
    XwMouseButtonState button_state; /**< @b Pressed buttons are masked in state. */
    XwModifierState    mod;          /**< @b Modifiers applied during click. */
    Uint32             x;            /**< @b Coordinate relative to window in pixels. */
    Uint32             y;            /**< @b Coordinate relative to window in pixels. */
} XwMouseInputEvent;

/**
 * @b Data passed with mouse wheel events
 * */
typedef struct XwMouseWheelEvent {
    Uint32          x;         /**< @b X position of scroll. */
    Uint32          y;         /**< @b Y position of scroll. */
    Bool            direction; /**< @b True if scrolling up, and False if scrolling down. */
    XwModifierState mod;       /**< @b Modifiers applied when mouse wheel scrolled. */
} XwMouseWheelEvent;

/**
 * @b Touch point data
 * */
typedef struct XwTouchPoint {
    Size   id;         /**< @b A unique id for each touch point. */
    Uint32 x;          /**< @b Touch coordinate relative to window in pixels. */
    Uint32 y;          /**< @b Touch coordinate relative to window in pixels. */
    Bool   is_changed; /**< @b Did the touch point change. */
} XwTouchPoint;

#define XW_TOUCH_COUNT_MAX 32

/**
 * @b Data passed for touch events
 * */
typedef struct XwTouchEvent {
    Uint32       touch_count;
    XwTouchPoint touches[XW_TOUCH_COUNT_MAX];
} XwTouchEvent;

/**
 * @b Gamepad Button pressed enum
 * */
typedef enum XwGamepadButton {
    XW_GAMEPAD_BUTTON_DPAD_UP = 0,
    XW_GAMEPAD_BUTTON_DPAD_DOWN,
    XW_GAMEPAD_BUTTON_DPAD_LEFT,
    XW_GAMEPAD_BUTTON_DPAD_RIGHT,
    XW_GAMEPAD_BUTTON_START,
    XW_GAMEPAD_BUTTON_BACK,
    XW_GAMEPAD_BUTTON_LTHUMB_CLICK,
    XW_GAMEPAD_BUTTON_RTHUMB_CLICK,
    XW_GAMEPAD_BUTTON_LSHOULDER,
    XW_GAMEPAD_BUTTON_RSHOULDER,
    XW_GAMEPAD_BUTTON_A,
    XW_GAMEPAD_BUTTON_B,
    XW_GAMEPAD_BUTTON_X,
    XW_GAMEPAD_BUTTON_Y,
    XW_GAMEPAD_BUTTON_MAX
} XwGamepadButton;

/**
 * Gamepad analog stick enum
 */
typedef enum XwAnalogInput {
    // gamepad
    XW_ANALOG_INPUT_LEFT_TRIGGER,
    XW_ANALOG_INPUT_RIGHT_TRIGGER,
    XW_ANALOG_INPUT_LEFT_STICKX,
    XW_ANALOG_INPUT_LEFT_STICKY,
    XW_ANALOG_INPUT_RIGHT_STICKX,
    XW_ANALOG_INPUT_RIGHT_STICKY,

    // mouse
    XW_ANALOG_INPUT_MOUSEX,
    XW_ANALOG_INPUT_MOUSEY,
    XW_ANALOG_INPUT_MOUSE_SCROLL,

    XW_ANALOG_INPUTS_MAX
} XwAnalogInput;

#define XW_GAMEPAD_AXES_COUNT_MAX   (Size)64
#define XW_GAMEPAD_BUTTON_COUNT_MAX (Size)64

/**
 * @b Data passed for gamepad events
 * */
typedef struct XwGamepadEvent {
    Bool    connected;  /**< @b If the gamepad is connected or not. */
    Size    index;      /**< @b Gamepad Index. */
    CString id;         /**< @b String id of the brand of the gamepad. */
    CString mapping;    /**< @b String id that lays out controller mapping (Southpaw, etc.). */
    Uint32  axes_count; /**< @b The number of analog axes. */
    /** @b Analog Axis data, such as joysticks, normalized range [-1, 1]. */
    Float64 axis[XW_GAMEPAD_AXES_COUNT_MAX];
    Uint32  button_count; /**< @b Number of buttons */
    /** @b Analog gamepad buttons like triggers, bound to [0, 1]. */
    Float64 analog_button[XW_GAMEPAD_BUTTON_COUNT_MAX];
    /** @b Number of digital buttons and analog buttons. */
    Bool digital_button[XW_GAMEPAD_BUTTON_COUNT_MAX];
} XwGamepadEvent;

/**
 * @b Represents an event in CrossWindow.
 *
 * SDL does something similar:
 * <https://www.libsdl.org/release/SDL-1.2.15/docs/html/sdlevent.html>
 */
typedef struct XwEvent {
    XwEventType type;   /**< @b Type of XwEvent */
    XwWindow   *window; /**< @b Pointer to a CrossWindow window. */

    union {
        XwStateChangeEvent       state_change;
        XwVisibilityEvent        visibility;
        XwEnterEvent             enter;
        XwLeaveEvent             leave;
        XwFocusEvent             focus;
        XwBorderWidthChangeEvent border_width_change;
        XwRepositionEvent        reposition;
        XwResizeEvent            resize;
        XwRestackEvent           restack;
        XwDpiChangeEvent         dpi_change;
        XwKeyboardInputEvent     keyboard_input;
        XwMouseMoveEvent         mouse_move;
        XwMouseInputEvent        mouse_input;
        XwMouseWheelEvent        mouse_wheel;
        XwTouchEvent             touch;
        XwGamepadEvent           gamepad;
    };
} XwEvent;

XwEvent *xw_event_poll (XwEvent *event);
XwEvent *xw_event_wait (XwEvent *event);

XwEvent *xw_event_state_change (XwEvent *event, XwWindowState new_state, XwWindow *win);
XwEvent *xw_event_visibility (XwEvent *event, Bool visible, XwWindow *win);
XwEvent *xw_event_close_window (XwEvent *event, XwWindow *win);
XwEvent *xw_event_enter (XwEvent *event, Uint32 x, Uint32 y, XwWindow *win);
XwEvent *xw_event_leave (XwEvent *event, Uint32 x, Uint32 y, XwWindow *win);
XwEvent *xw_event_focus (XwEvent *event, Bool focused, XwWindow *win);
XwEvent *xw_event_reposition (XwEvent *event, Uint32 x, Uint32 y, XwWindow *win);
XwEvent *xw_event_border_width_change (XwEvent *event, Uint32 w, XwWindow *win);
XwEvent *xw_event_resize (XwEvent *event, Uint32 width, Uint32 height, XwWindow *win);
XwEvent *xw_event_restack (XwEvent *event, XwWindow *above, XwWindow *win);
XwEvent *xw_event_paint (XwEvent *event, XwWindow *window);
XwEvent *xw_event_dpi_change (XwEvent *event, Float32 scale, XwWindow *win);
XwEvent *xw_event_keyboard_input (
    XwEvent        *event,
    XwKey           key,
    XwButtonState   state,
    XwModifierState mod,
    XwWindow       *win
);
XwEvent *
    xw_event_mouse_move (XwEvent *event, Uint32 x, Uint32 y, Int32 dx, Int32 dy, XwWindow *win);
XwEvent *xw_event_mouse_input (
    XwEvent           *event,
    XwMouseButtonState state,
    Uint32             x,
    Uint32             y,
    XwModifierState    mod,
    XwWindow          *win
);
XwEvent *xw_event_mouse_wheel (
    XwEvent        *event,
    Uint32          x,
    Uint32          y,
    Bool            direction,
    XwModifierState mod,
    XwWindow       *win
);
XwEvent *xw_event_touch (XwEvent *event, Size touch_count, XwTouchPoint *points, XwWindow *win);
XwEvent *xw_event_gamepad (
    XwEvent  *event,
    Bool      connected,
    Size      index,
    CString   id,
    CString   mapping,
    Uint32    num_axes,
    Float64  *axis,
    Uint32    button_count,
    Float64  *analog_button,
    Bool     *digital_button,
    XwWindow *win
);
XwEvent *xw_event_drop_file (XwEvent *event, XwWindow *win);
XwEvent *xw_event_hover_file (XwEvent *event, XwWindow *win);

#endif // CROSSWINDOW_EVENT_H
