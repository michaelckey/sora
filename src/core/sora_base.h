// sora_base.h

#ifndef SORA_BASE_H
#define SORA_BASE_H

// this layer contains fundamental definitions and utilities.
// it contains:
//  - context cracking.
//  - constants.
//  - useful macros.
//  - memory arenas.
//  - length based string implementation.
//  - math libary.
//

// NOTE: 
// 
// to set up your own profiling, define the following:
//  - prof_timestamp
//  - prof_begin
//  - prof_end
//
// to set up your own logging, define the following:
// - log_info, log_infof
// - log_warn, log_warnf
// - log_error, log_errorf
//
// arena backend support:
//        |  libc  | win32 | mmap |
// win32: |   X    |   X   |      |
// macos: |   X    |       |  X   |
// linux: |   X    |       |  X   |
//
// gfx backend support:
//        | d3d11 | d3d12 | opengl | vulkan | metal |
// win32: |   X   |   X   |   X    |   X    |       |
// macos: |       |       |   X    |   X    |   X   |
// linux: |       |       |   X    |   X    |       |
//
// audio backend support:
//
//        | wasapi | coreaudio | ALSA |
// win32: |   X    |           |      |
// macos: |        |     X     |      | 
// linux: |        |           |   X  |
//
// font backend support:
//
//        | dwrite | coretext | freetype |
// win32: |   X    |          |    X     |
// macos: |        |     X    |    X     | 
// linux: |        |          |    X     |
//


//~ compiler context

#if defined(__clang__)
#    define COMPILER_CLANG 1
#elif defined(_MSC_VER)
#    define COMPILER_MSVC 1
#elif defined(__GNUC__) || defined(__GNUG__)
#    define COMPILER_GCC 1
#endif

//~ os context

#if defined(_WIN32)
#    define OS_BACKEND_WIN32 1
#elif defined(__APPLE__) && defined(__MACH__)
#    define OS_BACKEND_MACOS 1
#elif defined(__linux__) 
#    define OS_BACKEND_LINUX 1
#endif

#if !defined(BASE_USE_SIMD)
#    define BASE_USE_SIMD 1
#endif 

//~ includes

#include <math.h> // math functions
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h> 
#include <string.h>

#if BASE_USE_SIMD
#include <pmmintrin.h> // simd functions
#endif 

//~ defines

#define function static
#define global static
#define persist static
#define inlnfunc static inline

#if COMPILER_MSVC
#    define thread_global __declspec(thread)
#elif COMPILER_CLANG
#    define thread_global __thread
#endif 

//- sizes
#define bytes(n)      (n)
#define kilobytes(n)  (n << 10)
#define megabytes(n)  (n << 20)
#define gigabytes(n)  (((u64)n) << 30)
#define terabytes(n)  (((u64)n) << 40)

//- arenas
#define arena_commit_size kilobytes(4)
#define arena_decommit_size megabytes(4)

//- logging 

#if !defined(log_info)
#    define log_info(s) printf("[info] %s\n", s)
#    define log_infof(fmt, ...) printf("[info] "fmt"\n", ## __VA_ARGS__)
#    define log_warn(s) printf("[warn] %s\n", s)
#    define log_warnf(fmt, ...) printf("[warn] "fmt"\n", ##__VA_ARGS__)
#    define log_error(s) printf("[error] %s\n", s)
#    define log_errorf(fmt, ...) printf("[error] "fmt"\n", ##__VA_ARGS__)
#endif

//- profiling

#if !defined(prof_begin) && !defined(prof_end)
#    define prof_get_timestamp() 
#    define prof_begin(name)
#    define prof_end()
#    define prof_scope(name)
#endif

//- type constants
#define u8_max  (0xff)
#define u8_min  (0)
#define u16_max (0xffff)
#define u16_min (0)
#define u32_max (0xffffffff)
#define u32_min (0)
#define u64_max (0xffffffffffffffffull)
#define u64_min (0)
#define i8_max ((i8)0x7f)
#define i8_min ((i8)0xff)
#define i16_max ((i16)0x7fff)
#define i16_min ((i16)0xffff)
#define i32_max ((i32)0x7fffffff)
#define i32_min ((i32)0xffffffff)
#define i64_max ((i64)0x7fffffffffffffffull)
#define i64_min ((i64)0xffffffffffffffffull)

