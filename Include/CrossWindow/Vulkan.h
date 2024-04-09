/**
 * @file Window.h 
 * @time 07/04/2024 23:05:25
 * @author Siddharth Mishra (admin@brightprogrammer.in)
 * @copyright Copyright (c) 2024 Siddharth Mishra
 * @copyright Copyright (c) 2024 Anvie Labs
 *
 * @brief Some Vulkan helper methods to create surface and other things.
 * */

#ifndef CROSSWINDOW_VULKAN_H
#define CROSSWINDOW_VULKAN_H

#include <Types.h>
#include <CrossWindow/Window.h>
#include <vulkan/vulkan.h>

/* vulkan specific methods */
CString *xw_get_required_extension_names (Size *ext_count);
VkResult xw_window_create_vulkan_surface (XwWindow *window, VkInstance inst, VkSurfaceKHR *surf);

#endif // CROSSWINDOW_VULKAN_H
