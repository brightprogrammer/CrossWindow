#include <Anvie/Common.h>

/* crosswindow */
#include <Anvie/CrossWindow/Event.h>
#include <Anvie/CrossWindow/Vulkan.h>
#include <Anvie/CrossWindow/Window.h>

/* libc */
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

/**************************************************************************************************/
/********************************************* VULKAN *********************************************/
/**************************************************************************************************/

typedef struct Vulkan {
    VkInstance        instance;  /**< @b Our connection with vulkan */
    Uint32            gpu_count; /**< @b Total number of usable physical devices on host. */
    VkPhysicalDevice *gpus;      /**< @b GPU handles */
} Vulkan;

Vulkan *vk_create();
Vulkan *vk_init (Vulkan *vk);
Vulkan *vk_deinit (Vulkan *vk);
void    vk_destroy (Vulkan *vk);

/**************************************************************************************************/
/******************************************** SURFACE *********************************************/
/**************************************************************************************************/

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

    VkPipeline       pipeline;
    VkPipelineLayout pipeline_layout;
} Surface;

Surface *surface_create (Vulkan *vk, XwWindow *xw_win);
Surface *surface_init (Surface *surf, Vulkan *vk, XwWindow *xw_win);
Surface *surface_deinit (Surface *surf, Vulkan *vk);
void     surface_destroy (Surface *surf, Vulkan *vk);
Surface *surface_recreate_swapchain (Surface *surf, XwWindow *win);
Surface *surface_wait_for_pending_operations (Surface *surface);

/**************************************************************************************************/
/********************************************** Math **********************************************/
/**************************************************************************************************/

typedef struct Position2D {
    Float32 x;
    Float32 y;
} Position2D;

typedef struct Color {
    Float32 r;
    Float32 g;
    Float32 b;
    Float32 a;
} Color;

typedef struct Vertex2D {
    Position2D position;
    Color      color;
} Vertex2D;

static const Vertex2D triangle_vertices[] = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.f}},
    {{-0.5f, 0.5f}, {0.0f, 1.0f, 0.0f, 1.f}},
    { {0.5f, 0.5f}, {0.0f, 0.0f, 1.0f, 1.f}}
};

/**************************************************************************************************/
/***************************************** BUFFER OBJECT ******************************************/
/**************************************************************************************************/

typedef struct BufferObject {
    VkBuffer       buffer;
    VkDeviceMemory memory;
} BufferObject;

BufferObject buffer_object_create (
    VkPhysicalDevice   gpu,
    VkDevice           device,
    VkBufferUsageFlags usage,
    Size               size,
    Uint32             queue_family_inddex
);
void         buffer_object_destroy (BufferObject bo, VkDevice device);
BufferObject buffer_object_upload_data (BufferObject bo, VkDevice device, void *data, Size size);

/**************************************************************************************************/
/************************************************  ************************************************/
/**************************************************************************************************/

int main() {
    const Uint32 width  = 360;
    const Uint32 height = 240;
    XwWindow    *win    = xw_window_create ("Ckeckl", width, height, 10, 20);

    Vulkan *vk = vk_create();
    GOTO_HANDLER_IF (!vk, VK_INIT_FAILED, "Failed to create Vulkan\n");

    Surface *surface = surface_create (vk, win);
    GOTO_HANDLER_IF (!surface, SURFACE_FAILED, "Failed to create Surface\n");

    BufferObject vbo = buffer_object_create (
        surface->selected_gpu,
        surface->device,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        sizeof (triangle_vertices),
        surface->graphics_family_index
    );

    buffer_object_upload_data (
        vbo,
        surface->device,
        (void *)triangle_vertices,
        sizeof (triangle_vertices)
    );

    /* event handlign looop */
    Bool    is_running = True;
    XwEvent e;
    Uint64  framenum = 0;
    while (is_running) {
        Bool resized = False;
        while (xw_event_poll (&e)) {
            switch (e.type) {
                case XW_EVENT_TYPE_CLOSE_WINDOW : {
                    is_running = False;
                    break;
                }
                case XW_EVENT_TYPE_RESIZE : {
                    resized = True;
                    break;
                }
                default :
                    break;
            }
        }

        VkFence fences[] = {surface->render_fence};

        /* wait for all gpu rendering to complete */
        VkResult res = vkWaitForFences (surface->device, ARRAY_SIZE (fences), fences, True, 1e9);
        GOTO_HANDLER_IF (
            res != VK_SUCCESS,
            DRAW_ERROR,
            "Timeout (1s) while waiting for fences. RET = %d\n",
            res
        );

        /* get next image index */
        Uint32 next_image_index = -1;

        res = vkAcquireNextImageKHR (
            surface->device,
            surface->swapchain,
            1e9,
            surface->present_semaphore,
            Null,
            &next_image_index
        );
        if (resized || res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR) {
            GOTO_HANDLER_IF (
                !surface_recreate_swapchain (surface, win),
                DRAW_ERROR,
                "Failed to recreate swapchain\n"
            );
            continue;
            res = VK_SUCCESS; /* to pass upcoming check */
        }
        GOTO_HANDLER_IF (
            res != VK_SUCCESS,
            DRAW_ERROR,
            "Failed to get next image index from swapchain. RET = %d\n",
            res
        );

        /* need to reset fence before we use it again */
        res = vkResetFences (surface->device, ARRAY_SIZE (fences), fences);
        GOTO_HANDLER_IF (res != VK_SUCCESS, DRAW_ERROR, "Failed to reset fences. RET = %d\n", res);

        /* reset command buffer and record draw commands again */
        res = vkResetCommandBuffer (surface->cmd_buffer, 0);
        GOTO_HANDLER_IF (
            res != VK_SUCCESS,
            DRAW_ERROR,
            "Failed to reset command buffer for recording new commands. RET = %d\n",
            res
        );

        VkCommandBuffer          cmd            = surface->cmd_buffer;
        VkCommandBufferBeginInfo cmd_begin_info = {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext            = Null,
            .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = Null
        };

        res = vkBeginCommandBuffer (cmd, &cmd_begin_info);
        GOTO_HANDLER_IF (
            res != VK_SUCCESS,
            DRAW_ERROR,
            "Failed to begin command buffer recording. RET = %d\n",
            res
        );

        VkClearValue clear_value = {
            .color =
                {{sin (framenum / 1000.f) * 0.3f,
                  cos (framenum / 1000.f) * 0.3f,
                  fabs (sin (framenum / 1000.f) - cos (framenum / 1000.f)) * 0.3f,
                  1}}
        };

        VkRenderPassBeginInfo render_pass_begin_info = {
            .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext       = Null,
            .renderPass  = surface->render_pass,
            .renderArea  = {.offset = {.x = 0, .y = 0}, .extent = surface->swapchain_image_extent},
            .framebuffer = surface->framebuffers[next_image_index],
            .clearValueCount = 1,
            .pClearValues    = &clear_value
        };

        vkCmdBeginRenderPass (cmd, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline (cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, surface->pipeline);
        Size offset = 0;
        vkCmdBindVertexBuffers (cmd, 0, 1, &vbo.buffer, &offset);
        vkCmdDraw (cmd, 3, 1, 0, 0);
        vkCmdEndRenderPass (cmd);

        res = vkEndCommandBuffer (cmd);
        GOTO_HANDLER_IF (
            res != VK_SUCCESS,
            DRAW_ERROR,
            "Failed to end command buffer recording. RET = %d\n",
            res
        );

        /* wait when rendered image is being presented */
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submit_info = {
            .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext                = Null,
            .waitSemaphoreCount   = 1,
            .pWaitSemaphores      = &surface->present_semaphore,
            .pWaitDstStageMask    = &wait_stage,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores    = &surface->render_semaphore,
            .commandBufferCount   = 1,
            .pCommandBuffers      = &cmd
        };

        res = vkQueueSubmit (surface->graphics_queue, 1, &submit_info, surface->render_fence);
        GOTO_HANDLER_IF (
            res != VK_SUCCESS,
            DRAW_ERROR,
            "Failed to submit command buffers for execution. RET = %d\n",
            res
        );

        VkPresentInfoKHR present_info = {
            .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext              = Null,
            .swapchainCount     = 1,
            .pSwapchains        = &surface->swapchain,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores    = &surface->render_semaphore,
            .pImageIndices      = &next_image_index
        };

        res = vkQueuePresentKHR (surface->graphics_queue, &present_info);
        if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR) {
            GOTO_HANDLER_IF (
                !surface_recreate_swapchain (surface, win),
                DRAW_ERROR,
                "Failed to recreate swapchain\n"
            );
            res = VK_SUCCESS; /* so that next check does not fail */
        }
        GOTO_HANDLER_IF (
            res != VK_SUCCESS,
            DRAW_ERROR,
            "Failed to present rendered images to surface. RET = %d\n",
            res
        );

        framenum++;
    }

    buffer_object_destroy (vbo, surface->device);
    surface_destroy (surface, vk);
    vk_destroy (vk);
    xw_window_destroy (win);

    return EXIT_SUCCESS;

DRAW_ERROR:
    surface_destroy (surface, vk);
SURFACE_FAILED:
    vk_destroy (vk);
VK_INIT_FAILED:
    xw_window_destroy (win);
    return EXIT_FAILURE;
}