#define f32_sign (0x80000000)
#define f32_exp (0x7F800000)
#define f32_mantissa (0x7FFFFF)
#define f32_max (3.402823e+38f)
#define f32_min (-3.402823e+38f)
#define f32_smallest_positive (1.1754943508e-38)
#define f32_epsilon (5.96046448e-8)

#define f64_min (2.2250738585072014e-308)
#define f64_max (1.7976931348623157e+308)

#define f32_pi (3.141592653597f)
#define f64_pi (3.141592653597)

//- math
#define min(a, b) (((a)<(b)) ? (a) : (b))
#define max(a, b) (((a)>(b)) ? (a) : (b))
#define clamp(x, a, b) (((a)>(x))?(a):((b)<(x))?(b):(x))
#define clamp_01(x) (((0)>(x))?(0):((1)<(x))?(1):(x))

//- arrays
#define array_count(a) (sizeof(a) / sizeof((a)[0]))

//- stack
#define stack_push_n(f, n, next) (((n)==0) ? 0 : ((n)->next = (f), (f) = (n)))
#define stack_pop_n(f, next) (((f) == 0) ? 0 : ((f) = (f)->next))

#define stack_push(f, n) stack_push_n(f, n, next)
#define stack_pop(f) stack_pop_n(f, next)

//- queue
#define queue_push_n(f, l, n, next) (((f) == 0) ? ((f)=(l)=(n), ((n)->next = 0)) : ((l)->next=(n),(l)=(n), ((n)->next = 0)))
#define queue_push_front_n(f, l, n, next) (((f)==0) ? ((f)=(l)=(n),((n)->next = 0)) : ((n)->next=(f),(f)=(n)))
#define queue_pop_n(f, l, next) (((f) == 0) ? 0 : ((f)==(l) ? (((f) = 0),((l) = 0)) : (f)=(f)->next))

#define queue_push(f, l, n) queue_push_n(f, l, n, next)
#define queue_push_front(f, l, n) queue_push_front_n(f, l, n, next)
#define queue_pop(f, l) queue_pop_n(f, l, next)

//- doubly linked list
#define dll_insert_np(f,l,p,n,next,prev) (((f) == 0) ? ((f) = (l) = (n), (((n)->next) = 0), (((n)->prev) = 0)) : ((p) == 0) ? ((n)->next = (f), (f)->prev = (n), (f) = (n), (((n)->prev) = 0)) : ((p) == (l)) ? ((l)->next = (n), (n)->prev = (l), (l) = (n), (((n)->next) = 0)) : (((!((p) == 0) && (((p)->next) == 0)) ? (0) : ((p)->next->prev = (n))), ((n)->next = (p)->next), ((p)->next = (n)), ((n)->prev = (p))))
#define dll_push_back_np(f,l,n,next,prev) (((f) == 0) ? ((f) = (l) = (n), (((n)->next) = 0), (((n)->prev) = 0)) : ((l) == 0) ? ((n)->next = (f), (f)->prev = (n), (f) = (n), (((n)->prev) = 0)) : ((l) == (l)) ? ((l)->next = (n), (n)->prev = (l), (l) = (n), (((n)->next) = 0)) : (((!((l) == 0) && (((l)->next) == 0)) ? (0) : ((l)->next->prev = (n))), ((n)->next = (l)->next), ((l)->next = (n)), ((n)->prev = (l))))
#define dll_push_front_np(f,l,n,next,prev) (((l) == 0) ? ((l) = (f) = (n), (((n)->prev) = 0), (((n)->next) = 0)) : ((f) == 0) ? ((n)->prev = (l), (l)->next = (n), (l) = (n), (((n)->next) = 0)) : ((f) == (f)) ? ((f)->prev = (n), (n)->next = (f), (f) = (n), (((n)->prev) = 0)) : (((!((f) == 0) && (((f)->prev) == 0)) ? (0) : ((f)->prev->next = (n))), ((n)->prev = (f)->prev), ((f)->prev = (n)), ((n)->next = (f))))
#define dll_remove_np(f,l,n,next,prev) (((n) == (f) ? (f) = (n)->next : (0)), ((n) == (l) ? (l) = (l)->prev : (0)), ((((n)->prev) == 0) ? (0) : ((n)->prev->next = (n)->next)), ((((n)->next) == 0) ? (0) : ((n)->next->prev = (n)->prev)))

