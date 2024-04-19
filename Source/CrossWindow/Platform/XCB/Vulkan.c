/**
 * @file Vulkan.c
 * @time 07/04/2024 20:31:26
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

#include <Anvie/Common.h>
#include <Anvie/CrossWindow/Vulkan.h>

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
