// shader_compute.hlsl

cbuffer globals {
    float time;
};

RWTexture2D<float4> output_texture;

float noise(int2 coord, float time) {
    uint seed = asuint(coord.x * 1973 + coord.y * 9277 + (int)(time * 1000) + 8912);
    seed = (seed << 13) ^ seed;
    return frac(1.0f - ((seed * (seed * seed * 15731u + 789221u) + 1376312589u) & 0x7fffffff) / 1073741824.0f);
}

[numthreads(32 , 32, 1)]
void cs_main(uint3 id : SV_DispatchThreadID, uint3 gid : SV_GroupID) {

	uint2 texture_size;
	float4 color = output_texture[id.xy];
    output_texture.GetDimensions(texture_size.x, texture_size.y);
	if (id.x >= texture_size.x || id.y >= texture_size.y) {
        return;
	}
	//float2 grid_uv = (float2)(id.xy % 32) / 32.0f;
	
    float n = noise(id.xy / 2, time / 10.0f);

    float grain_amount = 0.025;
    float3 grain = n * grain_amount;

    color.rgb += grain;
    color.rgb = saturate(color.rgb);

    output_texture[id.xy] = color;
}