#define dll_insert(f,l,p,n) dll_insert_np(f,l,p,n,next,prev)
#define dll_push_back(f,l,n) dll_push_back_np(f,l,n,next,prev)
#define dll_push_front(f,l,n) dll_push_front_np(f,l,n,next,prev)
#define dll_remove(f,l,n) dll_remove_np(f,l,n,next,prev)

//~ typedefs

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef bool b8;

typedef const char* cstr;

typedef volatile int32_t atomic_i32;
typedef volatile int64_t atomic_i64;
typedef volatile uint32_t atomic_u32;
typedef volatile uint64_t atomic_u64;
typedef volatile void* atomic_ptr;

//~ atmomics

#if COMPILER_MSVC

#include <intrin.h>

#define atomic_i32_load(x) _InterlockedOr((volatile long*)(x), 0)
#define atomic_i32_assign(x, c) _InterlockedExchange((volatile long*)(x), (c))
#define atomic_i32_inc(x) _InterlockedExchangeAdd((volatile long*)(x), 1)
#define atomic_i32_dec(x) _InterlockedExchangeAdd((volatile long*)(x), -1)
#define atomic_i32_add(x, c) _InterlockedExchangeAdd((volatile long*)(x), (c))
#define atomic_i32_cond_assign(x, k, c) (_InterlockedCompareExchange((volatile long*)(x), (k), (c)) == c)

#define atomic_i64_load(x) _InterlockedOr64((volatile long long*)(x), 0)
#define atomic_i64_assign(x, c) _InterlockedExchange64((volatile long long*)(x), (c))
#define atomic_i64_inc(x) _InterlockedExchangeAdd64((volatile long long*)(x), 1)
#define atomic_i64_dec(x) _InterlockedExchangeAdd64((volatile long long*)(x), -1)
#define atomic_i64_add(x, c) _InterlockedExchangeAdd64((volatile long long*)(x), (c))
#define atomic_i64_cond_assign(x, k, c) (_InterlockedCompareExchange64((volatile long long*)(x), (k), (c)) == c)

#define atomic_ptr_load(x) ((void*)_InterlockedCompareExchangePointer((void* volatile*)(x), NULL, NULL))
#define atomic_ptr_assign(x, v) ((void*)_InterlockedExchangePointer((void* volatile*)(x), (void*)(v)))
#define atomic_ptr_cond_assign(x, k, c) (_InterlockedCompareExchangePointer((void* volatile*)(x), (void*)(k), (void*)(c)) == (void*)(c))

#define atomic_memory_barrier() MemoryBarrier()

#elif COMPILER_CLANG || COMPILER_GCC

#define atomic_u32_load(x) __atomic_load_n((volatile u32*)(x), __ATOMIC_SEQ_CST)
#define atomic_u32_assign(x, c) __atomic_exchange_n((volatile u64 *)(x), c, __ATOMIC_SEQ_CST)
#define atomic_u32_inc(x) (__atomic_fetch_add((volatile u32*)(x), 1, __ATOMIC_SEQ_CST) + 1)
#define atomic_u32_dec(x) (__atomic_fetch_sub((volatile u32*)(x), 1, __ATOMIC_SEQ_CST) + 1)
#define atomic_u32_add(x, c) (__atomic_fetch_add((volatile u32*)(x), c, __ATOMIC_SEQ_CST) + (c))
#define atomic_u32_cond_assign(x, k, c) ({ u32 _new = (c); __atomic_compare_exchange_n((volatile u32 *)(x),&_new,(k),0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST); _new; })

#define atomic_u64_load(x)  __atomic_load_n((volatile u64*)(x), __ATOMIC_SEQ_CST)
#define atomic_u64_assign(x, c) __atomic_exchange_n((volatile u64 *)(x), c, __ATOMIC_SEQ_CST)
#define atomic_u64_inc(x) (__atomic_fetch_add((volatile u64 *)(x), 1, __ATOMIC_SEQ_CST) + 1)
#define atomic_u64_dec(x) (__atomic_fetch_sub((volatile u64 *)(x), 1, __ATOMIC_SEQ_CST) - 1)
#define atomic_u64_add(x, c) (__atomic_fetch_add((volatile u64 *)(x), c, __ATOMIC_SEQ_CST) + (c))
#define atomic_u64_cond_assign(x, k, c) ({ u64 _new = (c); __atomic_compare_exchange_n((volatile u64 *)(x),&_new,(k),0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST); _new; })

#endif 

//~ enums

