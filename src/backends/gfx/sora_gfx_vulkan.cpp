// sora_gfx_vulkan.cpp

#ifndef SORA_GFX_VULKAN_CPP
#define SORA_GFX_VULKAN_CPP

//- includes 

#pragma comment(lib, "vulkan-1.lib")

// debug callback

function VKAPI_ATTR VkBool32 VKAPI_CALL 
gfx_vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
                      VkDebugUtilsMessageTypeFlagsEXT messageType,
                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                      void* pUserData) {
    printf("[validation] %s\n", pCallbackData->pMessage);
    return VK_FALSE;
}

//- state functions

function void
gfx_init() {
    
    temp_t scratch = scratch_begin();
    
    // create arenas
	gfx_vk_state.resource_arena = arena_create(megabytes(64));
	gfx_vk_state.renderer_arena = arena_create(megabytes(64));
	
	// init resource list
	gfx_vk_state.resource_first = nullptr;
	gfx_vk_state.resource_last = nullptr;
	gfx_vk_state.resource_free = nullptr;
	
	// init renderer list
	gfx_vk_state.renderer_first = nullptr;
	gfx_vk_state.renderer_last = nullptr;
	gfx_vk_state.renderer_free = nullptr;
	gfx_vk_state.renderer_active = nullptr;
    
    VkResult result = VK_SUCCESS;
    
#if 0
    // supported layers
    {
        u32 supported_layer_count = 0;
        vkEnumerateInstanceLayerProperties(&supported_layer_count, nullptr);
        VkLayerProperties* supported_layers = (VkLayerProperties*)arena_alloc(scratch.arena, sizeof(VkLayerProperties) * supported_layer_count);
        vkEnumerateInstanceLayerProperties(&supported_layer_count, supported_layers);
        
        printf("[info] [gfx] supported layers: \n");
        for (i32 i = 0 ; i < supported_layer_count; i++) {
            printf("\t%s\n", supported_layers[i].layerName);
        }
    }
    
    // supported extensions
    {
        u32 supported_extension_count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &supported_extension_count, nullptr);
        VkExtensionProperties* supported_extensions = (VkExtensionProperties*)arena_alloc(scratch.arena, sizeof(VkExtensionProperties) * supported_extension_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &supported_extension_count, supported_extensions);
        
        printf("[info] [gfx] supported extensions: \n");
        for (i32 i = 0 ; i < supported_extension_count; i++) {
            printf("\t%s\n", supported_extensions[i].extensionName);
        }
    }
#endif
    
    // TODO: check required layers are supported
    // get required layers
    u32 required_layer_count = 0;
    cstr required_layers[] = {
        "VK_LAYER_KHRONOS_validation",
    };
    required_layer_count = array_count(required_layers);
    
    // TODO: check required extensions are supported
    // get required extensions (implemented per backend)
    u32 required_extension_count = 0;
    cstr* required_extensions = gfx_vk_get_extensions(&required_extension_count);
    
    // setup app info
    VkApplicationInfo app_info = { 0 };
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "sora engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;
    
    // create instance
    VkInstanceCreateInfo instance_create_info = { 0 };
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.flags = 0;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledLayerCount = required_layer_count;
    instance_create_info.ppEnabledLayerNames = required_layers;
    instance_create_info.enabledExtensionCount = required_extension_count;
    instance_create_info.ppEnabledExtensionNames = required_extensions;
    
    result = vkCreateInstance(&instance_create_info, nullptr, &gfx_vk_state.instance);
    if (result != VK_SUCCESS) {
        cstr result_message = gfx_vk_str_from_result(result);
        os_graphical_message(true, str("[gfx] Initialization Error"), str_format(scratch.arena, "Failed to create vulkan instance!\nResult: %s", result_message));
        os_abort(1);
    }
    
    // create debug layer
    VkDebugUtilsMessengerCreateInfoEXT debug_util_create_info = { 0 };
    debug_util_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_util_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_util_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_util_create_info.pfnUserCallback = gfx_vk_debug_callback;
    debug_util_create_info.pUserData = nullptr;
    
    // find vkCreateDebugUtilsMessengerEXT function
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(gfx_vk_state.instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(gfx_vk_state.instance, &debug_util_create_info, nullptr, &gfx_vk_state.debug_messenger);
    }
    
    // pick physical device
    u32 physical_device_count = 0;
    vkEnumeratePhysicalDevices(gfx_vk_state.instance, &physical_device_count, nullptr);
    
    if (physical_device_count == 0) {
        os_graphical_message(true, str("[gfx] Initialization Error"), str_format(scratch.arena, "Failed to find GPUs with Vulkan Support!"));
        os_abort(1);
    }
    
    VkPhysicalDevice* physical_devices = (VkPhysicalDevice*)arena_alloc(scratch.arena, sizeof(VkPhysicalDevice) * physical_device_count);
    vkEnumeratePhysicalDevices(gfx_vk_state.instance, &physical_device_count,  physical_devices);
    
