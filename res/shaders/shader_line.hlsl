// shader_line.hlsl

cbuffer camera_constants : register(b0) {
    float4x4 view_projection;
    float4x4 view;
    float4x4 projection;
    float4x4 inv_view;
    float4x4 inv_projection;
	float3 camera_position;
}

struct vs_in {
    float3 position : POS;
    uint color      : COL;
};

struct vs_out {
    float4 position : SV_Position;
    float4 color    : COL;
};


float4 unpack_color(uint color) {
    float r = (color & 0xff) / 255.0f;
    float g = ((color >> 8) & 0xff) / 255.0f;
    float b = ((color >> 16) & 0xff) / 255.0f;
    float a = ((color >> 24) & 0xff) / 255.0f;
    return float4(r, g, b, a);
}


//- vertex shader

vs_out vs_main(vs_in input) {
    vs_out output = (vs_out) 0;
    
    output.position = mul(float4(input.position, 1.0f), view_projection);
    output.color = unpack_color(input.color);
    
    return output;
}


float4 ps_main(vs_out input) : SV_TARGET {
    return input.color;
}
