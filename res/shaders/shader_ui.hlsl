// shader_ui.hlsl

//- defines

#define ui_r_shape_none (0)
#define ui_r_shape_rect (1)
#define ui_r_shape_line (2)
#define ui_r_shape_circle (3)
#define ui_r_shape_ring (4)
#define ui_r_shape_tri (5)

#define pi (3.14159265359)
#define two_pi (6.28318530718)
#define radians (0.01745329251)

#define ui_r_max_clip_mask 64
#define ui_r_max_colors 1024
#define ui_r_max_textures 16

//- structs

struct rect_t {
    float2 p0;
    float2 p1;
};

struct vs_in {
    float4 positions    : POS; // (p0_x, p0_y, p1_x, p1_y)
    float4 texcoords    : TEX; // (uv0_x, uv0_y, uv1_x, uv1_y)  

	// style
	float softness      : SFT;
    float thickness     : THC;
	float2 params0      : PAR0; // extra params (rounding, angles, positions, etc.)
	float2 params1      : PAR1; // extra params (rounding, angles, positions, etc.)
	float2 params2      : PAR2; // extra params (positions, etc.)

	// indices
    uint indices        : IDX; // (clip_mask_index, texture_index, shape_index, unused) *packed
    uint2 color_indices : COL; // (color_index0, color_index1, color_index2, color_index4) *packed

    uint vertex_id      : SV_VertexID;
};

struct ps_in {
	float4 position      : SV_POSITION;
    float2 half_size     : HSIZE;
    float2 texcoord      : TEX;
    float2 colcoord      : COLC;
	
	float softness       : SFT;
	float thickness      : THC;
	float2 params0       : PAR0;
	float2 params1       : PAR1;
	float2 params2       : PAR2;

	int shape_index     : SHP;
	int clip_mask_index : CLP;
	int texture_index   : TID;
	
	float4 color0        : COL0;
	float4 color1        : COL1;
	float4 color2        : COL2;
	float4 color3        : COL3;
};

//- constant buffers

cbuffer window : register(b0) {
    float2 window_size;
};

cbuffer clip_mask_buffer : register(b1) {
    rect_t clip_masks[ui_r_max_clip_mask];
};

cbuffer color_buffer : register(b2) {
    float4 colors[ui_r_max_colors];
};

//- textures
Texture2D color_texture[ui_r_max_textures];
SamplerState texture_sampler;

//- vertex shader

ps_in vs_main(vs_in input) {
    
    ps_in output = (ps_in)0;
    
    // unpack positions and texcoords
    float2 p0 = { input.positions.xy };
    float2 p1 = { input.positions.zw };
    float2 uv0 = { input.texcoords.xy };
    float2 uv1 = { input.texcoords.zw };
    
    // unpack indices
    int clip_mask_index = int(input.indices << 24) >> 24;
    int texture_index = int(input.indices << 16) >> 24;
    int shape_index = int(input.indices << 8) >> 24;
    int unused = int(input.indices) >> 24;

    // per-vertex arrays
    float2 vertex_positions[] = {
        float2(p0.x, p0.y),
        float2(p0.x, p1.y),
        float2(p1.x, p0.y),
        float2(p1.x, p1.y),
    };
    
    float2 vertex_texcoords[] = {
        float2(uv0.x, uv0.y),
        float2(uv0.x, uv1.y),
        float2(uv1.x, uv0.y),
        float2(uv1.x, uv1.y),
    };
    
    float2 vertex_colcoords[] = {
        float2(0.0f, 0.0f),
        float2(0.0f, 1.0f),
        float2(1.0f, 0.0f),
        float2(1.0f, 1.0f),
    };

	// unpack color indices

	uint color_index0 = input.color_indices.x & 0xffff;
    uint color_index1 = input.color_indices.x >> 16;
	uint color_index2 = input.color_indices.y & 0xffff;
    uint color_index3 = input.color_indices.y >> 16;
    


	float4 color0 = colors[color_index0];
	float4 color1 = colors[color_index1];
	float4 color2 = colors[color_index2];
	float4 color3 = colors[color_index3];
    
    // fill output
    output.position = float4(vertex_positions[input.vertex_id] * 2.0f / window_size - 1.0f, 0.0f, 1.0f);
    output.position.y = -output.position.y;
    output.half_size = (p1 - p0) * 0.5f;
    output.texcoord = vertex_texcoords[input.vertex_id];
    output.colcoord = vertex_colcoords[input.vertex_id];
    
    output.thickness = input.thickness;
    output.softness = input.softness;
    output.params0 = input.params0;
    output.params1 = input.params1;
    output.params2 = input.params2;

	output.shape_index = shape_index;
	output.clip_mask_index = clip_mask_index;
	output.texture_index = texture_index;

    output.color0 = color0;
    output.color1 = color1;
    output.color2 = color2;
    output.color3 = color3;
    
    return output;
}

//- sdf functions