#if 0
    printf("[info] [gfx] supported gpus:\n");
    for(i32 i = 0; i < physical_device_count; i++) {
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(physical_devices[i], & device_properties);
        printf("\t%s\n",  device_properties.deviceName);
    }
#endif
    
    for (i32 i = 0; i < physical_device_count; i++) {
        // get device properties and features
        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceProperties(physical_devices[i], & device_properties);
        vkGetPhysicalDeviceFeatures(physical_devices[i], &device_features);
    }
    
    // TODO: right now we just grab the first availible device, fix this. 
    gfx_vk_state.physical_device = physical_devices[0];
    
    // find graphics queue families
    gfx_vk_state.queue_indices = gfx_vk_get_queue_family_indices(gfx_vk_state.physical_device);
    
    // create queues
    f32 queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_info[2] = { { 0 }, { 0 } };
    queue_create_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info[0].queueFamilyIndex = gfx_vk_state.queue_indices.graphics_family_index;
    queue_create_info[0].queueCount = 1;
    queue_create_info[0].pQueuePriorities = &queue_priority;
    
    queue_create_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info[1].queueFamilyIndex = gfx_vk_state.queue_indices.present_family_index;
    queue_create_info[1].queueCount = 1;
    queue_create_info[2].pQueuePriorities = &queue_priority;
    
    // device extensions
    u32 device_extensions_count = 0;
    cstr device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    device_extensions_count = array_count(device_extensions);
    
    // device features
    VkPhysicalDeviceFeatures device_features = { 0 };
    
    // create logical device
    VkDeviceCreateInfo device_create_info = { 0 };
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.flags = 0;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = queue_create_info;
    device_create_info.enabledLayerCount = 0; // deprecated
    device_create_info.ppEnabledLayerNames = nullptr; // deprecated
    device_create_info.enabledExtensionCount = device_extensions_count;
    device_create_info.ppEnabledExtensionNames = device_extensions;
    device_create_info.pEnabledFeatures = &device_features;
    
    if (gfx_vk_state.queue_indices.graphics_family_index != gfx_vk_state.queue_indices.present_family_index) {
        device_create_info.queueCreateInfoCount = 2;
    }
    
    result = vkCreateDevice(gfx_vk_state.physical_device, &device_create_info, nullptr, &gfx_vk_state.device);
    if (result != VK_SUCCESS) {
        os_graphical_message(true, str("[gfx] Initialization Error"), str_format(scratch.arena, "Failed to create logical device!"));
        os_abort(1);
    }
    
    // get device queues
    vkGetDeviceQueue(gfx_vk_state.device, gfx_vk_state.queue_indices.graphics_family_index, 0, &gfx_vk_state.graphics_queue);
    vkGetDeviceQueue(gfx_vk_state.device, gfx_vk_state.queue_indices.present_family_index, 0, &gfx_vk_state.present_queue);
    
    scratch_end(scratch);
}

function void
gfx_release() {
    
    arena_release(gfx_vk_state.resource_arena);
    arena_release(gfx_vk_state.renderer_arena);
    
    // destroy debug message
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(gfx_vk_state.instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(gfx_vk_state.instance, gfx_vk_state.debug_messenger, nullptr);
    }
    
    vkDestroyDevice(gfx_vk_state.device, nullptr);
    vkDestroyInstance(gfx_vk_state.instance, nullptr);
    
}

function void
gfx_update() {
    // TODO: remove
}


//- renderer 