/**************************************************************************************************/
/********************************** PRIVATE METHOD DELCARATIONS ***********************************/
/**************************************************************************************************/

/* vulkan private methods */
static inline Vulkan *vk_create_instance (Vulkan *vk);
static inline Vulkan *vk_enumerate_gpus (Vulkan *vk);

/* surface private methods */
static inline Surface *surface_create_surface (Surface *surface, Vulkan *vk, XwWindow *win);
static inline Surface *surface_select_gpu (Surface *surface, Vulkan *vk);
static inline Surface *surface_fetch_queue_family_indices (Surface *surface);
static inline Surface *surface_create_logical_device (Surface *surface);
static inline Surface *surface_create_swapchain (Surface *surface, XwWindow *win);
static inline Surface *surface_fetch_swapchain_images (Surface *surface);
static inline Surface *surface_create_swapchain_image_views (Surface *surface);
static inline Surface *surface_create_command_objects (Surface *surface);
static inline Surface *surface_create_renderpass (Surface *surface);
static inline Surface *surface_create_framebuffers (Surface *surface);
static inline Surface *surface_create_sync_objects (Surface *surface);
static inline Surface *surface_create_pipeline_layout (Surface *surface);
static inline Surface *surface_create_pipeline (Surface *surface);

/* private helper methods */
static inline const CString *get_instance_layer_names (Size *count);
static inline const CString *get_instance_extension_names (Size *count);
static inline Int32 find_queue_family_index (VkPhysicalDevice gpu, VkQueueFlags queue_flags);
static inline VkShaderModule load_shader (VkDevice device, CString path);


/**************************************************************************************************/
/******************************** VULKAN PUBLIC METHOD DEFINITIONS ********************************/
/**************************************************************************************************/

/**
 * @b Create a new Vulkan object.
 *
 * @return @c Vulkan* on success.
 * @return @c Null otherwise.
 * */
Vulkan *vk_create() {
    Vulkan *vk = NEW (Vulkan);
    RETURN_VALUE_IF (!vk, Null, ERR_INVALID_ARGUMENTS);

    Vulkan *ivk = vk_init (vk);
    GOTO_HANDLER_IF (!ivk, INIT_FAILED, ERR_OBJECT_INITIALIZATION_FAILED);

    return ivk;

INIT_FAILED:
    vk_destroy (vk);
    return Null;
}

/**
 * @b Initialize the Vulkan state.
 *
 * @param vk
 *
 * @return @c vk On success.
 * @return Null on failure.
 * */
Vulkan *vk_init (Vulkan *vk) {
    RETURN_VALUE_IF (!vk, Null, ERR_INVALID_ARGUMENTS);

    GOTO_HANDLER_IF (
        !vk_create_instance (vk) || !vk_enumerate_gpus (vk),
        INIT_FAILED,
        "Failed to initialize Vulkan\n"
    );

    return vk;

    /* error handlers */
INIT_FAILED:
    vk_deinit (vk);
    return Null;
}

/**
 * @b Opposite call to @c vk_init. This shuts down vulkan connection.
 *    Destroys everything that was ever created! Complete havoc.
 *
 * @param vk Vulkan state.
 *
 * @return @c vk on success.
 * @return @c Null otherwise.
 * */
Vulkan *vk_deinit (Vulkan *vk) {
    RETURN_VALUE_IF (!vk, Null, ERR_INVALID_ARGUMENTS);

    if (vk->gpus) {
        FREE (vk->gpus);
        vk->gpus = Null;
    }

    if (vk->instance) {
        vkDestroyInstance (vk->instance, Null);
    }

    memset (vk, 0, sizeof (Vulkan));

    return vk;
}

/**
 * @b Deinit and destroy (free) given Vulkan object.
 *
 * @param vk
 * */
void vk_destroy (Vulkan *vk) {
    RETURN_IF (!vk, ERR_INVALID_ARGUMENTS);

    vk_deinit (vk);
    FREE (vk);
}

/**************************************************************************************************/
/******************************* SURFACE PUBLIC METHOD DEFINITIONS ********************************/
/**************************************************************************************************/

/**
 * @b Create a new surface on which Vulkan can render things to.
 *    Each surface has a one-to-one relationship between the window and itself.
 *
 * @param win Window to be used to create surface.
 *
 * @return @c Surface* on success.
 * @return @c Null otherwise.
 * */
Surface *surface_create (Vulkan *vk, XwWindow *win) {
    Surface *surf = NEW (Surface);

    Surface *isurf = surface_init (surf, vk, win);
    GOTO_HANDLER_IF (!isurf, INIT_FAILED, ERR_OBJECT_INITIALIZATION_FAILED);

    return isurf;

INIT_FAILED:
    surface_destroy (surf, vk);
    return Null;
}

/**
 * @b Initialize given surface object with new surface data
 *    as a new surface for given window.
 *
 * @param surf Surface to be initiazed
 * @param win Window to create new surface for.
 *
 * @return @c surf on success.
 * @return @c Null otherwise.
 * */
