// sora_gfx_d3d11.h

#ifndef SORA_GFX_D3D11_H
#define SORA_GFX_D3D11_H

//~ includes 

#include <d3d11_1.h>
#include <d3dcompiler.h>

//~ defines 

//~ enums 

enum gfx_d3d11_resource_type {
    gfx_d3d11_resource_type_null,
    gfx_d3d11_resource_type_buffer,
    gfx_d3d11_resource_type_texture,
    gfx_d3d11_resource_type_shader,
    gfx_d3d11_resource_type_render_target,
};


//~ structs

struct gfx_d3d11_resource_t {
    gfx_d3d11_resource_t* next;
    gfx_d3d11_resource_t* prev;
    
    gfx_d3d11_resource_type type;
    
    union {
        gfx_buffer_desc_t buffer;
        gfx_texture_desc_t texture;
        gfx_shader_desc_t shader;
        gfx_render_target_desc_t render_target;
    } desc;
    
    union {
        
        // buffer
        struct {
            ID3D11Buffer* id;
            ID3D11ShaderResourceView* srv;
            ID3D11UnorderedAccessView* uav;
        } buffer;
        
        // texture
        struct {
            ID3D11Texture2D* id;
            ID3D11ShaderResourceView* srv;
            ID3D11UnorderedAccessView* uav;
        } texture;
        
        // shader
        struct {
            
            union {
                ID3D11VertexShader* vertex;
                ID3D11PixelShader* pixel;
                ID3D11GeometryShader* geometry;
                ID3D11HullShader* hull;
                ID3D11DomainShader* domain;
                ID3D11ComputeShader* compute;
            };
            ID3DBlob* blob;
            
            // for vertex shader
            ID3D11InputLayout* input_layout;
            
        } shader;
        
        // render target
        struct {
            gfx_texture_t color_texture;
            gfx_texture_t depth_texture;
            ID3D11RenderTargetView* rtv;
            ID3D11DepthStencilView* dsv;
        } render_target;
        
    };
};

struct gfx_d3d11_context_t {
    gfx_d3d11_context_t* next;
    gfx_d3d11_context_t* prev;
    
    os_window_t window;
    gfx_context_flags flags;
    uvec2_t size;
    
    IDXGISwapChain1* swapchain;
	ID3D11Texture2D* framebuffer;
	ID3D11RenderTargetView* framebuffer_rtv;
	ID3D11Texture2D* depthbuffer;
	ID3D11DepthStencilView* depthbuffer_dsv;
    
};

struct gfx_d3d11_state_t {
    
    arena_t* arena;
    
    // d3d11
	ID3D11Device* device;
	ID3D11DeviceContext* device_context;
	IDXGIDevice1* dxgi_device;
	IDXGIAdapter* dxgi_adapter;
	IDXGIFactory2* dxgi_factory;
    
    // context
    gfx_d3d11_context_t* context_first;
    gfx_d3d11_context_t* context_last;
    gfx_d3d11_context_t* context_free;
    
    // resources
    gfx_d3d11_resource_t* resource_first;
    gfx_d3d11_resource_t* resource_last;
    gfx_d3d11_resource_t* resource_free;
    
};

//~ globals

global gfx_d3d11_state_t gfx_d3d11_state;

//~ d3d11 specific functions 

//- resources 

function gfx_d3d11_resource_t* gfx_d3d11_resource_alloc(gfx_d3d11_resource_type type);
function void gfx_d3d11_resource_release(gfx_d3d11_resource_t* resource);

#endif // SORA_GFX_D3D11_H