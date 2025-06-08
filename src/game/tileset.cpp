// tileset.cpp

#ifndef TILESET_CPP
#define TILESET_CPP

//~ implementation 


//- tileset functions 

function tileset_t* 
tileset_load(arena_t* arena, str_t filepath) {
    
    tileset_t* tileset = (tileset_t*)arena_alloc(arena, sizeof(tileset_t));
    
    tileset->arena = arena;
    tileset->debug_arena = arena_create(megabytes(64));
    _tileset_load_from_obj(tileset, filepath);
    
    return tileset;
}


//~ internal functions 

//- tileset functions 

function void
_tileset_load_from_obj(tileset_t* tileset, str_t filepath) {
    temp_t scratch = scratch_begin();
    
    gfx_handle_t* meshes = nullptr;
    u32 mesh_count = 0;
    
    _gfx_meshes_load_from_obj(scratch.arena, filepath, &meshes, &mesh_count);
    
    tileset->tiles = (tile_t*)arena_calloc(tileset->arena, sizeof(tile_t) * mesh_count);
    tileset->debug_tiles = (ts_debug_tile_t*)arena_calloc(tileset->debug_arena, sizeof(ts_debug_tile_t) * mesh_count);
    tileset->tile_count = mesh_count;
    
    for (i32 i = 0; i < mesh_count; i++) {
        
        // process tiles
        _tile_process(tileset->arena, tileset->debug_arena, meshes[i], &tileset->tiles[i], &tileset->debug_tiles[i]);
    }
    
    scratch_end(scratch);
}


function void
_tileset_write_to_binary(tileset_t* tileset, str_t filepath) {
    
    // TODO:
    
    
    
    
}



//- tile functions 