Surface *surface_init (Surface *surface, Vulkan *vk, XwWindow *win) {
    RETURN_VALUE_IF (!surface || !vk || !win, Null, ERR_INVALID_ARGUMENTS);

    GOTO_HANDLER_IF (
        !surface_create_surface (surface, vk, win) || !surface_select_gpu (surface, vk) ||
            !surface_fetch_queue_family_indices (surface) ||
            !surface_create_logical_device (surface) || !surface_create_swapchain (surface, win) ||
            !surface_fetch_swapchain_images (surface) ||
            !surface_create_swapchain_image_views (surface) ||
            !surface_create_command_objects (surface) || !surface_create_renderpass (surface) ||
            !surface_create_framebuffers (surface) || !surface_create_sync_objects (surface) ||
            !surface_create_pipeline_layout (surface) || !surface_create_pipeline (surface),
        INIT_FAILED,
        "Failed to initialize Surface object\n"
    );

    return surface;

INIT_FAILED:
    surface_deinit (surface, vk);
    return Null;
}

/**
 * @b De-initialize but don't free the given surface object.
 *
 * @param surf
 * @param vk 
 *
 * @return @c surface on success.
 * @return Null otherwise.
 * */
Surface *surface_deinit (Surface *surface, Vulkan *vk) {
    RETURN_VALUE_IF (!surface || !vk, Null, ERR_INVALID_ARGUMENTS);

    vkDeviceWaitIdle (surface->device);

    if (surface->device) {
        if (surface->pipeline_layout) {
            vkDestroyPipelineLayout (surface->device, surface->pipeline_layout, Null);
        }

        if (surface->pipeline) {
            vkDestroyPipeline (surface->device, surface->pipeline, Null);
        }

        if (surface->render_semaphore) {
            vkDestroySemaphore (surface->device, surface->render_semaphore, Null);
        }
        if (surface->present_semaphore) {
            vkDestroySemaphore (surface->device, surface->present_semaphore, Null);
        }
        if (surface->render_fence) {
            vkDestroyFence (surface->device, surface->render_fence, Null);
        }

        if (surface->framebuffers) {
            for (Size s = 0; s < surface->swapchain_image_count; s++) {
                if (surface->framebuffers[s]) {
                    vkDestroyFramebuffer (surface->device, surface->framebuffers[s], Null);
                }
            }
        }

        if (surface->render_pass) {
            vkDestroyRenderPass (surface->device, surface->render_pass, Null);
        }

        if (surface->cmd_pool) {
            vkDestroyCommandPool (surface->device, surface->cmd_pool, Null);
        }

        if (surface->swapchain_image_views) {
            for (Size s = 0; s < surface->swapchain_image_count; s++) {
                if (surface->swapchain_image_views[s]) {
                    vkDestroyImageView (surface->device, surface->swapchain_image_views[s], Null);
                    surface->swapchain_image_views[s] = VK_NULL_HANDLE;
                }
            }
        }

        if (surface->swapchain) {
            vkDestroySwapchainKHR (surface->device, surface->swapchain, Null);
        }

        vkDestroyDevice (surface->device, Null);
    }

    if (surface->surface) {
        vkDestroySurfaceKHR (vk->instance, surface->surface, Null);
    }

    /* all memory deallocations out of if statements, having their own checks */
    if (surface->framebuffers) {
        FREE (surface->framebuffers);
        surface->framebuffers = Null;
    }

    if (surface->swapchain_image_views) {
        FREE (surface->swapchain_image_views);
        surface->swapchain_image_views = Null;
    }

    if (surface->swapchain_images) {
        FREE (surface->swapchain_images);
        surface->swapchain_images = Null;
    }

    return surface;
}

/**
 * @b De-initialize object and destroy the given surface object.
 *
 * @param surf 
 * @param vk
 * */
void surface_destroy (Surface *surf, Vulkan *vk) {
    RETURN_IF (!surf || !vk, ERR_INVALID_ARGUMENTS);

    surface_deinit (surf, vk);
    FREE (surf);
}

/**
 * @b Recreate swapchain for this surface, by using old swapchain.
 *
 * @param surface 
 *
 * @return @c surface on success.
 * @return @c Null otherwise.
 * */
Surface *surface_recreate_swapchain (Surface *surface, XwWindow *win) {
    RETURN_VALUE_IF (!surface || !win, Null, ERR_INVALID_ARGUMENTS);

    RETURN_VALUE_IF (
        !surface->swapchain_image_views || !surface->framebuffers,
        Null,
        "Swapchain recreate called but, previous images/views/framebuffers are invalid\n"
    );

    surface_wait_for_pending_operations (surface);

    vkDestroySemaphore (surface->device, surface->present_semaphore, Null);
    vkDestroySemaphore (surface->device, surface->render_semaphore, Null);
    vkDestroyFence (surface->device, surface->render_fence, Null);

    vkDestroyCommandPool (surface->device, surface->cmd_pool, Null);

    /* destroy image views and framebuffers because they need to be recreated */
    for (Size s = 0; s < surface->swapchain_image_count; s++) {
        vkDestroyImageView (surface->device, surface->swapchain_image_views[s], Null);
        vkDestroyFramebuffer (surface->device, surface->framebuffers[s], Null);

        surface->swapchain_image_views[s] = VK_NULL_HANDLE;
        surface->framebuffers[s]          = VK_NULL_HANDLE;
    }

    /* store handle of old swapchain */
    VkSwapchainKHR old_swapchain = surface->swapchain;

    /* create new swapchain */
    RETURN_VALUE_IF (
        !surface_create_swapchain (surface, win) || !surface_fetch_swapchain_images (surface) ||
            !surface_create_swapchain_image_views (surface) ||
            !surface_create_framebuffers (surface) || !surface_create_sync_objects (surface) ||
            !surface_create_command_objects (surface),
        Null,
        "Failed to recreate swapchain\n"
    );

    /* destroy old swapchain after recreation */
    vkDestroySwapchainKHR (surface->device, old_swapchain, Null);

    return surface;
}

/**
 * @b Wait for all pending operations related to provided surface to end.
 *
 * @param surface 
 * 
 * @return @c surface on Success.
 * @return @c Null otherwise.
 * */
Surface *surface_wait_for_pending_operations (Surface *surface) {
    RETURN_VALUE_IF (!surface, Null, ERR_INVALID_ARGUMENTS);

    vkDeviceWaitIdle (surface->device);

    return surface;
}

/**************************************************************************************************/
/********************************** BUFFER OBJECT PUBLIC METHODS **********************************/
/**************************************************************************************************/

/**
 * @b Create a buffer object.
 *
 * @param gpu To query memory type and properties supported by physical device.
 * @param device Device to allocate memory on.
 * @param usage How will this buffer object be used.
 * @param size Number of bytes to allocate.
 * @param queue_family_index Index of queue family on which this buffer will be EXCLUSIVELY used.
 *
 * @return @c BufferObject on success.
 * @return @c {0, 0} otherwise.
 * */
