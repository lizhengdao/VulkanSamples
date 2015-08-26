/*
 * Vulkan Samples Kit
 *
 * Copyright (C) 2015 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
VULKAN_SAMPLE_DESCRIPTION
samples "init" utility functions
*/

#include <cstdlib>
#include <assert.h>
#include "util_init.hpp"

#ifdef _WIN32
#include <windows.h>
#define APP_NAME_STR_LEN 80
#else  // _WIN32
#include <xcb/xcb.h>
#endif // _WIN32

using namespace std;

/*
 * TODO: function description here
 */
VkResult init_global_extension_properties(
        struct sample_info &info,
        layer_properties *layer_props)
{
    VkExtensionProperties *instance_extensions;
    uint32_t instance_extension_count;
    VkResult res;
    char *layer_name = NULL;

    if (layer_props) {
        layer_name = layer_props->properties.layerName;
    }

    do {
        res = vkGetGlobalExtensionProperties(layer_name, &instance_extension_count, NULL);
        if (res)
            return res;

        if (instance_extension_count == 0) {
            return VK_SUCCESS;
        }

        if (layer_props) {
            layer_props->extensions.reserve(instance_extension_count);
            instance_extensions = layer_props->extensions.data();
        } else {
            info.instance_extension_properties.reserve(instance_extension_count);
            instance_extensions = info.instance_extension_properties.data();
        }
        res = vkGetGlobalExtensionProperties(
                  layer_name,
                  &instance_extension_count,
                  instance_extensions);
    } while (res == VK_INCOMPLETE);

    return res;
}

/*
 * TODO: function description here
 */
VkResult init_global_layer_properties(struct sample_info &info)
{
    uint32_t instance_layer_count;
    VkLayerProperties *vk_props = NULL;
    VkResult res;

    /*
     * It's possible, though very rare, that the number of
     * instance layers could change. For example, installing something
     * could include new layers that the loader would pick up
     * between the initial query for the count and the
     * request for VkLayerProperties. The loader indicates that
     * by returning a VK_INCOMPLETE status and will update the
     * the count parameter.
     * The count parameter will be updated with the number of
     * entries loaded into the data pointer - in case the number
     * of layers went down or is smaller than the size given.
     */
    do {
        res = vkGetGlobalLayerProperties(&instance_layer_count, NULL);
        if (res)
            return res;

        if (instance_layer_count == 0) {
            return VK_SUCCESS;
        }

        vk_props = (VkLayerProperties *) realloc(vk_props, instance_layer_count * sizeof(VkLayerProperties));

        res = vkGetGlobalLayerProperties(&instance_layer_count, vk_props);
    } while (res == VK_INCOMPLETE);

    /*
     * Now gather the extension list for each instance layer.
     */
    for (uint32_t i = 0; i < instance_layer_count; i++) {
        layer_properties layer_props;
        layer_props.properties = vk_props[i];
        res = init_global_extension_properties(
                  info, &layer_props);
        if (res)
            return res;
        info.instance_layer_properties.push_back(layer_props);
    }
    free(vk_props);

    return res;
}

VkResult init_instance(struct sample_info &info, char const*const app_short_name)
{
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = NULL;
    app_info.pAppName = app_short_name;
    app_info.appVersion = 1;
    app_info.pEngineName = app_short_name;
    app_info.engineVersion = 1;
    app_info.apiVersion = VK_API_VERSION;

    VkInstanceCreateInfo inst_info = {};
    inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pNext = NULL;
    inst_info.pAppInfo = &app_info;
    inst_info.pAllocCb = NULL;
    inst_info.extensionCount = 0;
    inst_info.ppEnabledExtensionNames = NULL;

    VkResult res = vkCreateInstance(&inst_info, &info.inst);
    assert(!res);

    return res;
}

