#include <Common.h>

/* crosswindow */
#include <CrossWindow/Event.h>
#include <CrossWindow/Vulkan.h>
#include <CrossWindow/Window.h>

/* libc */
#include <string.h>

static const CString *get_instance_layer_names (Size *count);
static const CString *get_instance_extension_names (Size *count);

typedef struct Vulkan {
    VkInstance        instance;     /**< @b Our connection with vulkan */
    Uint32            gpu_count;    /**< @b Total number of usable physical devices on host. */
    VkPhysicalDevice *gpus;         /**< @b GPU handles */
    VkPhysicalDevice  selected_gpu; /**< @b Handle of selected gpu from list of all available. */

    Int32 graphics_family_index; /**< @b Index of graphics queue family. -1 if invalid or not set */
    VkQueue graphics_queue;      /**< @b Handle to created graphics queue */

    VkDevice device;             /**< @b Created logical device. */
} Vulkan;

Vulkan                      *vk_init (Vulkan *vulkan);
Vulkan                      *vk_deinit (Vulkan *vulkan);
static inline VkSwapchainKHR vk_create_swapchain (Vulkan *vk, VkSurfaceKHR surface);

int main() {
    const Uint32 width  = 960;
    const Uint32 height = 540;
    XwWindow    *win    = xw_window_create ("Ckeckl", width, height, 10, 20);

    Vulkan vk = {0};
    vk_init (&vk);

    /* create surface */
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult     res     = xw_window_create_vulkan_surface (win, vk.instance, &surface);
    GOTO_HANDLER_IF (
        res != VK_SUCCESS,
        SURFACE_FAILED,
        "Failed to create Vulkan surface. RES = %d\n",
        res
    );

    /* create swapchain for our window */
    VkSwapchainKHR swapchain = vk_create_swapchain (&vk, surface);
    GOTO_HANDLER_IF (!swapchain, SWAPCHAIN_FAILED, "Failed to create Swapchain\n");

    /* event handlign looop */
    Bool    is_running = True;
    XwEvent e;
    while (is_running) {
        while (xw_event_poll (&e)) {
            switch (e.type) {
                case XW_EVENT_TYPE_CLOSE_WINDOW : {
                    is_running = False;
                    break;
                }
                default :
                    break;
            }
        }
    }

    vkDestroySwapchainKHR (vk.device, swapchain, Null);
    vkDestroySurfaceKHR (vk.instance, surface, Null);
    vk_deinit (&vk);
    xw_window_destroy (win);

    return EXIT_SUCCESS;

SWAPCHAIN_FAILED:
    vkDestroySurfaceKHR (vk.instance, surface, Null);
SURFACE_FAILED:
    vk_deinit (&vk);
    xw_window_destroy (win);
    return EXIT_FAILURE;
}

static inline Vulkan *vk_create_instance (Vulkan *vk);
static inline Vulkan *vk_get_gpu_handles (Vulkan *vk);
static inline Vulkan *vk_get_queue_family_indices (Vulkan *vk);
static inline Vulkan *vk_create_logical_device (Vulkan *vk);

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
        !vk_create_instance (vk) || !vk_get_gpu_handles (vk) || !vk_get_queue_family_indices (vk) ||
            !vk_create_logical_device (vk),
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

    if (vk->device) {
        vkDestroyDevice (vk->device, Null);
    }

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

/********************************** PRIVATE METHODS ****************************************/

