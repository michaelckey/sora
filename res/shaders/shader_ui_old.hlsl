// shader_2d.hlsl

// draw shapes
#define draw_shape_none (0)
#define draw_shape_rect (1)
#define draw_shape_quad (2)
#define draw_shape_line (3)
#define draw_shape_circle (4)
#define draw_shape_ring (5)
#define draw_shape_tri (6)

#define pi (3.14159265359)
#define two_pi (6.28318530718)

Texture2D color_texture[16];
SamplerState texture_sampler;

struct rect_t {
    float2 p0;
    float2 p1;
};

cbuffer globals {
    float2 window_size;
    float2 padding;
    rect_t clip_masks[128];
}

struct vs_in {
    float4 bbox          : BBOX;
    float4 texcoords     : TEX;
    float2 point0        : PNT0;
    float2 point1        : PNT1;
    float2 point2        : PNT2;
    float2 point3        : PNT3;
    float4 color0        : COL0;
    float4 color1        : COL1;
    float4 color2        : COL2;
    float4 color3        : COL3;
    float4 radii         : RAD;
    float2 style         : STY;
    uint   indices       : IDX;
    uint   vertex_id     : SV_VertexID;
};

struct ps_in {
    float4 pos         : SV_POSITION;
    float2 half_size   : HSIZE;
    float2 texcoord    : TEX;
    float2 col_coord   : COLC;
    float2 point0      : PNT0;
    float2 point1      : PNT1;
    float2 point2      : PNT2;
    float2 point3      : PNT3;
    float4 color0      : COL0;
    float4 color1      : COL1;
    float4 color2      : COL2;
    float4 color3      : COL3;
    float4 radii       : RAD;
    float2 style       : STY;
    uint shape          : SHP;
    uint texture_index  : TID;
    uint clip_index     : CLP;
};

ps_in vs_main(vs_in input) {
    
    ps_in output = (ps_in)0;
    
    // unpack bounding box
    float2 p0 = { input.bbox.xy };
    float2 p1 = { input.bbox.zw };
    float2 tex_p0 = { input.texcoords.xy };
    float2 tex_p1 = { input.texcoords.zw };
    
    // unpack indices
    uint shape_index = (input.indices >> 24) & 0xFF;
    uint texture_index = (input.indices >> 16) & 0xFF;
    uint clip_index = (input.indices >> 8) & 0xFF;
    
    // per-vertex arrays
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
    
    float2 vertex_color_coords[] = {
        float2(0.0f, 0.0f),
        float2(0.0f, 1.0f),
        float2(1.0f, 0.0f),
        float2(1.0f, 1.0f),
    };
    
    // fill output
    output.pos = float4(vertex_positions[input.vertex_id] * 2.0f / window_size - 1.0f, 0.0f, 1.0f);
    output.pos.y = -output.pos.y;
    output.half_size = (p1 - p0) * 0.5f;
    output.texcoord = vertex_texcoords[input.vertex_id];
    output.col_coord = vertex_color_coords[input.vertex_id];
    output.point0 = input.point0;
    output.point1 = input.point1;
    output.point2 = input.point2;
    output.point3 = input.point3;
    output.color0 = input.color0;
    output.color1 = input.color1;
    output.color2 = input.color2;
    output.color3 = input.color3;
    output.radii = input.radii;
    output.style = input.style;
    output.shape = shape_index;
    output.texture_index = texture_index;
    output.clip_index = clip_index;
    
    return output;
}

// sdf functions

float sdf_rect(float2 p, float2 h, float4 r) {
    r.xy = (p.x > 0.0f) ? r.xy : r.zw;
    r.x = (p.y > 0.0f) ? r.x : r.y;
    float2 q = abs(p) - h + r.x;
    return min(max(q.x, q.y), 0.0f) + length(max(q, 0.0f)) - r.x;
}

float sdf_quad(float2 p, float2 p0, float2 p1, float2 p2, float2 p3) {
    float2 e0 = p1 - p0;
    float2 e1 = p2 - p1;
    float2 e2 = p3 - p2;
    float2 e3 = p0 - p3;
    
    float2 v0 = p - p0;
    float2 v1 = p - p1;
    float2 v2 = p - p2;
    float2 v3 = p - p3;

    float2 pq0 = v0 - e0 * clamp(dot(v0, e0) / dot(e0, e0), 0.0f, 1.0f);
    float2 pq1 = v1 - e1 * clamp(dot(v1, e1) / dot(e1, e1), 0.0f, 1.0f);
    float2 pq2 = v2 - e2 * clamp(dot(v2, e2) / dot(e2, e2), 0.0f, 1.0f);
    float2 pq3 = v3 - e3 * clamp(dot(v3, e3) / dot(e3, e3), 0.0f, 1.0f);
    
    float2 ds = min(min(float2(dot(pq0, pq0), v0.x * e0.y - v0.y * e0.x),
                        float2(dot(pq1, pq1), v1.x * e1.y - v1.y * e1.x)),
                    min(float2(dot(pq2, pq2), v2.x * e2.y - v2.y * e2.x),
                        float2(dot(pq3, pq3), v3.x * e3.y - v3.y * e3.x)));

    float d = sqrt(ds.x);

    return (ds.y > 0.0f) ? -d : d;
}

float sdf_line(float2 p, float2 a, float2 b) {
    float2 ba = b - a;
    float2 pa = p - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - h * ba);
}

