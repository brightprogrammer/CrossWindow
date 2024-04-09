# CrossWindow

CrossWindow is a cross platform window library. I made this because I don't like currently existing solutions
to create cross platform windows and I wanted something that resembles my coding principles.

## Supported Platforms

CrossWindow supports only Xcb for now. I'll first work towards adding support only for Linux.
- [x] Xcb 
- [ ] Xlib
- [ ] Wayland
- [ ] Win32
- [ ] Android
- [ ] Cocoa
- [ ] iOS

The library is not completely in usable state as it is for now. There is now way to install headers or to install
created library/archive files. One can use this by including this as a subdirectory. Also, there are chances of
bugs in the event system. I hope all bugs will be removed automatically with time given we keep using it and improving it.

I also intend to make this library dependency free. Make use of dependencies only if necessary, like my life depends on it!

## Example/Intro

Using CrossWindow is easy to use. You can either take a look into the Examples directory in Sources or take a quick reference below : 

```c
#include <CrossWindow/Window.h>
int main() {
    const Uint32 width = 960;
    const Uint32 height = 540;
    const Uint32 posx = 10,
    const Uint32 posy = 50;
    XwWindow* win = xw_window_create("My Window", width, height, posx, posy);

    XwEvent e;
    Bool is_running = True;
    while(is_running) {
        while(xw_event_poll(&e)) {
            if(e.type == XW_EVENT_TYPE_CLOSE_WINDOW) {
                is_running = False;
                break;
            }

            /* handle other events here */
        }

        /* process changes made by all events here, like draw, clear, motions/animations etc... */
    }

    xw_window_destroy(win);
}
```

The code is well documented in my opinion so once can use it to read and understand what to do further till I add more examples and documentation.

## Contributing

Any help is welcome. I don't have a windows system, so if anyone is willing to add support for Windows platform, I'll be grateful. 
Make sure to follow similar coding style and use the `.clang-format` in project root to auto-format code before pushing changes.
