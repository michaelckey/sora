// shader_compute_fluid.hlsl

// structs

struct spatial_index_t {
	uint index;
	uint hash;
	uint key;
};

// settings

cbuffer sim_constants : register(b1) {
	uint particle_count;
	float smoothing_radius;
	float delta_time;
	float target_density;
	float pressure_multiplier;
	float near_pressure_multiplier;
};

static const int3 cell_offsets[27] = {
	int3(-1, -1, -1),
	int3(-1, -1, 0),
	int3(-1, -1, 1),
	int3(-1, 0, -1),
	int3(-1, 0, 0),
	int3(-1, 0, 1),
	int3(-1, 1, -1),
	int3(-1, 1, 0),
	int3(-1, 1, 1),
	int3(0, -1, -1),
	int3(0, -1, 0),
	int3(0, -1, 1),
	int3(0, 0, -1),
	int3(0, 0, 0),
	int3(0, 0, 1),
	int3(0, 1, -1),
	int3(0, 1, 0),
	int3(0, 1, 1),
	int3(1, -1, -1),
	int3(1, -1, 0),
	int3(1, -1, 1),
	int3(1, 0, -1),
	int3(1, 0, 0),
	int3(1, 0, 1),
	int3(1, 1, -1),
	int3(1, 1, 0),
	int3(1, 1, 1)
};


// buffers
RWStructuredBuffer<float3> positions_buffer : register(u0);
RWStructuredBuffer<float3> velocities_buffer : register(u1);
RWStructuredBuffer<float3> predicted_positions_buffer : register(u2);
RWStructuredBuffer<float2> densities_buffer : register(u3);
RWStructuredBuffer<uint3> spatial_indices_buffer : register(u4);
RWStructuredBuffer<uint> spatial_offsets_buffer : register(u5);
RWStructuredBuffer<uint> colors_buffer : register(u6);
RWStructuredBuffer<int3> cell_buffer : register(u7);

cbuffer sort_constants : register(b0) {
	uint buffer_count;
	uint group_width;
	uint group_height;
	uint step_index;
};

// constants

static const float PI = 3.1415926;

// helpers functions 

uint hash(uint x) {
    x ^= x >> 17;
    x *= 0xed5ad4bbU;
    x ^= x >> 11;
    x *= 0xac4c1b51U;
    x ^= x >> 15;
    x *= 0x31848babU;
    x ^= x >> 14;
    return x;
}

float random(uint seed) {
    return (hash(seed) & 0xFFFFFF) / 16777215.0;
}

// color functions 

float4 color_ramp_sample(float t) {
	t = saturate(t);
    
    const float4 colors[4] = {
        float4(0.22f, 0.18f, 0.81f, 1.0f),  // t = 0.0
        float4(0.58f, 1.0f, 0.8f, 1.0f),    // t = 0.6
        float4(1.0f, 0.77f, 0.12f, 1.0f),   // t = 0.75
        float4(1.0f, 0.38f, 0.07f, 1.0f)    // t = 1.0
    };
    
    if (t <= 0.6f) {
        float local_t = t / 0.6f;
        return lerp(colors[0], colors[1], local_t);
    } else if (t <= 0.75f) {
        float local_t = (t - 0.6f) / 0.15f;
        return lerp(colors[1], colors[2], local_t);
    } else {
        float local_t = (t - 0.75f) / 0.25f;
        return lerp(colors[2], colors[3], local_t);
    }

}

uint pack_color(float4 color) {
	uint r = (uint)(saturate(color.r) * 255.0f + 0.5f);
    uint g = (uint)(saturate(color.g) * 255.0f + 0.5f);
    uint b = (uint)(saturate(color.b) * 255.0f + 0.5f);
    uint a = (uint)(saturate(color.a) * 255.0f + 0.5f);
    return r | (g << 8) | (b << 16) | (a << 24);
}


// spatial hashing functions 

int3 cell_from_position(float3 position, float radius) {
	return (int3)floor(position / radius);
}

uint hash_from_cell(int3 cell) {
	//cell = (uint3)cell;
	return (cell.x * 15823) + (cell.y * 9737333) + (cell.z * 440817757);
}

uint key_from_hash(uint hash, uint table_size) {
	return hash % table_size;
}

// density functions

float density_kernel(float dist, float radius) {
	float result = 0.0f;
	if (dist < radius) {
		float scale = 15 / (2 * PI * pow(radius, 5));
		float v = radius - dist;
		result =  v * v * scale;
	}
	return result;
}

float near_density_kernel(float dist, float radius) {
	float result = 0.0f;
	if (dist < radius) {
		float scale = 15 / (PI * pow(radius, 6));
		float v = radius - dist;
		result = v * v * v * scale;
	}
	return result;
}

