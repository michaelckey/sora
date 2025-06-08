// planet.cpp

#ifndef PLANET_CPP
#define PLANET_CPP

//~ implementation

//- planet functions 

function planet_t* 
planet_create(str_t filepath) {
    
    arena_t* arena = arena_create(megabytes(64));
    planet_t* planet = (planet_t*)arena_alloc(arena, sizeof(planet_t));
    
    planet->arena = arena;
    
    temp_t scratch = scratch_begin();
    
    gfx_handle_t mesh = gfx_mesh_load(scratch.arena, filepath);
    
    planet->point_count = gfx_mesh_get_vertex_count(mesh);
    planet->points = (vec3_t*)arena_alloc(arena, sizeof(vec3_t) * planet->point_count);
    
    planet->triangle_count = gfx_mesh_get_index_count(mesh) / 3;
    planet->triangles = (p_tri_t*)arena_alloc(arena, sizeof(p_tri_t) * planet->triangle_count);
    
    // copy data
    gfx_vertex_t* vertices = gfx_mesh_get_vertices(mesh);
    for (i32 i = 0; i < planet->point_count; i++) {
        planet->points[i] = vertices[i].position;
    }
    
    i32* indices = gfx_mesh_get_indices(mesh);
    i32 j = 0;
    for (i32 i = 0; i < planet->triangle_count; i++) {
        planet->triangles[i] = { indices[j + 0], indices[j + 1], indices[j + 2] };
        j += 3;
    }
    
    scratch_end(scratch);
    
    return planet;
}

function void
planet_release(planet_t* planet) {
    arena_release(planet->arena);
}

function void
planet_generate(planet_t* planet, tileset_t* tileset) {
    
    
    
    
    
}

//- internal functions



// NOTE: the code below is not used.
// not gonna do this right now, maybe in the future.
//
// here are the struct used:
/*

struct tri_t {
    i32 vertices[3];
};

struct edge_t {
    i32 a, b, c;
};

struct point_index_pair_t {
    vec2_t point;
    i32 index;
};

struct planet_t {
    arena_t* arena;
    
    u32 point_count;
    vec3_t* points;
    vec3_t* projected_points;
    
    tri_t* triangles;
    u32 triangle_count;
    
};

*/