typedef u32 str_match_flags;
enum {
	str_match_flag_case_insensitive = (1 << 0),
	str_match_flag_right_side_sloppy = (1 << 1),
	str_match_flag_slash_insensitive = (1 << 2),
	str_match_flag_find_last = (1 << 3),
	str_match_flag_keep_empties = (1 << 4),
};

enum color_blend_mode {
	color_blend_mode_normal,
	color_blend_mode_mul,
	color_blend_mode_add,
	color_blend_mode_overlay,
};

//~ structs

//- memory arena

struct arena_t {
	u64 pos;
	u64 commit_pos;
	u64 align;
	u64 size;
};

struct temp_t {
	arena_t* arena;
	u64 pos;
};

struct thread_context_t {
    arena_t* scratch_arena;
};

//- strings

struct str_t {
	u8* data;
	u32 size;
};

struct str_node_t {
	str_node_t* next;
	str_node_t* prev;
	str_t string;
};

struct str_list_t {
	str_node_t* first;
	str_node_t* last;
	u32 count;
	u32 total_size;
};

struct str16_t {
	u16* data;
	u32 size;
};

struct str32_t {
	u32* data;
	u32 size;
};

struct codepoint_t {
	u32 codepoint;
	u32 advance;
};

struct fuzzy_match_node_t {
    fuzzy_match_node_t* next;
    u32 range_min;
    u32 range_max;
};

struct fuzzy_match_list_t {
    fuzzy_match_node_t* first;
    fuzzy_match_node_t* last;
    u32 count;
};


//- time 

struct date_time_t {
    u16 micro_second; // [0 - 999] 
    u16 milli_second; // [0 - 999] 
    u8 second; // [0 - 60]
    u8 minute; // [0 - 59]
    u8 hour; // [0 - 24]
    u8 day; // [0-31]
    u8 month; // [0-12]
    u32 year; // [0-u32_max]
};

//- math

//- vec2
union vec2_t {
    
	f32 data[2];
    
	struct {
		f32 x, y;
	};
    
	inline f32& operator[](i32 index) { return data[index]; }
	inline const f32& operator[](i32 index) const { return data[index]; }
};

//- ivec2
union ivec2_t {
    
	i32 data[2];
    
	struct {
		i32 x, y;
	};
    
	inline i32& operator[](i32 index) { return data[index]; }
	inline const i32& operator[](i32 index) const { return data[index]; }
};

//- uvec2
union uvec2_t {
    
	u32 data[2];
    
	struct {
		u32 x, y;
	};
    
	inline u32& operator[](i32 index) { return data[index]; }
	inline const u32& operator[](i32 index) const { return data[index]; }
};

//- vec3
union vec3_t {
    
	f32 data[3];
    
	struct {
		f32 x, y, z;
	};
    
	struct {
		vec2_t xy;
		f32 _unused0;
	};
    
	struct {
		f32 _unused1;
		vec2_t yz;
	};
    
	inline f32& operator[](i32 index) { return data[index]; }
	inline const f32& operator[](i32 index) const { return data[index]; }
};

//- ivec3

union ivec3_t {
	i32 data[3];
	
	struct {
		i32 x, y, z;
	};
    
	inline i32& operator[](i32 index) { return data[index]; }
	inline const i32& operator[](i32 index) const { return data[index]; }
    
    
};

//- uvec3

union uvec3_t {
    
	u32 data[3];
    
	struct {
		u32 x, y, z;
	};
    
	inline u32& operator[](i32 index) { return data[index]; }
	inline const u32& operator[](i32 index) const { return data[index]; }
    
};


//- vec4
union vec4_t {
    
	f32 data[4];
	
	struct {
		f32 x, y, z, w;
	};
    
	struct {
		vec2_t xy;
		f32 _unused0;
		f32 _unused1;
	};
    
	struct {
		f32 _unused2;
		vec2_t yz;
		f32 _unused3;
	};
    
	struct {
		f32 _unused4;
		f32 _unused5;
		vec2_t zw;
	};
    
	struct {
		vec3_t xyz;
		f32 _unused6;
	};
	
	struct {
		f32 _unused7;
		vec3_t yzw;
	};
    
#if BASE_USE_SIMD
	__m128 sse;
#endif
    
	inline f32& operator[](i32 index) { return data[index]; }
	inline const f32& operator[](i32 index) const { return data[index]; }
};

union quat_t {
    
	f32 data[4];
    