BufferObject buffer_object_create (
    VkPhysicalDevice   gpu,
    VkDevice           device,
    VkBufferUsageFlags usage,
    Size               size,
    Uint32             queue_family_index
) {
    RETURN_VALUE_IF (!gpu || !device || !size, ((BufferObject) {0, 0}), ERR_INVALID_ARGUMENTS);

    /* create vertex buffer */
    VkBuffer vbuffer = VK_NULL_HANDLE;
    {
        VkBufferCreateInfo buffer_create_info = {
            .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext                 = Null,
            .flags                 = 0,
            .size                  = size,
            .usage                 = usage,
            .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices   = (Uint32[]) {queue_family_index}
        };

        VkResult res = vkCreateBuffer (device, &buffer_create_info, Null, &vbuffer);
        RETURN_VALUE_IF (
            res != VK_SUCCESS,
            ((BufferObject) {0, 0}),
            "Failed to create buffer object"
        );
    }

    /* find required memory type */
    Size                  required_size     = 0;
    Uint32                memory_type_index = 0;
    VkMemoryPropertyFlags memory_property_flags =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    {
        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements (device, vbuffer, &memory_requirements);

        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties (gpu, &memory_properties);

        for (Uint32 i = 0; i < memory_properties.memoryTypeCount; i++) {
            /* if memory type bit is flagged, and property flags match then set that as memory type */
            if ((memory_requirements.memoryTypeBits & (1 << i)) &&
                ((memory_properties.memoryTypes[i].propertyFlags & memory_property_flags) ==
                 memory_property_flags)) {
                memory_type_index = i;
            }
        }

        GOTO_HANDLER_IF (
            !memory_type_index,
            MEMORY_TYPE_NOT_FOUND,
            "Required memory type not found!\n"
        );

        required_size = memory_requirements.size;
    }

    /* allocate memory for device buffer */
    VkDeviceMemory vmemory = VK_NULL_HANDLE;
    {
        VkMemoryAllocateInfo allocate_info = {
            .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext           = Null,
            .allocationSize  = required_size,
            .memoryTypeIndex = memory_type_index
        };

        VkResult res = vkAllocateMemory (device, &allocate_info, Null, &vmemory);
        GOTO_HANDLER_IF (
            res != VK_SUCCESS,
            MEMORY_ALLOC_FAILED,
            "Failed to allocate memory for new buffer\b"
        );
    }

    Size offset = 0;
    vkBindBufferMemory (device, vbuffer, vmemory, offset);

    return (BufferObject) {vbuffer, vmemory};

MEMORY_TYPE_NOT_FOUND:
MEMORY_ALLOC_FAILED:
    vkDestroyBuffer (device, vbuffer, Null);
    return (BufferObject) {0, 0};
}

/**
 * @b Destroy buffer object and free memory allocated on device.
 *
 * @param device 
 * @param bo 
 * */
void buffer_object_destroy (BufferObject bo, VkDevice device) {
    RETURN_IF (!device || !bo.buffer || !bo.memory, ERR_INVALID_ARGUMENTS);

    vkDeviceWaitIdle (device);

    vkFreeMemory (device, bo.memory, Null);
    vkDestroyBuffer (device, bo.buffer, Null);
}

/**
 * @b Upload buffer data to GPU by copying it from given main memory to device memory.
 *
 * @param bo 
 * @param device 
 * @param data Pointer to data to be copied.
 * @param size Size of data to be copied in number of bytes.
 *
 * @return @c bo on success.
 * @return @c {0, 0} otherwise
 * */
BufferObject buffer_object_upload_data (BufferObject bo, VkDevice device, void *data, Size size) {
    RETURN_VALUE_IF (
        !bo.memory || !bo.buffer || !device || !data || !size,
        ((BufferObject) {0, 0}),
        ERR_INVALID_ARGUMENTS
    );

    void *mapped_data = Null;
    vkMapMemory (device, bo.memory, 0, size, 0, &mapped_data);
    memcpy (mapped_data, data, size);
    vkUnmapMemory (device, bo.memory);

    return bo;
}

/**************************************************************************************************/
/******************************* VULKAN PRIVATE METHOD DEFINITIONS ********************************/
/**************************************************************************************************/

/**
 * @b Create a new vulkan instance and store it in given Vulkan object.
 *
 * @param vk 
 *
 * @return @c vk on success.
 * @return @c Null otherwise.
 * */
static inline Vulkan *vk_create_instance (Vulkan *vk) {
    RETURN_VALUE_IF (!vk, Null, ERR_INVALID_ARGUMENTS);

    Size           layer_count = 0, extension_count = 0;
    const CString *layers     = get_instance_layer_names (&layer_count);
    const CString *extensions = get_instance_extension_names (&extension_count);

    VkInstanceCreateInfo instance_create_info = {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = Null,
        .flags                   = 0,
        .pApplicationInfo        = Null,
        .enabledLayerCount       = layer_count,
        .ppEnabledLayerNames     = layers,
        .enabledExtensionCount   = extension_count,
        .ppEnabledExtensionNames = extensions
    };

    /* create vulkan instance */
    VkResult res = vkCreateInstance (&instance_create_info, Null, &vk->instance);
    RETURN_VALUE_IF (res != VK_SUCCESS, Null, "Failed to create Vulkan instance. RES = %d\n", res);

    return vk;
}

/**
 * @b Get and store all GPU handles on host in given Vulkan object.
 *
 * @param vk 
 *
 * @return @c vk on success.
 * @return @c Null otherwise.
 * */
static inline Vulkan *vk_enumerate_gpus (Vulkan *vk) {
    RETURN_VALUE_IF (!vk, Null, ERR_INVALID_ARGUMENTS);

    /* get the number of gpus on host */
    VkResult res = vkEnumeratePhysicalDevices (vk->instance, &vk->gpu_count, Null);
    RETURN_VALUE_IF (res != VK_SUCCESS, Null, "Failed to get GPU count. RET = %d\n", res);

    /* get the gpu handles */
    vk->gpus = Null;
    if (vk->gpu_count) {
        vk->gpus = ALLOCATE (VkPhysicalDevice, vk->gpu_count);
        GOTO_HANDLER_IF (!vk->gpus, GPU_ENUM_FAILED, ERR_OUT_OF_MEMORY);

        /* get gpu handles */
        res = vkEnumeratePhysicalDevices (vk->instance, &vk->gpu_count, vk->gpus);
        GOTO_HANDLER_IF (
            res != VK_SUCCESS,
            GPU_ENUM_FAILED,
            "Failed to get GPU handles. RET = %d\n",
            res
        );
    }

    return vk;

    /* error handlers */
GPU_ENUM_FAILED:
    FREE (vk->gpus);
    vk->gpus = Null;
    return Null;
}

/**************************************************************************************************/
/******************************* SURFACE PRIVATE METHOD DEFINITIONS *******************************/
/**************************************************************************************************/

/**
 * @b Create VkSurfaceKHR for given @c Surface object.
 *
 * @param surface Where surface will be stored after creation.
 * @param vk For access to VkInstance handle.
 * @param win @c XwWindow object to create this surface for.
 *
 * @return @c surface on success.
 * @return @c Null otherwise.
 * */
