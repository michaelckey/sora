// shader_point.hlsl

struct vs_in {
	uint vertex_id   : SV_VertexID;
	uint instance_id : SV_InstanceID;
};

struct ps_in {
	float4 position  : SV_Position;
	float2 texcoord  : TEX;
	float4 color     : COL;
};

cbuffer camera_constants {
	float4x4 view_projection;
    float4x4 view;
    float4x4 projection;
    float4x4 inv_view;
    float4x4 inv_projection;
	float3 camera_position;
};

//- buffers

StructuredBuffer<float3> positions : register(t0);
StructuredBuffer<uint> colors : register(t1);

//- vertex shader

float4 unpack_color(uint color) {
    float r = (color & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = ((color >> 16) & 0xFF) / 255.0f;
    float a = ((color >> 24) & 0xFF) / 255.0f;
    return float4(r, g, b, a);
}

ps_in vs_main(vs_in input) {
	ps_in output = (ps_in)0;

	float3 position = positions[input.instance_id];
	uint color = colors[input.instance_id];
	
	const float size = 0.25f;
	float2 corner;
    corner.x = (input.vertex_id & 1) ? size : -size;
    corner.y = (input.vertex_id & 2) ? size : -size;

	float2 texcoord;
    texcoord.x = (input.vertex_id & 1) ? 1.0f : 0.0f;
    texcoord.y = (input.vertex_id & 2) ? 1.0f : 0.0f;
	
    // get camera directions
    float3 cam_right = inv_view[0].xyz;
    float3 cam_up    = inv_view[1].xyz;
	
	// send to pixel shader
	float3 world_pos = position + cam_right * corner.x + cam_up * corner.y;
    output.position = mul(float4(world_pos, 1.0f), view_projection);
    output.texcoord = texcoord;

	// unpack color
	output.color = unpack_color(color);
	
	return output;
}

//- pixel shader

float4 ps_main(ps_in input) : SV_TARGET {
	
	float2 p = input.texcoord * 2.0f - 1.0f;
	float d = length(p);
	float s = fwidth(d);
	float a = 1.0f - smoothstep(1.0f - s, 1.0f, d);	

	return float4(input.color.rgb, input.color.a * a);
}