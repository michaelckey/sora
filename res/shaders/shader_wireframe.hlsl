// shader_wireframe.hlsl

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
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
};

struct vs_out {
    float4 position : SV_POSITION;
    float4 world_position : WORLD_POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
};

vs_out vs_main(vs_in input) {
    vs_out output = (vs_out) 0;

    output.position = float4(input.position, 1.0f);
    output.world_position = output.position;
    output.position = mul(output.position, view_projection);
    
	output.normal = input.normal;
    output.tangent = input.tangent;
    output.texcoord = input.texcoord;
    output.color = input.color;

    return output;
}

float4 ps_main(vs_out input) : SV_TARGET {
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
