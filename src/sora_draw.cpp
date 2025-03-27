// sora_draw.cpp

#ifndef SORA_DRAW_CPP
#define SORA_DRAW_CPP

// functions

function void 
draw_init() {
    
	// allocate arena
	draw_state.batch_arena = arena_create(gigabytes(1));
    
	// create buffers
	draw_state.instance_buffer = gfx_buffer_create(gfx_buffer_type_vertex, kilobytes(256));
	draw_state.constant_buffer = gfx_buffer_create(gfx_buffer_type_constant, kilobytes(4));
    
	// assets
	gfx_shader_attribute_t draw_shader_attributes[] = {
		{ "BBOX", 0, gfx_vertex_format_float4, gfx_vertex_class_per_instance },
		{ "TEX",  0, gfx_vertex_format_float4, gfx_vertex_class_per_instance },
		{ "PNT",  0, gfx_vertex_format_float2, gfx_vertex_class_per_instance },
		{ "PNT",  1, gfx_vertex_format_float2, gfx_vertex_class_per_instance },
		{ "PNT",  2, gfx_vertex_format_float2, gfx_vertex_class_per_instance },
		{ "PNT",  3, gfx_vertex_format_float2, gfx_vertex_class_per_instance },
		{ "COL",  0, gfx_vertex_format_float4, gfx_vertex_class_per_instance },
		{ "COL",  1, gfx_vertex_format_float4, gfx_vertex_class_per_instance },
		{ "COL",  2, gfx_vertex_format_float4, gfx_vertex_class_per_instance },
		{ "COL",  3, gfx_vertex_format_float4, gfx_vertex_class_per_instance },
		{ "RAD",  0, gfx_vertex_format_float4, gfx_vertex_class_per_instance },
		{ "STY",  0, gfx_vertex_format_float2, gfx_vertex_class_per_instance },
		{ "IDX",  0, gfx_vertex_format_uint,   gfx_vertex_class_per_instance },
	};
    
    str_t draw_shader = str(
                            "// shader_2d.hlsl\n"
                            "\n"
                            "// draw shapes\n"
                            "#define draw_shape_none (0)\n"
                            "#define draw_shape_rect (1)\n"
                            "#define draw_shape_quad (2)\n"
                            "#define draw_shape_line (3)\n"
                            "#define draw_shape_circle (4)\n"
                            "#define draw_shape_ring (5)\n"
                            "#define draw_shape_tri (6)\n"
                            "\n"
                            "#define pi (3.14159265359)\n"
                            "#define two_pi (6.28318530718)\n"
                            "\n"
                            "Texture2D color_texture[16];\n"
                            "SamplerState texture_sampler;\n"
                            "\n"
                            "struct rect_t {\n"
                            "float2 p0;\n"
                            "float2 p1;\n"
                            "};\n"
                            "\n"
                            "cbuffer globals {\n"
                            "float2 window_size;\n"
                            "float2 padding;\n"
                            "rect_t clip_masks[128];\n"
                            "}\n"
                            "\n"
                            "struct vs_in {\n"
                            "float4 bbox          : BBOX;\n"
                            "float4 texcoords     : TEX;\n"
                            "float2 point0        : PNT0;\n"
                            "float2 point1        : PNT1;\n"
                            "float2 point2        : PNT2;\n"
                            "float2 point3        : PNT3;\n"
                            "float4 color0        : COL0;\n"
                            "float4 color1        : COL1;\n"
                            "float4 color2        : COL2;\n"
                            "float4 color3        : COL3;\n"
                            "float4 radii         : RAD;\n"
                            "float2 style         : STY;\n"
                            "uint   indices       : IDX;\n"
                            "uint   vertex_id     : SV_VertexID;\n"
                            "};\n"
                            "\n"
                            "struct ps_in {\n"
                            "float4 pos         : SV_POSITION;\n"
                            "float2 half_size   : HSIZE;\n"
                            "float2 texcoord    : TEX;\n"
                            "float2 col_coord   : COLC;\n"
                            "float2 point0      : PNT0;\n"
                            "float2 point1      : PNT1;\n"
                            "float2 point2      : PNT2;\n"
                            "float2 point3      : PNT3;\n"
                            "float4 color0      : COL0;\n"
                            "float4 color1      : COL1;\n"
                            "float4 color2      : COL2;\n"
                            "float4 color3      : COL3;\n"
                            "float4 radii       : RAD;\n"
                            "float2 style       : STY;\n"
                            "uint shape          : SHP;\n"
                            "uint texture_index  : TID;\n"
                            "uint clip_index     : CLP;\n"
                            "};\n"
                            "\n"
                            "ps_in vs_main(vs_in input) {\n"
                            "\n"
                            "ps_in output = (ps_in)0;\n"
                            "\n"
                            "// unpack bounding box\n"
                            "float2 p0 = { input.bbox.xy };\n"
                            "float2 p1 = { input.bbox.zw };\n"
                            "float2 tex_p0 = { input.texcoords.xy };\n"
                            "float2 tex_p1 = { input.texcoords.zw };\n"
                            "\n"
                            "// unpack indices\n"
                            "uint shape_index = (input.indices >> 24) & 0xFF;\n"
                            "uint texture_index = (input.indices >> 16) & 0xFF;\n"
                            "uint clip_index = (input.indices >> 8) & 0xFF;\n"
                            "\n"
                            "// per-vertex arrays\n"
                            "float2 vertex_positions[] = {\n"
                            "float2(p0.x, p0.y),"
                            "float2(p0.x, p1.y),"
                            "float2(p1.x, p0.y),\n"
                            "float2(p1.x, p1.y),\n"
                            "};\n"
                            "\n"
                            "float2 vertex_texcoords[] = {\n"
                            "float2(tex_p0.x, tex_p0.y),\n"
                            "float2(tex_p0.x, tex_p1.y),\n"
                            "float2(tex_p1.x, tex_p0.y),\n"
                            "float2(tex_p1.x, tex_p1.y),\n"
                            "};\n"
                            "\n"
                            "float2 vertex_color_coords[] = {\n"
                            "float2(0.0f, 0.0f),\n"
                            "float2(0.0f, 1.0f),\n"
                            "float2(1.0f, 0.0f),\n"
                            "float2(1.0f, 1.0f),\n"
                            "};\n"
                            "\n"
                            "// fill output\n"
                            "output.pos = float4(vertex_positions[input.vertex_id] * 2.0f / window_size - 1.0f, 0.0f, 1.0f);\n"
                            "output.pos.y = -output.pos.y;\n"
                            "output.half_size = (p1 - p0) * 0.5f;\n"
                            "output.texcoord = vertex_texcoords[input.vertex_id];\n"
                            "output.col_coord = vertex_color_coords[input.vertex_id];\n"
                            "output.point0 = input.point0;\n"
                            "output.point1 = input.point1;\n"
                            "output.point2 = input.point2;\n"
                            "output.point3 = input.point3;\n"
                            "output.color0 = input.color0;\n"
                            "output.color1 = input.color1;\n"
                            "output.color2 = input.color2;\n"
                            "output.color3 = input.color3;\n"
                            "output.radii = input.radii;\n"
                            "output.style = input.style;\n"
                            "output.shape = shape_index;\n"
                            "output.texture_index = texture_index;\n"
                            "output.clip_index = clip_index;\n"
                            "\n"
                            "return output;\n"
                            "}\n"
                            "\n"
                            "// sdf functions\n"
                            "\n"
                            "float sdf_rect(float2 p, float2 h, float4 r) {\n"
                            "r.xy = (p.x > 0.0f) ? r.xy : r.zw;\n"
                            "r.x = (p.y > 0.0f) ? r.x : r.y;\n"
                            "float2 q = abs(p) - h + r.x;\n"
                            "return min(max(q.x, q.y), 0.0f) + length(max(q, 0.0f)) - r.x;\n"
                            "}\n"
                            "\n"
                            "float sdf_quad(float2 p, float2 p0, float2 p1, float2 p2, float2 p3) {\n"
                            "float2 e0 = p1 - p0;\n"
                            "float2 e1 = p2 - p1;\n"
                            "float2 e2 = p3 - p2;\n"
                            "float2 e3 = p0 - p3;\n"
                            "\n"
                            "float2 v0 = p - p0;\n"
                            "float2 v1 = p - p1;\n"
                            "float2 v2 = p - p2;\n"
                            "float2 v3 = p - p3;\n"
                            "\n"
                            "float2 pq0 = v0 - e0 * clamp(dot(v0, e0) / dot(e0, e0), 0.0f, 1.0f);\n"
                            "float2 pq1 = v1 - e1 * clamp(dot(v1, e1) / dot(e1, e1), 0.0f, 1.0f);\n"
                            "float2 pq2 = v2 - e2 * clamp(dot(v2, e2) / dot(e2, e2), 0.0f, 1.0f);\n"
                            "float2 pq3 = v3 - e3 * clamp(dot(v3, e3) / dot(e3, e3), 0.0f, 1.0f);\n"
                            "\n"
                            "float2 ds = min(min(float2(dot(pq0, pq0), v0.x * e0.y - v0.y * e0.x),\n"
                            "float2(dot(pq1, pq1), v1.x * e1.y - v1.y * e1.x)),\n"
                            "min(float2(dot(pq2, pq2), v2.x * e2.y - v2.y * e2.x),\n"
                            "float2(dot(pq3, pq3), v3.x * e3.y - v3.y * e3.x)));\n"
                            "\n"
                            "float d = sqrt(ds.x);\n"
                            "\n"
                            "return (ds.y > 0.0f) ? -d : d;\n"
                            "}\n"
                            "\n"
                            "float sdf_line(float2 p, float2 a, float2 b) {\n"
                            "float2 ba = b - a;\n"
                            "float2 pa = p - a;\n"
                            "float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);\n"
                            "return length(pa - h * ba);\n"
                            "}\n"
                            "\n"
                            "float sdf_circle(float2 p, float r) {\n"
                            "return (length(p) - r);\n"
                            "}\n"
                            "\n"
                            "float sdf_triangle(float2 p, float2 a, float2 b, float2 c) {\n"
                            "\n"
                            "float2 e0 = b - a;\n"
                            "float2 e1 = c - b;\n"
                            "float2 e2 = a - c;\n"
                            "\n"
                            "float2 v0 = p - a;\n"
                            "float2 v1 = p - b;\n"
                            "float2 v2 = p - c;\n"
                            "\n"
                            "float2 pq0 = v0 - e0 * clamp(dot(v0, e0) / dot(e0, e0), 0.0f, 1.0f);\n"
                            "float2 pq1 = v1 - e1 * clamp(dot(v1, e1) / dot(e1, e1), 0.0f, 1.0f);\n"
                            "float2 pq2 = v2 - e2 * clamp(dot(v2, e2) / dot(e2, e2), 0.0f, 1.0f);\n"
                            "\n"
                            "float s = e0.x * e2.y - e0.y * e2.x;\n"
                            "float2 d = min(min(float2(dot(pq0, pq0), s * (v0.x * e0.y - v0.y * e0.x)),\n"
                            "float2(dot(pq1, pq1), s * (v1.x * e1.y - v1.y * e1.x))),\n"
                            "float2(dot(pq2, pq2), s * (v2.x * e2.y - v2.y * e2.x)));\n"
                            "\n"
                            "return -sqrt(d.x) * sign(d.y);\n"
                            "}\n"
                            "\n"
                            "// helper functions\n"
                            "\n"
                            "float2x2 uv_rotate(float angle) {\n"
                            "return float2x2(float2(cos(angle), -sin(angle)), float2(sin(angle), cos(angle)));\n"
                            "}\n"
                            "\n"
                            "float2 polar(float2 p) {\n"
                            "return float2(length(p), atan2(p.y, p.x) / two_pi);\n"
                            "}\n"
                            "\n"
                            "float3 barycentric(float2 p, float2 a, float2 b, float2 c) {\n"
                            "float2 v0 = b - a;\n"
                            "float2 v1 = c - a;\n"
                            "float2 v2 = p - a;\n"
                            "\n"
                            "float d00 = dot(v0, v0);\n"
                            "float d01 = dot(v0, v1);\n"
                            "float d11 = dot(v1, v1);\n"
                            "float d20 = dot(v2, v0);\n"
                            "float d21 = dot(v2, v1);\n"
                            "\n"
                            "float denom = d00 * d11 - d01 * d01;\n"
                            "\n"
                            "float v = (d11 * d20 - d01 * d21) / denom;\n"
                            "float w = (d00 * d21 - d01 * d20) / denom;\n"
                            "float u = 1.0f - v - w;\n"
                            "\n"
                            "return float3(u, v, w);\n"
                            "}\n"
                            "\n"
                            "float4 blend_color(float4 color0, float4 color1, float4 color2, float4 color3, float2 uv) {\n"
                            "float4 color_a = lerp(color0, color2, uv.x);\n"
                            "float4 color_b = lerp(color1, color3, uv.x);\n"
                            "return lerp(color_a, color_b, uv.y);\n"
                            "}\n"
                            "\n"
                            "float4 sample_texture(int id, float2 uv) {\n"
                            "\n"
                            "float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);\n"
                            "\n"
                            "switch (id) {\n"
                            "case 0: { result = color_texture[0].Sample(texture_sampler, uv); break; }\n"
                            "case 1: { result = color_texture[1].Sample(texture_sampler, uv); break; }\n"
                            "case 2: { result = color_texture[2].Sample(texture_sampler, uv); break; }\n"
                            "case 3: { result = color_texture[3].Sample(texture_sampler, uv); break; }\n"
                            "case 4: { result = color_texture[4].Sample(texture_sampler, uv); break; }\n"
                            "case 5: { result = color_texture[5].Sample(texture_sampler, uv); break; }\n"
                            "case 6: { result = color_texture[6].Sample(texture_sampler, uv); break; }\n"
                            "case 7: { result = color_texture[7].Sample(texture_sampler, uv); break; }\n"
                            "case 8: { result = color_texture[8].Sample(texture_sampler, uv); break; }\n"
                            "case 9: { result = color_texture[9].Sample(texture_sampler, uv); break; }\n"
                            "case 10: { result = color_texture[10].Sample(texture_sampler, uv); break; }\n"
                            "case 11: { result = color_texture[11].Sample(texture_sampler, uv); break; }\n"
                            "case 12: { result = color_texture[12].Sample(texture_sampler, uv); break; }\n"
                            "case 13: { result = color_texture[13].Sample(texture_sampler, uv); break; }\n"
                            "case 14: { result = color_texture[14].Sample(texture_sampler, uv); break; }\n"
                            "case 15: { result = color_texture[15].Sample(texture_sampler, uv); break; }\n"
                            "}\n"
                            "\n"
                            "return result;\n"
                            "\n"
                            "}\n"
                            "\n"
                            "float4 ps_main(ps_in input) : SV_TARGET { \n"
                            "\n"
                            "// unpack style params\n"
                            "float thickness = input.style.x;\n"
                            "float softness = input.style.y;\n"
                            "\n"
                            "float4 sdf_color = float4(1.0f, 1.0f, 1.0f, 1.0f);\n"
                            "float4 tex_color = float4(1.0f, 1.0f, 1.0f, 1.0f);\n"
                            "\n"
                            "// clip if needed\n"
                            "rect_t clip_mask = clip_masks[input.clip_index];\n"
                            "if (input.pos.x < clip_mask.p0.x || input.pos.x > clip_mask.p1.x ||\n"
                            "input.pos.y < clip_mask.p0.y || input.pos.y > clip_mask.p1.y) {\n"
                            "discard;\n"
                            "}\n"
                            "\n"
                            "// sample texture\n"
                            "tex_color = sample_texture(input.texture_index, input.texcoord);\n"
                            "\n"
                            "// draw sdf shapes\n"
                            "float2 sample_pos = (2.0f * input.col_coord - 1.0f) * input.half_size;\n"
                            "switch (input.shape) {\n"
                            "\n"
                            "case draw_shape_rect:{\n"
                            "\n"
                            "float rect_s = sdf_rect(sample_pos, input.half_size - (softness * 2.0f), input.radii);\n"
                            "float rect_t = 1.0f - smoothstep(0.0f, 2.0f * softness, rect_s);\n"
                            "\n"
                            "float border_s = sdf_rect(sample_pos, input.half_size - softness * 2.0f - thickness, max(input.radii - thickness, 0.0f));\n"
                            "float border_t = smoothstep(0.0f, 2.0 * softness, border_s);\n"
                            "\n"
                            "if (thickness == 0.0f) {\n"
                            "border_t = 1.0f;\n"
                            "}\n"
                            "\n"
                            "sdf_color = blend_color(input.color0, input.color1, input.color2, input.color3, input.col_coord);\n"
                            "sdf_color.a *= rect_t;\n"
                            "sdf_color.a *= border_t;\n"
                            "\n"
                            "break;\n"
                            "}\n"
                            "\n"
                            "case draw_shape_quad:{\n"
                            "\n"
                            "float quad_s = sdf_quad(sample_pos, input.point0, input.point1, input.point2, input.point3);\n"
                            "float quad_t = 1.0f - smoothstep(0.0f, 2.0f * softness, quad_s);\n"
                            "\n"
                            "\n"
                            "sdf_color = blend_color(input.color0, input.color1, input.color2, input.color3, input.col_coord);\n"
                            "sdf_color.a *= quad_t;\n"
                            "\n"
                            "break;\n"
                            "}\n"
                            "\n"
                            "case draw_shape_line:{;\n"
                            "\n"
                            "float line_s = sdf_line(sample_pos, input.point1.xy, input.point0.xy) - (thickness * 0.25f);\n"
                            "float line_t = 1.0f - smoothstep(0.0f, 2.0f * softness, line_s);\n"
                            "\n"
                            "float col_t = length(sample_pos - input.point1.xy) / length(input.half_size * 2.0f);\n"
                            "\n"
                            "sdf_color = lerp(input.color0, input.color1, col_t);\n"
                            "sdf_color.a *= line_t;\n"
                            "\n"
                            "break;\n"
                            "}\n"
                            "\n"
                            "case draw_shape_circle:{ \n"
                            "\n"
                            "float circle_s = sdf_circle(sample_pos, input.half_size.x - (softness * 2.0f));\n"
                            "float circle_t = 1.0f - smoothstep(0.0f, 2.0f * softness, circle_s);\n"
                            "\n"
                            "float border_s = sdf_circle(sample_pos, input.half_size.x - softness * 2.0f - thickness);\n"
                            "float border_t = smoothstep(0.0f, 2.0f * softness, border_s);\n"
                            "\n"
                            "if (thickness == 0.0f) {\n"
                            "border_t = 1.0f;\n"
                            "}\n"
                            "\n"
                            "float angle_diff = (input.point0.y - input.point0.x);\n"
                            "float2 uv = mul(((input.col_coord - 0.5f) * 2.0f), uv_rotate(input.point0.x));\n"
                            "float a = atan2(uv.y, uv.x);\n"
                            "float theta = (a < 0.0f) ? (a + two_pi) / two_pi : a / two_pi;\n"
                            "float bar = step(theta, (angle_diff) / two_pi);\n"
                            "\n"
                            "float2 polar_uv = frac(polar(uv));\n"
                            "float2 sample_uv = float2(polar_uv.x, polar_uv.y * (two_pi / angle_diff));\n"
                            "\n"
                            "sdf_color = blend_color(input.color0, input.color1, input.color2, input.color3, sample_uv);\n"
                            "sdf_color.a *= circle_t;\n"
                            "sdf_color.a *= border_t;\n"
                            "sdf_color.a *= bar;\n"
                            "\n"
                            "break;\n"
                            "}\n"
                            "\n"
                            "case draw_shape_ring:{\n"
                            "// TODO;\n"
                            "break;\n"
                            "}\n"
                            "\n"
                            "case draw_shape_tri:{\n"
                            "\n"
                            "float tri_s = sdf_triangle(sample_pos, input.point0, input.point1, input.point2) - (softness * 2.0f);\n"
                            "float tri_t = 1.0f - smoothstep(0.0f, 2.0f * softness, tri_s);\n"
                            "\n"
                            "float border_s = sdf_triangle(sample_pos, input.point0, input.point1, input.point2) - (softness * 2.0f - thickness);\n"
                            "float border_t = smoothstep(0.0f, 2.0f * softness, border_s);\n"
                            "\n"
                            "if (thickness == 0.0f) {\n"
                            "border_t = 1.0f;\n"
                            "}\n"
                            "\n"
                            "float3 col_t = barycentric(sample_pos, input.point0, input.point1, input.point2);\n"
                            "\n"
                            "sdf_color = (col_t.x * input.color0) + (col_t.y * input.color1) + (col_t.z * input.color2);\n"
                            "sdf_color.a *= tri_t;\n"
                            "sdf_color.a *= border_t;\n"
                            "\n"
                            "break;\n"
                            "}\n"
                            "}\n"
                            "return sdf_color * tex_color;\n"
                            "}\n"
                            );
    
    draw_state.shader = gfx_shader_create(draw_shader, str("draw_shader"), draw_shader_attributes, array_count(draw_shader_attributes));
	draw_state.font = font_open(str("C:/Windows/Fonts/segoeui.ttf"));
    
	u32 data = 0xffffffff;
	draw_state.texture = gfx_texture_create(uvec2(1, 1), gfx_texture_format_rgba8, &data);
    
	draw_state.pipeline = gfx_pipeline_create();
	draw_state.pipeline.depth_mode = gfx_depth_none;
	draw_state.pipeline.topology = gfx_topology_tri_strip;
    
	// stack defaults
	draw_state.color0_default_node.v = color(0xffffffff);
	draw_state.color1_default_node.v = color(0xffffffff);
	draw_state.color2_default_node.v = color(0xffffffff);
	draw_state.color3_default_node.v = color(0xffffffff);
    
	draw_state.radius0_default_node.v = 0.0f;
	draw_state.radius1_default_node.v = 0.0f;
	draw_state.radius2_default_node.v = 0.0f;
	draw_state.radius3_default_node.v = 0.0f;
    
	draw_state.thickness_default_node.v = 0.0f;
	draw_state.softness_default_node.v = 0.33f;
    
	draw_state.font_default_node.v = draw_state.font;
	draw_state.font_size_default_node.v = 9.0f;
    
	draw_state.clip_mask_default_node.v = rect(0.0f, 0.0f, 4096.0f, 4096.0f);
    
	draw_state.texture_default_node.v = font_state.atlas_texture;
    
}