float density_derivative(float dist, float radius) {
	float result = 0.0f;
	if (dist <= radius) {
		float scale = 15 / (pow(radius, 5) * PI);
		float v = radius - dist;
		result =  -v * scale;
	}
	return result;
}

float near_density_derivative(float dist, float radius) {
	float result = 0.0f;
	if (dist <= radius) {
		float scale = 45 / (pow(radius, 6) * PI);
		float v = radius - dist;
		result = -v * v * scale;
	}
	return result;
}

float pressure_from_density(float density) {
	return (density - target_density) * pressure_multiplier;
}

float pressure_from_near_density(float near_density) {
	return near_density * near_pressure_multiplier;
}

[numthreads(256, 1, 1)]
void intial_setup_pass(uint3 id : SV_DispatchThreadID) {
	if (id.x >= particle_count) return;

    float3 pos;
    pos.x = (random(id.x * 3 + 0) - 0.5f) * 5.0f;
    pos.y = (random(id.x * 3 + 1) - 0.5f) * 5.0f;
    pos.z = (random(id.x * 3 + 2) - 0.5f) * 5.0f;
	
	positions_buffer[id.x] = pos;
	velocities_buffer[id.x] = float3(0.0f, 0.0f, 0.0f);
	colors_buffer[id.x] = pack_color(float4(0.4f, 0.7f, 1.0f, 1.0f));
}

[numthreads(256, 1, 1)]
void external_forces_pass(uint3 id : SV_DispatchThreadID) {
	if (id.x >= particle_count) return;
	
	// add gravity
	velocities_buffer[id.x] += float3(0.0f, -10.0f * delta_time, 0.0f);
	
	// predict the next position
	predicted_positions_buffer[id.x] = positions_buffer[id.x] + (velocities_buffer[id.x] * (1.0f / 120.0f));

	// update the color
	float speed = length(velocities_buffer[id.x]);
	colors_buffer[id.x] = pack_color(color_ramp_sample(speed / 15.0f));
	
}	


[numthreads(256, 1, 1)]
void spatial_grid_pass(uint3 id : SV_DispatchThreadID) {
	if (id.x >= particle_count) return;
	
	spatial_offsets_buffer[id.x] = particle_count;

	uint index = id.x;
	int3 cell = cell_from_position(predicted_positions_buffer[id.x], smoothing_radius);
	uint hash = hash_from_cell(cell);
	uint key = key_from_hash(hash, particle_count);
	spatial_indices_buffer[id.x] = uint3(index, hash, key);
	
}

[numthreads(256, 1, 1)]
void sort_pass(uint3 id : SV_DispatchThreadID) {

	uint i = id.x;

	uint h_index = i & (group_width - 1);
	uint index_left = h_index + (group_height + 1) * (i / group_width);
	uint right_step_size = step_index == 0 ? group_height - 2 * h_index : (group_height + 1) / 2;
	uint index_right = index_left + right_step_size;

	if (index_right >= buffer_count) return;

	uint value_left = spatial_indices_buffer[index_left][2];
	uint value_right = spatial_indices_buffer[index_right][2];

	if (value_left > value_right) {
		uint3 temp = spatial_indices_buffer[index_left];
		spatial_indices_buffer[index_left] = spatial_indices_buffer[index_right];
		spatial_indices_buffer[index_right] = temp;
	}

}


[numthreads(256, 1, 1)]
void calculate_offsets_pass(uint3 id : SV_DispatchThreadID) {

	if (id.x >= buffer_count) { return; }

	uint i = id.x;
	uint null = buffer_count;

	uint key = spatial_indices_buffer[i][2];
	uint key_prev = i == 0 ? null : spatial_indices_buffer[i - 1][2];

	if (key != key_prev) {
		spatial_offsets_buffer[key] = i;
	}

}