float sdf_rect(float2 p, float2 h, float4 r) {
    r.xy = (p.x > 0.0f) ? r.xy : r.zw;
    r.x = (p.y > 0.0f) ? r.x : r.y;
    float2 q = abs(p) - h + r.x;
    return min(max(q.x, q.y), 0.0f) + length(max(q, 0.0f)) - r.x;
}

float sdf_line(float2 p, float2 a, float2 b) {
    float2 ba = b - a;
    float2 pa = p - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
    return length(pa - h * ba);
}

float sdf_circle(float2 p, float r) {
    return (length(p) - r);
}

float sdf_ring(float2 p, float r, float a0, float a1) {
	
	float i_a = -(a0 + a1) * 0.5f;
	float2 f = float2(cos(i_a), -sin(i_a));
	p = float2(dot(p, f), dot(p, float2(-f.y, f.x)));    

    float a = (a1 - a0);
    float2 c = float2(sin(a * 0.5f), cos(a * 0.5f));
    
    p.x = abs(p.x);
    float l = length(p) - r;
    float m = length(p - c * clamp(dot(p, c), 0.0f, r)); // c=sin/cos of aperture
    return max(l, m * sign(c.y * p.x - c.x * p.y));
}

float sdf_triangle(float2 p, float2 a, float2 b, float2 c) {
    
    float2 e0 = b - a;
    float2 e1 = c - b;
    float2 e2 = a - c;

    float2 v0 = p - a;
    float2 v1 = p - b;
    float2 v2 = p - c;

    float2 pq0 = v0 - e0 * clamp(dot(v0, e0) / dot(e0, e0), 0.0f, 1.0f);
    float2 pq1 = v1 - e1 * clamp(dot(v1, e1) / dot(e1, e1), 0.0f, 1.0f);
    float2 pq2 = v2 - e2 * clamp(dot(v2, e2) / dot(e2, e2), 0.0f, 1.0f);
    
    float s = e0.x * e2.y - e0.y * e2.x;
    float2 d = min(min(float2(dot(pq0, pq0), s * (v0.x * e0.y - v0.y * e0.x)),
                       float2(dot(pq1, pq1), s * (v1.x * e1.y - v1.y * e1.x))),
                       float2(dot(pq2, pq2), s * (v2.x * e2.y - v2.y * e2.x)));

    return -sqrt(d.x) * sign(d.y);
}

//- helper functions

float4 sample_texture(int id, float2 uv) {
    
    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    switch (id) {
        case 0: { result = color_texture[0].Sample(texture_sampler, uv); break; }
        case 1: { result = color_texture[1].Sample(texture_sampler, uv); break; }
        case 2: { result = color_texture[2].Sample(texture_sampler, uv); break; }
        case 3: { result = color_texture[3].Sample(texture_sampler, uv); break; }
        case 4: { result = color_texture[4].Sample(texture_sampler, uv); break; }
        case 5: { result = color_texture[5].Sample(texture_sampler, uv); break; }
        case 6: { result = color_texture[6].Sample(texture_sampler, uv); break; }
        case 7: { result = color_texture[7].Sample(texture_sampler, uv); break; }
        case 8: { result = color_texture[8].Sample(texture_sampler, uv); break; }
        case 9: { result = color_texture[9].Sample(texture_sampler, uv); break; }
        case 10: { result = color_texture[10].Sample(texture_sampler, uv); break; }
        case 11: { result = color_texture[11].Sample(texture_sampler, uv); break; }
        case 12: { result = color_texture[12].Sample(texture_sampler, uv); break; }
        case 13: { result = color_texture[13].Sample(texture_sampler, uv); break; }
        case 14: { result = color_texture[14].Sample(texture_sampler, uv); break; }
        case 15: { result = color_texture[15].Sample(texture_sampler, uv); break; }
    }
    
    return result;
}

float3 barycentric(float2 p, float2 a, float2 b, float2 c) {
    float2 v0 = b - a;
    float2 v1 = c - a;
    float2 v2 = p - a;
    
    float d00 = dot(v0, v0);
    float d01 = dot(v0, v1);
    float d11 = dot(v1, v1);
    float d20 = dot(v2, v0);
    float d21 = dot(v2, v1);

    float denom = d00 * d11 - d01 * d01;

    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;

    return float3(u, v, w);
}

float4 color_blend(float4 color0, float4 color1, float4 color2, float4 color3, float2 uv) {
    float4 color_a = lerp(color0, color2, uv.x);
    float4 color_b = lerp(color1, color3, uv.x);
    return lerp(color_a, color_b, uv.y);
}

float4 color_blend3(float4 color0, float4 color1, float4 color2, float3 uvw) {
	return (uvw.x * color0) + (uvw.y * color1) + (uvw.z * color2);
}

//- pixel shader

