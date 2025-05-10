// sora_gfx_d3d11.h

#ifndef SORA_GFX_D3D11_H
#define SORA_GFX_D3D11_H

//- includes

#include <d3d11_1.h>
#include <d3dcompiler.h>

//- defines

#if BUILD_DEBUG 
#define gfx_check(hr, fmt, ...)\
if (FAILED(hr)) {\
temp_t scratch = scratch_begin();\
os_graphical_message(true, str_format(scratch.arena, "gfx error (hr: %x)", hr), str_format(scratch.arena, fmt, __VA_ARGS__));\
os_abort(1);\
}
#elif BUILD_RELEASE
#define gfx_check(hr, fmt, ...)
#endif 

//- structs

// buffer
struct gfx_d3d11_buffer_t {
    gfx_buffer_desc_t desc;
	ID3D11Buffer* id;
};

// texture
struct gfx_d3d11_texture_t {
    gfx_texture_desc_t desc;
	ID3D11Texture2D* id;
	ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav;
};

// shader
struct gfx_d3d11_shader_t {
    
    // desc
    gfx_shader_desc_t desc;
    
    union {
        ID3D11VertexShader* vertex_shader;
        ID3D11PixelShader* pixel_shader;
        ID3D11GeometryShader* geometry_shader;
        ID3D11HullShader* hull_shader;
        ID3D11DomainShader* domain_shader;
        ID3D11ComputeShader* compute_shader;
    };
    ID3DBlob* shader_blob;
    
    // for vertex shader
	ID3D11InputLayout* input_layout;
};

// render target
struct gfx_d3d11_render_target_t {
    gfx_render_target_desc_t desc;
	gfx_handle_t color_texture;
	gfx_handle_t depth_texture;
	ID3D11RenderTargetView* rtv;
	ID3D11DepthStencilView* dsv;
};

// resource
struct gfx_d3d11_resource_t {
	gfx_d3d11_resource_t* next;
	gfx_d3d11_resource_t* prev;
    
	gfx_resource_type type;
    
	// resource members
	union {
		gfx_d3d11_buffer_t buffer;
		gfx_d3d11_texture_t texture;
		gfx_d3d11_shader_t shader;
		gfx_d3d11_render_target_t render_target;
	};
};

// renderer

struct gfx_d3d11_renderer_t {
	gfx_d3d11_renderer_t* next;
	gfx_d3d11_renderer_t* prev;
    
	os_handle_t window;
	uvec2_t resolution;
    
	IDXGISwapChain1* swapchain;
	ID3D11Texture2D* framebuffer;
	ID3D11RenderTargetView* framebuffer_rtv;
};

// state

struct gfx_d3d11_state_t {
    
	// arenas
	arena_t* arena;
    
    // resources
	gfx_d3d11_resource_t* resource_first;
	gfx_d3d11_resource_t* resource_last;
	gfx_d3d11_resource_t* resource_free;
	
	// renderer
	gfx_d3d11_renderer_t* renderer_first;
	gfx_d3d11_renderer_t* renderer_last;
	gfx_d3d11_renderer_t* renderer_free;
	gfx_d3d11_renderer_t* renderer_active;
    
	// d3d11
	ID3D11Device* device;
	ID3D11DeviceContext* device_context;
	IDXGIDevice1* dxgi_device;
	IDXGIAdapter* dxgi_adapter;
	IDXGIFactory2* dxgi_factory;
    
	// d3d11 pipeline assets
	ID3D11SamplerState* linear_wrap_sampler;
	ID3D11SamplerState* linear_clamp_sampler;
	ID3D11SamplerState* nearest_wrap_sampler;
	ID3D11SamplerState* nearest_clamp_sampler;
    
	ID3D11DepthStencilState* depth_stencil_state;
	ID3D11DepthStencilState* no_depth_stencil_state;
    
	ID3D11RasterizerState* solid_cull_none_rasterizer;
	ID3D11RasterizerState* solid_cull_front_rasterizer;
	ID3D11RasterizerState* solid_cull_back_rasterizer;
	ID3D11RasterizerState* wireframe_cull_none_rasterizer;
	ID3D11RasterizerState* wireframe_cull_front_rasterizer;
	ID3D11RasterizerState* wireframe_cull_back_rasterizer;
    
	ID3D11BlendState* blend_state;
};

//- globals

global gfx_d3d11_state_t gfx_d3d11_state;

//- d3d11 specific functions

// texture
function void gfx_d3d11_texture_create_resources(gfx_d3d11_resource_t* texture, void* data);

// resource functions
function gfx_d3d11_resource_t* gfx_d3d11_resource_create(gfx_resource_type type);
function void gfx_d3d11_resource_release(gfx_d3d11_resource_t* resource);

// enum conversion functions
function D3D11_USAGE gfx_d3d11_d3d11_usage_from_gfx_usage(gfx_usage);
function UINT gfx_d3d11_access_flags_from_gfx_usage(gfx_usage);
function D3D11_BIND_FLAG gfx_d3d11_bind_flags_from_buffer_type(gfx_buffer_type);
function DXGI_FORMAT gfx_d3d11_dxgi_format_from_texture_format(gfx_texture_format);
function DXGI_FORMAT gfx_d3d11_srv_format_from_texture_format(gfx_texture_format);
function DXGI_FORMAT gfx_d3d11_dsv_format_from_texture_format(gfx_texture_format);
function u32 gfx_d3d11_byte_size_from_texture_format(gfx_texture_format);
function D3D11_PRIMITIVE_TOPOLOGY gfx_d3d11_prim_top_from_top_type(gfx_topology_type);
function DXGI_FORMAT gfx_d3d11_dxgi_format_from_vertex_format(gfx_vertex_format);
function D3D11_INPUT_CLASSIFICATION gfx_d3d11_input_class_from_shader_flags(gfx_shader_flags flags);
function str_t gfx_d3d11_shader_entry_name_from_shader_flags(gfx_shader_flags flags);
function str_t gfx_d3d11_shader_target_from_shader_flags(gfx_shader_flags flags);
function DXGI_FORMAT gfx_d3d11_dxgi_format_from_mask_component(u32 mask, D3D_REGISTER_COMPONENT_TYPE component);

#endif // SORA_GFX_D3D11_H