function void 
draw_release() {
    
	// release buffer
	gfx_buffer_release(draw_state.constant_buffer);
	gfx_buffer_release(draw_state.instance_buffer);
    
	// release assets
	gfx_texture_release(draw_state.texture);
	gfx_shader_release(draw_state.shader);
	font_close(draw_state.font);
    
	// release arena
	arena_release(draw_state.batch_arena);
}

function void 
draw_begin(gfx_handle_t renderer) {
	
    prof_scope("draw_begin") {
        
        
        // clear batch arena
        arena_clear(draw_state.batch_arena);
        draw_state.batch_first = nullptr;
        draw_state.batch_last = nullptr;
        
        // update pipeline and constant buffer
        uvec2_t renderer_size = gfx_renderer_get_size(renderer);
        rect_t screen = rect(0.0f, 0.0f, (f32)renderer_size.x, (f32)renderer_size.y);
        draw_state.pipeline.viewport = screen;
        draw_state.pipeline.scissor = screen;
        draw_state.constants.window_size = vec2(screen.x1, screen.y1);
        
        // reset texture list
        memset(draw_state.texture_list, 0, sizeof(gfx_handle_t) * draw_max_textures);
        draw_state.texture_count = 0;
        
        // reset clip mask
        memset(draw_state.constants.clip_masks, 0, sizeof(rect_t) * 128);
        draw_state.clip_mask_count = 0;
        
        // reset stacks
        draw_state.color0_stack.top = &draw_state.color0_default_node; draw_state.color0_stack.free = 0; draw_state.color0_stack.auto_pop = 0;
        draw_state.color1_stack.top = &draw_state.color1_default_node; draw_state.color1_stack.free = 0; draw_state.color1_stack.auto_pop = 0;
        draw_state.color2_stack.top = &draw_state.color2_default_node; draw_state.color2_stack.free = 0; draw_state.color2_stack.auto_pop = 0;
        draw_state.color3_stack.top = &draw_state.color3_default_node; draw_state.color3_stack.free = 0; draw_state.color3_stack.auto_pop = 0;
        
        draw_state.radius0_stack.top = &draw_state.radius0_default_node; draw_state.radius0_stack.free = 0; draw_state.radius0_stack.auto_pop = 0;
        draw_state.radius1_stack.top = &draw_state.radius1_default_node; draw_state.radius1_stack.free = 0; draw_state.radius1_stack.auto_pop = 0;
        draw_state.radius2_stack.top = &draw_state.radius2_default_node; draw_state.radius2_stack.free = 0; draw_state.radius2_stack.auto_pop = 0;
        draw_state.radius3_stack.top = &draw_state.radius3_default_node; draw_state.radius3_stack.free = 0; draw_state.radius3_stack.auto_pop = 0;
        
        draw_state.thickness_stack.top = &draw_state.thickness_default_node; draw_state.thickness_stack.free = 0; draw_state.thickness_stack.auto_pop = 0;
        draw_state.softness_stack.top = &draw_state.softness_default_node; draw_state.softness_stack.free = 0; draw_state.softness_stack.auto_pop = 0;
        draw_state.font_stack.top = &draw_state.font_default_node; draw_state.font_stack.free = 0; draw_state.font_stack.auto_pop = 0;
        draw_state.font_size_stack.top = &draw_state.font_size_default_node; draw_state.font_size_stack.free = 0; draw_state.font_size_stack.auto_pop = 0;
        
        draw_state.clip_mask_stack.top = &draw_state.clip_mask_default_node; draw_state.clip_mask_stack.free = 0; draw_state.clip_mask_stack.auto_pop = 0;
        
        draw_state.texture_stack.top = &draw_state.texture_default_node; draw_state.texture_stack.free = 0; draw_state.texture_stack.auto_pop = 0;
        
        // push default clip mask and texture
        draw_push_clip_mask(rect(0.0f, 0.0f, (f32)renderer_size.x, (f32)renderer_size.y));
        draw_push_texture(draw_state.texture);
        
        
    }
}