VkResult init_device(struct sample_info &info)
{
    VkDeviceQueueCreateInfo queue_info = {};
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;

    VkDeviceCreateInfo device_info = {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.pNext = NULL;
    device_info.queueRecordCount = 1;
    device_info.pRequestedQueues = &queue_info;
    device_info.extensionCount = 0;
    device_info.ppEnabledExtensionNames = NULL;
    device_info.flags = 0;

    VkResult res = vkCreateDevice(info.gpu, &device_info, &info.device);
    assert(!res);

    return res;
}

VkResult init_enumerate_device(struct sample_info &info, uint32_t gpu_count)
{
    uint32_t const req_count = gpu_count;
    VkResult res = vkEnumeratePhysicalDevices(info.inst, &gpu_count, &info.gpu);
    assert(!res && gpu_count >= req_count);

    return res;
}

void init_connection(struct sample_info &info)
{
#ifndef _WIN32
    const xcb_setup_t *setup;
    xcb_screen_iterator_t iter;
    int scr;

    info.connection = xcb_connect(NULL, &scr);
    if (info.connection == NULL) {
        std::cout << "Cannot find a compatible Vulkan ICD.\n";
        exit(-1);
    }

    setup = xcb_get_setup(info.connection);
    iter = xcb_setup_roots_iterator(setup);
    while (scr-- > 0)
        xcb_screen_next(&iter);

    info.screen = iter.data;
#endif // _WIN32
}
#ifdef _WIN32
static void run(struct sample_info *info)
{
 /* Placeholder for samples that want to show dynamic content */
}

// MS-Windows event handling function:
LRESULT CALLBACK WndProc(HWND hWnd,
                         UINT uMsg,
                         WPARAM wParam,
                         LPARAM lParam)
{
    struct sample_info *info = reinterpret_cast<struct sample_info*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch(uMsg)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    case WM_PAINT:
        run(info);
        return 0;
    default:
        break;
    }
    return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

void init_window(struct sample_info &info)
{
    WNDCLASSEX  win_class;

    info.connection = GetModuleHandle(NULL);

    // Initialize the window class structure:
    win_class.cbSize = sizeof(WNDCLASSEX);
    win_class.style = CS_HREDRAW | CS_VREDRAW;
    win_class.lpfnWndProc = WndProc;
    win_class.cbClsExtra = 0;
    win_class.cbWndExtra = 0;
    win_class.hInstance = info.connection; // hInstance
    win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    win_class.lpszMenuName = NULL;
    win_class.lpszClassName = info.name;
    win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
    // Register window class:
    if (!RegisterClassEx(&win_class)) {
        // It didn't work, so try to give a useful error:
        printf("Unexpected error trying to start the application!\n");
        fflush(stdout);
        exit(1);
    }
    // Create window with the registered class:
    RECT wr = { 0, 0, info.width, info.height };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    info.window = CreateWindowEx(0,
                                  info.name,           // class name
                                  info.name,           // app name
                                  WS_OVERLAPPEDWINDOW | // window style
                                  WS_VISIBLE |
                                  WS_SYSMENU,
                                  100,100,              // x/y coords
                                  wr.right-wr.left,     // width
                                  wr.bottom-wr.top,     // height
                                  NULL,                 // handle to parent
                                  NULL,                 // handle to menu
                                  info.connection,     // hInstance
                                  NULL);                // no extra parameters
    if (!info.window) {
        // It didn't work, so try to give a useful error:
        printf("Cannot create a window in which to draw!\n");
        fflush(stdout);
        exit(1);
    }
    SetWindowLongPtr(info.window, GWLP_USERDATA, (LONG_PTR) &info);
}
#else
void init_window(struct sample_info &info)
{
    uint32_t value_mask, value_list[32];

    info.window = xcb_generate_id(info.connection);

    value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    value_list[0] = info.screen->black_pixel;
    value_list[1] = XCB_EVENT_MASK_KEY_RELEASE |
                    XCB_EVENT_MASK_EXPOSURE;

    xcb_create_window(info.connection,
            XCB_COPY_FROM_PARENT,
            info.window, info.screen->root,
            0, 0, info.width, info.height, 0,
            XCB_WINDOW_CLASS_INPUT_OUTPUT,
            info.screen->root_visual,
            value_mask, value_list);

    /* Magic code that will send notification when window is destroyed */
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(info.connection, 1, 12,
                                                      "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(info.connection, cookie, 0);

    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(info.connection, 0, 16, "WM_DELETE_WINDOW");
    info.atom_wm_delete_window = xcb_intern_atom_reply(info.connection, cookie2, 0);

    xcb_change_property(info.connection, XCB_PROP_MODE_REPLACE,
                        info.window, (*reply).atom, 4, 32, 1,
                        &(*info.atom_wm_delete_window).atom);
    free(reply);

    xcb_map_window(info.connection, info.window);

    // Force the x/y coordinates to 100,100 results are identical in consecutive runs
    const uint32_t coords[] = {100,  100};
    xcb_configure_window(info.connection, info.window,
                         XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);
}
#endif // _WIN32

void init_depth_buffer(struct sample_info &info)
{
    VkResult res;
    VkImageCreateInfo image_info = {};
    const VkFormat depth_format = VK_FORMAT_D16_UNORM;
    VkFormatProperties props;
    res = vkGetPhysicalDeviceFormatProperties(info.gpu, depth_format, &props);
    assert(!res);
    if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        image_info.tiling = VK_IMAGE_TILING_LINEAR;
    } else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    } else {
        /* Try other depth formats? */
        std::cout << "VK_FORMAT_D16_UNORM Unsupported.\n";
        exit(-1);
    }

    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.pNext = NULL;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = depth_format;
    image_info.extent.width = info.width;
    image_info.extent.height = info.height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arraySize = 1;
    image_info.samples = 1;
    image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_BIT;
    image_info.flags = 0;

    VkMemoryAllocInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOC_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    VkAttachmentViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_ATTACHMENT_VIEW_CREATE_INFO;
    view_info.pNext = NULL;
    view_info.image.handle = VK_NULL_HANDLE;
    view_info.format = depth_format;
    view_info.mipLevel = 0;
    view_info.baseArraySlice = 0;
    view_info.arraySize = 1;
    view_info.flags = 0;

    VkMemoryRequirements mem_reqs;

    info.depth.format = depth_format;

    /* Create image */
    res = vkCreateImage(info.device, &image_info,
                        &info.depth.image);
    assert(!res);

    res = vkGetImageMemoryRequirements(info.device,
                                       info.depth.image, &mem_reqs);

    mem_alloc.allocationSize = mem_reqs.size;
    /* Use the memory properties to determine the type of memory required */
    res = memory_type_from_properties(info,
                                      mem_reqs.memoryTypeBits,
                                      VK_MEMORY_PROPERTY_DEVICE_ONLY,
                                      &mem_alloc.memoryTypeIndex);
    assert(!res);

    /* Allocate memory */
    res = vkAllocMemory(info.device, &mem_alloc, &info.depth.mem);
    assert(!res);

    /* Bind memory */
    res = vkBindImageMemory(info.device, info.depth.image,
                            info.depth.mem, 0);
    assert(!res);

    /* Set the image layout to depth stencil optimal */
    set_image_layout(info, info.depth.image,
                          VK_IMAGE_ASPECT_DEPTH,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    /* Create image view */
    view_info.image = info.depth.image;
    res = vkCreateAttachmentView(info.device, &view_info, &info.depth.view);
    assert(!res);
}

void init_wsi(struct sample_info &info)
{
    /* DEPENDS on init_connection() and init_window() */

    VkResult res;

    GET_INSTANCE_PROC_ADDR(info.inst, GetPhysicalDeviceSurfaceSupportWSI);
    GET_DEVICE_PROC_ADDR(info.device, GetSurfaceInfoWSI);
    GET_DEVICE_PROC_ADDR(info.device, CreateSwapChainWSI);
    GET_DEVICE_PROC_ADDR(info.device, CreateSwapChainWSI);
    GET_DEVICE_PROC_ADDR(info.device, DestroySwapChainWSI);
    GET_DEVICE_PROC_ADDR(info.device, GetSwapChainInfoWSI);
    GET_DEVICE_PROC_ADDR(info.device, AcquireNextImageWSI);
    GET_DEVICE_PROC_ADDR(info.device, QueuePresentWSI);

    res = vkGetPhysicalDeviceQueueCount(info.gpu, &info.queue_count);
    assert(!res);
    assert(info.queue_count >= 1);

    info.queue_props.reserve(info.queue_count);
    res = vkGetPhysicalDeviceQueueProperties(info.gpu, info.queue_count, info.queue_props.data());
    assert(!res);
    assert(info.queue_count >= 1);

    // Construct the WSI surface description:
    info.surface_description.sType = VK_STRUCTURE_TYPE_SURFACE_DESCRIPTION_WINDOW_WSI;
    info.surface_description.pNext = NULL;
#ifdef _WIN32
    info.surface_description.platform = VK_PLATFORM_WIN32_WSI;
    info.surface_description.pPlatformHandle = info.connection;
    info.surface_description.pPlatformWindow = info.window;
#else  // _WIN32
    info.platform_handle_xcb.connection = info.connection;
    info.platform_handle_xcb.root = info.screen->root;
    info.surface_description.platform = VK_PLATFORM_XCB_WSI;
    info.surface_description.pPlatformHandle = &info.platform_handle_xcb;
    info.surface_description.pPlatformWindow = &info.window;
#endif // _WIN32

    // Iterate over each queue to learn whether it supports presenting to WSI:
    VkBool32* supportsPresent = (VkBool32 *)malloc(info.queue_count * sizeof(VkBool32));
    for (uint32_t i = 0; i < info.queue_count; i++) {
        info.fpGetPhysicalDeviceSurfaceSupportWSI(info.gpu, i,
                                                   (VkSurfaceDescriptionWSI *) &info.surface_description,
                                                   &supportsPresent[i]);
    }

    // Search for a graphics queue and a present queue in the array of queue
    // families, try to find one that supports both
    uint32_t graphicsQueueNodeIndex = UINT32_MAX;
    uint32_t presentQueueNodeIndex  = UINT32_MAX;
    for (uint32_t i = 0; i < info.queue_count; i++) {
        if ((info.queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            if (graphicsQueueNodeIndex == UINT32_MAX) {
                graphicsQueueNodeIndex = i;
            }

            if (supportsPresent[i] == VK_TRUE) {
                graphicsQueueNodeIndex = i;
                presentQueueNodeIndex = i;
                break;
            }
        }
    }
    if (presentQueueNodeIndex == UINT32_MAX) {
        // If didn't find a queue that supports both graphics and present, then
        // find a separate present queue.
        for (uint32_t i = 0; i < info.queue_count; ++i) {
            if (supportsPresent[i] == VK_TRUE) {
                presentQueueNodeIndex = i;
                break;
            }
        }
    }
    free(supportsPresent);

    // Generate error if could not find both a graphics and a present queue
    if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX) {
        std::cout << "Could not find a graphics and a present queue\nCould not find a graphics and a present queue\n";
        exit(-1);
    }

    info.graphics_queue_family_index = graphicsQueueNodeIndex;

    res = vkGetDeviceQueue(info.device, info.graphics_queue_family_index,
            0, &info.queue);
    assert(!res);

    // Get the list of VkFormats that are supported:
    size_t formatsSize;
    res = info.fpGetSurfaceInfoWSI(info.device,
                                    (VkSurfaceDescriptionWSI *) &info.surface_description,
                                    VK_SURFACE_INFO_TYPE_FORMATS_WSI,
                                    &formatsSize, NULL);
    assert(!res);
    VkSurfaceFormatPropertiesWSI *surfFormats = (VkSurfaceFormatPropertiesWSI *)malloc(formatsSize);
    res = info.fpGetSurfaceInfoWSI(info.device,
                                    (VkSurfaceDescriptionWSI *) &info.surface_description,
                                    VK_SURFACE_INFO_TYPE_FORMATS_WSI,
                                    &formatsSize, surfFormats);
    assert(!res);
    // If the format list includes just one entry of VK_FORMAT_UNDEFINED,
    // the surface has no preferred format.  Otherwise, at least one
    // supported format will be returned.
    size_t formatCount = formatsSize / sizeof(VkSurfaceFormatPropertiesWSI);
    if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        info.format = VK_FORMAT_B8G8R8A8_UNORM;
    }
    else
    {
        assert(formatCount >= 1);
        info.format = surfFormats[0].format;
    }
}