[numthreads(256, 1, 1)]
void density_pass(uint3 id : SV_DispatchThreadID) {
	if (id.x >= particle_count) return;
	
	float3 pos = predicted_positions_buffer[id.x];
	int3 cell = cell_from_position(pos, smoothing_radius);
	float sqr_radius = smoothing_radius * smoothing_radius;
	float density = 0.0f;
	float near_density = 0.0f;	
	int count = 0;
	
	for (int i = 0; i < 27; i++) {
		uint hash = hash_from_cell(cell + cell_offsets[i]);
		uint key = key_from_hash(hash, particle_count);
		uint current_index = spatial_offsets_buffer[key];

		while (current_index < particle_count) {
			uint3 index_data = spatial_indices_buffer[current_index];
			current_index++;
			
			if (index_data[2] != key) {
				break;
			}
			if (index_data[1] != hash) {
				continue;
			}
		
			uint neighbor_index = index_data[0];
			float3 neighbor_pos = predicted_positions_buffer[neighbor_index];
			float3 offset_to_neighbor = neighbor_pos - pos;
			float sqr_dist_to_neighbor = dot(offset_to_neighbor, offset_to_neighbor);
			
			if (sqr_dist_to_neighbor > sqr_radius) {
				continue;
			}
			count++;
			float dist = sqrt(sqr_dist_to_neighbor);
			density += density_kernel(dist, smoothing_radius);
			near_density += near_density_kernel(dist, smoothing_radius);
		}
	}

	densities_buffer[id.x] = float2(density, near_density);
	cell_buffer[id.x] = int3(count, 0, 0);
	if (density == 0.0f) {
		cell_buffer[id.x].z = 1;
	}
	if (count == 0) {
		cell_buffer[id.x].y = 1;
	}
}


[numthreads(256, 1, 1)]
void pressure_force_pass(uint3 id : SV_DispatchThreadID) {
	if (id.x >= particle_count) return;

	float density = densities_buffer[id.x][0];
	float density_near = densities_buffer[id.x][1];
	float pressure = pressure_from_density(density);
	float near_pressure = pressure_from_near_density(density_near);
	float3 pressure_force = float3(0.0f, 0.0f, 0.0f);

	float3 pos = predicted_positions_buffer[id.x];
	int3 cell = cell_from_position(pos, smoothing_radius);
	float sqr_radius = smoothing_radius * smoothing_radius;
	
	for (int i = 0; i < 27; i++) {
		uint hash = hash_from_cell(cell + cell_offsets[i]);
		uint key = key_from_hash(hash, particle_count);
		uint current_index = spatial_offsets_buffer[key];
	
		while (current_index < particle_count) {
			uint3 index_data = spatial_indices_buffer[current_index];
			current_index++;
			
			if (index_data[2] != key) {
				break;
			}
			if (index_data[1] != hash) {
				continue;
			}
			if (index_data[0] == id.x) {
				continue;
			}
		
			uint neighbor_index = index_data[0];
			float3 neighbor_pos = predicted_positions_buffer[neighbor_index];
			float3 offset_to_neighbor = neighbor_pos - pos;
			float sqr_dist_to_neighbor = dot(offset_to_neighbor, offset_to_neighbor);
			
			if (sqr_dist_to_neighbor > sqr_radius) {
				continue;
			}

			float density_neighbor = densities_buffer[neighbor_index][0];
			float near_density_neighbor = densities_buffer[neighbor_index][1];
			float neighbor_pressure = pressure_from_density(density_neighbor);
			float neighbor_pressure_near = pressure_from_near_density(near_density_neighbor);

			float shared_pressure = (pressure + neighbor_pressure) / 2.0f;
			float shared_near_pressure = (near_pressure + neighbor_pressure_near) / 2.0f;

			float dist = sqrt(sqr_dist_to_neighbor);
			float3 dir = (dist > 0.0f) ? offset_to_neighbor / dist : float3(0.0f, 1.0f, 0.0f);

			pressure_force += dir * density_derivative(dist, smoothing_radius) * shared_pressure / density_neighbor;
			pressure_force += dir * near_density_derivative(dist, smoothing_radius) * shared_near_pressure / near_density_neighbor;	

		}
	}
	
	float3 acceleration = pressure_force / density;
	velocities_buffer[id.x] += acceleration * delta_time;
}

[numthreads(256, 1, 1)]
void integrate_pass(uint3 id : SV_DispatchThreadID) {
	if (id.x >= particle_count) return;

	const float collision_damping = 0.85f;
	const float3 bounds = float3(10.0f, 10.0f, 10.0f);
	
	// update position	
	positions_buffer[id.x] += (velocities_buffer[id.x] * delta_time);
	
	float3 pos = positions_buffer[id.x];
	float3 vel = velocities_buffer[id.x];

	// resolve collisions
	if (abs(pos.x) > bounds.x) {
		pos.x = bounds.x * sign(pos.x);
		vel.x *= -1 * collision_damping;
	}

	if (abs(pos.y) > bounds.y) {
		pos.y = bounds.y * sign(pos.y);
		vel.y *= -1 * collision_damping;
	}

	if (abs(pos.z) > bounds.z) {
		pos.z = bounds.z * sign(pos.z);
		vel.z *= -1 * collision_damping;
	}

	positions_buffer[id.x] = pos;
	velocities_buffer[id.x] = vel;


}