	struct {
		f32 x, y, z, w;
	};
    
	struct {
		vec3_t xyz;
		f32 _unused0;
	};
    
#if BASE_USE_SIMD
	__m128 sse;
#endif
};

//- matrices:
//
//             c0 c1 c2
//             |  |  | 
//             v  v  v
//	 r0 -> [ 1  2  3 ]
// A = r1 -> [ 4  5  6 ]
//     r2 -> [ 7  8  9 ]
// 
//   column-major:
// -----------------------------------------  
//   - accessed like:
//       A[row][col];
//	  
//   - store as:
//       A = [ 1, 4, 7, 2, 5, 8, 3, 6, 9 ];
//   
//   row major:
// -----------------------------------------  
//   - accessed like:
//       A[col][row];
//   
//   - stored as:
//       A = [ 1, 2, 3, 4, 5, 6, 7, 8, 9 ];
//   

// TODO: implement functions
union mat2_t {
	f32 data[4];
	vec2_t columns[2];
    
	inline vec2_t& operator[](i32 index) { return columns[index]; }
	inline const vec2_t& operator[](i32 index) const { return columns[index]; }
};

// TODO: implement functions
union mat3_t {
	f32 data[9];
	vec3_t columns[3];
    
	inline vec3_t& operator[](i32 index) { return columns[index]; }
	inline const vec3_t& operator[](i32 index) const { return columns[index]; }
};

union mat4_t {
	f32 data[16];
	vec4_t columns[4];
    
	inline vec4_t& operator[](i32 index) { return columns[index]; }
	inline const vec4_t& operator[](i32 index) const { return columns[index]; }
};

//- misc

union rect_t {
    
	struct {
		f32 x0, y0;
		f32 x1, y1;
	};
    
	struct {
		vec2_t v0;
		vec2_t v1;
	};
    
    vec2_t v[2];
	
};

union color_t {
    
    f32 data[4];
    
	struct { 
		f32 r, g, b, a; 
	};
    
	struct {
		f32 h, s, v, _unused0;
	};
    
    inline f32& operator[](i32 index) { return data[index]; }
	inline const f32& operator[](i32 index) const { return data[index]; }
};


union complex_t {
    
    struct {
        f32 real, imag;
    };
    
    vec2_t vec;
};



//~ globals

global u32 random_state = 0;
global u8 utf8_class[32] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5,
};

thread_global thread_context_t thread_context_local;

//~ functions

//- memory 
function void* os_base_mem_reserve(u64 size);
function void os_base_mem_release(void* ptr, u64 size);
function void os_base_mem_commit(void* ptr, u64 size);
function void os_base_mem_decommit(void* ptr, u64 size);

//- arenas
function arena_t* arena_create(u64 size);
function void arena_release(arena_t* arena);
function void* arena_alloc(arena_t* arena, u64 size);
function void* arena_calloc(arena_t* arena, u64 size);
function void arena_pop_to(arena_t* arena, u64 pos);
function void arena_clear(arena_t* arena);

function temp_t temp_begin(arena_t* arena);
function void temp_end(temp_t temp);

function temp_t scratch_begin();
function void scratch_end(temp_t temp);

//- thread context
function void thread_context_create();
function void thread_context_release();
function thread_context_t* thread_context_get();
function arena_t* thread_context_get_scratch();

//- char functions
function b8 char_is_whitespace(char c);
function b8 char_is_alpha(char c);
function b8 char_is_alpha_upper(char c);
function b8 char_is_alpha_lower(char c);
function b8 char_is_digit(char c);
function b8 char_is_symbol(char c);
function b8 char_is_space(char c);
function char char_to_upper(char c);
function char char_to_lower(char c);
function char char_to_forward_slash(char c);

//- cstr
function u32 cstr_length(cstr cstr);
function b8 cstr_equals(cstr cstr1, cstr cstr2);

//- unicode
function codepoint_t utf8_decode(u8* data, u32 max);
function codepoint_t utf16_decode(u16* data, u32 max);
function u32 utf8_encode(u8* out, codepoint_t codepoint);
function u32 utf16_encode(u16* out, codepoint_t codepoint);