function void 
draw_end(gfx_handle_t renderer) {
	prof_scope("draw_end") {
        
        // update constant buffer
        gfx_buffer_fill(draw_state.constant_buffer, &draw_state.constants, sizeof(draw_constants_t));
        
        // set state
        gfx_set_pipeline(draw_state.pipeline);
        gfx_set_shader(draw_state.shader);
        gfx_set_buffer(draw_state.constant_buffer);
        gfx_set_texture_array(draw_state.texture_list, draw_state.texture_count, 0, gfx_texture_usage_ps);
        
        for (draw_batch_t* batch = draw_state.batch_first; batch != 0; batch = batch->next) {
            
            // fill instance buffer
            gfx_buffer_fill(draw_state.instance_buffer, batch->instances, batch->instance_count * sizeof(draw_instance_t));
            gfx_set_buffer(draw_state.instance_buffer, 0, sizeof(draw_instance_t));
            
            gfx_draw_instanced(4, batch->instance_count);
        }
        
    }
}


function draw_instance_t*
draw_get_instance() {
    
	// find a batch
	draw_batch_t* batch = nullptr;
    
	// search batch list
	for (draw_batch_t* b = draw_state.batch_first; b != 0; b = b->next) {
        
		// if batch has space, and texture matches
		if (((b->instance_count + 1) * sizeof(draw_instance_t)) < (kilobytes(256))) {
			batch = b;
			break;
		}
	}
    
	// else create one
	if (batch == nullptr) {
        
		batch = (draw_batch_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_batch_t));
		
		batch->instances = (draw_instance_t*)arena_alloc(draw_state.batch_arena, kilobytes(256));
		batch->instance_count = 0;
		
		// add to batch list
		dll_push_back(draw_state.batch_first, draw_state.batch_last, batch);
        
	}
    
	// get instance
	draw_instance_t* instance = &batch->instances[batch->instance_count++];
	memset(instance, 0, sizeof(draw_instance_t));
    
	return instance;
}