function gfx_handle_t
gfx_renderer_create(os_handle_t window, color_t clear_color) {
    
    temp_t scratch = scratch_begin();
    
    // get from resource pool or create one
	gfx_vk_renderer_t* renderer = gfx_vk_state.renderer_free;
	if (renderer != nullptr) {
		stack_pop(gfx_vk_state.renderer_free);
	} else {
		renderer = (gfx_vk_renderer_t*)arena_alloc(gfx_vk_state.renderer_arena, sizeof(gfx_vk_renderer_t));
	}
	memset(renderer, 0, sizeof(gfx_vk_renderer_t));
	dll_push_back(gfx_vk_state.renderer_first, gfx_vk_state.renderer_last, renderer);
    
    renderer->window = window;
	renderer->clear_color = clear_color;
	renderer->size = os_window_get_size(window);
    
    renderer->arena = arena_create(megabytes(64));
    
    VkResult result = VK_SUCCESS;
    
    // create surface (implemented per backend)
    gfx_vk_surface_create(renderer);
    
    // create swapchain
    gfx_vk_renderer_create_swapchain(renderer);
    
    // create per frame resources
    
    VkSemaphoreCreateInfo semaphore_create_info = { 0 };
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fence_create_info = { 0 };
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    renderer->image_available_semaphore = (VkSemaphore*)arena_alloc(renderer->arena, sizeof(VkSemaphore) * renderer->swapchain_image_count);
    renderer->rendering_finished_semaphore = (VkSemaphore*)arena_alloc(renderer->arena, sizeof(VkSemaphore) * renderer->swapchain_image_count);
    renderer->in_flight_fences = (VkFence*)arena_alloc(renderer->arena, sizeof(VkFence) * renderer->swapchain_image_count);
    
    for (i32 i = 0; i < renderer->swapchain_image_count; i++) {
        result = vkCreateSemaphore(gfx_vk_state.device, &semaphore_create_info, nullptr, &renderer->image_available_semaphore[i]);
        result = vkCreateSemaphore(gfx_vk_state.device, &semaphore_create_info, nullptr, &renderer->rendering_finished_semaphore[i]);
        result = vkCreateFence(gfx_vk_state.device, &fence_create_info, nullptr, &renderer->in_flight_fences[i]);
    }
    
    // create command queues
    VkCommandPoolCreateInfo command_pool_create_info = { 0 };
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.queueFamilyIndex = gfx_vk_state.queue_indices.present_family_index;
    
    result = vkCreateCommandPool(gfx_vk_state.device, &command_pool_create_info, nullptr, &renderer->command_pool);
    if (result != VK_SUCCESS) {
        cstr result_string = gfx_vk_str_from_result(result);
        os_graphical_message(true, str("[gfx] Renderer Initialization Error"), str_format(scratch.arena, "Failed to create command pool!\nresult: %s", result_string));
        os_abort(1);
    }
    
    renderer->command_buffers = (VkCommandBuffer*)arena_alloc(renderer->arena, sizeof(VkCommandBuffer) * renderer->swapchain_image_count);
    
    VkCommandBufferAllocateInfo command_buffer_alloc_info = { 0 };
    command_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_alloc_info.commandPool = renderer->command_pool;
    command_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_alloc_info.commandBufferCount = renderer->swapchain_image_count;
    
    result = vkAllocateCommandBuffers(gfx_vk_state.device, &command_buffer_alloc_info, renderer->command_buffers);
    if (result != VK_SUCCESS) {
        cstr result_string = gfx_vk_str_from_result(result);
        os_graphical_message(true, str("[gfx] Renderer Initialization Error"), str_format(scratch.arena, "Failed to create command pool buffers!\nresult: %s", result_string));
        os_abort(1);
    }
    
    // prepare command buffers
    VkCommandBufferBeginInfo command_buffer_begin_info = { 0 };
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    
    VkClearColorValue vk_clear_color = { { clear_color.r, clear_color.g, clear_color.b, clear_color.a } };
    
    VkImageSubresourceRange image_subresource_range = { 0 };
    image_subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_subresource_range.baseMipLevel = 0;
    image_subresource_range.levelCount = 1;
    image_subresource_range.baseArrayLayer = 0;
    image_subresource_range.layerCount = 1;
    
    for (i32 i = 0; i < renderer->swapchain_image_count; i++) {
        
        VkImageMemoryBarrier present_to_clear_barrier = { 0 };
        present_to_clear_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        present_to_clear_barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        present_to_clear_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        present_to_clear_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        present_to_clear_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        present_to_clear_barrier.srcQueueFamilyIndex = gfx_vk_state.queue_indices.present_family_index;
        present_to_clear_barrier.dstQueueFamilyIndex = gfx_vk_state.queue_indices.present_family_index;
        present_to_clear_barrier.image = renderer->swapchain_images[i];
        present_to_clear_barrier.subresourceRange = image_subresource_range;
        
        VkImageMemoryBarrier clear_to_present_barrier = { 0 };
        clear_to_present_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        clear_to_present_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        clear_to_present_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        clear_to_present_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        clear_to_present_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        clear_to_present_barrier.srcQueueFamilyIndex = gfx_vk_state.queue_indices.present_family_index;
        clear_to_present_barrier.dstQueueFamilyIndex = gfx_vk_state.queue_indices.present_family_index;
        clear_to_present_barrier.image =  renderer->swapchain_images[i];
        clear_to_present_barrier.subresourceRange = image_subresource_range;
        
        vkBeginCommandBuffer(renderer->command_buffers[i], &command_buffer_begin_info);
        vkCmdPipelineBarrier(renderer->command_buffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &present_to_clear_barrier );
        vkCmdClearColorImage(renderer->command_buffers[i], renderer->swapchain_images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &vk_clear_color, 1, &image_subresource_range);
        vkCmdPipelineBarrier(renderer->command_buffers[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &clear_to_present_barrier);
        
        result = vkEndCommandBuffer(renderer->command_buffers[i]);
        if (result != VK_SUCCESS) {
            cstr result_string = gfx_vk_str_from_result(result);
            os_graphical_message(true, str("[gfx] Renderer Initialization Error"), str_format(scratch.arena, "Failed setup command buffers!\nresult: %s", result_string));
            os_abort(1);
        }
        
    }
    
    gfx_handle_t handle = { 0 };
    handle = { (u64)renderer };
    
    scratch_end(scratch);
    
    return handle;
}

function void 
gfx_renderer_release(gfx_handle_t renderer) {
    
    // get renderer
	gfx_vk_renderer_t* vk_renderer = (gfx_vk_renderer_t*)renderer.data[0];
    
    vkDeviceWaitIdle(gfx_vk_state.device);
    
    // release command buffers
    vkFreeCommandBuffers(gfx_vk_state.device, vk_renderer->command_pool, vk_renderer->swapchain_image_count, vk_renderer->command_buffers);
    
    // release command pool
    vkDestroyCommandPool(gfx_vk_state.device, vk_renderer->command_pool, nullptr);
    
    // release semaphores
    for (i32 i = 0; i < vk_renderer->swapchain_image_count; i++) {
        vkDestroySemaphore(gfx_vk_state.device, vk_renderer->image_available_semaphore[i], nullptr);
        vkDestroySemaphore(gfx_vk_state.device, vk_renderer->rendering_finished_semaphore[i], nullptr);
        vkDestroyFence(gfx_vk_state.device, vk_renderer->in_flight_fences[i], nullptr);
    }
    
    // release swapchain image views
    for (i32 i = 0; i < vk_renderer->swapchain_image_count; i++) {
        vkDestroyImageView(gfx_vk_state.device, vk_renderer->swapchain_image_views[i], nullptr);
    }
    
    // release swapchain
    vkDestroySwapchainKHR(gfx_vk_state.device, vk_renderer->swapchain, nullptr);
    
    // release surface
    vkDestroySurfaceKHR(gfx_vk_state.instance, vk_renderer->surface, nullptr);
    
    // release arena 
    arena_release(vk_renderer->arena);
	
    // push to free stack
	dll_remove(gfx_vk_state.renderer_first, gfx_vk_state.renderer_last, vk_renderer);
	stack_push(gfx_vk_state.renderer_free, vk_renderer);
    
}

function void
gfx_renderer_begin(gfx_handle_t renderer) {
    
    // get renderer
	gfx_vk_renderer_t* vk_renderer = (gfx_vk_renderer_t*)renderer.data[0];
    VkResult result = VK_SUCCESS;
    
    // check if renderer's size differs from window's size
    // TODO: maybe the user should handle this
    uvec2_t window_size = os_window_get_size(vk_renderer->window);
    if (!uvec2_equals(vk_renderer->size, window_size)) {
        gfx_renderer_resize(renderer, window_size);
    }
    
    vkDeviceWaitIdle(gfx_vk_state.device);
    
    // get next image
    result = vkAcquireNextImageKHR(gfx_vk_state.device, vk_renderer->swapchain, u64_max, vk_renderer->image_available_semaphore[vk_renderer->current_frame], VK_NULL_HANDLE, &vk_renderer->image_index);
    if (result != VK_SUCCESS) {
        os_graphical_message(true, str("[gfx] Renderer begin error"), str("Failed to acquire next image!"));
        os_abort(1);
    }
    
}

function void
gfx_renderer_end(gfx_handle_t renderer) {
    
    gfx_vk_renderer_t* vk_renderer = (gfx_vk_renderer_t*)(renderer.data[0]);
    VkResult result = VK_SUCCESS;
    
    // submit command buffer
    VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    
    VkSubmitInfo submit_info = { 0 };
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    // wait 
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &vk_renderer->image_available_semaphore[vk_renderer->current_frame];
    submit_info.pWaitDstStageMask = &wait_stage_mask;
    
    // command
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &vk_renderer->command_buffers[vk_renderer->current_frame];
    
    // signal
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &vk_renderer->rendering_finished_semaphore[vk_renderer->current_frame];
    
    result = vkQueueSubmit(gfx_vk_state.graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
        os_graphical_message(true, str("[gfx] Renderer begin error"), str("Failed to submit draw command buffer!"));
        os_abort(1);
    }
    
    VkPresentInfoKHR present_info = { 0 };
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    
    // wait
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &vk_renderer->rendering_finished_semaphore[vk_renderer->current_frame];
    
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &vk_renderer->swapchain;
    present_info.pImageIndices = &vk_renderer->image_index;
    
    result = vkQueuePresentKHR(gfx_vk_state.present_queue, &present_info);
    if (result != VK_SUCCESS) {
        os_graphical_message(true, str("[gfx] Renderer begin error"), str("Failed to present!"));
        os_abort(1);
    }
    
    vk_renderer->current_frame = (vk_renderer->current_frame + 1) % vk_renderer->swapchain_image_count;
    
    
}

function void
gfx_renderer_resize(gfx_handle_t renderer, uvec2_t size) {
    
    // get renderer 
    gfx_vk_renderer_t* vk_renderer = (gfx_vk_renderer_t*)(renderer.data[0]);
    vk_renderer->size = size;
    gfx_vk_renderer_create_swapchain(vk_renderer);
    
}




//- vulkan specific functions 


// queue family functions

function gfx_vk_queue_family_indices_t 
gfx_vk_get_queue_family_indices(VkPhysicalDevice device) {
    
    temp_t scratch = scratch_begin();
    gfx_vk_queue_family_indices_t result = { -1, -1, -1, -1 };
    
    // get physical device properties
    VkPhysicalDeviceDriverProperties driver_properties = { 0 };
    driver_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
    VkPhysicalDeviceProperties2 device_properties2 = { 0 };
    device_properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    device_properties2.pNext = &driver_properties;
    vkGetPhysicalDeviceProperties2(device, &device_properties2);
    VkPhysicalDeviceProperties device_properties = device_properties2.properties;
    
    // get physical device features
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(device, &device_features);
    
    // get queue families
    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0);
    VkQueueFamilyProperties* queue_families = (VkQueueFamilyProperties*)arena_alloc(scratch.arena, sizeof(VkQueueFamilyProperties) * queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);
    
    for (i32 i = 0; i < queue_family_count; i++) {
        
        // graphics queue
        if (result.graphics_family_index == -1 && queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            result.graphics_family_index = i;
            
            // present queue (implemented per backend)
            if (gfx_vk_presentation_support(device, i)) {
                result.present_family_index = i;
            }
        }
        
        // compute queue
        if (result.compute_family_index == -1 && queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            result.compute_family_index = i;
        }
        
        // transfer queue
        if (result.transfer_family_index == -1 && queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            result.transfer_family_index = i;
        }
        
    }
    
    if (result.present_family_index == -1) {
        for (i32 i = 0; i < queue_family_count; i++) {
            if (gfx_vk_presentation_support(device, i)) {
                result.present_family_index = i;
                break;
            }
        }
    }
    
    scratch_end(scratch);
    
    return result;
}

