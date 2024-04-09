#include <Common.h>

/* crosswindow */
#include <CrossWindow/Event.h>
#include <CrossWindow/Vulkan.h>
#include <CrossWindow/Window.h>

#define VULKAN_INSTANCE_CREATION_FAILED "Failed to create a Vulkan instance\n"
#define VULKAN_SURFACE_CREATION_FAILED  "Failed to create a Vulkan urface\n"

static const CString layers[] = {"VK_LAYER_KHRONOS_validation"};

int main() {
    const Uint32 width  = 960;
    const Uint32 height = 540;
    XwWindow    *win    = xw_window_create ("Ckeckl", width, height, 10, 20);
    xw_window_set_state (win, XW_WINDOW_STATE_MASK_MAXIMIZED);

    /* extensions required by window */
    Size     window_ext_count = 0;
    CString *window_exts      = xw_get_required_extension_names (&window_ext_count);

    /* extensions required by me */
    CString my_exts[]    = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
    Size    my_ext_count = ARRAY_SIZE (my_exts);

    /* extensions required by all */
    Size    ext_count = window_ext_count + my_ext_count;
    CString exts[ext_count];
    for (Size s = 0; s < window_ext_count; s++) {
        exts[s] = window_exts[s];
    }
    for (Size s = 0; s < my_ext_count; s++) {
        exts[window_ext_count + s] = my_exts[s];
    }

    VkInstanceCreateInfo instance_create_info = {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = Null,
        .flags                   = 0,
        .pApplicationInfo        = Null,
        .enabledLayerCount       = ARRAY_SIZE (layers),
        .ppEnabledLayerNames     = layers,
        .enabledExtensionCount   = ext_count,
        .ppEnabledExtensionNames = exts
    };

    VkInstance instance = VK_NULL_HANDLE;
    VkResult   res      = vkCreateInstance (&instance_create_info, Null, &instance);
    GOTO_HANDLER_IF (res != VK_SUCCESS, INSTANCE_CREATION_FAILED, VULKAN_INSTANCE_CREATION_FAILED);

    VkSurfaceKHR surface;
    res = xw_window_create_vulkan_surface (win, instance, &surface);
    GOTO_HANDLER_IF (res != VK_SUCCESS, SURFACE_CREATION_FAILED, VULKAN_SURFACE_CREATION_FAILED);

    Bool    is_running = True;
    XwEvent e;
    while (is_running) {
        while (xw_event_poll (&e)) {
            switch (e.type) {
                case XW_EVENT_TYPE_CLOSE_WINDOW : {
                    is_running = False;
                    break;
                }
                case XW_EVENT_TYPE_STATE_CHANGE : {
                    XwWindowSize cur_size = xw_window_get_size (win);
                    XwWindowSize max_size = xw_window_get_max_size (win);

                    /* if both masks are set at once then change them both all at once. */
                    if ((e.state_change.new_state & XW_WINDOW_STATE_MASK_MAXIMIZED) ==
                        XW_WINDOW_STATE_MASK_MAXIMIZED) {
                        xw_window_set_size (win, max_size);
                    } else {
                        if (e.state_change.new_state & XW_WINDOW_STATE_MASK_MAXIMIZED_VERT) {
                            xw_window_set_size (
                                win,
                                (XwWindowSize) {cur_size.width, max_size.height}
                            );
                        }
                        if (e.state_change.new_state & XW_WINDOW_STATE_MASK_MAXIMIZED_HORZ) {
                            xw_window_set_size (
                                win,
                                (XwWindowSize) {max_size.width, cur_size.height}
                            );
                        }
                    }
                    break;
                }
                default :
                    break;
            }
        }
    }

    /* TODO : CAP Max window size, Min window size, whether it is resizable or not and so on... */

    vkDestroySurfaceKHR (instance, surface, Null);
    vkDestroyInstance (instance, Null);
    xw_window_destroy (win);

    return EXIT_SUCCESS;

SURFACE_CREATION_FAILED:
    vkDestroyInstance (instance, Null);
INSTANCE_CREATION_FAILED:
    xw_window_destroy (win);
    return EXIT_FAILURE;
}

/********************************** PRIVATE METHODS ****************************************/