function i32
draw_get_texture_index(gfx_handle_t texture) {
    
	// find index if in list
	i32 index = 0;
	for (; index < draw_state.texture_count; index++) {
		if (gfx_handle_equals(texture, draw_state.texture_list[index])) {
			break;
		}
	}
    
	// we didn't find one, add to list
	if (index == draw_state.texture_count) {
		draw_state.texture_list[draw_state.texture_count] = texture;
		draw_state.texture_count++;
	}
    
	return index;
    
}

function i32
draw_get_clip_mask_index(rect_t rect) {
    
	// find index if in list
	i32 index = 0;
	for (; index < draw_state.clip_mask_count; index++) {
		if (rect_equals(rect, draw_state.constants.clip_masks[index])) {
			break;
		}
	}
    
	// we didn't find one, add to list
	if (index == draw_state.clip_mask_count) {
		draw_state.constants.clip_masks[draw_state.clip_mask_count] = rect;
		draw_state.clip_mask_count++;
	}
    
	return index;
}

inlnfunc u32
draw_pack_indices(u32 shape, u32 texture, u32 clip) {
	return (shape << 24) | (texture << 16) | (clip << 8);
}



function void 
draw_rect(rect_t rect) {
    
	draw_instance_t* instance = draw_get_instance();
    
	rect_validate(rect);
    
	instance->bbox = rect;
	instance->tex = {0.0f, 0.0f, 1.0f, 1.0f};
	
	instance->color0 = draw_top_color0().vec;
	instance->color1 = draw_top_color1().vec;
	instance->color2 = draw_top_color2().vec;
	instance->color3 = draw_top_color3().vec;
    
	instance->radii = draw_top_rounding();
	instance->thickness = draw_top_thickness();
	instance->softness = draw_top_softness();
    
	instance->indices = draw_pack_indices(
                                          draw_shape_rect,
                                          draw_get_texture_index(draw_top_texture()),
                                          draw_get_clip_mask_index(draw_top_clip_mask())
                                          );
    
	draw_auto_pop_stacks();
}