static inline VkSwapchainKHR vk_create_swapchain (Vulkan *vk, VkSurfaceKHR surface) {
    RETURN_VALUE_IF (!vk || !surface, VK_NULL_HANDLE, ERR_INVALID_ARGUMENTS);

    /* get surface capabilities */
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR (vk->selected_gpu, surface, &capabilities);

    /* get number of available present modes */
    Uint32   present_mode_count = 0;
    VkResult res                = vkGetPhysicalDeviceSurfacePresentModesKHR (
        vk->selected_gpu,
        surface,
        &present_mode_count,
        Null
    );
    RETURN_VALUE_IF (
        res != VK_SUCCESS,
        VK_NULL_HANDLE,
        "Failed to get number of present modes. RET = %d\n",
        res
    );

    /* get list of present modes */
    VkPresentModeKHR present_modes[present_mode_count];
    res = vkGetPhysicalDeviceSurfacePresentModesKHR (
        vk->selected_gpu,
        surface,
        &present_mode_count,
        present_modes
    );
    RETURN_VALUE_IF (
        res != VK_SUCCESS,
        VK_NULL_HANDLE,
        "Failed to get present modes. RET = %d\n",
        res
    );

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
        vk->selected_gpu,
        surface,
        &surface_format_count,
        Null
    );
    RETURN_VALUE_IF (
        res != VK_SUCCESS,
        VK_NULL_HANDLE,
        "Failed to get number of surface formats. RET = %d\n",
        res
    );

    /* get list of surface formats */
    VkSurfaceFormatKHR surface_formats[surface_format_count];
    res = vkGetPhysicalDeviceSurfaceFormatsKHR (
        vk->selected_gpu,
        surface,
        &surface_format_count,
        surface_formats
    );
    RETURN_VALUE_IF (
        res != VK_SUCCESS,
        VK_NULL_HANDLE,
        "Failed to get surface formats. RET = %d\n",
        res
    );

    /* the first format is the best format for now */
    VkSurfaceFormatKHR surface_format = surface_formats[0];

    /* only graphics family will be using our images */
    const Uint32 queue_family_indices[] = {vk->graphics_family_index};

    VkSwapchainCreateInfoKHR swapchain_create_info = {
        .sType         = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext         = Null,
        .flags         = 0,
        .surface       = surface,
        .minImageCount = CLAMP (
            capabilities.minImageCount,
            capabilities.minImageCount + 1,
            capabilities.maxImageCount ? capabilities.maxImageCount : capabilities.minImageCount + 1
        ),
        .imageFormat           = surface_format.format,
        .imageColorSpace       = surface_format.colorSpace,
        .imageExtent           = capabilities.currentExtent,
        .imageArrayLayers      = 1,
        .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = ARRAY_SIZE (queue_family_indices),
        .pQueueFamilyIndices   = queue_family_indices,
        .preTransform   = capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode    = present_mode,
        .clipped        = VK_TRUE,
        .oldSwapchain   = VK_NULL_HANDLE
    };

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    res = vkCreateSwapchainKHR (vk->device, &swapchain_create_info, Null, &swapchain);
    RETURN_VALUE_IF (
        res != VK_SUCCESS,
        VK_NULL_HANDLE,
        "Failed to create Vulkan swapchain. RET = %d\n",
        res
    );

    return swapchain;
}

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

static inline Vulkan *vk_get_gpu_handles (Vulkan *vk) {
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

        /* select the first gpu for now */
        vk->selected_gpu = vk->gpus[0];
    }

    return vk;

    /* error handlers */
GPU_ENUM_FAILED:
    FREE (vk->gpus);
    vk->gpus = Null;
    return Null;
}

static inline Vulkan *vk_get_queue_family_indices (Vulkan *vk) {
    RETURN_VALUE_IF (!vk, Null, ERR_INVALID_ARGUMENTS);

    /* get queue family count */
    Uint32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties (vk->selected_gpu, &queue_family_count, Null);
    RETURN_VALUE_IF (
        !queue_family_count,
        Null,
        "Failed to get queue family count in selected GPU\n"
    );

    VkQueueFamilyProperties queue_family_properties[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties (
        vk->selected_gpu,
        &queue_family_count,
        queue_family_properties
    );

    vk->graphics_family_index = -1;
    for (Size s = 0; s < queue_family_count; s++) {
        if (queue_family_properties[s].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            vk->graphics_family_index = s;
        }
    }

    RETURN_VALUE_IF (
        vk->graphics_family_index == -1,
        Null,
        "Failed to find graphics queue family in selected GPU\n"
    );

    return vk;
}

static inline Vulkan *vk_create_logical_device (Vulkan *vk) {
    RETURN_VALUE_IF (!vk, Null, ERR_INVALID_ARGUMENTS);
    RETURN_VALUE_IF (!vk->selected_gpu, Null, "No GPU has been selected yet!\n");

    /* Create queue create info for graphics family queues
     * We need only one queue, so only one priority will be provided as well... */
    Float32                 queue_priorities  = 1.f;
    VkDeviceQueueCreateInfo queue_create_info = {
        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext            = Null,
        .flags            = 0,
        .queueFamilyIndex = vk->graphics_family_index,
        .queueCount       = 1,
        .pQueuePriorities = &queue_priorities
    };

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
    res          = vkCreateDevice (vk->selected_gpu, &device_create_info, Null, &vk->device);
    RETURN_VALUE_IF (res != VK_SUCCESS, Null, "Failed to create Logical Device. RES = %d\n", res);

    vkGetDeviceQueue (vk->device, vk->graphics_family_index, 0, &vk->graphics_queue);

    return vk;
}

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
static const CString *get_instance_layer_names (Size *count) {
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
static const CString *get_instance_extension_names (Size *count) {
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