static inline Surface *surface_create_surface (Surface *surface, Vulkan *vk, XwWindow *win) {
    RETURN_VALUE_IF (!surface || !vk || !win, Null, ERR_INVALID_ARGUMENTS);

    VkResult res = xw_window_create_vulkan_surface (win, vk->instance, &surface->surface);
    RETURN_VALUE_IF (
        res != VK_SUCCESS,
        Null,
        "Failed to create Vulkan surface for given window. RET = %d\n",
        res
    );

    return surface;
}

/**
 * @b Select a GPU for creating a logical device.
 *
 * @param surface
 * @param vk
 *
 * @return @c surf on success.
 * @return @c Null otherwise
 * */
static inline Surface *surface_select_gpu (Surface *surface, Vulkan *vk) {
    RETURN_VALUE_IF (!surface || !vk, Null, ERR_INVALID_ARGUMENTS);

    surface->selected_gpu = vk->gpus[0];

    return surface;
}

/**
 * @b Find all the required queue family indices and store it in surface object.
 *
 * @param surface 
 *
 * @return @c surface on success.
 * @return @c Null otherwise.
 * */
static inline Surface *surface_fetch_queue_family_indices (Surface *surface) {
    RETURN_VALUE_IF (!surface, Null, ERR_INVALID_ARGUMENTS);

    surface->graphics_family_index =
        find_queue_family_index (surface->selected_gpu, VK_QUEUE_GRAPHICS_BIT);
    RETURN_VALUE_IF (
        surface->graphics_family_index == -1,
        Null,
        "Failed to find graphics queue in selected GPU\n"
    );

    return surface;
}

/**
 * @b Create logical device for surface.
 *
 * @param surface
 *
 * @return @c surface on success.
 * @return @c Null otherwise.
 * */
static inline Surface *surface_create_logical_device (Surface *surface) {
    RETURN_VALUE_IF (!surface, Null, ERR_INVALID_ARGUMENTS);

    Float32                 queue_priorities  = 1.f;
    VkDeviceQueueCreateInfo queue_create_info = {
        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext            = Null,
        .flags            = 0,
        .queueFamilyIndex = surface->graphics_family_index,
        .queueCount       = 1,
        .pQueuePriorities = &queue_priorities
    };

    /* fill in required extensions here */
    static const CString extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo device_create_info = {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = Null,
        .flags                   = 0,
        .queueCreateInfoCount    = 1,
        .pQueueCreateInfos       = &queue_create_info,
        .enabledLayerCount       = 0,
        .ppEnabledLayerNames     = Null,
        .enabledExtensionCount   = ARRAY_SIZE (extensions),
        .ppEnabledExtensionNames = extensions,
        .pEnabledFeatures        = Null
    };

    VkResult res = VK_SUCCESS;
    res = vkCreateDevice (surface->selected_gpu, &device_create_info, Null, &surface->device);
    RETURN_VALUE_IF (res != VK_SUCCESS, Null, "Failed to create Logical Device. RES = %d\n", res);

    vkGetDeviceQueue (surface->device, surface->graphics_family_index, 0, &surface->graphics_queue);

    return surface;
}

