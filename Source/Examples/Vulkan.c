#include <CrossWindow/Vulkan.h>
#include "Vulkan.h"

#include "Common.h"

#include <string.h>

/* vulkan private methods */
static inline Vulkan        *vk_create_instance (Vulkan *vk);
static inline Vulkan        *vk_enumerate_gpus (Vulkan *vk);
static inline const CString *get_instance_layer_names (Size *count);
static inline const CString *get_instance_extension_names (Size *count);

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


/************************************** PRIVATE METHODS *****************************************/

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