function void
gfx_vk_renderer_create_swapchain(gfx_vk_renderer_t* renderer) {
    
    temp_t scratch = scratch_begin();
    VkResult result = VK_SUCCESS;
    vkDeviceWaitIdle(gfx_vk_state.device);
    
    // release old resources if needed
    for (i32 i = 0; i < renderer->swapchain_image_count; i++) {
        if (renderer->swapchain_image_views[i] != VK_NULL_HANDLE) {
            vkDestroyImageView(gfx_vk_state.device, renderer->swapchain_image_views[i], nullptr);
        }
    }
    if (renderer->swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(gfx_vk_state.device, renderer->swapchain, nullptr);
    }
    
    // get surface capabilities
    VkSurfaceCapabilitiesKHR surface_capabilities = { 0 };
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gfx_vk_state.physical_device, renderer->surface, &surface_capabilities);
    
    VkSurfaceFormatKHR surface_format = { 0 };
    surface_format.format = VK_FORMAT_R8G8B8A8_UNORM;
    surface_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    //renderer->swapchain_image_format =  surface_format.format;
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    VkExtent2D swap_extent = { renderer->size.x, renderer->size.y };
    //renderer->swapchain_extent = swap_extent;
    
    // create swap chain
    VkSwapchainCreateInfoKHR swapchain_create_info = { 0 };
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = renderer->surface;
    swapchain_create_info.minImageCount = surface_capabilities.minImageCount + 1;
    swapchain_create_info.imageFormat = surface_format.format;
    swapchain_create_info.imageColorSpace = surface_format.colorSpace;
    swapchain_create_info.imageExtent = swap_extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.queueFamilyIndexCount = 0;
    swapchain_create_info.pQueueFamilyIndices = nullptr;
    swapchain_create_info.preTransform = surface_capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;
    
    result = vkCreateSwapchainKHR(gfx_vk_state.device, &swapchain_create_info, nullptr, &renderer->swapchain);
    if (result != VK_SUCCESS) {
        cstr result_string = gfx_vk_str_from_result(result);
        os_graphical_message(true, str("[gfx] Renderer Initialization Error"), str_format(scratch.arena, "Failed to create swapchain!\nresult: %s", result_string));
        os_abort(1);
    }
    
    // get swapchain images
    renderer->swapchain_image_count = 0;
    vkGetSwapchainImagesKHR(gfx_vk_state.device, renderer->swapchain, &renderer->swapchain_image_count, nullptr);
    renderer->swapchain_images = (VkImage*)arena_alloc(renderer->arena, sizeof(VkImage) * renderer->swapchain_image_count);
    vkGetSwapchainImagesKHR(gfx_vk_state.device, renderer->swapchain, &renderer->swapchain_image_count, renderer->swapchain_images);
    
    // create swapchain image views
    renderer->swapchain_image_views = (VkImageView*)arena_alloc(renderer->arena, sizeof(VkImageView) * renderer->swapchain_image_count); 
    renderer->current_frame = 0;
    
    for (i32 i = 0; i < renderer->swapchain_image_count; i++) {
        
        VkImageViewCreateInfo image_view_create_info = { 0 };
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = renderer->swapchain_images[i];
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = surface_format.format;
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;
        
        result = vkCreateImageView(gfx_vk_state.device, &image_view_create_info, nullptr, &renderer->swapchain_image_views[i]);
        if (result) {
            cstr result_string = gfx_vk_str_from_result(result);
            os_graphical_message(true, str("[gfx] Renderer Initialization Error"), str_format(scratch.arena, "Failed to create swapchain image views!\nresult: %s", result_string));
            os_abort(1);
        }
        
    }
    
    scratch_end(scratch);
    
    
}