//- str
function str_t str(char* cstr);
function str_t str(char* cstr, u32 size);
function str_t str_copy(arena_t* arena, str_t string);
function str_t str_substr(str_t string, u32 min_pos, u32 max_pos);
function str_t str_range(u8* first, u8* last);
function str_t str_skip(str_t string, u32 min);
function str_t str_chop(str_t string, u32 max);
function str_t str_prefix(str_t string, u32 size);
function str_t str_suffix(str_t string, u32 size);
function b8 str_match(str_t a, str_t b, str_match_flags flags);
function u32 str_find_substr(str_t haystack, str_t needle, u32 start_pos, str_match_flags flags);
function str_t str_get_file_name(str_t string);
function str_t str_get_file_extension(str_t string);
function str_t str_formatv(arena_t* arena, char* fmt, va_list args);
function str_t str_format(arena_t* arena, char* fmt, ...);
function void str_scan(str_t string, char* fmt, ...);
function u32 str_find_word_index(str_t string, u32 start_index, i32 dir);
function u64 str_hash(u64 seed, str_t string);
function str_t str_replace_range(arena_t* arena, str_t string, ivec2_t range, str_t replace);

//- str list
function void str_list_push_node(str_list_t* list, str_node_t* node);
function void str_list_push(arena_t* arena, str_list_t* list, str_t string);
function str_list_t str_split(arena_t* arena, str_t string, u8* splits, u32 split_count);
function str_t* str_array_from_list(arena_t* arena, str_list_t* list);

//- str16
function str16_t str16(u16* data);
function str16_t str16(u16* data, u32 size);

//- str conversions
function str_t str_from_str16(arena_t* arena, str16_t string);
function str16_t str16_from_str(arena_t* arena, str_t string);

//- number/string conversions
function f32 f32_from_str(str_t string);
function str_t str_from_f32(f32 value);
function i32 i32_from_str(str_t string);

//- fuzzy matching 
function fuzzy_match_list_t str_fuzzy_match_find(arena_t* arena, str_t needle, str_t haystack);

//- time 
function date_time_t date_time_from_dense_time(u64 densetime);
function u64 dense_time_from_data_time(date_time_t datetime);

function date_time_t date_time_from_microseconds(u32 microseconds);

//- random
function void random_seed(u32 seed);
function u32 random_u32();
function u32 random_u32_range(u32 min_value, u32 max_value);
function f32 random_f32();
function f32 random_f32_range(f32 min_value, f32 max_value);

//- math
inlnfunc f32 radians(f32 deg);
inlnfunc f32 degrees(f32 rad);
inlnfunc f32 remap(f32 value, f32 from_min, f32 from_max, f32 to_min, f32 to_max);
inlnfunc f32 lerp(f32 a, f32 b, f32 t);
inlnfunc f32 wrap(f32 v, f32 min, f32 max);

//- vec2 
inlnfunc vec2_t vec2(f32);
inlnfunc vec2_t vec2(f32, f32);
inlnfunc vec2_t vec2_add(vec2_t, f32);
inlnfunc vec2_t vec2_add(vec2_t, vec2_t);
inlnfunc vec2_t vec2_sub(vec2_t, f32);
inlnfunc vec2_t vec2_sub(vec2_t, vec2_t);
inlnfunc vec2_t vec2_mul(vec2_t, f32);
inlnfunc vec2_t vec2_mul(vec2_t, vec2_t);
inlnfunc vec2_t vec2_div(vec2_t, f32);
inlnfunc vec2_t vec2_div(vec2_t, vec2_t);
inlnfunc b8     vec2_equals(vec2_t, vec2_t);
inlnfunc vec2_t vec2_min(vec2_t, vec2_t);
inlnfunc vec2_t vec2_max(vec2_t, vec2_t);
inlnfunc f32    vec2_dot(vec2_t, vec2_t);
inlnfunc f32    vec2_cross(vec2_t, vec2_t);
inlnfunc f32    vec2_length(vec2_t);
inlnfunc vec2_t vec2_normalize(vec2_t);
inlnfunc vec2_t vec2_direction(vec2_t, vec2_t);
inlnfunc f32    vec2_to_angle(vec2_t);
inlnfunc vec2_t vec2_from_angle(f32, f32 = 1);
inlnfunc vec2_t vec2_rotate(vec2_t, f32);
inlnfunc vec2_t vec2_lerp(vec2_t, vec2_t, f32);

//- ivec2
inlnfunc ivec2_t ivec2(i32);
inlnfunc ivec2_t ivec2(i32, i32);
inlnfunc b8 ivec2_equals(i32, i32);

