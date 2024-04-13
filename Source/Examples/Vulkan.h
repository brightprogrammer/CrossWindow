#ifndef CROSSWINDOW_EXAMPLE_VULKAN_H
#define CROSSWINDOW_EXAMPLE_VULKAN_H

#include <Types.h>
#include <vulkan/vulkan.h>

typedef struct Vulkan {
    VkInstance        instance;  /**< @b Our connection with vulkan */
    Uint32            gpu_count; /**< @b Total number of usable physical devices on host. */
    VkPhysicalDevice *gpus;      /**< @b GPU handles */
} Vulkan;

Vulkan *vk_create();
Vulkan *vk_init (Vulkan *vk);
Vulkan *vk_deinit (Vulkan *vk);
void    vk_destroy (Vulkan *vk);

#endif // CROSSWINDOW_EXAMPLE_VULKAN_H