function void
_tile_process(arena_t* arena, arena_t* debug, gfx_handle_t mesh, tile_t* tile, ts_debug_tile_t* debug_tile) {
    
    const f32 quantize_amount = 0.025f;
    
    // get mesh data
    gfx_vertex_t* vertices = gfx_mesh_get_vertices(mesh);
    u32 vertex_count = gfx_mesh_get_vertex_count(mesh);
    
    i32* indices = gfx_mesh_get_indices(mesh);
    u32 index_count = gfx_mesh_get_index_count(mesh);
    
    // compute centroid
    vec3_t centroid = vec3(0.0f);
    for (u32 i = 0; i < vertex_count; i++) {
        gfx_vertex_t* vertex = &vertices[i];
        centroid = vec3_add(centroid, vertex->position);
    }
    centroid = vec3_div(centroid, vertex_count);
    debug_tile->centroid = centroid;
    
    // calculate bounds
    f32 max_height = f32_min;
    f32 min_height = f32_max;
    ts_index_dist_pair_t* distances = (ts_index_dist_pair_t*)arena_alloc(debug, sizeof(ts_index_dist_pair_t) * vertex_count);
    
    for (u32 i = 0; i < vertex_count; i++) {
        gfx_vertex_t* vertex = &vertices[i];
        
        // calculate height bounds
        if (vertex->position.y > max_height) { max_height = vertex->position.y; }
        if (vertex->position.y < min_height) { min_height = vertex->position.y; }
        
        // add dist to list
        distances[i].index = i;
        distances[i].distance = vec3_length(vec3_sub(vertex->position, centroid));
    }
    
    // sort distances
    qsort(distances, vertex_count, sizeof(ts_index_dist_pair_t), _ma_qsort_dist_compare);
    
    // get the 3 furthest vertices. these should be the 
    gfx_vertex_t* v0 = &vertices[distances[0].index];
    gfx_vertex_t* v1 = &vertices[distances[1].index];
    gfx_vertex_t* v2 = &vertices[distances[2].index];
    
    // create bounds points
    vec3_t b0 = vec3(v0->position.x, min_height, v0->position.z);
    vec3_t b1 = vec3(v1->position.x, min_height, v1->position.z);
    vec3_t b2 = vec3(v2->position.x, min_height, v2->position.z);
    
    vec3_t t0 = vec3(v0->position.x, max_height, v0->position.z);
    vec3_t t1 = vec3(v1->position.x, max_height, v1->position.z);
    vec3_t t2 = vec3(v2->position.x, max_height, v2->position.z);
    
    tile->triangle = { b0, b1, b2 };
    
    // calculate bary positions and store mesh
    tile->vertices = (tile_vertex_t*)arena_alloc(arena, sizeof(tile_vertex_t) * vertex_count);
    tile->vertex_count = vertex_count;
    
    for (i32 i = 0; i < vertex_count; i++) {
        gfx_vertex_t* vertex = &vertices[i];
        
        // TODO: not sure if we need to convert the normal and tangent to bary coords as well.
        vec3_t bary_pos = _tile_bary_from_cart(vertex->position, b0, b1, b2);
        tile->vertices[i] = { bary_pos, vertex->normal, vertex->tangent, vertex->texcoord };
    }
    
    // store indice
    tile->indices = (i32*)arena_alloc(arena, sizeof(i32) * index_count);
    memcpy(tile->indices, indices, sizeof(i32) * index_count);
    tile->index_count = index_count;
    
    // debug
    debug_tile->bound_bottom = { b0, b1, b2 };
    debug_tile->bound_top = { t0, t1, t2 };
    
    // calculate lateral face planes
    ts_plane_t planes[3] = {
        _ma_plane_from_quad(b0, b1, t0, t1),
        _ma_plane_from_quad(b1, b2, t1, t2),
        _ma_plane_from_quad(b2, b0, t2, t0),
    };
    
    for (i32 i = 0; i < 3; i++) {
        ts_plane_debug_info_t* debug_info = &debug_tile->plane_debug_info[i];
        
        // get plane
        ts_plane_t plane = planes[i];
        
        // debug: compute plane center
        vec3_t v1 = vec3_sub(plane.b, plane.a);
        vec3_t v2 = vec3_sub(plane.c, plane.a);
        vec3_t center = vec3_add(plane.a, vec3_mul(vec3_add(v1, v2), 0.5f));
        debug_info->plane = plane;
        debug_info->plane_center = center;
        
        // find indices of the vertices near the plane 
        // TODO: right now this is capped at 32 vertices, but maybe we might need more.
        i32* plane_vertex_indices = (i32*)arena_alloc(debug, sizeof(i32) * 32);
        u32 plane_vertex_count = 0;
        _ma_find_vertices_on_plane(vertices, vertex_count, plane, plane_vertex_indices, &plane_vertex_count, 32);
        
        // debug: store indices
        memcpy(debug_info->plane_vertex_indices, plane_vertex_indices, sizeof(i32) * plane_vertex_count);
        debug_info->plane_vertex_count = plane_vertex_count;
        
        vec2_t* plane_vertices = (vec2_t*)arena_alloc(debug, sizeof(vec2_t) * plane_vertex_count);
        for (i32 j = 0; j < plane_vertex_count; j++) {
            vec3_t position = vertices[plane_vertex_indices[j]].position;
            vec2_t projected_position = _ma_plane_project(plane, position);
            vec2_t quantized_position = _ma_vec2_quantize(projected_position, quantize_amount);
            plane_vertices[j] = quantized_position;
        }
        
        // mirror the vertices
        vec2_t* mirrored_plane_vertices = (vec2_t*)arena_alloc(debug, sizeof(vec2_t) * plane_vertex_count);
        memcpy(mirrored_plane_vertices, plane_vertices, sizeof(vec2_t) * plane_vertex_count);
        _ma_vec2_mirror(mirrored_plane_vertices, plane_vertex_count);
        
        // create single list
        vec2_t* total_plane_vertices = (vec2_t*)arena_alloc(debug, sizeof(vec2_t) * plane_vertex_count * 2);
        memcpy(total_plane_vertices, plane_vertices, sizeof(vec2_t) * plane_vertex_count);
        memcpy(total_plane_vertices + plane_vertex_count, mirrored_plane_vertices, sizeof(vec2_t) * plane_vertex_count);
        
        // sort the lateral face positions
        qsort(total_plane_vertices, plane_vertex_count * 2, sizeof(vec2_t), _ma_qsort_vec2_compare);
        
        // debug store distances
        debug_tile->distances[i] = total_plane_vertices;
        debug_tile->distance_count[i] = plane_vertex_count * 2;
        
        // hash the positions
        u64 hash = 14695981039346656037ULL;
        for (u32 j = 0; j < plane_vertex_count * 2; j++) {
            hash ^= _ma_vec2_hash(total_plane_vertices[j]);
            hash *= 1099511628211ULL;
        }
        tile->edge_hashes[i] = hash;
        
    }
    
}

function vec3_t 
_tile_bary_from_cart(vec3_t p, vec3_t a, vec3_t b, vec3_t c) {
    
    vec3_t v0 = vec3_sub(b, a);
    vec3_t v1 = vec3_sub(c, a);
    vec3_t v2 = vec3_sub(p, a);
    
    f32 d00 = vec3_dot(v0, v0);
    f32 d01 = vec3_dot(v0, v1);
    f32 d11 = vec3_dot(v1, v1);
    f32 d20 = vec3_dot(v2, v0);
    f32 d21 = vec3_dot(v2, v1);
    
    f32 denom = d00 * d11 - d01 * d01;
    f32 v = (d11 * d20 - d01 * d21) / denom;
    f32 w = (d00 * d21 - d01 * d20) / denom;
    f32 u = 1.0f - v - w;
    
    return { u, v, w };
}

function vec3_t 
_tile_cart_from_bary(vec3_t p, vec3_t a, vec3_t b, vec3_t c) {
    
    f32 u = p.x;
    f32 v = p.y;
    f32 w = p.z;
    
    vec3_t scaled_a = vec3_mul(a, u);
    vec3_t scaled_b = vec3_mul(b, v);
    vec3_t scaled_c = vec3_mul(c, w);
    
    vec3_t result = vec3_add(scaled_a, vec3_add(scaled_b, scaled_c));
    
    return result;
}

//- mesh analyze functions 