static inline Surface *surface_create_swapchain (Surface *surface, XwWindow *win) {
    RETURN_VALUE_IF (!surface || !win, VK_NULL_HANDLE, ERR_INVALID_ARGUMENTS);

    /* get surface capabilities */
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR (
        surface->selected_gpu,
        surface->surface,
        &capabilities
    );

    VkExtent2D image_extent;
    if (capabilities.currentExtent.width == UINT32_MAX) {
        XwWindowSize win_size = xw_window_get_size (win);
        image_extent          = (VkExtent2D) {.width = win_size.width, .height = win_size.height};
    } else {
        image_extent = (VkExtent2D
        ) {.width = CLAMP (
               capabilities.currentExtent.width,
               capabilities.minImageExtent.width,
               capabilities.maxImageExtent.width
           ),
           .height = CLAMP (
               capabilities.currentExtent.height,
               capabilities.minImageExtent.height,
               capabilities.maxImageExtent.height
           )};
    }

    /* get number of available present modes */
    Uint32   present_mode_count = 0;
    VkResult res                = vkGetPhysicalDeviceSurfacePresentModesKHR (
        surface->selected_gpu,
        surface->surface,
        &present_mode_count,
        Null
    );
    RETURN_VALUE_IF (
        res != VK_SUCCESS,
        Null,
        "Failed to get number of present modes. RET = %d\n",
        res
    );

    /* get list of present modes */
    VkPresentModeKHR present_modes[present_mode_count];
    res = vkGetPhysicalDeviceSurfacePresentModesKHR (
        surface->selected_gpu,
        surface->surface,
        &present_mode_count,
        present_modes
    );
    RETURN_VALUE_IF (res != VK_SUCCESS, Null, "Failed to get present modes. RET = %d\n", res);

    /* select the best present mode out of all available. */
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (Size s = 0; s < present_mode_count; s++) {
        if (present_modes[s] == VK_PRESENT_MODE_MAILBOX_KHR) {
            present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }

    /* get number of available surface formats */
    Uint32 surface_format_count = 0;
    res                         = vkGetPhysicalDeviceSurfaceFormatsKHR (
        surface->selected_gpu,
        surface->surface,
        &surface_format_count,
        Null
    );
    RETURN_VALUE_IF (
        res != VK_SUCCESS,
        Null,
        "Failed to get number of surface formats. RET = %d\n",
        res
    );

    /* get list of surface formats */
    VkSurfaceFormatKHR surface_formats[surface_format_count];
    res = vkGetPhysicalDeviceSurfaceFormatsKHR (
        surface->selected_gpu,
        surface->surface,
        &surface_format_count,
        surface_formats
    );
    RETURN_VALUE_IF (res != VK_SUCCESS, Null, "Failed to get surface formats. RET = %d\n", res);

    /* the first format is the best format for now */
    VkSurfaceFormatKHR surface_format = surface_formats[0];

    /* only graphics family will be using our images */
    const Uint32 queue_family_indices[] = {surface->graphics_family_index};

    VkSwapchainCreateInfoKHR swapchain_create_info = {
        .sType         = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext         = Null,
        .flags         = 0,
        .surface       = surface->surface,
        .minImageCount = CLAMP (
            capabilities.minImageCount,
            capabilities.minImageCount + 1,
            capabilities.maxImageCount ? capabilities.maxImageCount : capabilities.minImageCount + 1
        ),
        .imageFormat           = surface_format.format,
        .imageColorSpace       = surface_format.colorSpace,
        .imageExtent           = image_extent,
        .imageArrayLayers      = 1,
        .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = ARRAY_SIZE (queue_family_indices),
        .pQueueFamilyIndices   = queue_family_indices,
        .preTransform   = capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode    = present_mode,
        .clipped        = VK_TRUE,
        .oldSwapchain   = surface->swapchain /* use old swapchain, will be destroyed after this */
    };

    res = vkCreateSwapchainKHR (surface->device, &swapchain_create_info, Null, &surface->swapchain);
    RETURN_VALUE_IF (res != VK_SUCCESS, Null, "Failed to create Vulkan swapchain. RET = %d\n", res);

    /* store required data in surface after swapchin creation */
    surface->swapchain_image_format = surface_format.format;
    surface->swapchain_image_extent = image_extent;

    return surface;
}

/**
 * @b Get images from swapchain and store them in Surface object.
 *
 * @param surface @c Surface which contains the valid swapchain.
 *
 * @return @c surf on success.
 * @return @c Null otherwise.
 * */
static inline Surface *surface_fetch_swapchain_images (Surface *surface) {
    RETURN_VALUE_IF (!surface, Null, ERR_INVALID_ARGUMENTS);

    /* get number of swapchain images */
    surface->swapchain_image_count = 0;
    VkResult res                   = vkGetSwapchainImagesKHR (
        surface->device,
        surface->swapchain,
        &surface->swapchain_image_count,
        Null
    );
    RETURN_VALUE_IF (
        res != VK_SUCCESS,
        Null,
        "Failed to get number of swapchain images. RET = %d\n",
        res
    );

    /* create space for images */
    surface->swapchain_images =
        REALLOCATE (surface->swapchain_images, VkImage, surface->swapchain_image_count);
    RETURN_VALUE_IF (!surface->swapchain_images, Null, ERR_OUT_OF_MEMORY);

    /* get swapchain image handles s */
    res = vkGetSwapchainImagesKHR (
        surface->device,
        surface->swapchain,
        &surface->swapchain_image_count,
        surface->swapchain_images
    );
    GOTO_HANDLER_IF (
        res != VK_SUCCESS,
        IMAGE_GET_FAILED,
        "Failed to get all swapchain image handles. RET = %d\n",
        res
    );

    return surface;

IMAGE_GET_FAILED:
    FREE (surface->swapchain_images);
    return Null;
}

/**
 * @b Create color attachments (image views) for swapchain images.
 *
 * @param surface
 * 
 * @return @c surface on success.
 * @return @c Null otherwise.
 * */
static inline Surface *surface_create_swapchain_image_views (Surface *surface) {
    RETURN_VALUE_IF (!surface, Null, ERR_INVALID_ARGUMENTS);

    /* Create space to store image views in surface object.
     * Reallocate will reuse the memory if it's already allocated */
    surface->swapchain_image_views =
        REALLOCATE (surface->swapchain_image_views, VkImageView, surface->swapchain_image_count);
    RETURN_VALUE_IF (!surface->swapchain_image_views, Null, ERR_OUT_OF_MEMORY);

    /* template for creating image views */
    VkImageViewCreateInfo image_view_create_info = {
        .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext    = Null,
        .flags    = 0,
        .image    = VK_NULL_HANDLE,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format   = surface->swapchain_image_format,
        .components =
            {.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                         .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                         .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                         .a = VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange =
            {.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                         .layerCount     = 1,
                         .baseArrayLayer = 0,
                         .levelCount     = 1,
                         .baseMipLevel   = 0}
    };

    /* create image view handles corresponding to each image in swapchain */
    for (Size s = 0; s < surface->swapchain_image_count; s++) {
        image_view_create_info.image = surface->swapchain_images[s];
        VkResult res                 = vkCreateImageView (
            surface->device,
            &image_view_create_info,
            Null,
            &surface->swapchain_image_views[s]
        );
        GOTO_HANDLER_IF (
            res != VK_SUCCESS,
            IMAGE_VIEW_CREATE_FAILED,
            "Failed to create Image view. RET = %d\n",
            res
        );
    }

    return surface;

IMAGE_VIEW_CREATE_FAILED:
    for (Size s = 0; s < surface->swapchain_image_count; s++) {
        if (surface->swapchain_image_views[s]) {
            vkDestroyImageView (surface->device, surface->swapchain_image_views[s], Null);
        }
    }
    FREE (surface->swapchain_image_views);
    surface->swapchain_image_views = Null;
    return Null;
}

/**
 * @b Create command pool and allocate command buffers here.
 *
 * @param surface
 *
 * @return @c surface on success.
 * @return @c Null otherwise
 * */
static inline Surface *surface_create_command_objects (Surface *surface) {
    RETURN_VALUE_IF (!surface, Null, ERR_INVALID_ARGUMENTS);

    VkCommandPoolCreateInfo command_pool_create_info = {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext            = Null,
        .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = surface->graphics_family_index
    };

    /* create command pool */
    VkResult res =
        vkCreateCommandPool (surface->device, &command_pool_create_info, Null, &surface->cmd_pool);
    RETURN_VALUE_IF (res != VK_SUCCESS, Null, "Failed to create Command Pool. RET = %d\n", res);

    VkCommandBufferAllocateInfo command_buffer_allocate_info = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext              = Null,
        .commandPool        = surface->cmd_pool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    /* allocate command buffer(s) */
    res = vkAllocateCommandBuffers (
        surface->device,
        &command_buffer_allocate_info,
        &surface->cmd_buffer
    );
    RETURN_VALUE_IF (
        res != VK_SUCCESS,
        Null,
        "Failed to allocate Command Buffers. RET = %d\n",
        res
    );

    return surface;
}

/**
 * @b Create a renderpass object for this surface.
 * */
static inline Surface *surface_create_renderpass (Surface *surface) {
    RETURN_VALUE_IF (!surface, Null, ERR_INVALID_ARGUMENTS);

    VkAttachmentDescription color_attachment = {
        .flags          = 0,
        .format         = surface->swapchain_image_format,
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };
    VkAttachmentReference color_attachment_reference = {
        .attachment = 0, /* index of color attachment*/
        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference subpass_attachment_references[] = {color_attachment_reference};

    VkSubpassDescription subpass = {
        .flags                   = 0,
        .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount    = 0,
        .pInputAttachments       = Null,
        .colorAttachmentCount    = ARRAY_SIZE (subpass_attachment_references),
        .pColorAttachments       = subpass_attachment_references,
        .pResolveAttachments     = Null,
        .pDepthStencilAttachment = Null,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments    = Null
    };

    VkAttachmentDescription render_pass_attachments[] = {color_attachment};
    VkSubpassDescription    render_pass_subpasses[]   = {subpass};

    VkRenderPassCreateInfo render_pass_create_info = {
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext           = Null,
        .flags           = 0,
        .attachmentCount = ARRAY_SIZE (render_pass_attachments),
        .pAttachments    = render_pass_attachments,
        .subpassCount    = ARRAY_SIZE (render_pass_subpasses),
        .pSubpasses      = render_pass_subpasses,
        .dependencyCount = 0,
        .pDependencies   = Null,
    };

    VkResult res =
        vkCreateRenderPass (surface->device, &render_pass_create_info, Null, &surface->render_pass);
    RETURN_VALUE_IF (
        res != VK_SUCCESS,
        Null,
        "Failed to create Vulkan Render Pass. RET = %d\n",
        res
    );

    return surface;
}

/**
 * @b Create framebuffers for the swapchain inside surface.
 *
 * @param surface 
 *
 * @return @c surface on Success 
 * @return @c Null otherwise.
 * */
static inline Surface *surface_create_framebuffers (Surface *surface) {
    RETURN_VALUE_IF (!surface, Null, ERR_INVALID_ARGUMENTS);

    surface->framebuffers =
        REALLOCATE (surface->framebuffers, VkFramebuffer, surface->swapchain_image_count);
    RETURN_VALUE_IF (!surface->framebuffers, Null, ERR_OUT_OF_MEMORY);

    VkFramebufferCreateInfo framebuffer_create_info = {
        .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext           = Null,
        .flags           = 0,
        .renderPass      = surface->render_pass,
        .attachmentCount = 1, /* only color attachment for now */
        .pAttachments    = Null,
        .width           = surface->swapchain_image_extent.width,
        .height          = surface->swapchain_image_extent.height,
        .layers          = 1
    };

    for (Size s = 0; s < surface->swapchain_image_count; s++) {
        framebuffer_create_info.pAttachments = &surface->swapchain_image_views[s];
        VkResult res                         = vkCreateFramebuffer (
            surface->device,
            &framebuffer_create_info,
            Null,
            &surface->framebuffers[s]
        );
        RETURN_VALUE_IF (res != VK_SUCCESS, Null, "Failed to create framebuffer. RET = %d\n", res);
    }

    return surface;
}

/**
 * @b Create synchronization objects like semaphores, fences, etc.... and store in surface.
 *
 * @param surface 
 *
 * @return @c surface on success.
 * @return @c Null otherwise,
 * */
static inline Surface *surface_create_sync_objects (Surface *surface) {
    RETURN_VALUE_IF (!surface, Null, ERR_INVALID_ARGUMENTS);

    VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = 0,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    VkResult res =
        vkCreateFence (surface->device, &fence_create_info, Null, &surface->render_fence);
    RETURN_VALUE_IF (res != VK_SUCCESS, Null, "Failed to create Fence. RET = %d\n", res);

    VkSemaphoreCreateInfo semaphore_create_info =
        {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .pNext = Null, .flags = 0};

    res = vkCreateSemaphore (
        surface->device,
        &semaphore_create_info,
        Null,
        &surface->render_semaphore
    );
    RETURN_VALUE_IF (res != VK_SUCCESS, Null, "Failed to create Semaphore. RET = %d\n", res);

    res = vkCreateSemaphore (
        surface->device,
        &semaphore_create_info,
        Null,
        &surface->present_semaphore
    );
    RETURN_VALUE_IF (res != VK_SUCCESS, Null, "Failed to create Semaphore. RET = %d\n", res);

    return surface;
}

static inline Surface *surface_create_pipeline_layout (Surface *surface) {
    RETURN_VALUE_IF (!surface, Null, ERR_INVALID_ARGUMENTS);

    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {0};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkResult res = vkCreatePipelineLayout (
        surface->device,
        &pipeline_layout_create_info,
        Null,
        &surface->pipeline_layout
    );
    RETURN_VALUE_IF (res != VK_SUCCESS, Null, "Failed to create pipeline layout. RET = %d\n", res);

    return surface;
}

/**
 * @b Create graphics pipeline and store it in given Surface.
 *
 * @param surface
 *
 * @return @c surface on success.
 * @return @c Null otherwise.
 * */
static inline Surface *surface_create_pipeline (Surface *surface) {
    RETURN_VALUE_IF (!surface, Null, ERR_INVALID_ARGUMENTS);

    /* create shader modules */
    VkShaderModule vert_shader = VK_NULL_HANDLE, frag_shader = VK_NULL_HANDLE;
    vert_shader = load_shader (surface->device, "bin/Shaders/triangle.vert.spv");
    frag_shader = load_shader (surface->device, "bin/Shaders/triangle.frag.spv");
    GOTO_HANDLER_IF (
        !vert_shader || !frag_shader,
        SHADER_LOAD_FAILED,
        "Failed to load vertex/fragment shaders\n"
    );

    /* add shader modules and shader stages to be used in this pipeline here */
    struct {
        VkShaderModule     module;
        VkShaderStageFlags stage;
    } stage_infos[] = {
        {.module = vert_shader,   .stage = VK_SHADER_STAGE_VERTEX_BIT},
        {.module = frag_shader, .stage = VK_SHADER_STAGE_FRAGMENT_BIT}
    };

    /* create shader stages array */
    VkPipelineShaderStageCreateInfo shader_stages[ARRAY_SIZE (stage_infos)];
    for (Size s = 0; s < ARRAY_SIZE (shader_stages); s++) {
        shader_stages[s] = (VkPipelineShaderStageCreateInfo
        ) {.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
           .pNext               = Null,
           .flags               = 0,
           .stage               = stage_infos[s].stage,
           .module              = stage_infos[s].module,
           .pName               = "main",
           .pSpecializationInfo = Null};
    }

    /* describe how vertex data is sent to GPU */
    VkVertexInputBindingDescription vertex_binding_desc =
        {.binding = 0, .stride = sizeof (Vertex2D), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription vertex_attribute_desc[] = {
        {.location = 0,
         .binding  = 0,
         .format   = VK_FORMAT_R32G32_SFLOAT,
         .offset   = offsetof (Vertex2D, position)},
        {.location = 1,
         .binding  = 0,
         .format   = VK_FORMAT_R32G32B32A32_SFLOAT,
         .offset   = offsetof (Vertex2D,    color)},
    };

    /* describe vertex input state */
    VkPipelineVertexInputStateCreateInfo vertex_input_state = {
        .sType                         = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext                         = Null,
        .flags                         = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions    = &vertex_binding_desc,
        .vertexAttributeDescriptionCount = ARRAY_SIZE (vertex_attribute_desc),
        .pVertexAttributeDescriptions    = vertex_attribute_desc
    };

    /* how to assemble input vertex data */
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {0};
    input_assembly_state.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    /* describe tesselation state */
    VkPipelineTessellationStateCreateInfo tesselation_state = {0};
    tesselation_state.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;

    /* describe viewport and scissor */
    VkPipelineViewportStateCreateInfo viewport_state = {
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext         = Null,
        .flags         = 0,
        .viewportCount = 1,
        .pViewports    = (VkViewport[]
        ) {{.x        = 0,
               .y        = 0,
               .width    = surface->swapchain_image_extent.width,
               .height   = surface->swapchain_image_extent.height,
               .minDepth = 0.f,
               .maxDepth = 1.f}},
        .scissorCount  = 1,
        .pScissors =
            (VkRect2D[]) {{.offset = {.x = 0, .y = 0}, .extent = surface->swapchain_image_extent}}
    };

    /* describe rasterization state */
    VkPipelineRasterizationStateCreateInfo rasterization_state = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext                   = Null,
        .flags                   = 0,
        .depthClampEnable        = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode             = VK_POLYGON_MODE_FILL,
        .cullMode                = VK_CULL_MODE_NONE,
        .frontFace               = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable         = VK_FALSE,
        .depthBiasConstantFactor = 1.f,
        .depthBiasClamp          = 0.f,
        .depthBiasSlopeFactor    = 1.f,
        .lineWidth               = 1.f
    };

    /* describe how multisampling of rendered images will be performed */
    VkPipelineMultisampleStateCreateInfo multisample_state = {
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext                 = Null,
        .flags                 = 0,
        .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable   = VK_FALSE,
        .minSampleShading      = 1.f,
        .pSampleMask           = Null,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable      = VK_FALSE
    };

    /* describe z-fighting behavior : disabled for now */
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {0};
    depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    /* we have a single color attachment in renderpass so we need this */
    VkPipelineColorBlendAttachmentState color_blend_attachment = {0};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    /* describe color-blending */
    VkPipelineColorBlendStateCreateInfo color_blend_state = {0};
    color_blend_state.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state.attachmentCount = 1;
    color_blend_state.pAttachments    = &color_blend_attachment;

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = Null,
        .flags               = 0,
        .stageCount          = ARRAY_SIZE (shader_stages),
        .pStages             = shader_stages,
        .pVertexInputState   = &vertex_input_state,
        .pInputAssemblyState = &input_assembly_state,
        .pTessellationState  = &tesselation_state,
        .pViewportState      = &viewport_state,
        .pRasterizationState = &rasterization_state,
        .pMultisampleState   = &multisample_state,
        .pDepthStencilState  = &depth_stencil_state,
        .pColorBlendState    = &color_blend_state,
        .pDynamicState       = Null,
        .layout              = surface->pipeline_layout,
        .renderPass          = surface->render_pass,
        .subpass             = 0,
        .basePipelineHandle  = VK_NULL_HANDLE,
        .basePipelineIndex   = 0
    };

    /* create graphics pipelines */
    VkResult res = vkCreateGraphicsPipelines (
        surface->device,
        VK_NULL_HANDLE,
        1,
        &graphics_pipeline_create_info,
        Null,
        &surface->pipeline
    );
    GOTO_HANDLER_IF (
        res != VK_SUCCESS,
        PIPELINE_CREATE_FAILED,
        "Failed to create graphics pipelines. RET = %d\n",
        res
    );

    /* destroy shader modules because we don't need them anymore */
    vkDestroyShaderModule (surface->device, vert_shader, Null);
    vkDestroyShaderModule (surface->device, frag_shader, Null);

    return surface;

PIPELINE_CREATE_FAILED:
SHADER_LOAD_FAILED:
    if (vert_shader) {
        vkDestroyShaderModule (surface->device, vert_shader, Null);
    }
    if (frag_shader) {
        vkDestroyShaderModule (surface->device, frag_shader, Null);
    }
    return Null;
}

/**************************************************************************************************/
/******************************* PRIVATE HELPER METHOD DEFINITIONS ********************************/
/**************************************************************************************************/

/**
 * @b Get required vulkan validation layer names.
 *
 * The returned array must not be freed, because it's a static array.
 *
 * @param count Pointer to variable where number of layers will
 *        be stored.
 *
 * @return CString* on success.
 * @return Null otherwise.
 * */
static inline const CString *get_instance_layer_names (Size *count) {
    RETURN_VALUE_IF (!count, Null, ERR_INVALID_ARGUMENTS);

    static const CString layers[] = {"VK_LAYER_KHRONOS_validation"};

    *count = ARRAY_SIZE (layers);
    return layers;
}

/**
 * @b Get names of required instance extensions.
 *
 * The returned array must not be freed, because it's a static array.
 *
 * @param count Pointer to variable where number of extensions will
 *        be stored.
 *
 * @return CString* on success.
 * @return Null otherwise.
 * */
static inline const CString *get_instance_extension_names (Size *count) {
    /* Extensions required by CrossWindow */
    Size     window_ext_count = 0;
    CString *window_exts      = xw_get_required_extension_names (&window_ext_count);

    /* Extensions required by this application on it's own 
     * Any new required extensions must be added to this array */
    static const CString my_exts[]    = {};
    Size                 my_ext_count = ARRAY_SIZE (my_exts);

    /* total number of extensions */
    Size ext_count = window_ext_count + my_ext_count;

    /* Assuming here that there will never be more than 8 extensions.
     * This is to fix the size of exts array */
#define MAX_EXT_COUNT 8

    /* extensions required by all */
    static CString exts[MAX_EXT_COUNT];
    for (Size s = 0; s < window_ext_count; s++) {
        exts[s] = window_exts[s];
    }
    for (Size s = 0; s < my_ext_count; s++) {
        exts[window_ext_count + s] = my_exts[s];
    }

    *count = ext_count;
    return exts;
}

/**
 * @b Get index of queue family with given queue flags enabled.
 *
 * @param gpu Gpu to search queue family inside.
 * @param queue_flags Flag bits to be checked against.
 *
 * @return Non-negative value on success.
 * @return -1 on failure.
 * */
static inline Int32 find_queue_family_index (VkPhysicalDevice gpu, VkQueueFlags queue_flags) {
    RETURN_VALUE_IF (!gpu, -1, ERR_INVALID_ARGUMENTS);

    /* get queue family count */
    Uint32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties (gpu, &queue_family_count, Null);
    RETURN_VALUE_IF (!queue_family_count, -1, "Failed to get queue family count in selected GPU\n");

    /* get queue family properties */
    VkQueueFamilyProperties queue_family_properties[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties (gpu, &queue_family_count, queue_family_properties);

    /* find queue family with given queue flags */
    Int32 family_index = -1;
    for (Size s = 0; s < queue_family_count; s++) {
        if ((queue_family_properties[s].queueFlags & queue_flags) == queue_flags) {
            family_index = s;
        }
    }

    RETURN_VALUE_IF (
        family_index == -1,
        -1,
        "Failed to find queue family with queue flags \"%x\" in selected GPU\n",
        queue_flags
    );

    return family_index;
}

/**
 * @b Create a shader module by loading it from file.
 *
 * @param device To use to create shader module.
 * @param path Path of SPIR-V shader code.
 *
 * @return VkShaderModule on success.
 * @return VK_NULL_HANDLE otherwise.
 * */
static inline VkShaderModule load_shader (VkDevice device, CString path) {
    RETURN_VALUE_IF (!device || !path, VK_NULL_HANDLE, ERR_INVALID_ARGUMENTS);

    FILE *file = fopen (path, "r");
    RETURN_VALUE_IF (!file, VK_NULL_HANDLE, ERR_INVALID_ARGUMENTS);

    fseek (file, 0, SEEK_END);
    Size file_size = ftell (file);
    GOTO_HANDLER_IF (!file_size, FILE_SIZE_ZERO, "Shader file (\"%s\") size must not be 0\n", path);
    fseek (file, 0, SEEK_SET);

    Uint8 *fdata = ALLOCATE (Uint8, file_size);
    fread (fdata, 1, file_size, file);
    fclose (file);

    VkShaderModuleCreateInfo shader_module_create_info = {
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext    = Null,
        .flags    = 0,
        .codeSize = file_size,
        .pCode    = (Uint32 *)fdata
    };

    VkShaderModule shader = VK_NULL_HANDLE;
    VkResult       res = vkCreateShaderModule (device, &shader_module_create_info, Null, &shader);
    GOTO_HANDLER_IF (
        res != VK_SUCCESS,
        SHADER_CREATE_FAILED,
        "Failed to create shader module. RET = %d\n",
        res
    );

    FREE (fdata);

    return shader;

SHADER_CREATE_FAILED:
    FREE (fdata);
FILE_SIZE_ZERO:
    fclose (file);
    return VK_NULL_HANDLE;
}
