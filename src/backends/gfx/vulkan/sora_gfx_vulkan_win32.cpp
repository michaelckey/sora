// gfx_vulkan_win32.cpp

#ifndef GFX_VULKAN_WIN32_CPP
#define GFX_VULKAN_WIN32_CPP

function cstr* 
gfx_vk_get_extensions(u32* count) {
    
    persist cstr vk_required_extensions[] = {
        "VK_EXT_debug_utils",
        "VK_KHR_surface",
        "VK_KHR_win32_surface",
    };
    
    *count = array_count(vk_required_extensions);
    
    return vk_required_extensions;
}

function void
gfx_vk_surface_create(gfx_vk_renderer_t* renderer) {
    
    // get w32 window
    os_w32_window_t* w32_window = os_w32_window_from_handle(renderer->window);
    
    // create info
    VkWin32SurfaceCreateInfoKHR create_info = { 0 };
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hwnd = w32_window->handle;
    create_info.hinstance = GetModuleHandle(nullptr);
    
    VkResult result = vkCreateWin32SurfaceKHR(gfx_vk_state.instance, &create_info, nullptr, &renderer->surface);
    if (result != VK_SUCCESS) {
        os_graphical_message(true, str("[gfx] Renderer initialization"), str("Failed to create window surface!"));
        os_abort(1);
    }
    
}

function b8 
gfx_vk_presentation_support(VkPhysicalDevice device, u32 queue_family_index) {
    return (b8)vkGetPhysicalDeviceWin32PresentationSupportKHR(device, queue_family_index);
}

#endif // GFX_VULKAN_WIN32_CPP