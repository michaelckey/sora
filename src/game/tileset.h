// tileset.h

#ifndef TILESET_H
#define TILESET_H

//- defines 

//- structs 

// mesh importer structs

struct ts_tri_t {
    vec3_t p0;
    vec3_t p1;
    vec3_t p2;
};

struct ts_index_dist_pair_t {
    i32 index;
    f32 distance;
};

struct ts_plane_t {
    vec3_t origin;
    vec3_t u;
    vec3_t v;
    vec3_t normal;
    
    // points
    vec3_t a;
    vec3_t b;
    vec3_t c;
    vec3_t d;
};

// debug structs

struct ts_plane_debug_info_t {
    ts_plane_t plane;
    vec3_t plane_center;
    
    i32 plane_vertex_indices[32];
    u32 plane_vertex_count;
    
};

struct ts_debug_tile_t {
    ts_tri_t bound_top;
    ts_tri_t bound_bottom;
    vec3_t centroid;
    
    ts_plane_debug_info_t plane_debug_info[3];
    vec2_t* distances[3];
    u32 distance_count[3];
};

struct tri_t {
    vec3_t p0;
    vec3_t p1;
    vec3_t p2;
};

struct tile_vertex_t {
    vec3_t position;
    vec3_t normal;
    vec3_t tangent;
    vec2_t texcoord;
};

struct tile_t {
    
    tri_t triangle;
    u64 edge_hashes[3];
    
    // mesh data
    tile_vertex_t* vertices;
    u32 vertex_count;
    
    i32* indices;
    u32 index_count;
};

struct tileset_t {
    
    arena_t* arena;
    
    tile_t* tiles;
    u32 tile_count;
    
    arena_t* debug_arena;
    ts_debug_tile_t* debug_tiles;
};

//- functions 

// tileset
function tileset_t* tileset_load(arena_t* arena, str_t filepath);

//- internal functions 

// tileset
function void _tileset_load_from_obj(tileset_t* tileset, str_t filepath);
function void _tileset_load_from_binary(tileset_t* tileset, str_t filepath);
function void _tileset_write_to_binary(tileset_t* tileset, str_t filepath);

// tile
function void _tile_process(arena_t* arena, arena_t* debug, gfx_handle_t mesh, tile_t* tile, ts_debug_tile_t* debug_tile);
function vec3_t _tile_bary_from_cart(vec3_t p, vec3_t a, vec3_t b, vec3_t c);
function vec3_t _tile_cart_from_bary(vec3_t p, vec3_t a, vec3_t b, vec3_t c);

// mesh analyzer
function i32 _ma_qsort_dist_compare(const void* a, const void* b);
function i32 _ma_qsort_f32_compare(const void* a, const void* b);
function i32 _ma_qsort_vec2_compare(const void* a, const void* b);
function i32 _ma_qsort_vec3_compare(const void* a, const void* b);
inlnfunc f32 _ma_quantize(f32 value, f32 epsilon);
function vec2_t _ma_vec2_quantize(vec2_t point, f32 epsilon);
function vec3_t _ma_vec3_quantize(vec3_t point, f32 epsilon);
function u64  _ma_f32_hash(f32 point);
function u64  _ma_vec2_hash(vec2_t point);
function u64  _ma_vec3_hash(vec3_t point);
function void _ma_vec2_mirror(vec2_t* points, i32 count);
function ts_plane_t _ma_plane_from_quad(vec3_t a, vec3_t b, vec3_t c, vec3_t d);
function vec2_t _ma_plane_project(ts_plane_t plane, vec3_t point);
function void _ma_find_vertices_on_plane(gfx_vertex_t* vertices, u32 vertex_count, ts_plane_t plane, i32* out_indices, u32* out_index_count, u32 max_count);

#endif // TILESET_H