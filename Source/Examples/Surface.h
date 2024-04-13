#ifndef CROSSWINDOW_EXAMPLE_SURFACE_H
#define CROSSWINDOW_EXAMPLE_SURFACE_H

#include "Vulkan.h"

#include <CrossWindow/Window.h>

typedef struct Surface {
    VkPhysicalDevice selected_gpu;          /**< @b Selected GPU for rendering to this surface. */
    Int32            graphics_family_index; /**< @b Index of graphics queue family */
    VkDevice         device;         /**< @b Created logical device for rendering to this surface */
    VkQueue          graphics_queue; /**< @b Graphics queue handle after creating logical device */

    VkSurfaceKHR surface;            /**< @b Surface created for corrsponding XwWindow. */

    VkSwapchainKHR swapchain;        /**< @b Swapchain created for the window. */
    VkExtent2D     swapchain_image_extent; /**< @b Current swapchain image extent */
    VkFormat swapchain_image_format;    /**< @b Format of image stored during swapchain creation. */
    Uint32   swapchain_image_count;     /**< @b Number of images in swapchain. */
    VkImage *swapchain_images;          /**< @b Handle to images inside swapchain. */
    VkImageView *swapchain_image_views; /**< @b Image views created for images in swapchain. */

    VkCommandPool   cmd_pool;
    VkCommandBuffer cmd_buffer;

    VkRenderPass   render_pass;
    VkFramebuffer *framebuffers;

    VkSemaphore render_semaphore;
    VkSemaphore present_semaphore;
    VkFence     render_fence;
} Surface;

Surface *surface_create (Vulkan *vk, XwWindow *xw_win);
Surface *surface_init (Surface *surf, Vulkan *vk, XwWindow *xw_win);
Surface *surface_deinit (Surface *surf, Vulkan *vk);
void     surface_destroy (Surface *surf, Vulkan *vk);
Surface *surface_recreate_swapchain (Surface *surf, XwWindow *win);
Surface *surface_wait_for_pending_operations (Surface *surface);

#endif // CROSSWINDOW_EXAMPLE_SURFACE_H