function void
draw_image(rect_t rect) {
    
	draw_instance_t* instance = draw_get_instance();
    
	rect_validate(rect);
    
	instance->bbox = rect;
	instance->tex = { 0.0f, 0.0f, 1.0f, 1.0f };
    
	instance->color0 = draw_top_color0().vec;
	instance->color1 = draw_top_color1().vec;
	instance->color2 = draw_top_color2().vec;
	instance->color3 = draw_top_color3().vec;
    
	instance->radii = draw_top_rounding();
	instance->thickness = draw_top_thickness();
	instance->softness = draw_top_softness();
    
	instance->indices = draw_pack_indices(
                                          draw_shape_rect,
                                          draw_get_texture_index(draw_top_texture()),
                                          draw_get_clip_mask_index(draw_top_clip_mask())
                                          );
    
	draw_auto_pop_stacks();
    
}

function void
draw_quad(vec2_t p0, vec2_t p1, vec2_t p2, vec2_t p3) {
    
	// order: (0, 0), (0, 1), (1, 1), (1, 0);
    
	draw_instance_t* instance = draw_get_instance();
    
	f32 softness = draw_top_softness();
    
	f32 min_x = min(min(min(p0.x, p1.x), p2.x), p3.x);
	f32 min_y = min(min(min(p0.y, p1.y), p2.y), p3.y);
	f32 max_x = max(max(max(p0.x, p1.x), p2.x), p3.x);
	f32 max_y = max(max(max(p0.y, p1.y), p2.y), p3.y);
    
	rect_t bbox = rect_grow(rect(min_x, min_y, max_x, max_y), softness);
    
	vec2_t c = rect_center(bbox);
	vec2_t c_p0 = vec2_sub(p0, c);
	vec2_t c_p1 = vec2_sub(p1, c);
	vec2_t c_p2 = vec2_sub(p2, c);
	vec2_t c_p3 = vec2_sub(p3, c);
    
	instance->bbox = bbox;
    
	instance->color0 = draw_top_color0().vec;
	instance->color1 = draw_top_color1().vec;
	instance->color2 = draw_top_color2().vec;
	instance->color3 = draw_top_color3().vec;
    
	instance->point0 = c_p0;
	instance->point1 = c_p1;
	instance->point2 = c_p2;
	instance->point3 = c_p3;
    
	instance->thickness = draw_top_thickness();
	instance->softness = softness;
    
	instance->indices = draw_pack_indices(
                                          draw_shape_quad,
                                          draw_get_texture_index(draw_top_texture()),
                                          draw_get_clip_mask_index(draw_top_clip_mask())
                                          );
    
	draw_auto_pop_stacks();
}

function void
draw_line(vec2_t p0, vec2_t p1) {
    
	draw_instance_t* instance = draw_get_instance();
    
	f32 thickness = draw_top_thickness();
	f32 softness = draw_top_softness();
    
	f32 min_x = min(p0.x, p1.x);
	f32 min_y = min(p0.y, p1.y);
	f32 max_x = max(p0.x, p1.x);
	f32 max_y = max(p0.y, p1.y);
    
	rect_t bbox = rect_grow(rect(min_x, min_y, max_x, max_y), softness + thickness + 2.0f);
    
	vec2_t c = rect_center(bbox);
	vec2_t c_p0 = vec2_sub(c, p0);
	vec2_t c_p1 = vec2_sub(c, p1);
    
	instance->bbox = bbox;
    
	instance->color0 = draw_top_color0().vec;
	instance->color1 = draw_top_color1().vec;
    
	instance->point0 = c_p0;
	instance->point1 = c_p1;
    
	instance->thickness = thickness;
	instance->softness = softness;
    
	instance->indices = draw_pack_indices(
                                          draw_shape_line,
                                          draw_get_texture_index(draw_top_texture()),
                                          draw_get_clip_mask_index(draw_top_clip_mask())
                                          );
    
	draw_auto_pop_stacks();
}

function void 
draw_circle(vec2_t pos, f32 radius, f32 start_angle, f32 end_angle) {
	
	draw_instance_t* instance = draw_get_instance();
	
	f32 softness = draw_top_softness();
    
	instance->bbox = rect_grow(rect(pos.x - radius, pos.y - radius, pos.x + radius, pos.y + radius), softness);
    
	instance->color0 = draw_top_color0().vec;
	instance->color1 = draw_top_color1().vec;
	instance->color2 = draw_top_color2().vec;
	instance->color3 = draw_top_color3().vec;
    
	instance->point0 = vec2(radians(start_angle), radians(end_angle));
	
	instance->thickness = draw_top_thickness();
	instance->softness = softness;
    
	instance->indices = draw_pack_indices(
                                          draw_shape_circle,
                                          draw_get_texture_index(draw_top_texture()),
                                          draw_get_clip_mask_index(draw_top_clip_mask())
                                          );
    
    
	draw_auto_pop_stacks();
} 

function void
draw_tri(vec2_t p0, vec2_t p1, vec2_t p2) {
    
	draw_instance_t* instance = draw_get_instance();
    
	f32 softness = draw_top_softness();
    
	f32 min_x = min(min(p0.x, p1.x), p2.x);
	f32 min_y = min(min(p0.y, p1.y), p2.y);
	f32 max_x = max(max(p0.x, p1.x), p2.x);
	f32 max_y = max(max(p0.y, p1.y), p2.y);
    
	rect_t bbox = rect_grow(rect(min_x, min_y, max_x, max_y), softness * 5.0f);
    
	vec2_t c = rect_center(bbox);
	vec2_t c_p0 = vec2_sub(p0, c);
	vec2_t c_p1 = vec2_sub(p1, c);
	vec2_t c_p2 = vec2_sub(p2, c);
    
	instance->bbox = bbox;
    
	instance->color0 = draw_top_color0().vec;
	instance->color1 = draw_top_color1().vec;
	instance->color2 = draw_top_color2().vec;
    
	instance->point0 = c_p0;
	instance->point1 = c_p1;
	instance->point2 = c_p2;
    
	instance->radii = draw_top_rounding();
    
	instance->thickness = draw_top_thickness();
	instance->softness = softness;
    
	instance->indices = draw_pack_indices(
                                          draw_shape_tri,
                                          draw_get_texture_index(draw_top_texture()),
                                          draw_get_clip_mask_index(draw_top_clip_mask())
                                          );
    
    
	draw_auto_pop_stacks();
}

function void
draw_text(str_t text, vec2_t pos) {
    
	f32 font_size = draw_top_font_size();
	font_handle_t font = draw_top_font();
    
	for (u32 i = 0; i < text.size; i++) {
        
		draw_instance_t* instance = draw_get_instance();
		
		u8 codepoint = *(text.data + i);
		font_glyph_t* glyph = font_get_glyph(font, font_size, codepoint);
        
		instance->bbox = rect(pos.x, pos.y, pos.x + glyph->pos.x1, pos.y + glyph->pos.y1);
		instance->tex = glyph->uv;
        
		instance->color0 = draw_top_color0().vec;
		instance->color1 = draw_top_color1().vec;
		instance->color2 = draw_top_color2().vec;
		instance->color3 = draw_top_color3().vec;
        
		instance->indices = draw_pack_indices(
                                              draw_shape_rect,
                                              draw_get_texture_index(font_state.atlas_texture),
                                              draw_get_clip_mask_index(draw_top_clip_mask())
                                              );
        
		pos.x += glyph->advance;
	}
    
	draw_auto_pop_stacks();
}

function void
draw_bezier(vec2_t p0, vec2_t p1, vec2_t c0, vec2_t c1) {
    
	const i32 count = 32;
	vec2_t prev_point = p0;
    
    for (i32 i = 1; i < count; i++) {
        
		f32 t = (f32)i / (f32)count;
        
		vec2_t v0 = vec2_lerp(p0, c0, t);
		vec2_t v1 = vec2_lerp(c0, c1, t);
		vec2_t v2 = vec2_lerp(c1, p1, t);
        
		vec2_t v3 = vec2_lerp(v0, v1, t);
		vec2_t v4 = vec2_lerp(v1, v2, t);
        
		vec2_t curve_point = vec2_lerp(v3, v4, t);
        
		draw_line(prev_point, curve_point);
        
		prev_point = curve_point;
	}
    
}

