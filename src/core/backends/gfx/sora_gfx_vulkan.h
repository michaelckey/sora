// sora_gfx_vulkan.h

#ifndef SORA_GFX_VULKAN_H
#define SORA_GFX_VULKAN_H

//
// todo:
//
// [ ] - state:
//     [ ] - create instance
//     [ ] - find physical device
//     [ ] - find queue families
//     [ ] - create logical device
// [ ] - renderer:
//     [ ] - create window surface
//     [ ] - create swapchain
//     [ ] - create graphics pipeline
//
//


//- includes 

#undef function
#include <vulkan/vulkan_core.h>
#define function static

//- structs 

struct gfx_vk_buffer_t {
    
};

struct gfx_vk_texture_t {
    
};

struct gfx_vk_shader_t {
    
};

struct gfx_vk_compute_shader_t {
    
};

struct gfx_vk_render_target_t {
    
};

// resource
struct gfx_vk_resource_t {
    gfx_vk_resource_t* next;
    gfx_vk_resource_t* prev;
    
    gfx_resource_type type;
    
    // resource descriptions
	union {
		gfx_buffer_desc_t buffer_desc;
		gfx_texture_desc_t texture_desc;
		gfx_shader_desc_t shader_desc;
		gfx_compute_shader_desc_t compute_shader_desc;
		gfx_render_target_desc_t render_target_desc;
	};
    
    // resource members
	union {
		gfx_vk_buffer_t buffer;
		gfx_vk_texture_t texture;
		gfx_vk_shader_t shader;
		gfx_vk_compute_shader_t compute_shader;
		gfx_vk_render_target_t render_target;
	};
    
};

struct gfx_vk_queue_family_indices_t  {
    i32 graphics_family_index;
    i32 present_family_index;
    i32 compute_family_index;
    i32 transfer_family_index;
};

// renderer
struct gfx_vk_renderer_t {
    gfx_vk_renderer_t* next;
    gfx_vk_renderer_t* prev;
    
    // context
	os_handle_t window;
	color_t clear_color;
	uvec2_t size;
    
    arena_t* arena;
    
    // vulkan
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    
    u32 swapchain_image_count;
    VkImage* swapchain_images;
    VkImageView* swapchain_image_views;
    VkSemaphore* image_available_semaphore;
    VkSemaphore* rendering_finished_semaphore;
    VkFence* in_flight_fences;
    
    VkCommandPool command_pool;
    VkCommandBuffer* command_buffers;
    
    u32 image_index;
    u32 current_frame;
    
};

// state
struct gfx_vk_state_t {
    
    // arenas
	arena_t* renderer_arena;
	arena_t* resource_arena;
    
    // resources
	gfx_vk_resource_t* resource_first;
	gfx_vk_resource_t* resource_last;
	gfx_vk_resource_t* resource_free;
	
	// renderer
	gfx_vk_renderer_t* renderer_first;
	gfx_vk_renderer_t* renderer_last;
	gfx_vk_renderer_t* renderer_free;
	gfx_vk_renderer_t* renderer_active;
    
    // vulkan
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    gfx_vk_queue_family_indices_t queue_indices;
};

//- globals 

global gfx_vk_state_t gfx_vk_state;

//- vullkan specific functions 

// implemented per backend
function cstr* gfx_vk_get_extensions(u32* count);
function void gfx_vk_surface_create(gfx_vk_renderer_t* renderer);
function b8 gfx_vk_presentation_support(VkPhysicalDevice device, u32 queue_family_index);
function void gfx_vk_renderer_create_swapchain(gfx_vk_renderer_t* renderer);

// queue families
function gfx_vk_queue_family_indices_t  gfx_vk_get_queue_family_indices(VkPhysicalDevice device);

// enum conversions
function VkFormat gfx_vk_format_from_texture_format(gfx_texture_format format);

// string conversions
function cstr gfx_vk_str_from_result(VkResult result);


//- include os specifics 

#if OS_BACKEND_WIN32
#include <vulkan/vulkan_win32.h>
#include "vulkan/sora_gfx_vulkan_win32.h"
#elif OS_BACKEND_MACOS
#include "vulkan/sora_gfx_vulkan_macos.h"
#elif OS_BACKEND_LINUX
#include "vulkan/sora_gfx_vulkan_linux.h"
#endif

#endif // SORA_GFX_VULKAN_H

