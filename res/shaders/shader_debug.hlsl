// shader_debug.hlsl

cbuffer camera_constants : register(b0) {
    float4x4 view_projection;
    float4x4 view;
    float4x4 projection;
    float4x4 inv_view;
    float4x4 inv_projection;
	float3 camera_position;
}

struct vs_in {
    float3 position : POSITION;
    float4 color : COLOR;
};

struct vs_out {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};


vs_out vs_main(vs_in input) {
    vs_out output = (vs_out) 0;
    
    output.position = mul(float4(input.position, 1.0f), view_projection);
    output.color = input.color;
    
    return output;
}


float4 ps_main(vs_out input) : SV_TARGET {
    return input.color;
}
