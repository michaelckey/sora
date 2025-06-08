// planet.h

#ifndef PLANET_H
#define PLANET_H

//- structs 

struct p_tri_t {
    i32 vertices[3];
};


struct planet_t {
    arena_t* arena;
    
    u32 point_count;
    vec3_t* points;
    
    p_tri_t* triangles;
    u32 triangle_count;
    
    // mesh
    gfx_vertex_t* vertices;
    u32 vertex_count;
    
    i32* indices;
    u32 index_count;
    
    
};

//- functions 

// planet
function planet_t* planet_create(str_t filepath);
function void planet_release(planet_t* planet);
function void planet_generate(planet_t* planet, tileset_t* tileset);

// internal


#endif // PLANET_H