// stack functions

function void
draw_auto_pop_stacks() {
    
    if (draw_state.color0_stack.auto_pop) { draw_pop_color0(); draw_state.color0_stack.auto_pop = false; }
    if (draw_state.color1_stack.auto_pop) { draw_pop_color1(); draw_state.color1_stack.auto_pop = false; }
    if (draw_state.color2_stack.auto_pop) { draw_pop_color2(); draw_state.color2_stack.auto_pop = false; }
    if (draw_state.color3_stack.auto_pop) { draw_pop_color3(); draw_state.color3_stack.auto_pop = false; }
    if (draw_state.radius0_stack.auto_pop) { draw_pop_radius0(); draw_state.radius0_stack.auto_pop = false; }
    if (draw_state.radius1_stack.auto_pop) { draw_pop_radius1(); draw_state.radius1_stack.auto_pop = false; }
    if (draw_state.radius2_stack.auto_pop) { draw_pop_radius2(); draw_state.radius2_stack.auto_pop = false; }
    if (draw_state.radius3_stack.auto_pop) { draw_pop_radius3(); draw_state.radius3_stack.auto_pop = false; }
    if (draw_state.thickness_stack.auto_pop) { draw_pop_thickness(); draw_state.thickness_stack.auto_pop = false; }
    if (draw_state.softness_stack.auto_pop) { draw_pop_softness(); draw_state.softness_stack.auto_pop = false; }
    if (draw_state.font_stack.auto_pop) { draw_pop_font(); draw_state.font_stack.auto_pop = false; }
    if (draw_state.font_size_stack.auto_pop) { draw_pop_font_size(); draw_state.font_size_stack.auto_pop = false; }
    if (draw_state.clip_mask_stack.auto_pop) { draw_pop_clip_mask(); draw_state.clip_mask_stack.auto_pop = false; }
    if (draw_state.texture_stack.auto_pop) { draw_pop_texture(); draw_state.texture_stack.auto_pop = false; }
    
}

// color0
function color_t 
draw_top_color0() {
	return draw_state.color0_stack.top->v;
}

function color_t 
draw_push_color0(color_t v) {
	draw_color0_node_t* node = draw_state.color0_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.color0_stack.free);
	} else {
		node = (draw_color0_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_color0_node_t));
	} 
	color_t old_value = draw_state.color0_stack.top->v; 
	node->v = v; stack_push(draw_state.color0_stack.top, node);
	draw_state.color0_stack.auto_pop = false; 
	return old_value;
}

function color_t 
draw_pop_color0() {
	draw_color0_node_t* popped = draw_state.color0_stack.top; 
    color_t result = { 0 };
	if (popped != 0) {
        result = popped->v;
		stack_pop(draw_state.color0_stack.top); 
		stack_push(draw_state.color0_stack.free, popped);
		draw_state.color0_stack.auto_pop = false;
	} 
	return result;
}

function color_t 
draw_set_next_color0(color_t v) {
	draw_color0_node_t* node = draw_state.color0_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.color0_stack.free);
	} else {
		node = (draw_color0_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_color0_node_t));
	} 
	color_t old_value = draw_state.color0_stack.top->v;
	node->v = v; 
	stack_push(draw_state.color0_stack.top, node);
	draw_state.color0_stack.auto_pop = true; 
	return old_value;
};


// color1
function color_t 
draw_top_color1() {
	return draw_state.color1_stack.top->v;
}

function color_t 
draw_push_color1(color_t v) {
	draw_color1_node_t* node = draw_state.color1_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.color1_stack.free);
	} else {
		node = (draw_color1_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_color1_node_t));
	} 
	color_t old_value = draw_state.color1_stack.top->v; 
	node->v = v; stack_push(draw_state.color1_stack.top, node);
	draw_state.color1_stack.auto_pop = false; 
	return old_value;
}

function color_t 
draw_pop_color1() {
	draw_color1_node_t* popped = draw_state.color1_stack.top; 
	color_t result = { 0 };
	if (popped != 0) {
        result = popped->v;
		stack_pop(draw_state.color1_stack.top); 
		stack_push(draw_state.color1_stack.free, popped);
		draw_state.color1_stack.auto_pop = false;
	} 
	return result;
}

function color_t 
draw_set_next_color1(color_t v) {
	draw_color1_node_t* node = draw_state.color1_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.color1_stack.free);
	} else {
		node = (draw_color1_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_color1_node_t));
	} 
	color_t old_value = draw_state.color1_stack.top->v;
	node->v = v; 
	stack_push(draw_state.color1_stack.top, node);
	draw_state.color1_stack.auto_pop = true; 
	return old_value;
};

// color2

function color_t 
draw_top_color2() {
	return draw_state.color2_stack.top->v;
}

function color_t 
draw_push_color2(color_t v) {
	draw_color2_node_t* node = draw_state.color2_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.color2_stack.free);
	} else {
		node = (draw_color2_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_color2_node_t));
	} 
	color_t old_value = draw_state.color2_stack.top->v; 
	node->v = v; stack_push(draw_state.color2_stack.top, node);
	draw_state.color2_stack.auto_pop = false; 
	return old_value;
}

function color_t 
draw_pop_color2() {
	draw_color2_node_t* popped = draw_state.color2_stack.top; 
	color_t result = { 0 };
	if (popped != 0) {
        result = popped->v;
		stack_pop(draw_state.color2_stack.top); 
		stack_push(draw_state.color2_stack.free, popped);
		draw_state.color2_stack.auto_pop = false;
	} 
	return result;
}

function color_t 
draw_set_next_color2(color_t v) {
	draw_color2_node_t* node = draw_state.color2_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.color2_stack.free);
	} else {
		node = (draw_color2_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_color2_node_t));
	} 
	color_t old_value = draw_state.color2_stack.top->v;
	node->v = v; 
	stack_push(draw_state.color2_stack.top, node);
	draw_state.color2_stack.auto_pop = true; 
	return old_value;
};

// color3
function color_t 
draw_top_color3() {
	return draw_state.color3_stack.top->v;
}

function color_t 
draw_push_color3(color_t v) {
	draw_color3_node_t* node = draw_state.color3_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.color3_stack.free);
	} else {
		node = (draw_color3_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_color3_node_t));
	} 
	color_t old_value = draw_state.color3_stack.top->v; 
	node->v = v; stack_push(draw_state.color3_stack.top, node);
	draw_state.color3_stack.auto_pop = false; 
	return old_value;
}

function color_t 
draw_pop_color3() {
	draw_color3_node_t* popped = draw_state.color3_stack.top; 
	color_t result = { 0 };
	if (popped != 0) {
        result = popped->v;
		stack_pop(draw_state.color3_stack.top); 
		stack_push(draw_state.color3_stack.free, popped);
		draw_state.color3_stack.auto_pop = false;
	} 
	return result;
}

function color_t 
draw_set_next_color3(color_t v) {
	draw_color3_node_t* node = draw_state.color3_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.color3_stack.free);
	} else {
		node = (draw_color3_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_color3_node_t));
	} 
	color_t old_value = draw_state.color3_stack.top->v;
	node->v = v; 
	stack_push(draw_state.color3_stack.top, node);
	draw_state.color3_stack.auto_pop = true; 
	return old_value;
};

// radius0

function f32 
draw_top_radius0() {
	return draw_state.radius0_stack.top->v;
}

function f32 
draw_push_radius0(f32 v) {
	draw_radius0_node_t* node = draw_state.radius0_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.radius0_stack.free);
	} else {
		node = (draw_radius0_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_radius0_node_t));
	} 
	f32 old_value = draw_state.radius0_stack.top->v; 
	node->v = v; stack_push(draw_state.radius0_stack.top, node);
	draw_state.radius0_stack.auto_pop = false; 
	return old_value;
}

