// sora_gfx_d3d11.h

#ifndef SORA_GFX_D3D11_H
#define SORA_GFX_D3D11_H

//- includes
#include <d3d11_1.h>
#include <d3dcompiler.h>

//- defines

#ifdef BUILD_DEBUG
#define gfx_assert(hr, msg, ...) { if (FAILED(hr)) { printf("[error] "msg" (%x)\n", __VA_ARGS__, hr); os_abort(1); } }
#define gfx_check_error(hr, msg, ...) { if (FAILED(hr)) { printf("[error] "msg" (%x)\n", __VA_ARGS__, hr); } }
#else
#define gfx_assert(hr, msg, ...)
#define gfx_check_error(hr, msg, ...)
#endif

//- structs

struct gfx_d3d11_buffer_t {
	ID3D11Buffer* id;
};

struct gfx_d3d11_texture_t {
	ID3D11Texture2D* id;
	ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav;
};

struct gfx_d3d11_shader_t {
	ID3D11VertexShader* vertex_shader;
	ID3D11PixelShader* pixel_shader;
	ID3D11InputLayout* input_layout;
};

struct gfx_d3d11_render_target_t {
	gfx_handle_t color_texture;
	gfx_handle_t depth_texture;
	ID3D11RenderTargetView* rtv;
	ID3D11DepthStencilView* dsv;
};

struct gfx_d3d11_compute_shader_t {
	ID3D11ComputeShader* compute_shader;
};

struct gfx_d3d11_resource_t {
	gfx_d3d11_resource_t* next;
	gfx_d3d11_resource_t* prev;
    
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
		gfx_d3d11_buffer_t buffer;
		gfx_d3d11_texture_t texture;
		gfx_d3d11_shader_t shader;
		gfx_d3d11_compute_shader_t compute_shader;
		gfx_d3d11_render_target_t render_target;
	};
};

// texture 3d

struct gfx_texture_3d_t {
	gfx_texture_3d_t* next;
	gfx_texture_3d_t* prev;
    
	gfx_texture_3d_desc_t desc;
	ID3D11Texture3D* id;
	ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav;
};

// renderer

struct gfx_d3d11_renderer_t {
	gfx_d3d11_renderer_t* next;
	gfx_d3d11_renderer_t* prev;
    
	// context
	os_handle_t window;
	color_t clear_color;
	uvec2_t resolution;
    
	// d3d11
	IDXGISwapChain1* swapchain;
	ID3D11Texture2D* framebuffer;
	ID3D11RenderTargetView* framebuffer_rtv;
};

// state

struct gfx_d3d11_state_t {
    
	// arenas
	arena_t* renderer_arena;
	arena_t* resource_arena;
    
	// d3d11
	ID3D11Device* device;
	ID3D11DeviceContext* device_context;
	IDXGIDevice1* dxgi_device;
	IDXGIAdapter* dxgi_adapter;
	IDXGIFactory2* dxgi_factory;
	
	// resources
	gfx_d3d11_resource_t* resource_first;
	gfx_d3d11_resource_t* resource_last;
	gfx_d3d11_resource_t* resource_free;
	
	// renderer
	gfx_d3d11_renderer_t* renderer_first;
	gfx_d3d11_renderer_t* renderer_last;
	gfx_d3d11_renderer_t* renderer_free;
	gfx_d3d11_renderer_t* renderer_active;
    
	// pipeline assets
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

global gfx_d3d11_state_t gfx_state;

//- d3d11 specific functions

// texture
function void gfx_d3d11_texture_create_resources(gfx_d3d11_resource_t* texture, void* data);

// renderer
function gfx_d3d11_renderer_t* gfx_d3d11_renderer_from_handle(gfx_handle_t handle);
function gfx_handle_t gfx_d3d11_handle_from_renderer(gfx_d3d11_renderer_t* renderer);

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
function DXGI_FORMAT gfx_d3d11_dxgi_format_from_uniform_type(gfx_uniform_type);
function D3D11_INPUT_CLASSIFICATION gfx_d3d11_input_class_from_vertex_class(gfx_vertex_class);

#endif // SORA_GFX_D3D11_H