// shader_3d.hlsl

cbuffer camera_constants : register(b0) {
    float4x4 view_projection;
    float4x4 view;
    float4x4 projection;
    float4x4 inv_view;
    float4x4 inv_projection;
	float3 camera_position;
};

cbuffer transform_constants : register(b1) {
	float4x4 transform;
};

struct vs_in {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
};

struct vs_out {
    float4 position : SV_POSITION;
    //float4 world_position : WORLD_POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
};

vs_out vs_main(vs_in input) {
    vs_out output = (vs_out) 0;

    output.position = float4(input.position, 1.0f);
    output.position = mul(output.position, transform);
    output.position = mul(output.position, view_projection);
    
	// matcap
    //output.normal = normalize(mul((float3x3)view, input.normal));
    //output.texcoord = output.normal.xy * 0.5f + 0.5f;
	
	output.normal = input.normal;
    output.tangent = input.tangent;
    output.texcoord = input.texcoord;
    output.color = input.color;

    return output;
}

Texture2D color_texture : register(t0);
SamplerState texture_sampler : register(s0);

float4 ps_main(vs_out input) : SV_TARGET {
 
	//float4 texture_color = color_texture.Sample(texture_sampler, input.texcoord);
	
	float3 l = normalize(float3(1.0f, 1.5f, 0.75f));
	float3 n = normalize(input.normal);
	float n_dot_l = max(dot(n, l), 0.2f);
	float diffuse = clamp(n_dot_l, 0.0f, 1.0f);
	
    return float4(diffuse, diffuse, diffuse, 1.0f);
}
