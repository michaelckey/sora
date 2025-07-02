// shader_point.hlsl

struct vs_in {
	float3 position  : POS;
    uint color       : COL;

	uint vertex_id   : SV_VertexID;
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

	// per vertex arrays
	const float size = 0.25f;
	float2 vertex_positions[] = {
		float2(-size, -size),
		float2(-size,  size),
		float2( size, -size),
		float2( size,  size),
	};
	
	float2 vertex_texcoords[] = {
		float2(0.0f, 0.0f),
		float2(0.0f, 1.0f),
		float2(1.0f, 0.0f),
		float2(1.0f, 1.0f),
	};

    // get camera directions
    float3 cam_right = normalize(inv_view[0].xyz);
    float3 cam_up    = normalize(inv_view[1].xyz);

    // calculate position
    float2 corner = vertex_positions[input.vertex_id];
    float3 offset = cam_right * corner.x + cam_up * corner.y;
    float3 world_pos = input.position + offset;

	// send to pixel shader
	output.position = float4(world_pos, 1.0f);
	output.position = mul(output.position, view_projection);
	output.texcoord = vertex_texcoords[input.vertex_id];

	// unpack color
	output.color = unpack_color(input.color);
	
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