// enum conversions
function VkFormat
gfx_vk_format_from_texture_format(gfx_texture_format format) {
    
    VkFormat result = VK_FORMAT_UNDEFINED;
    
    switch (format) {
        case gfx_texture_format_r8: { result = VK_FORMAT_R8_UNORM; break; }
		case gfx_texture_format_rg8: { result = VK_FORMAT_R8G8_UNORM; break; }
		case gfx_texture_format_rgba8: { result = VK_FORMAT_R8G8B8A8_UNORM; break; }
		case gfx_texture_format_bgra8: { result = VK_FORMAT_B8G8R8A8_UNORM; break; }
		case gfx_texture_format_r16: { result = VK_FORMAT_R16_UNORM; break; }
		case gfx_texture_format_rgba16: { result = VK_FORMAT_R16G16B16A16_UNORM; break; }
		case gfx_texture_format_r32: { result = VK_FORMAT_R32_SFLOAT; break; }
		case gfx_texture_format_rg32: { result = VK_FORMAT_R32G32_SFLOAT; break; }
		case gfx_texture_format_rgba32: { result = VK_FORMAT_R32G32B32A32_SFLOAT; break; }
		case gfx_texture_format_d24s8: { result = VK_FORMAT_D24_UNORM_S8_UINT; break; }
		case gfx_texture_format_d32: { result = VK_FORMAT_D32_SFLOAT; break; }
    }
    
    return result;
}



