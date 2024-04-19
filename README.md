# CrossWindow

[![](https://img.shields.io/badge/Discord-7289DA?style=for-the-badge&logo=discord&logoColor=white)](https://discord.gg/https://discord.gg/J9b45jbAdH) <a href="https://www.buymeacoffee.com/brightprogrammer" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/default-orange.png" alt="Buy Me A Coffee" height="28" width="105"></a>

CrossWindow is (aims to be) a cross platform window library. I made this because I don't like
currently existing solutions to create cross platform windows and I wanted something that
resembles my coding principles.

## Supported Platforms

CrossWindow supports only Xcb for now. I'll first work towards adding support only for Linux.
- [x] Xcb (linux) 
- [ ] Xlib (linux)
- [ ] Wayland (linux)
- [ ] WinNT (windows)
- [ ] Android (android)
- [ ] MacOS (mac)
- [ ] iOS (mac)

I intend to make this library dependency free. Make use of dependencies only if necessary, like
my life depends on it!

## Example/Intro

Using CrossWindow is easy to use. You can either take a look into the Examples directory in Sources
or take a quick reference below : 

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
            is_running = e.type != XW_EVENT_TYPE_CLOSE_WINDOW;
            /* handle other events here */
        }

        /* process changes made by all events here, like draw, clear, motions/animations etc... */
    }

    xw_window_destroy(win);
}
```

The code is well documented in my opinion so once can use it to read and understand what to do further
till I add more examples and documentation.

## Build 

To build and install CrossWindow as a library on your system, follow the following steps :
- Create a build directory where you want to build the library. I usually name this as `Build` inside the project root itself.
- Go inside `Build` directory and open a terminal, or you can follow any other method to go to that directory inside the terminal.
- Run the following command :
    - If you have Ninja installed on your system, then use it, it's better : `cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release`
    - If you don't have Ninja installed, then we'll just use GNU Make : `cmake .. -DCMAKE_BUILD_TYPE=Release`
- The last step will generate build files needed for building the project, and now you can run :
    - `ninja` if you have Ninja installed
    - `make -j${nproc}` if you have make installed. You can run just `make` as well. The only difference is number of threads provided for building the project.
- The last step will build the project, and now you can install it using :
    - `sudo ninja install` if you have Ninja 
    - `sudo make install` if you have GNU Make.

## Contributing

Any help is welcome. I don't have a windows system, so if anyone is willing to add support for Windows
platform, I'll be grateful. Make sure to follow similar coding style as explained in 
[Contribution Guide](CONTRIBUTING.md).
