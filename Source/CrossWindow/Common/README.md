# CrossWindow/Common

This part contains code that's platform-independent. I cannot think of a shorter and sensible name
than Common. The name can be misleading to some but I can't do anything about it now, it's written
on stone xD

Platform dependent part mostly contains initializers for different types of CrossWindow events.
This is used by all platform-dependent code to initialize the `XwEvent` object with correct data.