function f32 
draw_pop_radius0() {
	draw_radius0_node_t* popped = draw_state.radius0_stack.top; 
    f32 result = { 0 };
	if (popped != 0) {
        result = popped->v;
		stack_pop(draw_state.radius0_stack.top); 
		stack_push(draw_state.radius0_stack.free, popped);
		draw_state.radius0_stack.auto_pop = false;
	} 
	return result;
}

function f32 
draw_set_next_radius0(f32 v) {
	draw_radius0_node_t* node = draw_state.radius0_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.radius0_stack.free);
	} else {
		node = (draw_radius0_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_radius0_node_t));
	} 
	f32 old_value = draw_state.radius0_stack.top->v;
	node->v = v; 
	stack_push(draw_state.radius0_stack.top, node);
	draw_state.radius0_stack.auto_pop = true; 
	return old_value;
};


// radius1

function f32 
draw_top_radius1() {
	return draw_state.radius1_stack.top->v;
}

function f32 
draw_push_radius1(f32 v) {
	draw_radius1_node_t* node = draw_state.radius1_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.radius1_stack.free);
	} else {
		node = (draw_radius1_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_radius1_node_t));
	} 
	f32 old_value = draw_state.radius1_stack.top->v; 
	node->v = v; stack_push(draw_state.radius1_stack.top, node);
	draw_state.radius1_stack.auto_pop = false; 
	return old_value;
}

function f32 
draw_pop_radius1() {
	draw_radius1_node_t* popped = draw_state.radius1_stack.top; 
    f32 result = 0.0f;
	if (popped != 0) {
        result = popped->v;
		stack_pop(draw_state.radius1_stack.top); 
		stack_push(draw_state.radius1_stack.free, popped);
		draw_state.radius1_stack.auto_pop = false;
	} 
	return result;
}

function f32 
draw_set_next_radius1(f32 v) {
	draw_radius1_node_t* node = draw_state.radius1_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.radius1_stack.free);
	} else {
		node = (draw_radius1_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_radius1_node_t));
	} 
	f32 old_value = draw_state.radius1_stack.top->v;
	node->v = v; 
	stack_push(draw_state.radius1_stack.top, node);
	draw_state.radius1_stack.auto_pop = true; 
	return old_value;
};


// radius2

function f32 
draw_top_radius2() {
	return draw_state.radius2_stack.top->v;
}

function f32 
draw_push_radius2(f32 v) {
	draw_radius2_node_t* node = draw_state.radius2_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.radius2_stack.free);
	} else {
		node = (draw_radius2_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_radius2_node_t));
	} 
	f32 old_value = draw_state.radius2_stack.top->v; 
	node->v = v; stack_push(draw_state.radius2_stack.top, node);
	draw_state.radius2_stack.auto_pop = false; 
	return old_value;
}

function f32 
draw_pop_radius2() {
	draw_radius2_node_t* popped = draw_state.radius2_stack.top; 
    f32 result = 0.0f;
	if (popped != 0) {
        result = popped->v;
		stack_pop(draw_state.radius2_stack.top); 
		stack_push(draw_state.radius2_stack.free, popped);
		draw_state.radius2_stack.auto_pop = false;
	} 
	return result;
}

function f32 
draw_set_next_radius2(f32 v) {
	draw_radius2_node_t* node = draw_state.radius2_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.radius2_stack.free);
	} else {
		node = (draw_radius2_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_radius2_node_t));
	} 
	f32 old_value = draw_state.radius2_stack.top->v;
	node->v = v; 
	stack_push(draw_state.radius2_stack.top, node);
	draw_state.radius2_stack.auto_pop = true; 
	return old_value;
};


// radius3

function f32 
draw_top_radius3() {
	return draw_state.radius3_stack.top->v;
}

function f32 
draw_push_radius3(f32 v) {
	draw_radius3_node_t* node = draw_state.radius3_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.radius3_stack.free);
	} else {
		node = (draw_radius3_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_radius3_node_t));
	} 
	f32 old_value = draw_state.radius3_stack.top->v; 
	node->v = v; stack_push(draw_state.radius3_stack.top, node);
	draw_state.radius3_stack.auto_pop = false; 
	return old_value;
}

function f32 
draw_pop_radius3() {
	draw_radius3_node_t* popped = draw_state.radius3_stack.top; 
    f32 result = 0.0f;
	if (popped != 0) {
        result = popped->v;
		stack_pop(draw_state.radius3_stack.top); 
		stack_push(draw_state.radius3_stack.free, popped);
		draw_state.radius3_stack.auto_pop = false;
	} 
	return result;
}

function f32 
draw_set_next_radius3(f32 v) {
	draw_radius3_node_t* node = draw_state.radius3_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.radius3_stack.free);
	} else {
		node = (draw_radius3_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_radius3_node_t));
	} 
	f32 old_value = draw_state.radius3_stack.top->v;
	node->v = v; 
	stack_push(draw_state.radius3_stack.top, node);
	draw_state.radius3_stack.auto_pop = true; 
	return old_value;
};





// thickness

function f32 
draw_top_thickness() {
	return draw_state.thickness_stack.top->v;
}

function f32 
draw_push_thickness(f32 v) {
	draw_thickness_node_t* node = draw_state.thickness_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.thickness_stack.free);
	} else {
		node = (draw_thickness_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_thickness_node_t));
	} 
	f32 old_value = draw_state.thickness_stack.top->v; 
	node->v = v; stack_push(draw_state.thickness_stack.top, node);
	draw_state.thickness_stack.auto_pop = false; 
	return old_value;
}

function f32 
draw_pop_thickness() {
	draw_thickness_node_t* popped = draw_state.thickness_stack.top; 
	f32 result = 0.0f;
	if (popped != 0) {
        result = popped->v;
		stack_pop(draw_state.thickness_stack.top); 
		stack_push(draw_state.thickness_stack.free, popped);
		draw_state.thickness_stack.auto_pop = false;
	} 
	return result;
}

function f32 
draw_set_next_thickness(f32 v) {
	draw_thickness_node_t* node = draw_state.thickness_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.thickness_stack.free);
	} else {
		node = (draw_thickness_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_thickness_node_t));
	} 
	f32 old_value = draw_state.thickness_stack.top->v;
	node->v = v; 
	stack_push(draw_state.thickness_stack.top, node);
	draw_state.thickness_stack.auto_pop = true; 
	return old_value;
};


// softness

function f32 
draw_top_softness() {
	return draw_state.softness_stack.top->v;
}

function f32 
draw_push_softness(f32 v) {
	draw_softness_node_t* node = draw_state.softness_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.softness_stack.free);
	} else {
		node = (draw_softness_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_softness_node_t));
	} 
	f32 old_value = draw_state.softness_stack.top->v; 
	node->v = v; stack_push(draw_state.softness_stack.top, node);
	draw_state.softness_stack.auto_pop = false; 
	return old_value;
}

function f32 
draw_pop_softness() {
	draw_softness_node_t* popped = draw_state.softness_stack.top; 
	f32 result = 0.0f;
	if (popped != 0) {
        result = popped->v;
		stack_pop(draw_state.softness_stack.top); 
		stack_push(draw_state.softness_stack.free, popped);
		draw_state.softness_stack.auto_pop = false;
	} 
	return result;
}

function f32 
draw_set_next_softness(f32 v) {
	draw_softness_node_t* node = draw_state.softness_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.softness_stack.free);
	} else {
		node = (draw_softness_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_softness_node_t));
	} 
	f32 old_value = draw_state.softness_stack.top->v;
	node->v = v; 
	stack_push(draw_state.softness_stack.top, node);
	draw_state.softness_stack.auto_pop = true; 
	return old_value;
};