float4 ps_main(ps_in input) : SV_TARGET { 

	float4 color0 = input.color0;
	float4 color1 = input.color1;
	float4 color2 = input.color2;
	float4 color3 = input.color3;

    float4 sdf_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 tex_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    // clip
	if (input.clip_mask_index != -1) {
    	rect_t clip_mask = clip_masks[input.clip_mask_index];
    	if (input.position.x < clip_mask.p0.x || input.position.x > clip_mask.p1.x ||
        	input.position.y < clip_mask.p0.y || input.position.y > clip_mask.p1.y) {
        	discard;
    	}
	}
    
    // sample texture
	if (input.texture_index != -1) {
		tex_color = sample_texture(input.texture_index, input.texcoord);
	}

    // draw sdf shapes
    float2 sample_pos = (2.0f * input.colcoord - 1.0f) * input.half_size;
    switch (input.shape_index) {
        
		case ui_r_shape_none: {
			sdf_color = float4(1.0f, 0.0f, 1.0f, 1.0f);
			break;
		}

        case ui_r_shape_rect: {

			float2 size = input.half_size - (input.softness * 2.0f);
			float4 rounding = float4(input.params0, input.params1);

            float rect_s = sdf_rect(sample_pos, size, rounding);
            float rect_t = 1.0f - smoothstep(0.0f, 2.0f * input.softness, rect_s);
            
            float border_s = sdf_rect(sample_pos, size - input.thickness, max(rounding - input.thickness, 0.0f));
            float border_t = smoothstep(0.0f, 2.0 * input.softness, border_s);
            
            if (input.thickness == 0.0f) {
                border_t = 1.0f;
            }
            
            sdf_color = color_blend(color0, color1, color2, color3, input.colcoord);
            sdf_color.a *= rect_t;
            sdf_color.a *= border_t;
            
            break;
        }
        
        case ui_r_shape_line: {
            
			float2 p0 = input.params0;
			float2 p1 = input.params1;
			
            float line_s = sdf_line(sample_pos, p0, p1) - (input.thickness - 1.0f);
            float line_t = 1.0f - smoothstep(0.0f, 2.0f * input.softness, line_s);
            
            float col_t = length(sample_pos - p1) / length(input.half_size * 2.0f);
            
            sdf_color = lerp(color0, color1, col_t);
            sdf_color.a *= line_t;
            
            break;
        }
        
        case ui_r_shape_circle:{ 
            
            float circle_s = sdf_circle(sample_pos, input.half_size.x - (input.softness * 2.0f));
            float circle_t = 1.0f - smoothstep(0.0f, 2.0f * input.softness, circle_s);
            
            float border_s = sdf_circle(sample_pos, input.half_size.x - input.softness * 2.0f - input.thickness);
            float border_t = smoothstep(0.0f, 2.0f * input.softness, border_s);
            
            if (input.thickness == 0.0f) {
                border_t = 1.0f;
            }
            
            sdf_color = color_blend(color0, color1, color2, color3, input.colcoord);
            sdf_color.a *= circle_t;
            sdf_color.a *= border_t;
             
            break;
        }
        
        case ui_r_shape_ring:{
           
			float outer_radius = input.half_size.x - (input.softness * 2.0f);
			float inner_radius = outer_radius - input.thickness;
			float angle0 = input.params0.x * radians;
			float angle1 = input.params0.y * radians;

			float ring_s = sdf_ring(sample_pos, outer_radius, angle0, angle1);
			float ring_t = 1.0f - smoothstep(0.0f, 2.0f * input.softness, ring_s);

			float border_s = sdf_circle(sample_pos, inner_radius);
            float border_t = smoothstep(0.0f, 2.0f * input.softness, border_s);
			
			if (input.thickness == 0.0f) {
            	border_t = 1.0f;
            }
			
			float radius = length(sample_pos);
			float angle = atan2(-sample_pos.x, sample_pos.y);
			angle = (angle < 0.0f) ? angle + 2.0f * 3.14159265f : angle;

			float u = (radius - inner_radius) / (outer_radius - inner_radius);
			float v = (angle - angle0) / (angle1 - angle0);	
	
			
			sdf_color = color_blend(color0, color1, color2, color3, float2(u, v));
			sdf_color.a *= ring_t;
            sdf_color.a *= border_t;

            break;
        }
        
        case ui_r_shape_tri:{
    
			float2 p0 = input.params0;
			float2 p1 = input.params1;
			float2 p2 = input.params2;

            float tri_s = sdf_triangle(sample_pos, p0, p1, p2) - (input.softness * 2.0f);
            float tri_t = 1.0f - smoothstep(0.0f, 2.0f * input.softness, tri_s);
    
            float border_s = sdf_triangle(sample_pos, p0, p1, p2) - ((input.softness * 2.0f) - input.thickness);
            float border_t = smoothstep(0.0f, 2.0f * input.softness, border_s);
    
            if (input.thickness == 0.0f) {
                border_t = 1.0f;
            }
    
            float3 uvw = barycentric(sample_pos, p0, p1, p2);
    
            sdf_color = color_blend3(color0, color1, color2, uvw);
            sdf_color.a *= tri_t;
            sdf_color.a *= border_t;
            
            break;
        }
        
    }
    
    return sdf_color * tex_color;
}