//- uvec2
inlnfunc uvec2_t uvec2(u32);
inlnfunc uvec2_t uvec2(u32, u32);
inlnfunc b8 uvec2_equals(u32, u32);

//- vec3
inlnfunc vec3_t vec3(f32);
inlnfunc vec3_t vec3(f32, f32, f32);
inlnfunc vec3_t vec3(vec2_t, f32);
inlnfunc vec3_t vec3_add(vec3_t, vec3_t);
inlnfunc vec3_t vec3_add(vec3_t, f32);
inlnfunc vec3_t vec3_sub(vec3_t, vec3_t);
inlnfunc vec3_t vec3_sub(vec3_t, f32);
inlnfunc vec3_t vec3_mul(vec3_t, vec3_t);
inlnfunc vec3_t vec3_mul(vec3_t, f32);
inlnfunc vec3_t vec3_div(vec3_t, vec3_t);
inlnfunc vec3_t vec3_div(vec3_t, f32);
inlnfunc b8     vec3_equals(vec3_t, vec3_t);
inlnfunc f32    vec3_dot(vec3_t, vec3_t);
inlnfunc vec3_t vec3_cross(vec3_t, vec3_t);
inlnfunc f32    vec3_length(vec3_t);
inlnfunc vec3_t vec3_normalize(vec3_t);
inlnfunc vec3_t vec3_negate(vec3_t);
inlnfunc vec3_t vec3_lerp(vec3_t, vec3_t, f32);
inlnfunc f32    vec3_angle_between(vec3_t, vec3_t);
inlnfunc vec3_t vec3_project(vec3_t, vec3_t);
inlnfunc vec3_t vec3_rotate(vec3_t, quat_t);
inlnfunc vec3_t vec3_clamp(vec3_t, f32, f32);

//- ivec3
inlnfunc ivec3_t ivec3(i32);
inlnfunc ivec3_t ivec3(i32, i32, i32);
inlnfunc b8      ivec3_equals(ivec3_t, ivec3_t);

//- uvec3
inlnfunc uvec3_t uvec3(u32);
inlnfunc uvec3_t uvec3(u32, u32, u32);
inlnfunc b8      uvec3_equals(uvec3_t, uvec3_t);

//- vec4
inlnfunc vec4_t vec4(f32);
inlnfunc vec4_t vec4(f32, f32, f32, f32);
inlnfunc vec4_t vec4_add(vec4_t, vec4_t);
inlnfunc vec4_t vec4_add(vec4_t, f32);
inlnfunc vec4_t vec4_sub(vec4_t, vec4_t);
inlnfunc vec4_t vec4_sub(vec4_t, f32);
inlnfunc vec4_t vec4_mul(vec4_t, vec4_t);
inlnfunc vec4_t vec4_mul(vec4_t, f32);
inlnfunc vec4_t vec4_mul(vec4_t, mat4_t);
inlnfunc vec4_t vec4_div(vec4_t, vec4_t);
inlnfunc vec4_t vec4_div(vec4_t, f32);
inlnfunc b8     vec4_equals(vec4_t, vec4_t);
inlnfunc f32    vec4_dot(vec4_t, vec4_t);
inlnfunc f32    vec4_length(vec4_t);
inlnfunc vec4_t vec4_normalize(vec4_t);
inlnfunc vec4_t vec4_lerp(vec4_t, vec4_t, f32);
inlnfunc f32    vec4_angle_between(vec4_t, vec4_t);
inlnfunc vec4_t vec4_project(vec4_t, vec4_t);

//- rect
inlnfunc rect_t rect(f32, f32, f32, f32);
inlnfunc rect_t rect(vec2_t, vec2_t);
inlnfunc b8     rect_equals(rect_t a, rect_t b);
inlnfunc void   rect_validate(rect_t&);
inlnfunc b8     rect_contains(rect_t, vec2_t);
inlnfunc b8     rect_contains(rect_t, rect_t);
inlnfunc rect_t rect_intersection(rect_t, rect_t);
inlnfunc f32    rect_width(rect_t);
inlnfunc f32    rect_height(rect_t);
inlnfunc vec2_t rect_size(rect_t);
inlnfunc vec2_t rect_center(rect_t);
inlnfunc rect_t rect_grow(rect_t, f32);
inlnfunc rect_t rect_grow(rect_t, vec2_t);
inlnfunc rect_t rect_shrink(rect_t, f32);
inlnfunc rect_t rect_shrink(rect_t, vec2_t);
inlnfunc rect_t rect_translate(rect_t, f32);
inlnfunc rect_t rect_translate(rect_t, vec2_t);
inlnfunc rect_t rect_bbox(vec2_t*, u32);
inlnfunc rect_t rect_round(rect_t);
inlnfunc rect_t rect_lerp(rect_t a, rect_t b, f32 t);