// font

function font_handle_t 
draw_top_font() {
	return draw_state.font_stack.top->v;
}

function font_handle_t 
draw_push_font(font_handle_t v) {
	draw_font_node_t* node = draw_state.font_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.font_stack.free);
	} else {
		node = (draw_font_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_font_node_t));
	} 
	font_handle_t old_value = draw_state.font_stack.top->v; 
	node->v = v; stack_push(draw_state.font_stack.top, node);
	draw_state.font_stack.auto_pop = false; 
	return old_value;
}

function font_handle_t 
draw_pop_font() {
	draw_font_node_t* popped = draw_state.font_stack.top; 
    font_handle_t result = { 0 };
	if (popped != 0) {
        result = popped->v;
		stack_pop(draw_state.font_stack.top); 
		stack_push(draw_state.font_stack.free, popped);
		draw_state.font_stack.auto_pop = false;
	} 
	return result;
}

function font_handle_t 
draw_set_next_font(font_handle_t v) {
	draw_font_node_t* node = draw_state.font_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.font_stack.free);
	} else {
		node = (draw_font_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_font_node_t));
	} 
	font_handle_t old_value = draw_state.font_stack.top->v;
	node->v = v; 
	stack_push(draw_state.font_stack.top, node);
	draw_state.font_stack.auto_pop = true; 
	return old_value;
};


// font_size

function f32 
draw_top_font_size() {
	return draw_state.font_size_stack.top->v;
}

function f32 
draw_push_font_size(f32 v) {
	draw_font_size_node_t* node = draw_state.font_size_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.font_size_stack.free);
	} else {
		node = (draw_font_size_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_font_size_node_t));
	} 
	f32 old_value = draw_state.font_size_stack.top->v; 
	node->v = v; stack_push(draw_state.font_size_stack.top, node);
	draw_state.font_size_stack.auto_pop = false; 
	return old_value;
}

function f32 
draw_pop_font_size() {
	draw_font_size_node_t* popped = draw_state.font_size_stack.top; 
	f32 result = 0.0f;
	if (popped != 0) {
        result = popped->v;
		stack_pop(draw_state.font_size_stack.top); 
		stack_push(draw_state.font_size_stack.free, popped);
		draw_state.font_size_stack.auto_pop = false;
	} 
	return result;
}

function f32 
draw_set_next_font_size(f32 v) {
	draw_font_size_node_t* node = draw_state.font_size_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.font_size_stack.free);
	} else {
		node = (draw_font_size_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_font_size_node_t));
	} 
	f32 old_value = draw_state.font_size_stack.top->v;
	node->v = v; 
	stack_push(draw_state.font_size_stack.top, node);
	draw_state.font_size_stack.auto_pop = true; 
	return old_value;
};



// clip_mask

function rect_t 
draw_top_clip_mask() {
	return draw_state.clip_mask_stack.top->v;
}

function rect_t 
draw_push_clip_mask(rect_t v) {
	draw_clip_mask_node_t* node = draw_state.clip_mask_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.clip_mask_stack.free);
	} else {
		node = (draw_clip_mask_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_clip_mask_node_t));
	} 
	rect_t old_value = draw_state.clip_mask_stack.top->v; 
	node->v = v; stack_push(draw_state.clip_mask_stack.top, node);
	draw_state.clip_mask_stack.auto_pop = false; 
	return old_value;
}

function rect_t 
draw_pop_clip_mask() {
	draw_clip_mask_node_t* popped = draw_state.clip_mask_stack.top; 
    rect_t result = { 0 };
	if (popped != 0) {
        result = popped->v;
		stack_pop(draw_state.clip_mask_stack.top); 
		stack_push(draw_state.clip_mask_stack.free, popped);
		draw_state.clip_mask_stack.auto_pop = false;
	} 
	return result;
}

function rect_t 
draw_set_next_clip_mask(rect_t v) {
	draw_clip_mask_node_t* node = draw_state.clip_mask_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.clip_mask_stack.free);
	} else {
		node = (draw_clip_mask_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_clip_mask_node_t));
	} 
	rect_t old_value = draw_state.clip_mask_stack.top->v;
	node->v = v; 
	stack_push(draw_state.clip_mask_stack.top, node);
	draw_state.clip_mask_stack.auto_pop = true; 
	return old_value;
};


// texture

function gfx_handle_t 
draw_top_texture() {
	return draw_state.texture_stack.top->v;
}

function gfx_handle_t 
draw_push_texture(gfx_handle_t v) {
	draw_texture_node_t* node = draw_state.texture_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.texture_stack.free);
	} else {
		node = (draw_texture_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_texture_node_t));
	} 
	gfx_handle_t old_value = draw_state.texture_stack.top->v; 
	node->v = v; stack_push(draw_state.texture_stack.top, node);
	draw_state.texture_stack.auto_pop = false; 
	return old_value;
}

function gfx_handle_t 
draw_pop_texture() {
	draw_texture_node_t* popped = draw_state.texture_stack.top; 
    gfx_handle_t result = { 0 };
	if (popped != 0) {
        result = popped->v;
		stack_pop(draw_state.texture_stack.top); 
		stack_push(draw_state.texture_stack.free, popped);
		draw_state.texture_stack.auto_pop = false;
	} 
	return result;
}

function gfx_handle_t 
draw_set_next_texture(gfx_handle_t v) {
	draw_texture_node_t* node = draw_state.texture_stack.free; 
	if (node != 0) {
		stack_pop(draw_state.texture_stack.free);
	} else {
		node = (draw_texture_node_t*)arena_alloc(draw_state.batch_arena, sizeof(draw_texture_node_t));
	} 
	gfx_handle_t old_value = draw_state.texture_stack.top->v;
	node->v = v; 
	stack_push(draw_state.texture_stack.top, node);
	draw_state.texture_stack.auto_pop = true; 
	return old_value;
};

// group stacks

// colors
function void
draw_push_color(color_t color) {
	draw_push_color0(color);
	draw_push_color1(color);
	draw_push_color2(color);
	draw_push_color3(color);
}

function void
draw_set_next_color(color_t color) {
	draw_set_next_color0(color);
	draw_set_next_color1(color);
	draw_set_next_color2(color);
	draw_set_next_color3(color);
}

function void
draw_pop_color() {
	draw_pop_color0();
	draw_pop_color1();
	draw_pop_color2();
	draw_pop_color3();
}

// rounding
function vec4_t 
draw_top_rounding() {
	f32 x = draw_top_radius0();
	f32 y = draw_top_radius1();
	f32 z = draw_top_radius2();
	f32 w = draw_top_radius3();
	return vec4(x, y, z, w);
}

function void
draw_push_rounding(f32 radius) {
	draw_push_radius0(radius);
	draw_push_radius1(radius);
	draw_push_radius2(radius);
	draw_push_radius3(radius);
}

function void
draw_push_rounding(vec4_t radii) {
	draw_push_radius0(radii.x);
	draw_push_radius1(radii.y);
	draw_push_radius2(radii.z);
	draw_push_radius3(radii.w);
}

function void
draw_set_next_rounding(f32 radius) {
	draw_set_next_radius0(radius);
	draw_set_next_radius1(radius);
	draw_set_next_radius2(radius);
	draw_set_next_radius3(radius);
}

function void
draw_set_next_rounding(vec4_t radii) {
	draw_set_next_radius0(radii.x);
	draw_set_next_radius1(radii.y);
	draw_set_next_radius2(radii.z);
	draw_set_next_radius3(radii.w);
}

function void
draw_pop_rounding() {
	draw_pop_radius0();
	draw_pop_radius1();
	draw_pop_radius2();
	draw_pop_radius3();
}


#endif // SORA_DRAW_CPP