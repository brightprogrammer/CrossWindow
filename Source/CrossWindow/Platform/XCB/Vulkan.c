#include <Common.h>
#include <CrossWindow/Vulkan.h>

/* local includes */
#include "State.h"
#include "Window.h"

/* vulkan includes */
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_xcb.h>

static CString required_exts[] = {
    VK_KHR_XCB_SURFACE_EXTENSION_NAME,
    VK_KHR_SURFACE_EXTENSION_NAME
};
extern XwState xw_state;

/**
 * @b Get names of Vulkan instance extensions required to create a surface.
 *
 * @param ext_count Pointer to Size variable where size of returned array
 *        will be stored.
 *
 * @return CString array guaranteed. Must not be freed because it's static.
 * */
CString *xw_get_required_extension_names (Size *ext_count) {
    if (ext_count) {
        *ext_count = ARRAY_SIZE (required_exts);
    }

    return required_exts;
}

/**
 * @b Create a new Vulkan surface for given window object.
 *
 * @param window @c XwWindow object to create this Vulkan surface for.
 * @param vki Vulkan instance 
 * @param vks Vulkan surface. This is where result will be stored.
 *
 * @return VkResult returned result from surface creation process.
 * @return VK_ERROR_UNKNOWN if arguments are wrong.
 * */
VkResult xw_window_create_vulkan_surface (XwWindow *window, VkInstance vki, VkSurfaceKHR *vks) {
    RETURN_VALUE_IF (!vks || !vki || !window, VK_ERROR_UNKNOWN, ERR_INVALID_ARGUMENTS);

    VkXcbSurfaceCreateInfoKHR surface_create_info = {
        .sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        .pNext      = Null,
        .flags      = 0,
        .connection = xw_state.connection,
        .window     = window->xcb_window_id
    };

    VkResult res = VK_SUCCESS;
    res          = vkCreateXcbSurfaceKHR (vki, &surface_create_info, Null, vks);
    return res;
}