void init_swap_chain(struct sample_info &info)
{
    /* DEPENDS on info.cmd and info.queue initialized */

    VkResult res;
    size_t capsSize;
    size_t presentModesSize;
    res = info.fpGetSurfaceInfoWSI(info.device,
        (const VkSurfaceDescriptionWSI *)&info.surface_description,
        VK_SURFACE_INFO_TYPE_PROPERTIES_WSI, &capsSize, NULL);
    assert(!res);
    res = info.fpGetSurfaceInfoWSI(info.device,
        (const VkSurfaceDescriptionWSI *)&info.surface_description,
        VK_SURFACE_INFO_TYPE_PRESENT_MODES_WSI, &presentModesSize, NULL);
    assert(!res);

    VkSurfacePropertiesWSI *surfProperties =
        (VkSurfacePropertiesWSI *)malloc(capsSize);
    VkSurfacePresentModePropertiesWSI *presentModes =
        (VkSurfacePresentModePropertiesWSI *)malloc(presentModesSize);

    res = info.fpGetSurfaceInfoWSI(info.device,
        (const VkSurfaceDescriptionWSI *)&info.surface_description,
        VK_SURFACE_INFO_TYPE_PROPERTIES_WSI, &capsSize, surfProperties);
    assert(!res);
    res = info.fpGetSurfaceInfoWSI(info.device,
        (const VkSurfaceDescriptionWSI *)&info.surface_description,
        VK_SURFACE_INFO_TYPE_PRESENT_MODES_WSI, &presentModesSize, presentModes);
    assert(!res);

    VkExtent2D swapChainExtent;
    // width and height are either both -1, or both not -1.
    if (surfProperties->currentExtent.width == -1)
    {
        // If the surface size is undefined, the size is set to
        // the size of the images requested.
        swapChainExtent.width = info.width;
        swapChainExtent.height = info.height;
    }
    else
    {
        // If the surface size is defined, the swap chain size must match
        swapChainExtent = surfProperties->currentExtent;
    }

    // If mailbox mode is available, use it, as is the lowest-latency non-
    // tearing mode.  If not, try IMMEDIATE which will usually be available,
    // and is fastest (though it tears).  If not, fall back to FIFO which is
    // always available.
    VkPresentModeWSI swapChainPresentMode = VK_PRESENT_MODE_FIFO_WSI;
    size_t presentModeCount = presentModesSize / sizeof(VkSurfacePresentModePropertiesWSI);
    for (size_t i = 0; i < presentModeCount; i++) {
        if (presentModes[i].presentMode == VK_PRESENT_MODE_MAILBOX_WSI) {
            swapChainPresentMode = VK_PRESENT_MODE_MAILBOX_WSI;
            break;
        }
        if ((swapChainPresentMode != VK_PRESENT_MODE_MAILBOX_WSI) &&
            (presentModes[i].presentMode == VK_PRESENT_MODE_IMMEDIATE_WSI)) {
            swapChainPresentMode = VK_PRESENT_MODE_IMMEDIATE_WSI;
        }
    }

#define WORK_AROUND_CODE
#ifdef WORK_AROUND_CODE
    uint32_t desiredNumberOfSwapChainImages = 2;
#else  // WORK_AROUND_CODE
    // Determine the number of VkImage's to use in the swap chain (we desire to
    // own only 1 image at a time, besides the images being displayed and
    // queued for display):
    uint32_t desiredNumberOfSwapChainImages = surfProperties->minImageCount + 1;
    if ((surfProperties->maxImageCount > 0) &&
        (desiredNumberOfSwapChainImages > surfProperties->maxImageCount))
    {
        // Application must settle for fewer images than desired:
        desiredNumberOfSwapChainImages = surfProperties->maxImageCount;
    }
#endif // WORK_AROUND_CODE

    VkSurfaceTransformWSI preTransform;
    if (surfProperties->supportedTransforms & VK_SURFACE_TRANSFORM_NONE_BIT_WSI) {
        preTransform = VK_SURFACE_TRANSFORM_NONE_WSI;
    } else {
        preTransform = surfProperties->currentTransform;
    }

    VkSwapChainCreateInfoWSI swap_chain = {};
    swap_chain.sType = VK_STRUCTURE_TYPE_SWAP_CHAIN_CREATE_INFO_WSI;
    swap_chain.pNext = NULL;
    swap_chain.pSurfaceDescription = (const VkSurfaceDescriptionWSI *)&info.surface_description;
    swap_chain.minImageCount = desiredNumberOfSwapChainImages;
    swap_chain.imageFormat = info.format;
    swap_chain.imageExtent.width = swapChainExtent.width;
    swap_chain.imageExtent.height = swapChainExtent.height;
    swap_chain.preTransform = preTransform;
    swap_chain.imageArraySize = 1;
    swap_chain.presentMode = swapChainPresentMode;
    swap_chain.oldSwapChain.handle = 0;
    swap_chain.clipped = true;

    res = info.fpCreateSwapChainWSI(info.device, &swap_chain, &info.swap_chain);
    assert(!res);

    size_t swapChainImagesSize;
    res = info.fpGetSwapChainInfoWSI(info.device, info.swap_chain,
                                      VK_SWAP_CHAIN_INFO_TYPE_IMAGES_WSI,
                                      &swapChainImagesSize, NULL);
    assert(!res);

    VkSwapChainImagePropertiesWSI* swapChainImages = (VkSwapChainImagePropertiesWSI*)malloc(swapChainImagesSize);
    assert(swapChainImages);
    res = info.fpGetSwapChainInfoWSI(info.device, info.swap_chain,
                                      VK_SWAP_CHAIN_INFO_TYPE_IMAGES_WSI,
                                      &swapChainImagesSize, swapChainImages);
    assert(!res);

#ifdef WORK_AROUND_CODE
    info.swapChainImageCount = 2;
#else  // WORK_AROUND_CODE
    // The number of images within the swap chain is determined based on the size of the info returned
    info.swapChainImageCount = swapChainImagesSize / sizeof(VkSwapChainImagePropertiesWSI);
#endif // WORK_AROUND_CODE

    info.buffers.reserve(info.swapChainImageCount);

    for (int i = 0; i < info.swapChainImageCount; i++) {
        swap_chain_buffer sc_buffer;

        VkAttachmentViewCreateInfo color_attachment_view = {};
        color_attachment_view.sType = VK_STRUCTURE_TYPE_ATTACHMENT_VIEW_CREATE_INFO;
        color_attachment_view.pNext = NULL;
        color_attachment_view.format = info.format;
        color_attachment_view.mipLevel = 0;
        color_attachment_view.baseArraySlice = 0;
        color_attachment_view.arraySize = 1;


        sc_buffer.image = swapChainImages[i].image;

        set_image_layout(info, sc_buffer.image,
                               VK_IMAGE_ASPECT_COLOR,
                               VK_IMAGE_LAYOUT_UNDEFINED,
                               VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        color_attachment_view.image = sc_buffer.image;

        res = vkCreateAttachmentView(info.device,
                &color_attachment_view, &sc_buffer.view);
        info.buffers.push_back(sc_buffer);
        assert(!res);
    }
}

void init_descriptor_and_pipeline_layouts(struct sample_info &info)
{
    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_binding.arraySize = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layout_binding.pImmutableSamplers = NULL;

    /* Next take layout bindings and use them to create a descriptor set layout */
    VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
    descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout.pNext = NULL;
    descriptor_layout.count = 1;
    descriptor_layout.pBinding = &layout_binding;

    VkResult err;

    err = vkCreateDescriptorSetLayout(info.device,
            &descriptor_layout, &info.desc_layout);
    assert(!err);

    /* Now use the descriptor layout to create a pipeline layout */
    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType              = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext              = NULL;
    pPipelineLayoutCreateInfo.descriptorSetCount = 1;
    pPipelineLayoutCreateInfo.pSetLayouts        = &info.desc_layout;

    err = vkCreatePipelineLayout(info.device,
                                 &pPipelineLayoutCreateInfo,
                                 &info.pipeline_layout);
    assert(!err);
}

void init_command_buffer(struct sample_info &info)
{
    /* DEPENDS on init_wsi() */

    VkResult res;

    VkCmdPoolCreateInfo cmd_pool_info = {};
    cmd_pool_info.sType = VK_STRUCTURE_TYPE_CMD_POOL_CREATE_INFO;
    cmd_pool_info.pNext = NULL;
    cmd_pool_info.queueFamilyIndex = info.graphics_queue_family_index;
    cmd_pool_info.flags = 0;

    res = vkCreateCommandPool(info.device, &cmd_pool_info, &info.cmd_pool);
    assert(!res);

    VkCmdBufferCreateInfo cmd = {};
    cmd.sType = VK_STRUCTURE_TYPE_CMD_BUFFER_CREATE_INFO;
    cmd.pNext = NULL;
    cmd.cmdPool = info.cmd_pool;
    cmd.level = VK_CMD_BUFFER_LEVEL_PRIMARY;
    cmd.flags = 0;

    res = vkCreateCommandBuffer(info.device, &cmd, &info.cmd);
    assert(!res);
}

void init_device_queue(struct sample_info &info)
{
    /* DEPENDS on init_wsi() */

    VkResult res;
    res = vkGetDeviceQueue(info.device, info.graphics_queue_family_index,
            0, &info.queue);
    assert(!res);
}