function cstr 
gfx_vk_str_from_result(VkResult result) {
    cstr string;
    
    switch (result) {
        case VK_SUCCESS: { string = "VK_SUCCESS"; break; }
        case VK_NOT_READY: { string = "VK_NOT_READY"; break;}
        case VK_TIMEOUT: { string = "VK_TIMEOUT"; break;}
        case VK_EVENT_SET: { string = "VK_EVENT_SET"; break;}
        case VK_EVENT_RESET: { string = "VK_EVENT_RESET"; break; }
        case VK_INCOMPLETE: { string = "VK_INCOMPLETE"; break;}
        case VK_ERROR_OUT_OF_HOST_MEMORY: { string = "VK_ERROR_OUT_OF_HOST_MEMORY"; break;}
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: { string = "VK_ERROR_OUT_OF_DEVICE_MEMORY";break; }
        case VK_ERROR_INITIALIZATION_FAILED: { string = "VK_ERROR_INITIALIZATION_FAILED";break; }
        case VK_ERROR_DEVICE_LOST: { string = "VK_ERROR_DEVICE_LOST";break; }
        case VK_ERROR_MEMORY_MAP_FAILED: { string = "VK_ERROR_MEMORY_MAP_FAILED"; break;}
        case VK_ERROR_LAYER_NOT_PRESENT: { string = "VK_ERROR_LAYER_NOT_PRESENT"; break;}
        case VK_ERROR_EXTENSION_NOT_PRESENT: { string = "VK_ERROR_EXTENSION_NOT_PRESENT"; break;}
        case VK_ERROR_FEATURE_NOT_PRESENT: { string = "VK_ERROR_FEATURE_NOT_PRESENT"; break;}
        case VK_ERROR_INCOMPATIBLE_DRIVER: { string = "VK_ERROR_INCOMPATIBLE_DRIVER"; break;}
        case VK_ERROR_TOO_MANY_OBJECTS: { string = "VK_ERROR_TOO_MANY_OBJECTS"; break;}
        case VK_ERROR_FORMAT_NOT_SUPPORTED: { string = "VK_ERROR_FORMAT_NOT_SUPPORTED"; break;}
        case VK_ERROR_FRAGMENTED_POOL: { string = "VK_ERROR_FRAGMENTED_POOL";break; }
        case VK_ERROR_UNKNOWN: { string = "VK_ERROR_UNKNOWN"; break;}
        default: {string = "unknown/undocumented"; break;}
    }
    
    return string;
}



// include os specifics
#if OS_BACKEND_WIN32
#include "vulkan/sora_gfx_vulkan_win32.cpp"
#elif OS_BACKEND_MACOS
#include "vulkan/sora_gfx_vulkan_macos.cpp"
#elif OS_BACKEND_LINUX
#include "vulkan/sora_gfx_vulkan_linux.cpp"
#endif

#endif // SORA_GFX_VULKAN_CPP