function i32 
_ma_qsort_dist_compare(const void* a, const void* b) {
    ts_index_dist_pair_t *pair_a = (ts_index_dist_pair_t*)a;
    ts_index_dist_pair_t *pair_b = (ts_index_dist_pair_t*)b;
    
    i32 result = 0;
    
    if (pair_a->distance > pair_b->distance) {
        result = -1;
    } else if (pair_a->distance < pair_b->distance) {
        result = 1;
    }
    
    return result;
}

function i32
_ma_qsort_f32_compare(const void* a, const void* b) {
    f32* va = (f32*)a;
    f32* vb = (f32*)b;
    
    i32 result = 0;
    if (va != vb) {
        result = (va < vb) ? -1 : 1;
    }
    
    return result;
}

function i32 
_ma_qsort_vec2_compare(const void* a, const void* b) {
    vec2_t* va = (vec2_t*)a;
    vec2_t* vb = (vec2_t*)b;
    
    i32 result = 0;
    
    if (va->x != vb->x) {
        result = (va->x < vb->x) ? -1 : 1;
    } else if (va->y != vb->y) {
        result = (va->y < vb->y) ? -1 : 1;
    }
    
    return result;
}


function i32 
_ma_qsort_vec3_compare(const void* a, const void* b) {
    vec3_t* va = (vec3_t*)a;
    vec3_t* vb = (vec3_t*)b;
    
    i32 result = 0;
    
    if (va->x != vb->x) {
        result = (va->x < vb->x) ? -1 : 1;
    } else if (va->y != vb->y) {
        result = (va->y < vb->y) ? -1 : 1;
    } else if (va->z != vb->z) {
        result = (va->z < vb->z) ? -1 : 1;
    }
    
    return result;
}

inlnfunc f32 
_ma_quantize(f32 value, f32 epsilon) {
    return floorf(value / epsilon + 0.5f) * epsilon;
}

function vec2_t
_ma_vec2_quantize(vec2_t value, f32 epsilon) {
    vec2_t q;
    q.x = _ma_quantize(value.x, epsilon);
    q.y = _ma_quantize(value.y, epsilon);
    return q;
}

function vec3_t
_ma_vec3_quantize(vec3_t value, f32 epsilon) {
    vec3_t q;
    q.x = _ma_quantize(value.x, epsilon);
    q.y = _ma_quantize(value.y, epsilon);
    q.z = _ma_quantize(value.z, epsilon);
    return q;
}

function u64
_ma_f32_hash(f32 point) {
    u64 hash = 14695981039346656037ULL;
    hash ^= (u32)point;
    hash *= 1099511628211ULL;
    return hash;
}

function u64 
_ma_vec2_hash(vec2_t point) {
    u64 hash = 14695981039346656037ULL;
    u32* parts = (u32*)&point;
    for (int i = 0; i < 2; i++) {
        hash ^= parts[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

function u64 
_ma_vec3_hash(vec3_t point) {
    u64 hash = 14695981039346656037ULL;
    u32* parts = (u32*)&point;
    for (int i = 0; i < 3; i++) {
        hash ^= parts[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}


function void
_ma_vec2_mirror(vec2_t* points, i32 count) {
    f32 min_x = f32_max;
    f32 max_x = f32_min;
    
    for (i32 i = 0; i < count; ++i) {
        if (points[i].x < min_x) min_x = points[i].x;
        if (points[i].x > max_x) max_x = points[i].x;
    }
    
    f32 center_x = (min_x + max_x) / 2.0f;
    
    for (i32 i = 0; i < count; ++i) {
        points[i].x = 2.0f * center_x - points[i].x;
    }
}

function ts_plane_t 
_ma_plane_from_quad(vec3_t a, vec3_t b, vec3_t c, vec3_t d) {
    ts_plane_t result = { 0 };
    
    // calculate plane
    result.origin = a;
    result.u = vec3_normalize(vec3_sub(b, a));
    result.normal = vec3_normalize(vec3_cross(result.u,  vec3_sub(c, a)));
    result.v = vec3_cross(result.normal, result.u);
    
    // save original points
    result.a = a;
    result.b = b;
    result.c = c;
    result.d = d;
    
    return result;
}

function vec2_t 
_ma_plane_project(ts_plane_t plane, vec3_t point) {
    vec3_t local = vec3_sub(point, plane.origin);
    vec2_t result;
    result.x = vec3_dot(local, plane.u);
    result.y = vec3_dot(local, plane.v);
    return result;
}

function void 
_ma_find_vertices_on_plane(gfx_vertex_t* vertices, u32 vertex_count, ts_plane_t plane, i32* out_indices, u32* out_count, u32 max_count) {
    
    u32 count = 0;
    f32 epsilon = 0.02f;
    
    for (u32 i = 0; i < vertex_count; i++) {
        vec3_t v = vertices[i].position;
        vec3_t to_point = vec3_sub(v, plane.origin);
        f32 dist = vec3_dot(to_point, plane.normal);
        
        if (fabsf(dist) < epsilon) {
            if (count < max_count) {
                out_indices[count++] = i;
            }
        }
    }
    
    *out_count = count;
}



#endif // TILESET_CPP