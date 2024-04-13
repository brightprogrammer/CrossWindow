#include <Common.h>

/* crosswindow */
#include <CrossWindow/Event.h>
#include <CrossWindow/Vulkan.h>
#include <CrossWindow/Window.h>

/* local includes */
#include "Surface.h"
#include "Vulkan.h"

static Surface *surface_draw_frame (Surface *surface, XwWindow *win, Bool resized);

int main() {
    const Uint32 width  = 960;
    const Uint32 height = 540;
    XwWindow    *win    = xw_window_create ("Ckeckl", width, height, 10, 20);

    Vulkan *vk = vk_create();
    GOTO_HANDLER_IF (!vk, VK_INIT_FAILED, "Failed to create Vulkan\n");

    Surface *surface = surface_create (vk, win);
    GOTO_HANDLER_IF (!surface, SURFACE_FAILED, "Failed to create Surface\n");

    /* event handlign looop */
    Bool    is_running = True;
    XwEvent e;
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

        surface_draw_frame (surface, win, resized);
    }

    surface_destroy (surface, vk);
    vk_destroy (vk);
    xw_window_destroy (win);

    return EXIT_SUCCESS;

SURFACE_FAILED:
    vk_destroy (vk);
VK_INIT_FAILED:
    xw_window_destroy (win);
    return EXIT_FAILURE;
}

static Surface *surface_draw_frame (Surface *surface, XwWindow *win, Bool resized) {
    /* wait for all gpu rendering to complete */
    VkResult res = vkWaitForFences (surface->device, 1, &surface->render_fence, True, 1e9);
    RETURN_VALUE_IF (
        res != VK_SUCCESS,
        Null,
        "Timeout (1s) while waiting for fences. RET = %d\n",
        res
    );

    /* get next image index */
    Uint32 next_image_index = -1;
    res                     = vkAcquireNextImageKHR (
        surface->device,
        surface->swapchain,
        1e9,
        surface->present_semaphore,
        Null,
        &next_image_index
    );
    if (resized || res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR) {
        RETURN_VALUE_IF (
            !surface_recreate_swapchain (surface, win),
            Null,
            "Failed to recreate swapchain\n"
        );
        return surface;
    }
    RETURN_VALUE_IF (
        res != VK_SUCCESS,
        Null,
        "Failed to get next image index from swapchain. RET = %d\n",
        res
    );

    /* need to reset fence before we use it again */
    res = vkResetFences (surface->device, 1, &surface->render_fence);
    RETURN_VALUE_IF (res != VK_SUCCESS, Null, "Failed to reset fences. RET = %d\n", res);

    /* reset command buffer and record draw commands again */
    res = vkResetCommandBuffer (surface->cmd_buffer, 0);
    RETURN_VALUE_IF (
        res != VK_SUCCESS,
        Null,
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
    RETURN_VALUE_IF (
        res != VK_SUCCESS,
        Null,
        "Failed to begin command buffer recording. RET = %d\n",
        res
    );

    VkClearValue clear_value = {.color = {{0.8, 0, 0.8, 1}}};

    VkRenderPassBeginInfo render_pass_begin_info = {
        .sType      = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext      = Null,
        .renderPass = surface->render_pass,
        .renderArea =
            {.offset = {.x = 0, .y = 0}, .extent = surface->swapchain_image_extent},
        .framebuffer     = surface->framebuffers[next_image_index],
        .clearValueCount = 1,
        .pClearValues    = &clear_value
    };

    vkCmdBeginRenderPass (cmd, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass (cmd);

    res = vkEndCommandBuffer (cmd);
    RETURN_VALUE_IF (
        res != VK_SUCCESS,
        Null,
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
    RETURN_VALUE_IF (
        res != VK_SUCCESS,
        Null,
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
        RETURN_VALUE_IF (
            !surface_recreate_swapchain (surface, win),
            Null,
            "Failed to recreate swapchain\n"
        );
        return surface;
    }
    RETURN_VALUE_IF (
        res != VK_SUCCESS,
        Null,
        "Failed to present rendered images to surface. RET = %d\n",
        res
    );

    return surface;
}
