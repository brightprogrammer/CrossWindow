# CrossWindow/Platform

This section of code contains platform dependent part of creating windows. Platform dependent code
needs to handle the following things which are separated in their own headers and sources :   
- Conversion of platform-dependent events to CrossWindow events.
- Wrapping platform-dependent window interaction methods with CrossWindow cross-platform API.
- Providing required Vulkan extensions and a function to create surface.
- Some platforms (like Linux) need to make a connection to their compositor. This is handled in
  a global state variable. This is named `XwState` in code, and the name of global variable is
  `xw_state`. This part of code is not visible at all to the user.

One does not need to call a method like `xw_init(...)` or `xw_deinit(...)` because that's
automatically called for the user as soon as the library loads. These methods do exist, but are
marked as `__attribute__((constructor))` and `__attribute__((destructor))` respectively. This means,
user just needs to take care of the window they create and the library will take care of global
objects it creates. If in any case the library fails to make a connection to the compositor, then
it'll simply call `abort` or `exit` to terminate the application instantly. This may be changed
in future if it does not feel right later on ;-)