//- quat
inlnfunc quat_t quat(f32, f32, f32, f32);
inlnfunc quat_t quat(vec4_t);
inlnfunc quat_t quat_from_axis_angle(vec3_t, f32);
inlnfunc quat_t quat_from_euler_angle(vec3_t);
inlnfunc vec3_t quat_to_euler_angle(quat_t);
inlnfunc vec3_t quat_to_dir(quat_t);
inlnfunc quat_t quat_add(quat_t, quat_t);
inlnfunc quat_t quat_sub(quat_t, quat_t);
inlnfunc quat_t quat_mul(quat_t, quat_t);
inlnfunc quat_t quat_mul(quat_t, f32);
inlnfunc quat_t quat_div(quat_t, f32);
inlnfunc f32    quat_dot(quat_t, quat_t);
inlnfunc quat_t quat_inverse(quat_t);
inlnfunc f32    quat_length(quat_t);
inlnfunc quat_t quat_normalize(quat_t);
inlnfunc quat_t quat_negate(quat_t);
inlnfunc quat_t quat_lerp(quat_t, quat_t, f32);
inlnfunc quat_t quat_slerp(quat_t, quat_t, f32);
inlnfunc quat_t quat_look_at(vec3_t from, vec3_t to, vec3_t up);

//- mat4
inlnfunc mat4_t mat4(f32);
inlnfunc b8     mat4_equals(mat4_t, mat4_t);
inlnfunc mat4_t mat4_transpose(mat4_t);
inlnfunc mat4_t mat4_from_quat(quat_t);

inlnfunc mat4_t mat4_translate(vec3_t);
inlnfunc mat4_t mat4_translate(mat4_t, vec3_t);
inlnfunc mat4_t mat4_scale(vec3_t);

inlnfunc mat4_t mat4_mul(mat4_t, mat4_t);
inlnfunc vec4_t mat4_mul(mat4_t, vec4_t);
inlnfunc mat4_t mat4_inverse(mat4_t);
inlnfunc mat4_t mat4_inv_perspective(mat4_t);

inlnfunc mat4_t mat4_orthographic(f32, f32, f32, f32, f32, f32);
inlnfunc mat4_t mat4_perspective(f32, f32, f32, f32);
inlnfunc mat4_t mat4_lookat(vec3_t, vec3_t, vec3_t);
function void   mat4_print(mat4_t);

//- color
inlnfunc color_t color(u32 hex);
inlnfunc color_t color(f32 r, f32 g, f32 b, f32 a = 1.0f);
inlnfunc color_t color_hsv(f32 h, f32 s, f32 v, f32 a = 1.0f);
inlnfunc b8      color_equals(color_t a, color_t b);
inlnfunc color_t color_add(color_t a, f32 b);
inlnfunc color_t color_add(color_t a, color_t b);
inlnfunc color_t color_lerp(color_t a, color_t b, f32 t);
inlnfunc color_t color_hsv_from_rgb(color_t rgb);
inlnfunc color_t color_rgb_from_hsv(color_t hsv);
function color_t color_blend(color_t a, color_t b, color_blend_mode mode = color_blend_mode_normal);
inlnfunc u32     color_to_hex(color_t color);

//- complex 

inlnfunc complex_t complex(f32 real, f32 imag);
inlnfunc complex_t complex_add(complex_t a, complex_t b);
inlnfunc complex_t complex_sub(complex_t a, complex_t b);
inlnfunc complex_t complex_mul(complex_t a, complex_t b);
inlnfunc complex_t complex_div(complex_t a, complex_t b);
inlnfunc f32       complex_modulus(complex_t c);
inlnfunc complex_t complex_conjugate(complex_t c);
inlnfunc f32       complex_argument(complex_t c);
inlnfunc complex_t complex_exponential(f32 angle);

//- misc
function vec3_t barycentric(vec2_t p, vec2_t a, vec2_t b, vec2_t c);
function b8 tri_contains(vec2_t a, vec2_t b, vec2_t c, vec2_t p);

#endif // SORA_BASE_H