float sdf_circle(float2 p, float r) {
    return (length(p) - r);
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

// helper functions

float2x2 uv_rotate(float angle) {
    return float2x2(float2(cos(angle), -sin(angle)), float2(sin(angle), cos(angle)));
}

float2 polar(float2 p) {
    return float2(length(p), atan2(p.y, p.x) / two_pi);
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

float4 blend_color(float4 color0, float4 color1, float4 color2, float4 color3, float2 uv) {
    float4 color_a = lerp(color0, color2, uv.x);
    float4 color_b = lerp(color1, color3, uv.x);
    return lerp(color_a, color_b, uv.y);
}

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

float4 ps_main(ps_in input) : SV_TARGET { 
    
    // unpack style params
    float thickness = input.style.x;
    float softness = input.style.y;
    
    float4 sdf_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 tex_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    // clip if needed
    rect_t clip_mask = clip_masks[input.clip_index];
    if (input.pos.x < clip_mask.p0.x || input.pos.x > clip_mask.p1.x ||
        input.pos.y < clip_mask.p0.y || input.pos.y > clip_mask.p1.y) {
        discard;
    }
    
    // sample texture
    tex_color = sample_texture(input.texture_index, input.texcoord);
    
    // draw sdf shapes
    float2 sample_pos = (2.0f * input.col_coord - 1.0f) * input.half_size;
    switch (input.shape) {
        
        case draw_shape_rect:{

            float rect_s = sdf_rect(sample_pos, input.half_size - (softness * 2.0f), input.radii);
            float rect_t = 1.0f - smoothstep(0.0f, 2.0f * softness, rect_s);
            
            float border_s = sdf_rect(sample_pos, input.half_size - softness * 2.0f - thickness, max(input.radii - thickness, 0.0f));
            float border_t = smoothstep(0.0f, 2.0 * softness, border_s);
            
            if (thickness == 0.0f) {
                border_t = 1.0f;
            }
            
            sdf_color = blend_color(input.color0, input.color1, input.color2, input.color3, input.col_coord);
            sdf_color.a *= rect_t;
            sdf_color.a *= border_t;
            
            break;
        }
        
        case draw_shape_quad:{
            
            float quad_s = sdf_quad(sample_pos, input.point0, input.point1, input.point2, input.point3);
            float quad_t = 1.0f - smoothstep(0.0f, 2.0f * softness, quad_s);
            
            
            sdf_color = blend_color(input.color0, input.color1, input.color2, input.color3, input.col_coord);
            sdf_color.a *= quad_t;
            
            break;
        }
        
        case draw_shape_line:{;
            
            float line_s = sdf_line(sample_pos, input.point1.xy, input.point0.xy) - (thickness * 0.25f);
            float line_t = 1.0f - smoothstep(0.0f, 2.0f * softness, line_s);
            
            float col_t = length(sample_pos - input.point1.xy) / length(input.half_size * 2.0f);
            
            sdf_color = lerp(input.color0, input.color1, col_t);
            sdf_color.a *= line_t;
            
            break;
        }
        
        case draw_shape_circle:{ 
            
            float circle_s = sdf_circle(sample_pos, input.half_size.x - (softness * 2.0f));
            float circle_t = 1.0f - smoothstep(0.0f, 2.0f * softness, circle_s);
            
            float border_s = sdf_circle(sample_pos, input.half_size.x - softness * 2.0f - thickness);
            float border_t = smoothstep(0.0f, 2.0f * softness, border_s);
            
            if (thickness == 0.0f) {
                border_t = 1.0f;
            }
            
            float angle_diff = (input.point0.y - input.point0.x);
            float2 uv = mul(((input.col_coord - 0.5f) * 2.0f), uv_rotate(input.point0.x));
            float a = atan2(uv.y, uv.x);
            float theta = (a < 0.0f) ? (a + two_pi) / two_pi : a / two_pi;
            float bar = step(theta, (angle_diff) / two_pi);
            
            float2 polar_uv = frac(polar(uv));
            float2 sample_uv = float2(polar_uv.x, polar_uv.y * (two_pi / angle_diff));
            
            sdf_color = blend_color(input.color0, input.color1, input.color2, input.color3, sample_uv);
            sdf_color.a *= circle_t;
            sdf_color.a *= border_t;
            sdf_color.a *= bar;
             
            break;
        }
        
        case draw_shape_ring:{
            // TODO;
            break;
        }
        
        case draw_shape_tri:{
    
            float tri_s = sdf_triangle(sample_pos, input.point0, input.point1, input.point2) - (softness * 2.0f);
            float tri_t = 1.0f - smoothstep(0.0f, 2.0f * softness, tri_s);
    
            float border_s = sdf_triangle(sample_pos, input.point0, input.point1, input.point2) - (softness * 2.0f - thickness);
            float border_t = smoothstep(0.0f, 2.0f * softness, border_s);
    
            if (thickness == 0.0f) {
                border_t = 1.0f;
            }
    
            float3 col_t = barycentric(sample_pos, input.point0, input.point1, input.point2);
    
            sdf_color = (col_t.x * input.color0) + (col_t.y * input.color1) + (col_t.z * input.color2);
            sdf_color.a *= tri_t;
            sdf_color.a *= border_t;
            
            break;
        }
        
    }
    
    return sdf_color * tex_color;
}