//function void 
//_planet_generate_points_0(planet_t* planet) {
//
//f32 s = 3.6f / sqrt(planet->point_count);
//f32 dz = 2.0f / planet->point_count;
//f32 long_angle = 0.0f;
//f32 z = 1.0f - dz / 2.0f;
//
//for (i32 i = 0; i < planet->point_count; i++) {
//f32 r = sqrt(1.0f - z*z);
//planet->points[i].x = cosf(long_angle) * r;
//planet->points[i].y = sinf(long_angle) * r;
//planet->points[i].z = z;
//
//z = z - dz;
//long_angle = long_angle + s/r;
//}
//
//}
//
//function void 
//_planet_generate_points_1(planet_t* planet) {
//
//f32 dlong = f32_pi * (3.0f - sqrt(5.0f));
//f32 dz = 2.0f / planet->point_count;
//f32 long_angle = 0.0f;
//f32 z = 1.0f - dz / 2.0f;
//
//for (i32 i = 0; i < planet->point_count; i++) {
//f32 r = sqrt(1.0f - z*z);
//planet->points[i].x = cosf(long_angle) * r;
//planet->points[i].y = sinf(long_angle) * r;
//planet->points[i].z = z;
//
//z = z - dz;
//long_angle = long_angle + dlong;
//}
//
//}
//
//function void
//_planet_sterographic_project(planet_t* planet) {
//
//for (i32 i = 0; i < planet->point_count; i++) {
//vec3_t* point = &planet->points[i];
//if (fabsf(point->z - 1.0f) > 1e-10) {
//f32 factor = 1.0f / (1.0f - point->z);
//planet->projected_points[i].x = point->x * factor;
//planet->projected_points[i].y = point->y * factor;
//planet->projected_points[i].z = 0.0f;
//}
//
//}
//
//}
//
//function b8 
//_planet_point_in_circumcircle(vec2_t p, vec2_t p0, vec2_t p1, vec2_t p2) {
//
//f32 a = p0.x - p.x;
//f32 b = p0.y - p.y;
//f32 c = (p0.x*p0.x - p.x*p.x) + (p0.y*p0.y - p.y*p.y);
//
//f32 d = p1.x - p.x;
//f32 e = p1.y - p.y;
//f32 f = (p1.x*p1.x - p.x*p.x) + (p1.y*p1.y - p.y*p.y);
//
//f32 g = p2.x - p.x;
//f32 h = p2.y - p.y;
//f32 i = (p2.x*p2.x - p.x*p.x) + (p2.y*p2.y - p.y*p.y);
//
//f32 determinant = a*(e*i - f*h) - b*(d*i - f*g) + c*(d*h - e*g);
//
//return determinant < 0;
//}
//
//function void 
//_planet_add_edge(edge_t* edges, u32* edge_count, i32 a, i32 b) {
//
//for (u32 i = 0; i < *edge_count; i++) {
//edge_t e = edges[i];
//if ((e.a == b && e.b == a) || (e.a == a && e.b == b)) {
//edges[i] = edges[--(*edge_count)];
//return;
//}
//}
//
//edges[(*edge_count)++] = { a, b };
//
//}
//
//function i32
//_planet_qsort_vec2_compare(const void* a, const void* b) {
//
//point_index_pair_t* pa = (point_index_pair_t*)a;
//point_index_pair_t* pb = (point_index_pair_t*)b;
//
//vec2_t* va = &pa->point;
//vec2_t* vb = &pb->point;
//
//i32 result = 0;
//
//if (va->x != vb->x) {
//result = (va->x < vb->x) ? -1 : 1;
//} else if (va->y != vb->y) {
//result = (va->y < vb->y) ? -1 : 1;
//}
//
//return result;
//}
//
//function void
//_planet_convex_hull(arena_t* arena, vec2_t* points, i32 point_count, i32* out, i32* out_count) {
//
//point_index_pair_t* idx = (point_index_pair_t*)arena_alloc(arena, sizeof(point_index_pair_t) * point_count);
//
//for (i32 i = 0; i < point_count; i++) {
//idx[i].point = points[i];
//idx[i].index = i;
//}
//
// sort
//qsort(idx, point_count, sizeof(point_index_pair_t), _planet_qsort_vec2_compare);
//
//i32 k = 0;
//
//for (i32 i = 0; i < point_count; i++) {
//while (k >= 2) {
//vec2_t a = points[out[k - 2]];
//vec2_t b = points[out[k - 1]];
//vec2_t c = points[idx[i].index];
//f32 cross = (b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x);
//if (cross > 0) break;
//k--;
//}
//out[k++] = idx[i].index;
//}
//
//i32 t = k + 1;
//for (i32 i = point_count - 2; i >= 0; i--) {
//while (k >= t) {
//vec2_t a = points[out[k - 2]];
//vec2_t b = points[out[k - 1]];
//vec2_t c = points[idx[i].index];
//float cross = (b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x);
//if (cross > 0) break;
//k--;
//}
//out[k++] = idx[i].index;
//}
//
//*out_count = k - 1;
//}
//
//function void
//_planet_delaunay_triangulate(planet_t* planet) {
//temp_t scratch = scratch_begin();
//
//tri_t* tris = (tri_t*)arena_alloc(scratch.arena, sizeof(tri_t) * 1024);
//u32 tri_count = 0;
//
// create super triangle
//f32 min_x = planet->projected_points[0].x;
//f32 max_x = planet->projected_points[0].x;
//f32 min_y = planet->projected_points[0].y;
//f32 max_y = planet->projected_points[0].y;
//for (i32 i = 1; i < planet->point_count; i++) {
//if (planet->projected_points[i].x < min_x) min_x = planet->projected_points[i].x;
//if (planet->projected_points[i].x > max_x) max_x = planet->projected_points[i].x;
//if (planet->projected_points[i].y < min_y) min_y = planet->projected_points[i].y;
//if (planet->projected_points[i].y > max_y) max_y = planet->projected_points[i].y;
//}
//
//f32 dx = max_x - min_x, dy = max_y - min_y;
//f32 delta_max = dx > dy ? dx : dy;
//vec2_t mid = { (min_x + max_x) * 0.5f, (min_y + max_y) * 0.5f };
//
//vec2_t p0 = { mid.x - 20 * delta_max, mid.y - delta_max };
//vec2_t p1 = { mid.x, mid.y + 20 * delta_max };
//vec2_t p2 = { mid.x + 20 * delta_max, mid.y - delta_max };
//
//i32 i0 = planet->point_count;
//i32 i1 = i0 + 1;
//i32 i2 = i0 + 2;
//
//vec2_t* points = (vec2_t*)arena_alloc(scratch.arena, sizeof(vec2_t) * (planet->point_count + 3));
//
// TODO: copy points better
//for (i32 i = 0; i < planet->point_count; i++) {
//points[i] = vec2(planet->projected_points[i].x, planet->projected_points[i].y);
//}
//
//points[i0] = p0;
//points[i1] = p1;
//points[i2] = p2;
//
//tris[tri_count++] = { { i0, i1, i2 } };
//
//for (i32 i = 0; i < planet->point_count; i++) {
//vec2_t p = points[i];
//tri_t* bad_tris = (tri_t*)arena_alloc(scratch.arena, sizeof(tri_t) * tri_count);
//u32 bad_count = 0;
//
//for (u32 j = 0; j < tri_count; j++) {
//tri_t t = tris[j];
//if (_planet_point_in_circumcircle(p, points[t.vertices[0]], points[t.vertices[1]], points[t.vertices[2]])) {
//bad_tris[bad_count++] = t;
//tris[j--] = tris[--tri_count];
//}
//}
//
//edge_t* edges = (edge_t*)arena_alloc(scratch.arena, sizeof(edge_t) * bad_count * 3);
//u32 edge_count = 0;
//for (u32 j = 0; j < bad_count; j++) {
//tri_t t = bad_tris[j];
//_planet_add_edge(edges, &edge_count, t.vertices[0], t.vertices[1]);
//_planet_add_edge(edges, &edge_count, t.vertices[1], t.vertices[2]);
//_planet_add_edge(edges, &edge_count, t.vertices[2], t.vertices[0]);
//}
//
//for (u32 j = 0; j < edge_count; j++) {
//edge_t e = edges[j];
//tris[tri_count++] = { { e.a, e.b, i } };
//}
//}
//
// remove super triangle
//for (u32 i = 0; i < tri_count;) {
//tri_t t = tris[i];
//if (t.vertices[0] >= planet->point_count || t.vertices[1] >= planet->point_count || t.vertices[2] >= planet->point_count) {
//tris[i] = tris[--tri_count];
//} else {
//i++;
//}
//}
//
// allocate final tri
//planet->triangles = (tri_t*)arena_alloc(planet->arena, sizeof(tri_t) * tri_count);
//planet->triangle_count = tri_count;
//
// copy data
//memcpy(planet->triangles, tris, sizeof(tri_t) * tri_count);
//
//scratch_end(scratch);
//}

#endif // PLANET_CPP