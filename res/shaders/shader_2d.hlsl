// shader_2d.hlsl

cbuffer globals {
	float2 window_size;
};

struct vs_in {
	float4 pos : POS;
	float4 tex : TEX;
	float4 color : COL;
	float thickness : THC;
	float softness : SFT;
	uint vertex_id : SV_VertexID;
};

struct ps_in {
	float4 pos : SV_POSITION;
	float2 tex : TEX;
	float4 color : COL;
	float thickness : THC;
	float softness : SFT;
};

ps_in vs_main(vs_in input) {
	ps_in output = (ps_in)0;

	float2 p0 = { input.pos.xy };
	float2 p1 = { input.pos.zw };
	float2 tex_p0 = { input.tex.xy };
	float2 tex_p1 = { input.tex.zw };

	float2 vertex_positions[] = {
		float2(p0.x, p0.y),
		float2(p0.x, p1.y),
		float2(p1.x, p0.y),
		float2(p1.x, p1.y),
	};

	float2 vertex_texcoords[] = {
		float2(tex_p0.x, tex_p0.y),
		float2(tex_p0.x, tex_p1.y),
		float2(tex_p1.x, tex_p0.y),
		float2(tex_p1.x, tex_p1.y),
	};

	output.pos = float4(vertex_positions[input.vertex_id] * 2.0f / window_size - 1.0f, 0.0f, 1.0f);
	output.pos.y = -output.pos.y;
	output.tex = vertex_texcoords[input.vertex_id];
	
	return output;
}

float4 ps_main(ps_in input) : SV_TARGET {
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}