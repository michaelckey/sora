// sora_base.cpp

#ifndef SORA_BASE_CPP
#define SORA_BASE_CPP

//~ implementation

//- entry point (defined by user)
i32 app_entry_point(i32 argc, char** argv);

#if BUILD_DEBUG
int main(int argc, char** argv) {
    thread_context_create();
    i32 result = app_entry_point(argc, argv);
    thread_context_release();
	return result;
}
#elif BUILD_RELEASE

#    if OS_BACKEND_WIN32
int WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
    thread_context_create();
    i32 result = app_entry_point(__argc, __argv);
    thread_context_release();
	return result;
}
#    endif // OS_BACKEND_WIN32

#endif // BUILD_RELEASE

//- memory 

#if !defined(os_mem_reserve)
#    include <stdlib.h> // malloc
#    define os_mem_reserve(size) os_base_mem_reserve(size)
#    define os_mem_release(ptr, size) os_base_mem_release(ptr, size)
#    define os_mem_commit(ptr, size) os_base_mem_commit(ptr, size)
#    define os_mem_decommit(ptr, size) os_base_mem_decommit(ptr, size)
#endif 

function void*
os_base_mem_reserve(u64 size) {
    return malloc(size);
}

function void
os_base_mem_release(void* ptr, u64 size) {
    free(ptr);
}

function void
os_base_mem_commit(void* ptr, u64 size) {
    // no op
}

function void
os_base_mem_decommit(void* ptr, u64 size) {
    // no op
}

//- arenas

function arena_t*
arena_create(u64 size) {
    
	// roundup
	u64 size_roundup = megabytes(64);
	size += size_roundup - 1;
	size -= size % size_roundup;
    
	// reserve memory
	void* block = os_mem_reserve(size);
    
	// initial commit
	u64 initial_commit_size = arena_commit_size;
	os_mem_commit(block, initial_commit_size);
    
	// fill struct
	arena_t* arena = (arena_t*)block;
	arena->pos = sizeof(arena_t);
	arena->commit_pos = initial_commit_size;
	arena->align = 8;
	arena->size = size;
    
	return arena;
}

function void
arena_release(arena_t* arena) {
	os_mem_release(arena, arena->size);
}

function void*
arena_alloc(arena_t* arena, u64 size) {
    void* result = nullptr;
    
	if (arena == nullptr) {
        log_errorf("arena %p was not initialized!", arena);
	} else {
        
        if (arena->pos + size <= arena->size) {
            
            u8* base = (u8*)arena;
            
            // align
            u64 post_align_pos = (arena->pos + (arena->align - 1));
            post_align_pos -= post_align_pos % arena->align;
            u64 align = post_align_pos - arena->pos;
            result = base + arena->pos + align;
            arena->pos += size + align;
            
            // commit
            if (arena->commit_pos < arena->pos) {
                u64 size_to_commit = arena->pos - arena->commit_pos;
                size_to_commit += arena_commit_size - 1;
                size_to_commit -= size_to_commit % arena_commit_size;
                os_mem_commit(base + arena->commit_pos, size_to_commit);
                arena->commit_pos += size_to_commit;
            }
            
        } else {
            log_errorf("arena %p is full!", arena);
        }
        
    }
    
	return result;
}

function void*
arena_calloc(arena_t* arena, u64 size) {
	void* result = arena_alloc(arena, size);
	memset(result, 0, size);
	return result;
}

function void
arena_pop_to(arena_t* arena, u64 pos) {
    
	// find pos
	u64 min_pos = sizeof(arena);
	u64 new_pos = max(min_pos, pos);
	arena->pos = new_pos;
    
	// align
	u64 pos_aligned_to_commit_chunks = arena->pos + arena_commit_size - 1;
	pos_aligned_to_commit_chunks -= pos_aligned_to_commit_chunks % arena_commit_size;
    
	// decommit
	if (pos_aligned_to_commit_chunks + arena_decommit_size <= arena->commit_pos) {
		u8* base = (u8*)arena;
		u64 size_to_decommit = arena->commit_pos - pos_aligned_to_commit_chunks;
		os_mem_decommit(base + pos_aligned_to_commit_chunks, size_to_decommit);
		arena->commit_pos -= size_to_decommit;
	}
    
}

function void
arena_clear(arena_t* arena) {
	arena_pop_to(arena, sizeof(arena_t));
}

function temp_t 
temp_begin(arena_t* arena) {
	u64 pos = arena->pos;
	temp_t temp = { arena, pos };
	return temp;
}

function void 
temp_end(temp_t temp) {
	arena_pop_to(temp.arena, temp.pos);
}

function temp_t
scratch_begin() {
    arena_t* thread_arena = thread_context_get_scratch();
    temp_t temp = temp_begin(thread_arena);
	return temp;
}

function void 
scratch_end(temp_t temp) {
	temp_end(temp);
}

//- thread context functions 

function void
thread_context_create() {
    thread_context_local.scratch_arena = arena_create(gigabytes(1));
}

function void
thread_context_release() {
    arena_release(thread_context_local.scratch_arena);
}

function thread_context_t*
thread_context_get() {
    return &thread_context_local;
}

function arena_t*
thread_context_get_scratch() {
    return thread_context_local.scratch_arena;
}

//- char functions


function b8
char_is_whitespace(char c) {
	return (c == ' ' || c == '\t' || c == '\v' || c == '\f');
}

function b8
char_is_alpha(char c) {
	return char_is_alpha_upper(c) || char_is_alpha_lower(c);
}

function b8
char_is_alpha_upper(char c) {
	return c >= 'A' && c <= 'Z';
}

function b8
char_is_alpha_lower(char c) {
	return c >= 'a' && c <= 'z';
}

function b8
char_is_digit(char c) {
	return (c >= '0' && c <= '9');
}

function b8
char_is_symbol(char c) {
	return (c == '~' || c == '!' || c == '$' || c == '%' || c == '^' ||
            c == '&' || c == '*' || c == '-' || c == '=' || c == '+' ||
            c == '<' || c == '.' || c == '>' || c == '/' || c == '?' ||
            c == '|' || c == '\\' || c == '{' || c == '}' || c == '(' ||
            c == ')' || c == '\\' || c == '[' || c == ']' || c == '#' ||
            c == ',' || c == ';' || c == ':' || c == '@');
}

function b8
char_is_space(char c) {
	return c == ' ' || c == '\r' || c == '\t' || c == '\f' || c == '\v' || c == '\n';
}

function char
char_to_upper(char c) {
	return (c >= 'a' && c <= 'z') ? ('A' + (c - 'a')) : c;
}

function char
char_to_lower(char c) {
	return (c >= 'A' && c <= 'Z') ? ('a' + (c - 'A')) : c;
}

function char
char_to_forward_slash(char c) {
	return (c == '\\' ? '/' : c);
}


//- cstr functions

function u32
cstr_length(cstr cstr) {
	u32 i;
	for (i = 0; cstr[i] != '\0'; i++);
	return i;
}

function b8
cstr_equals(cstr cstr1, cstr cstr2) {
	while (*cstr1 && (*cstr1 == *cstr2)) {
		cstr1++;
		cstr2++;
	}
	return (*(const unsigned char*)cstr1 - *(const unsigned char*)cstr2) == 0;
}

// unicode

function codepoint_t 
utf8_decode(u8* data, u32 max) {
    
	codepoint_t result = { ~((u32)0), 1 };
	u8 byte = data[0];
	u8 byte_class = utf8_class[byte >> 3];
    
	switch (byte_class) {
		case 1: {
			result.codepoint = byte;
			break;
		}
        
		case 2: {
			if (2 <= max) {
				u8 cont_byte = data[1];
				if (utf8_class[cont_byte >> 3] == 0) {
					result.codepoint = (byte & 0x1F) << 6;
					result.codepoint |= (cont_byte & 0x3F);
					result.advance = 2;
				}
			}
			break;
		}
        
		case 3: {
			if (3 <= max) {
				u8 cont_byte[2] = { data[1], data[2] };
				if (utf8_class[cont_byte[0] >> 3] == 0 &&
					utf8_class[cont_byte[1] >> 3] == 0) {
					result.codepoint = (byte & 0x0F) << 12;
					result.codepoint |= ((cont_byte[0] & 0x3F) << 6);
					result.codepoint |= (cont_byte[1] & 0x3F);
					result.advance = 3;
				}
			}
			break;
		}
        
		case 4: {
			if (4 <= max) {
				u8 cont_byte[3] = { data[1], data[2], data[3] };
				if (utf8_class[cont_byte[0] >> 3] == 0 &&
					utf8_class[cont_byte[1] >> 3] == 0 &&
					utf8_class[cont_byte[2] >> 3] == 0) {
					result.codepoint = (byte & 0x07) << 18;
					result.codepoint |= ((cont_byte[0] & 0x3F) << 12);
					result.codepoint |= ((cont_byte[1] & 0x3F) << 6);
					result.codepoint |= (cont_byte[2] & 0x3F);
					result.advance = 4;
				}
			}
			break;
		}
	}
    
	return result;
}

function codepoint_t 
utf16_decode(u16* data, u32 max) {
	codepoint_t result = { ~((u32)0), 1 };
	result.codepoint = data[0];
	result.advance = 1;
	if (1 < max && 0xD800 <= data[0] && data[0] < 0xDC00 && 0xDC00 <= data[1] && data[1] < 0xE000) {
		result.codepoint = ((data[0] - 0xD800) << 10) | (data[1] - 0xDC00) + 0x10000;
		result.advance = 2;
	}
	return result;
}

function u32 
utf8_encode(u8* out, codepoint_t codepoint) {
	u32 advance = 0;
	if (codepoint.codepoint <= 0x7F) {
		out[0] = (u8)codepoint.codepoint;
		advance = 1;
	} else if (codepoint.codepoint <= 0x7FF) {
		out[0] = (0x03 << 6) | ((codepoint.codepoint >> 6) & 0x1F);
		out[1] = 0x80 | (codepoint.codepoint & 0x3F);
		advance = 2;
	} else if (codepoint.codepoint <= 0xFFFF) {
		out[0] = (0x07 << 5) | ((codepoint.codepoint >> 12) & 0x0F);
		out[1] = 0x80 | ((codepoint.codepoint >> 6) & 0x3F);
		out[2] = 0x80 | (codepoint.codepoint & 0x3F);
		advance = 3;
	} else if (codepoint.codepoint <= 0x10FFFF) {
		out[0] = (0x0F << 4) | ((codepoint.codepoint >> 18) & 0x07);
		out[1] = 0x80 | ((codepoint.codepoint >> 12) & 0x3F);
		out[2] = 0x80 | ((codepoint.codepoint >> 6) & 0x3F);
		out[3] = 0x80 | (codepoint.codepoint & 0x3F);
		advance = 4;
	} else {
		out[0] = '?';
		advance = 1;
	}
	return advance;
}

function u32 
utf16_encode(u16* out, codepoint_t codepoint) {
	u32 advance = 1;
	if (codepoint.codepoint == ~((u32)0)) {
		out[0] = (u16)'?';
	} else if (codepoint.codepoint < 0x10000) {
		out[0] = (u16)codepoint.codepoint;
	} else {
		u32 v = codepoint.codepoint - 0x10000;
		out[0] = 0xD800 + (v >> 10);
		out[1] = 0xDC00 + (v & 0x03FF);
		advance = 2;
	}
	return advance;
}

// str functions

function str_t 
str(char* cstr) {
	str_t string;
	string.data = (u8*)cstr;
	string.size = cstr_length(cstr);
	return string;
}

function str_t
str(cstr cstr) {
	str_t string;
	string.data = (u8*)cstr;
	string.size = cstr_length(cstr);
	return string;
}

function str_t
str(char* cstr, u32 size) {
	str_t string;
	string.data = (u8*)cstr;
	string.size = size;
	return string;
}

function str_t 
str_copy(arena_t* arena, str_t string) {
	char* data = (char*)arena_alloc(arena, sizeof(char) * string.size);
	memcpy(data, string.data, sizeof(char) * string.size);
	str_t result = str(data, string.size);
	return result;
}

function str_t
str_substr(str_t string, u32 min_pos, u32 max_pos) {
	u32 min = min_pos;
	u32 max = max_pos;
	if (max > string.size) {
		max = string.size;
	}
	if (min > string.size) {
		min = string.size;
	}
	if (min > max) {
		u32 swap = min;
		min = max;
		max = swap;
	}
	string.size = max - min;
	string.data += min;
	return string;
}

function str_t
str_range(u8* first, u8* last) {
	str_t result = { 0 };
	result.data = first;
	result.size = (u32)(last - first);
	return result;
}

function str_t
str_skip(str_t string, u32 min) {
	return str_substr(string, min, string.size);
}

function str_t
str_chop(str_t string, u32 max) {
	return str_substr(string, 0, string.size - max);
}

function str_t
str_prefix(str_t string, u32 size) {
	return str_substr(string, 0, size);
}

function str_t
str_suffix(str_t string, u32 size) {
	return str_substr(string, string.size - size, string.size);
}

function b8
str_match(str_t a, str_t b, str_match_flags flags = 0) {
	b8 result = 0;
    
	if (a.size == b.size || flags & str_match_flag_right_side_sloppy) {
		result = 1;
		for (u32 i = 0; i < a.size; i++) {
			b8 match = (a.data[i] == b.data[i]);
            
			if (flags & str_match_flag_case_insensitive) {
				match |= (char_to_lower(a.data[i]) == char_to_lower(b.data[i]));
			}
            
			if (flags & str_match_flag_slash_insensitive) {
				match |= (char_to_forward_slash(a.data[i]) == char_to_forward_slash(b.data[i]));
			}
            
			if (match == 0) {
				result = 0;
				break;
			}
		}
	}
	return result;
}

function u32
str_find_substr(str_t haystack, str_t needle, u32 start_pos = 0, str_match_flags flags = 0) {
	b8 found = 0;
	u32 found_idx = haystack.size;
	for (u32 i = start_pos; i < haystack.size; i++) {
		if (i + needle.size <= haystack.size) {
			str_t substr = str_substr(haystack, i, i + needle.size);
			if (str_match(substr, needle, flags)) {
				found_idx = i;
				found = 1;
				if (!(flags & str_match_flag_find_last)) {
					break;
				}
			}
		}
	}
	return found_idx;
}

function str_t 
str_get_file_name(str_t string) {
    
	u32 slash_pos = str_find_substr(string, str("/"), 0, str_match_flag_case_insensitive | str_match_flag_find_last);
	if (slash_pos < string.size) {
		string.data += slash_pos + 1;
		string.size -= slash_pos + 1;
	}
	
	u32 period_pos = str_find_substr(string, str("."), 0, str_match_flag_find_last);
	if (period_pos < string.size) {
		string.size = period_pos;
	}
    
	return string;
}

function str_t 
str_get_file_extension(str_t string) {
    
	u32 period_pos = str_find_substr(string, str("."), 0, str_match_flag_find_last);
	if (period_pos < string.size) {
		string.data += period_pos + 1;
		string.size -= period_pos + 1;
	}
    
	return string;
}

function str_t
str_formatv(arena_t* arena, char* fmt, va_list args) {
	str_t result = { 0 };
	va_list args2;
	va_copy(args2, args);
	u32 needed_bytes = vsnprintf(0, 0, fmt, args) + 1;
	result.data = (u8*)arena_alloc(arena, sizeof(u8) * needed_bytes);
	result.size = needed_bytes - 1;
	vsnprintf((char*)result.data, needed_bytes, fmt, args2);
	return result;
}

function str_t
str_format(arena_t* arena, char* fmt, ...) {
	str_t result = { 0 };
	va_list args;
	va_start(args, fmt);
	result = str_formatv(arena, fmt, args);
	va_end(args);
	return result;
}

function void
str_scan(str_t string, char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vsscanf((char*)string.data, fmt, args);
	va_end(args);
}

function u32 
str_find_word_index(str_t string, u32 start_index, i32 dir) {
    
    u32 result = start_index;
    
    if (dir > 0) {
        while (result < string.size && !char_is_whitespace(string.data[result])) { result++; }
        while (result < string.size && char_is_whitespace(string.data[result])) { result++; }
    } else {
        result--;
        while (result > 0 && char_is_whitespace(string.data[result])) { result--; }
        while (result > 0 && !char_is_whitespace(string.data[result - 1])) { result--; }
    }
    
    return result;
    
}

function str_t 
str_replace_range(arena_t* arena, str_t string, ivec2_t range, str_t replace) {
    
    ivec2_t adj_range = ivec2(range.x - 1, range.y - 1); 
    
    if(adj_range.x > string.size) {
        adj_range.x = 0;
    }
    
    if(adj_range.y > string.size) {
        adj_range.y = string.size;
    }
    
    // calculate new size
    u32 old_size = string.size;
    u32 new_size = old_size - (adj_range.y - adj_range.x) + replace.size;
    
    u8* base = (u8*)arena_alloc(arena, sizeof(u8) * new_size);
    
    memcpy(base + 0, string.data, adj_range.x);
    memcpy(base + adj_range.x + replace.size, string.data + adj_range.y, string.size - adj_range.y);
    
    if(replace.data != 0) {
        memcpy(base + adj_range.x, replace.data, replace.size);
    }
    
    str_t result = str((char*)base, new_size);
    
    return result;
}

//- str list

function void
str_list_push_node(str_list_t* list, str_node_t* node) {
	dll_push_back(list->first, list->last, node);
	list->count++;
	list->total_size += node->string.size;
}

function void
str_list_push(arena_t* arena, str_list_t* list, str_t string) {
	str_node_t* n = (str_node_t*)arena_alloc(arena, sizeof(str_node_t));
	n->string = string;
	str_list_push_node(list, n);
}

function str_list_t
str_split(arena_t* arena, str_t string, u8* splits, u32 split_count) {
	str_list_t list = { 0 };
    
	u8* ptr = (u8*)string.data;
	u8* opl = (u8*)string.data + string.size;
	for (; ptr < opl;) {
		u8* first = ptr;
		for (; ptr < opl; ptr += 1) {
			u8 c = *ptr;
			b8 is_split = 0;
			for (u64 i = 0; i < split_count; i += 1) {
				if (splits[i] == c) {
					is_split = 1;
					break;
				}
			}
			if (is_split) {
				break;
			}
		}
        
		str_t sub_string = str_range(first, ptr);
		if (sub_string.size > 0) {
			str_list_push(arena, &list, sub_string);
		}
		ptr += 1;
	}
    
	return(list);
}

function u64
str_hash(u64 seed, str_t string) {
    u64 result = seed;
    if (string.size != 0) {
        for (u64 i = 0; i < string.size; i += 1) {
            result = ((result << 5) + result) + string.data[i];
        }
    }
    return result;
}


//- str16 functions

function str16_t
str16(u16* data) {
	str16_t result;
	result.data = data;
	u16* i = data;
	for (; *i; i += 1);
	result.size = i - data;
	return result;
}

function str16_t
str16(u16* data, u32 size) {
	str16_t result;
	result.data = data;
	result.size = size;
	return result;
}

//- string conversions

function str_t
str_from_str16(arena_t* arena, str16_t string) {
    
    u64 capacity = string.size;
    
    u8 *data = (u8*)arena_alloc(arena, sizeof(u8) * (capacity + 1));
    u16 *ptr = string.data;
    u16 *opl = ptr + string.size;
    
    u64 size = 0;
    codepoint_t codepoint;
    
    for(;ptr < opl;) {
        codepoint = utf16_decode(ptr, opl - ptr);
        ptr += codepoint.advance;
        size += utf8_encode(data + size, codepoint);
    }
    data[size] = 0;
    
    str_t result;
    result.data = data;
    result.size = size;
    
    return result;
}

function str16_t
str16_from_str(arena_t* arena, str_t string) {
    
	u32 capacity = string.size * 2;
    
	u16* data = (u16*)arena_alloc(arena, sizeof(u16) * capacity + 1);
	u8* ptr = (u8*)string.data;
	u8* opl = ptr + string.size;
    
	u32 size = 0;
	codepoint_t codepoint;
    
	for (; ptr < opl;) {
		codepoint = utf8_decode(ptr, opl - ptr);
		ptr += codepoint.advance;
		size += utf16_encode(data + size, codepoint);
	}
	data[size] = 0;
    
	str16_t result;
	result.data = data;
	result.size = size;
	return result;
    
}

//- number/string conversions

function f32
f32_from_str(str_t string) {
    
    i32 i = 0;
    i32 sign = 1;
    i32 integer_part = 0;
    i32 fraction_part = 0;
    i32 divisor = 1;
    
    if (string.data[i] == '-') {
        sign = -1;
        i++;
    } else if (string.data[i] == '+') {
        i++;
    }
    
    while(char_is_digit(string.data[i]) && i < string.size) {
        integer_part = integer_part * 10 + (string.data[i] - '0');
        i++;
    }
    
    if (string.data[i] == '.') {
        i++;
        while (char_is_digit(string.data[i]) && i < string.size) {
            fraction_part = fraction_part * 10 + (string.data[i] - '0');
            divisor *= 10;
            i++;
        }
    }
    
    return sign * ((f32)integer_part + (f32)fraction_part / (f32)divisor);
}

//- fuzzy matching 

function fuzzy_match_list_t 
str_fuzzy_match_find(arena_t* arena, str_t needle, str_t haystack) {
    
    fuzzy_match_list_t result = { 0 };
    temp_t scratch = scratch_begin();
    
    str_list_t needles = str_split(scratch.arena, needle, (u8*)" ", 1);
    for (str_node_t* n = needles.first; n != nullptr; n = n->next) {
        
        u32 find_pos = 0;
        for (;find_pos < haystack.size;) {
            
            find_pos = str_find_substr(haystack, n->string, find_pos, str_match_flag_case_insensitive);
            
            b8 is_in_gathered_ranges = false;
            for (fuzzy_match_node_t* match_node = result.first; match_node != nullptr; match_node = match_node->next) {
                if (match_node->range_min <= find_pos && find_pos < match_node->range_max) {
                    is_in_gathered_ranges = true;
                    find_pos = match_node->range_max;
                    break;
                }
            }
            
            if (!is_in_gathered_ranges) {
                break;
            }
            
        }
        
        if (find_pos < haystack.size) {
            
            fuzzy_match_node_t* node = (fuzzy_match_node_t*)arena_alloc(arena, sizeof(fuzzy_match_list_t));
            node->range_min = find_pos;
            node->range_max = find_pos + n->string.size;
            
            queue_push(result.first, result.last, node);
            result.count++;
            
        }
        
    }
    
    
    scratch_end(scratch);
    return result;
}

//- time

function date_time_t 
date_time_from_dense_time(u64 densetime) {
    date_time_t result = { 0 };
    
    result.milli_second = densetime % 1000;
    densetime /= 1000;
    result.second  = densetime % 61;
    densetime /= 61;
    result.minute  = densetime % 60;
    densetime /= 60;
    result.hour = densetime % 24;
    densetime /= 24;
    result.day  = densetime % 31;
    densetime /= 31;
    result.month  = densetime % 12;
    densetime /= 12;
    result.year = (u32)densetime;
    
    return result;
}

function u64 
dense_time_from_data_time(date_time_t datetime) {
    u64 result = 0;
    
    result += datetime.year;
    result *= 12;
    result += datetime.month;
    result *= 31;
    result += datetime.day;
    result *= 24;
    result += datetime.hour;
    result *= 60;
    result += datetime.minute;
    result *= 61;
    result += datetime.second;
    result *= 1000;
    result += datetime.milli_second;
    
    return result;
}

function date_time_t
date_time_from_microseconds(u64 microseconds) {
    date_time_t result = { 0 };
    
    result.micro_second = microseconds%1000;
    microseconds /= 1000;
    result.milli_second = microseconds%1000;
    microseconds /= 1000;
    result.second = microseconds%60;
    microseconds /= 60;
    result.minute = microseconds%60;
    microseconds /= 60;
    result.hour = microseconds%24;
    microseconds /= 24;
    result.day = microseconds%31;
    microseconds /= 31;
    result.month = microseconds%12;
    microseconds /= 12;
    result.year = (u32)microseconds;
    
    return result;
}

//- random

function void
random_seed(u32 seed) {
	random_state = seed;
}

function u32
random_u32() {
	random_state += 0xe120fc12;
	u64 temp;
	temp = (u64)random_state * 0x4139b70d;
	u32 m1 = (temp >> 32) ^ temp;
	temp = (u64)m1 * 0x12fad5c9;
	u32 m2 = (temp >> 32) ^ temp;
	return m2;
}

function u32
random_u32_range(u32 min_value, u32 max_value) {
	return (random_u32() % (max_value - min_value + 1)) + min_value;
}

function f32
random_f32() {
	return ((f32)random_u32() / (f32)u32_max);
}

function f32
random_f32_range(f32 min_value, f32 max_value) {
	return min_value + random_f32() * (max_value - min_value);
}

//- math

inlnfunc f32
radians(f32 degrees) {
	return degrees * 0.0174533f;
}

inlnfunc f32
degrees(f32 radians) {
	return radians * 57.2958f;
}

inlnfunc f32
remap(f32 value, f32 from_min, f32 from_max, f32 to_min, f32 to_max) {
	return to_min + (value - from_min) * (to_max - to_min) / (from_max - from_min);
}

inlnfunc f32
lerp(f32 a, f32 b, f32 t) {
	return (a * (1.0f - t)) + (b * t);
}

inlnfunc f32 
wrap(f32 v, f32 min, f32 max) {
	f32 range = max - min;
	while (v < min) v += range;
	while (v >= max) v -= range;
	return v;
}

//- vec2 

inlnfunc vec2_t
vec2(f32 v = 0.0f) {
	vec2_t result;
	result.x = v;
	result.y = v;
	return result;
}

inlnfunc vec2_t
vec2(f32 x, f32 y) {
	vec2_t result;
	result.x = x;
	result.y = y;
	return result;
}

inlnfunc vec2_t
vec2_add(vec2_t a, vec2_t b) {
	vec2_t result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

inlnfunc vec2_t
vec2_add(vec2_t a, f32 b) {
	vec2_t result;
	result.x = a.x + b;
	result.y = a.y + b;
	return result;
}

inlnfunc vec2_t
vec2_sub(vec2_t a, vec2_t b) {
	vec2_t result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	return result;
}

inlnfunc vec2_t
vec2_sub(vec2_t a, f32 b) {
	vec2_t result;
	result.x = a.x - b;
	result.y = a.y - b;
	return result;
}

inlnfunc vec2_t
vec2_mul(vec2_t a, vec2_t b) {
	vec2_t result;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	return result;
}

inlnfunc vec2_t
vec2_mul(vec2_t a, f32 b) {
	vec2_t result;
	result.x = a.x * b;
	result.y = a.y * b;
	return result;
}

inlnfunc vec2_t
vec2_div(vec2_t a, vec2_t b) {
	vec2_t result;
	result.x = a.x / b.x;
	result.y = a.y / b.y;
	return result;
}

inlnfunc vec2_t
vec2_div(vec2_t a, f32 b) {
	vec2_t result;
	result.x = a.x / b;
	result.y = a.y / b;
	return result;
}

inlnfunc b8 
vec2_equals(vec2_t a, vec2_t b) {
	return (a.x == b.x && a.y == b.y);
}

inlnfunc vec2_t 
vec2_min(vec2_t a, vec2_t b) {
	return { min(a.x, b.x), min(a.y, b.y) };
}

inlnfunc vec2_t 
vec2_max(vec2_t a, vec2_t b) {
	return { max(a.x, b.x), max(a.y, b.y) };
}

inlnfunc f32
vec2_dot(vec2_t a, vec2_t b) {
	return (a.x * b.x) + (a.y * b.y);
}

inlnfunc f32
vec2_cross(vec2_t a, vec2_t b) {
	return (a.x * b.y) - (a.y * b.x);
}

inlnfunc f32
vec2_length(vec2_t a) {
	return sqrtf(vec2_dot(a,a));
}

inlnfunc vec2_t
vec2_normalize(vec2_t a) {
	return vec2_mul(a, 1.0f / vec2_length(a));
}

inlnfunc vec2_t 
vec2_direction(vec2_t a, vec2_t b) {
	vec2_t result;
	result.x = b.x - a.x;
	result.y = b.y - a.y;
	return result;
}

inlnfunc f32
vec2_to_angle(vec2_t a) {
	return atan2f(a.y, a.x);
}

inlnfunc vec2_t 
vec2_from_angle(f32 a, f32 m) {
	return { m * cosf(a), m * sinf(a) };
}

inlnfunc vec2_t
vec2_rotate(vec2_t v, f32 a) {
	return { v.x * cosf(a) - v.y * sinf(a), v.x * sinf(a) - v.y * cosf(a) };
}

inlnfunc vec2_t
vec2_lerp(vec2_t a, vec2_t b, f32 t) {
	return vec2_add(vec2_mul(a, 1.0f - t), vec2_mul(b, t));
}

//- ivec2

inlnfunc ivec2_t 
ivec2(i32 v = 0) {
	return { v, v };
}

inlnfunc ivec2_t 
ivec2(i32 x, i32 y) {
	return { x, y };
}

inlnfunc b8
ivec2_equals(ivec2_t a, ivec2_t b) {
	return ((a.x == b.x) && (a.y == b.y));
}

//- uvec2

inlnfunc uvec2_t 
uvec2(u32 v = 0) {
	return { v, v };
}

inlnfunc uvec2_t 
uvec2(u32 x, u32 y) {
	return { x, y };
}

inlnfunc b8 
uvec2_equals(uvec2_t a, uvec2_t b) {
	return ((a.x == b.x) && (a.y == b.y));
}

//- vec3

inlnfunc vec3_t
vec3(f32 v = 0.0f) {
	vec3_t result;
	result.x = v;
	result.y = v;
	result.z = v;
	return result;
}

inlnfunc vec3_t
vec3(f32 x, f32 y, f32 z) {
	vec3_t result;
	result.x = x;
	result.y = y;
	result.z = z;
	return result;
}

inlnfunc vec3_t
vec3(vec2_t xy, f32 z = 0.0f) {
	vec3_t result;
	result.xy = xy;
	result.z = z;
	return result;
}

inlnfunc vec3_t
vec3_add(vec3_t a, vec3_t b) {
	vec3_t result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	return result;
}

inlnfunc vec3_t
vec3_add(vec3_t a, f32 b) {
	vec3_t result;
	result.x = a.x + b;
	result.y = a.y + b;
	result.z = a.z + b;
	return result;
}

inlnfunc vec3_t
vec3_sub(vec3_t a, vec3_t b) {
	vec3_t result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return result;
}

inlnfunc vec3_t
vec3_sub(vec3_t a, f32 b) {
	vec3_t result;
	result.x = a.x - b;
	result.y = a.y - b;
	result.z = a.z - b;
	return result;
}

inlnfunc vec3_t
vec3_mul(vec3_t a, vec3_t b) {
	vec3_t result;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	return result;
}

inlnfunc vec3_t
vec3_mul(vec3_t a, f32 b) {
	vec3_t result;
	result.x = a.x * b;
	result.y = a.y * b;
	result.z = a.z * b;
	return result;
}

inlnfunc vec3_t
vec3_div(vec3_t a, vec3_t b) {
	vec3_t result;
	result.x = a.x / b.x;
	result.y = a.y / b.y;
	result.z = a.z / b.z;
	return result;
}

inlnfunc vec3_t
vec3_div(vec3_t a, f32 b) {
	vec3_t result;
	result.x = a.x / b;
	result.y = a.y / b;
	result.z = a.z / b;
	return result;
}

inlnfunc b8 
vec3_equals(vec3_t a, vec3_t b) {
	if (a.x == b.x && a.y == b.y && a.z == b.z);
}

inlnfunc f32
vec3_dot(vec3_t a, vec3_t b) {
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

inlnfunc vec3_t
vec3_cross(vec3_t a, vec3_t b) {
	vec3_t result;
	result.x = (a.y * b.z) - (a.z * b.y);
	result.y = (a.z * b.x) - (a.x * b.z);
	result.z = (a.x * b.y) - (a.y * b.x);
	return result;
}

inlnfunc f32
vec3_length(vec3_t a) {
	return sqrtf(vec3_dot(a, a));
}

inlnfunc vec3_t
vec3_normalize(vec3_t v) {
	if (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f) return vec3(0.0f);
	return vec3_mul(v, 1.0f / vec3_length(v));
}

inlnfunc vec3_t
vec3_negate(vec3_t v) {
	return vec3(-v.x, -v.y, -v.z);
}

inlnfunc vec3_t
vec3_lerp(vec3_t a, vec3_t b, f32 t) {
	return vec3_add(vec3_mul(a, 1.0f - t), vec3_mul(b, t));
}

inlnfunc f32
vec3_angle_between(vec3_t a, vec3_t b) {
	return acosf(vec3_dot(a, b) / (vec3_length(a) * vec3_length(b)));
}

inlnfunc vec3_t
vec3_project(vec3_t a, vec3_t b) {
	return vec3_mul(b, vec3_dot(a, b) / vec3_dot(b, b));
}

inlnfunc vec3_t
vec3_rotate(vec3_t v, quat_t q) {
	vec3_t t = vec3_mul(vec3_cross(v, q.xyz), 2.0f);
	return vec3_add(v, vec3_add(vec3_mul(t, q.w), vec3_cross(t, q.xyz)));
}

inlnfunc vec3_t 
vec3_clamp(vec3_t v, f32 a, f32 b) {
	return { clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b) };
}

//- ivec3
inlnfunc ivec3_t 
ivec3(i32 v) {
	return { v, v, v };
}

inlnfunc ivec3_t 
ivec3(i32 x, i32 y, i32 z) {
	return { x, y, z };
}

inlnfunc b8 
ivec3_equals(ivec3_t a, ivec3_t b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

//- uvec3
inlnfunc uvec3_t 
uvec3(u32 v) {
	return { v, v, v };
}

inlnfunc uvec3_t 
uvec3(u32 x, u32 y, u32 z) {
	return { x, y, z };
}

inlnfunc b8 
uvec3_equals(uvec3_t a, uvec3_t b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}


//- vec4

inlnfunc vec4_t
vec4(f32 v) {
	vec4_t result;
#if BASE_USE_SIMD
	result.sse = _mm_setr_ps(v, v, v, v);
#else
	result.x = v;
	result.y = v;
	result.z = v;
	result.w = v;
#endif
	return result;
}

inlnfunc vec4_t
vec4(f32 x, f32 y, f32 z, f32 w) {
	vec4_t result;
#if BASE_USE_SIMD
	result.sse = _mm_setr_ps(x, y, z, w);
#else
	result.x = x;
	result.y = y;
	result.z = z;
	result.w = w;
#endif
	return result;
}

inlnfunc vec4_t 
vec4(vec3_t xyz, f32 w = 0.0f) {
	vec4_t result;
#if BASE_USE_SIMD
	result.sse = _mm_setr_ps(xyz.x, xyz.y, xyz.z, w);
#else
	result.xyz = xyz;
	result.w = w;
#endif
	return result;
}

inlnfunc vec4_t
vec4_add(vec4_t a, vec4_t b) {
	vec4_t result;
#if BASE_USE_SIMD
	result.sse = _mm_add_ps(a.sse, b.sse);
#else
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	result.w = a.w + b.w;
#endif
	return result;
}

inlnfunc vec4_t
vec4_add(vec4_t a, f32 b) {
	vec4_t result;
#if BASE_USE_SIMD
	__m128 _b = _mm_set1_ps(b);
	result.sse = _mm_add_ps(a.sse, _b);
#else
	result.x = a.x + b;
	result.y = a.y + b;
	result.z = a.z + b;
	result.w = a.w + b;
#endif
	return result;
}

inlnfunc vec4_t
vec4_sub(vec4_t a, vec4_t b) {
	vec4_t result;
#if BASE_USE_SIMD
	result.sse = _mm_sub_ps(a.sse, b.sse);
#else
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	result.w = a.w - b.w;
#endif
	return result;
}

inlnfunc vec4_t
vec4_sub(vec4_t a, f32 b) {
	vec4_t result;
#if BASE_USE_SIMD
	__m128 _b = _mm_set1_ps(b);
	result.sse = _mm_sub_ps(a.sse, _b);
#else
	result.x = a.x - b;
	result.y = a.y - b;
	result.z = a.z - b;
	result.w = a.w - b;
#endif
	return result;
}

inlnfunc vec4_t
vec4_mul(vec4_t a, vec4_t b) {
	vec4_t result;
#if BASE_USE_SIMD
	result.sse = _mm_mul_ps(a.sse, b.sse);
#else
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	result.w = a.w * b.w;
#endif
	return result;
}

inlnfunc vec4_t
vec4_mul(vec4_t a, f32 b) {
	vec4_t result;
#if BASE_USE_SIMD
	__m128 _b = _mm_set1_ps(b);
	result.sse = _mm_mul_ps(a.sse, _b);
#else
	result.x = a.x * b;
	result.y = a.y * b;
	result.z = a.z * b;
	result.w = a.w * b;
#endif
	return result;
}

inlnfunc vec4_t
vec4_mul(vec4_t a, mat4_t b) {
	vec4_t result;
#if BASE_USE_SIMD
	result.sse = _mm_mul_ps(_mm_shuffle_ps(a.sse, a.sse, 0x00), b.columns[0].sse);
	result.sse = _mm_add_ps(result.sse, _mm_mul_ps(_mm_shuffle_ps(a.sse, a.sse, 0x55), b.columns[1].sse));
	result.sse = _mm_add_ps(result.sse, _mm_mul_ps(_mm_shuffle_ps(a.sse, a.sse, 0xaa), b.columns[2].sse));
	result.sse = _mm_add_ps(result.sse, _mm_mul_ps(_mm_shuffle_ps(a.sse, a.sse, 0xff), b.columns[3].sse));
#else
	result.x = a.data[0] * b.columns[0].x;
	result.y = a.data[0] * b.columns[0].y;
	result.z = a.data[0] * b.columns[0].z;
	result.w = a.data[0] * b.columns[0].w;
    
	result.x += a.data[1] * b.columns[1].x;
	result.y += a.data[1] * b.columns[1].y;
	result.z += a.data[1] * b.columns[1].z;
	result.w += a.data[1] * b.columns[1].w;
    
	result.x += a.data[2] * b.columns[2].x;
	result.y += a.data[2] * b.columns[2].y;
	result.z += a.data[2] * b.columns[2].z;
	result.w += a.data[2] * b.columns[2].w;
    
	result.x += a.data[3] * b.columns[3].x;
	result.y += a.data[3] * b.columns[3].y;
	result.z += a.data[3] * b.columns[3].z;
	result.w += a.data[3] * b.columns[3].w;
#endif
	return result;
}

inlnfunc vec4_t
vec4_div(vec4_t a, vec4_t b) {
	vec4_t result;
#if BASE_USE_SIMD
	result.sse = _mm_div_ps(a.sse, b.sse);
#else
	result.x = a.x / b.x;
	result.y = a.y / b.y;
	result.z = a.z / b.z;
	result.w = a.w / b.w;
#endif
	return result;
}

inlnfunc vec4_t
vec4_div(vec4_t a, f32 b) {
	vec4_t result;
#if BASE_USE_SIMD
	__m128 _b = _mm_set1_ps(b);
	result.sse = _mm_div_ps(a.sse, _b);
#else
	result.x = a.x / b;
	result.y = a.y / b;
	result.z = a.z / b;
	result.w = a.w / b;
#endif
	return result;
}

inlnfunc b8 
vec4_equals(vec4_t a, vec4_t b) {
	return (a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w);
}

inlnfunc f32
vec4_dot(vec4_t a, vec4_t b) {
	f32 result;
    
#if BASE_USE_SIMD
	// TODO: check
	//__m128 product = _mm_mul_ps(a.sse, b.sse);
	//__m128 sum = _mm_hadd_ps(product, product);
	//sum = _mm_hadd_ps(sum, sum);
	//result = _mm_cvtss_f32(sum);
    
	__m128 sseresultone = _mm_mul_ps(a.sse, b.sse);
	__m128 sseresulttwo = _mm_shuffle_ps(sseresultone, sseresultone, _MM_SHUFFLE(2, 3, 0, 1));
	sseresultone = _mm_add_ps(sseresultone, sseresulttwo);
	sseresulttwo = _mm_shuffle_ps(sseresultone, sseresultone, _MM_SHUFFLE(0, 1, 2, 3));
	sseresultone = _mm_add_ps(sseresultone, sseresulttwo);
	_mm_store_ss(&result, sseresultone);
#else
	result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
#endif
	return result;
}

inlnfunc f32
vec4_length(vec4_t a) {
	return sqrtf(vec4_dot(a,a));
}

inlnfunc vec4_t
vec4_normalize(vec4_t a) {
	return vec4_mul(a, 1.0f / vec4_length(a));
}

inlnfunc vec4_t
vec4_lerp(vec4_t a, vec4_t b, f32 t) {
	return vec4_add(vec4_mul(a, 1.0f - t), vec4_mul(b, t));
}

inlnfunc f32
vec4_angle_between(vec4_t a, vec4_t b) {
	return acosf(vec4_dot(a, b) / (vec4_length(a) * vec4_length(b)));
}

inlnfunc vec4_t
vec4_project(vec4_t a, vec4_t b) {
	return vec4_mul(b, vec4_dot(a, b) / vec4_dot(b, b));
}

//- quat

inlnfunc quat_t
quat(f32 x, f32 y, f32 z, f32 w) {
	quat_t result;
#if BASE_USE_SIMD
	result.sse = _mm_setr_ps(x, y, z, w);
#else
	result.x = x;
	result.y = y;
	result.z = z;
	result.w = w;
#endif
	return result;
}

inlnfunc quat_t 
quat(vec4_t v) {
	quat_t result;
#if BASE_USE_SIMD
	result.sse = v.sse;
#else
	result.x = v.x;
	result.y = v.y;
	result.z = v.z;
	result.w = v.w;
#endif 
	return result;
}

inlnfunc quat_t
quat_from_axis_angle(vec3_t axis, f32 angle) {
	quat_t result;
    
	vec3_t axis_normalized = vec3_normalize(axis);
	f32 sine_of_rotation = sinf(angle / 2.0f);
    
	result.xyz = vec3_mul(axis_normalized, sine_of_rotation);
	result.w = cosf(angle / 2.0f);
    
	return result;
}

inlnfunc quat_t
quat_from_euler_angle(vec3_t euler) {
	quat_t result = { 0.0f };
    
	f32 cy = cosf(euler.z * 0.5f);
	f32 sy = sinf(euler.z * 0.5f);
	f32 cp = cosf(euler.y * 0.5f);
	f32 sp = sinf(euler.y * 0.5f);
	f32 cr = cosf(euler.x * 0.5f);
	f32 sr = sinf(euler.x * 0.5f);
    
	result.w = cr * cp * cy + sr * sp * sy;
	result.x = sr * cp * cy - cr * sp * sy;
	result.y = cr * sp * cy + sr * cp * sy;
	result.z = cr * cp * sy - sr * sp * cy;
    
	return result;
}

inlnfunc vec3_t
quat_to_euler_angle(quat_t quat) {
    
	vec3_t result;
    
	const f32 xx = quat.x;
	const f32 yy = quat.y;
	const f32 zz = quat.z;
	const f32 ww = quat.w;
	const f32 xsq = xx * xx;
	const f32 ysq = yy * yy;
	const f32 zsq = zz * zz;
    
	return vec3(
                atan2f(2.0f * (xx * ww - yy * zz), 1.0f - 2.0f * (xsq + zsq)),
                atan2f(2.0f * (yy * ww + xx * zz), 1.0f - 2.0f * (ysq + zsq)),
                asinf(2.0f * (xx * yy + zz * ww))
                );
}

inlnfunc vec3_t
quat_to_dir(quat_t q) {
	vec3_t dir;
	dir.x = -(2.0f * (q.x * q.z + q.w * q.y));
	dir.y = -(2.0f * (q.y * q.z - q.w * q.x));
	dir.z = -(1.0f - 2.0f * (q.x * q.x + q.y * q.y));
	return dir;
}

inlnfunc quat_t
quat_add(quat_t a, quat_t b) {
	quat_t result;
#if BASE_USE_SIMD
	result.sse = _mm_add_ps(a.sse, b.sse);
#else
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	result.w = a.w + b.w;
#endif
	return result;
}

inlnfunc quat_t
quat_sub(quat_t a, quat_t b) {
	quat_t result;
#if BASE_USE_SIMD
	result.sse = _mm_sub_ps(a.sse, b.sse);
#else
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	result.w = a.w - b.w;
#endif
	return result;
}

inlnfunc quat_t
quat_mul(quat_t a, quat_t b) {
	quat_t result;
#if BASE_USE_SIMD
	__m128 sseresultone = _mm_xor_ps(_mm_shuffle_ps(a.sse, a.sse, _MM_SHUFFLE(0, 0, 0, 0)), _mm_setr_ps(0.0f, -0.0f, 0.0f, -0.0f));
	__m128 sseresulttwo = _mm_shuffle_ps(b.sse, b.sse, _MM_SHUFFLE(0, 1, 2, 3));
	__m128 sseresultthree = _mm_mul_ps(sseresulttwo, sseresultone);
    
	sseresultone = _mm_xor_ps(_mm_shuffle_ps(a.sse, a.sse, _MM_SHUFFLE(1, 1, 1, 1)), _mm_setr_ps(0.0f, 0.0f, -0.0f, -0.0f));
	sseresulttwo = _mm_shuffle_ps(b.sse, b.sse, _MM_SHUFFLE(1, 0, 3, 2));
	sseresultthree = _mm_add_ps(sseresultthree, _mm_mul_ps(sseresulttwo, sseresultone));
    
	sseresultone = _mm_xor_ps(_mm_shuffle_ps(a.sse, a.sse, _MM_SHUFFLE(2, 2, 2, 2)), _mm_setr_ps(-0.0f, 0.0f, 0.0f, -0.0f));
	sseresulttwo = _mm_shuffle_ps(b.sse, b.sse, _MM_SHUFFLE(2, 3, 0, 1));
	sseresultthree = _mm_add_ps(sseresultthree, _mm_mul_ps(sseresulttwo, sseresultone));
    
	sseresultone = _mm_shuffle_ps(a.sse, a.sse, _MM_SHUFFLE(3, 3, 3, 3));
	sseresulttwo = _mm_shuffle_ps(b.sse, b.sse, _MM_SHUFFLE(3, 2, 1, 0));
	result.sse = _mm_add_ps(sseresultthree, _mm_mul_ps(sseresulttwo, sseresultone));
#else
	result.x = b.data[3] * +a.data[0];
	result.y = b.data[2] * -a.data[0];
	result.z = b.data[1] * +a.data[0];
	result.w = b.data[0] * -a.data[0];
    
	result.x += b.data[2] * +a.data[1];
	result.y += b.data[3] * +a.data[1];
	result.z += b.data[0] * -a.data[1];
	result.w += b.data[1] * -a.data[1];
    
	result.x += b.data[1] * -a.data[2];
	result.y += b.data[0] * +a.data[2];
	result.z += b.data[3] * +a.data[2];
	result.w += b.data[2] * -a.data[2];
    
	result.x += b.data[0] * +a.data[3];
	result.y += b.data[1] * +a.data[3];
	result.z += b.data[2] * +a.data[3];
	result.w += b.data[3] * +a.data[3];
#endif
	return result;
}

inlnfunc quat_t
quat_mul(quat_t a, f32 b) {
	quat_t result;
#if BASE_USE_SIMD
	__m128 _b = _mm_set1_ps(b);
	result.sse = _mm_mul_ps(a.sse, _b);
#else
	result.x = a.x * b;
	result.y = a.y * b;
	result.z = a.z * b;
	result.w = a.w * b;
#endif
	return result;
}

inlnfunc quat_t
quat_div(quat_t a, f32 b) {
	quat_t result;
#if BASE_USE_SIMD
	__m128 _b = _mm_set1_ps(b);
	result.sse = _mm_div_ps(a.sse, _b);
#else
	result.x = a.x / b;
	result.y = a.y / b;
	result.z = a.z / b;
	result.w = a.w / b;
#endif
	return result;
}

inlnfunc f32
quat_dot(quat_t a, quat_t b) {
    
	f32 result;
#if BASE_USE_SIMD
	__m128 sseresultone = _mm_mul_ps(a.sse, b.sse);
	__m128 sseresulttwo = _mm_shuffle_ps(sseresultone, sseresultone, _MM_SHUFFLE(2, 3, 0, 1));
	sseresultone = _mm_add_ps(sseresultone, sseresulttwo);
	sseresulttwo = _mm_shuffle_ps(sseresultone, sseresultone, _MM_SHUFFLE(0, 1, 2, 3));
	sseresultone = _mm_add_ps(sseresultone, sseresulttwo);
	_mm_store_ss(&result, sseresultone);
#else
	result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
#endif
    
	return result;
}

inlnfunc quat_t
quat_inverse(quat_t q) {
	quat_t result;
	result.x = -q.x;
	result.y = -q.y;
	result.z = -q.z;
	result.w = q.w;
	return quat_div(result, (quat_dot(q, q)));
}

inlnfunc f32
quat_length(quat_t q) {
	return sqrtf(quat_dot(q, q));
}

inlnfunc quat_t
quat_normalize(quat_t q) {
	quat_t result = q;
	f32 inv_len = 1.0f / quat_length(q);
	if (inv_len > 0.0f) {
		result = quat_mul(q, 1.0f / quat_length(q));
	}
	return result;
}

inlnfunc quat_t
quat_negate(quat_t a) {
	return { -a.x, -a.y, -a.z, -a.w };
}

inlnfunc quat_t
quat_mix(quat_t a, f32 t_a, quat_t b, f32 t_b) {
    
	quat_t result;
    
#if BASE_USE_SIMD
	__m128 scalar_a = _mm_set1_ps(t_a);
	__m128 scalar_b = _mm_set1_ps(t_b);
	__m128 sse_result_one = _mm_mul_ps(a.sse, scalar_a);
	__m128 sse_result_two = _mm_mul_ps(b.sse, scalar_b);
	result.sse = _mm_add_ps(sse_result_one, sse_result_two);
#else 
	result.x = a.x * t_a + b.x * t_b;
	result.y = a.y * t_a + b.y * t_b;
	result.z = a.z * t_a + b.z * t_b;
	result.w = a.w * t_a + b.w * t_b;
#endif
	return result;
}

inlnfunc quat_t
quat_lerp(quat_t a, quat_t b, f32 t) {
	return quat_normalize(quat_mix(a, 1.0f - t, b, t));
}

inlnfunc quat_t
quat_slerp(quat_t a, quat_t b, f32 t) {
    
	quat_t result;
    
	f32 cos_theta = quat_dot(a, b);
    
	if (cos_theta < 0.0f) {
		cos_theta = -cos_theta;
		b = quat(-b.x, -b.y, -b.z, -b.w);
	}
    
	if (cos_theta > 0.9995f) {
		result = quat_lerp(a, b, t);
	} else {
		f32 angle = acosf(cos_theta);
		f32 mix_a = sinf((1.0f - t) * angle);
		f32 mix_b = sinf(t * angle);
        
		result = quat_normalize(quat_mix(a, mix_a, b, mix_b));
	}
    
	return result;
}


//- mat3

inlnfunc mat3_t
mat3(f32 v = 1.0f) {
	mat3_t result = { 0.0f };
	result[0][0] = v;
	result[1][1] = v;
	result[2][2] = v;
	return result;
}

//- mat4

inlnfunc mat4_t
mat4(f32 v = 1.0f) {
	mat4_t result = { 0.0f };
	result[0][0] = v;
	result[1][1] = v;
	result[2][2] = v;
	result[3][3] = v;
	return result;
}

inlnfunc mat4_t
mat4_transpose(mat4_t m) {
	mat4_t result;
#if BASE_USE_SIMD
	result = m;
	__m128 _tmp3, _tmp2, _tmp1, _tmp0; 
	_tmp0 = _mm_shuffle_ps(result.columns[0].sse, result.columns[1].sse, 0x44); 
	_tmp2 = _mm_shuffle_ps(result.columns[0].sse, result.columns[1].sse, 0xee); 
	_tmp1 = _mm_shuffle_ps(result.columns[2].sse, result.columns[3].sse, 0x44); 
	_tmp3 = _mm_shuffle_ps(result.columns[2].sse, result.columns[3].sse, 0xee); 
	result.columns[0].sse = _mm_shuffle_ps(_tmp0, _tmp1, 0x88);
	result.columns[1].sse = _mm_shuffle_ps(_tmp0, _tmp1, 0xdd); 
	result.columns[2].sse = _mm_shuffle_ps(_tmp2, _tmp3, 0x88); 
	result.columns[3].sse = _mm_shuffle_ps(_tmp2, _tmp3, 0xdd); 
#else
	result[0][0] = m[0][0];
	result[0][1] = m[1][0];
	result[0][2] = m[2][0];
	result[0][3] = m[3][0];
	result[1][0] = m[0][1];
	result[1][1] = m[1][1];
	result[1][2] = m[2][1];
	result[1][3] = m[3][1];
	result[2][0] = m[0][2];
	result[2][1] = m[1][2];
	result[2][2] = m[2][2];
	result[2][3] = m[3][2];
	result[3][0] = m[0][3];
	result[3][1] = m[1][3];
	result[3][2] = m[2][3];
	result[3][3] = m[3][3];
#endif 
    
	return result;
}

inlnfunc mat4_t
mat4_add(mat4_t a, mat4_t b) {
	mat4_t result;
	result.columns[0] = vec4_add(a.columns[0], b.columns[0]);
	result.columns[1] = vec4_add(a.columns[1], b.columns[1]);
	result.columns[2] = vec4_add(a.columns[2], b.columns[2]);
	result.columns[3] = vec4_add(a.columns[3], b.columns[3]);
	return result;
}

inlnfunc mat4_t
mat4_sub(mat4_t a, mat4_t b) {
	mat4_t result;
	result.columns[0] = vec4_sub(a.columns[0], b.columns[0]);
	result.columns[1] = vec4_sub(a.columns[1], b.columns[1]);
	result.columns[2] = vec4_sub(a.columns[2], b.columns[2]);
	result.columns[3] = vec4_sub(a.columns[3], b.columns[3]);
	return result;
}

inlnfunc mat4_t 
mat4_mul(mat4_t a, mat4_t b) {
	mat4_t result;
    
	result.columns[0] = vec4_mul(b.columns[0], a);
	result.columns[1] = vec4_mul(b.columns[1], a);
	result.columns[2] = vec4_mul(b.columns[2], a);
	result.columns[3] = vec4_mul(b.columns[3], a);
	
	return result;
}

inlnfunc mat4_t
mat4_mul(mat4_t a, f32 b) {
	mat4_t result;
    
#if BASE_USE_SIMD
	__m128 scalar = _mm_set1_ps(b);
	result.columns[0].sse = _mm_mul_ps(a.columns[0].sse, scalar);
	result.columns[1].sse = _mm_mul_ps(a.columns[1].sse, scalar);
	result.columns[2].sse = _mm_mul_ps(a.columns[2].sse, scalar);
	result.columns[3].sse = _mm_mul_ps(a.columns[3].sse, scalar);
#else 
	result[0][0] = a[0][0] * b;
	result[0][1] = a[0][1] * b;
	result[0][2] = a[0][2] * b;
	result[0][3] = a[0][3] * b;
	result[1][0] = a[1][0] * b;
	result[1][1] = a[1][1] * b;
	result[1][2] = a[1][2] * b;
	result[1][3] = a[1][3] * b;
	result[2][0] = a[2][0] * b;
	result[2][1] = a[2][1] * b;
	result[2][2] = a[2][2] * b;
	result[2][3] = a[2][3] * b;
	result[3][0] = a[3][0] * b;
	result[3][1] = a[3][1] * b;
	result[3][2] = a[3][2] * b;
	result[3][3] = a[3][3] * b;
#endif 
	return result;
}

inlnfunc vec4_t
mat4_mul(mat4_t a, vec4_t b) {
	return vec4_mul(b, a);
}

inlnfunc mat4_t
mat4_div(mat4_t a, f32 b) {
	mat4_t result;
    
#if BASE_USE_SIMD
	__m128 scalar = _mm_set1_ps(b);
	result.columns[0].sse = _mm_div_ps(a.columns[0].sse, scalar);
	result.columns[1].sse = _mm_div_ps(a.columns[1].sse, scalar);
	result.columns[2].sse = _mm_div_ps(a.columns[2].sse, scalar);
	result.columns[3].sse = _mm_div_ps(a.columns[3].sse, scalar);
#else 
	result[0][0] = a[0][0] / b;
	result[0][1] = a[0][1] / b;
	result[0][2] = a[0][2] / b;
	result[0][3] = a[0][3] / b;
	result[1][0] = a[1][0] / b;
	result[1][1] = a[1][1] / b;
	result[1][2] = a[1][2] / b;
	result[1][3] = a[1][3] / b;
	result[2][0] = a[2][0] / b;
	result[2][1] = a[2][1] / b;
	result[2][2] = a[2][2] / b;
	result[2][3] = a[2][3] / b;
	result[3][0] = a[3][0] / b;
	result[3][1] = a[3][1] / b;
	result[3][2] = a[3][2] / b;
	result[3][3] = a[3][3] / b;
#endif 
	return result;
}

inlnfunc f32
mat4_det(mat4_t m) {
    
	vec3_t c01 = vec3_cross(m.columns[0].xyz, m.columns[1].xyz);
	vec3_t c23 = vec3_cross(m.columns[2].xyz, m.columns[3].xyz);
	vec3_t b10 = vec3_sub(vec3_mul(m.columns[0].xyz, m.columns[1].w), vec3_mul(m.columns[1].xyz, m.columns[0].w));
	vec3_t b32 = vec3_sub(vec3_mul(m.columns[2].xyz, m.columns[3].w), vec3_mul(m.columns[3].xyz, m.columns[2].w));
    
	return vec3_dot(c01, b32) + vec3_dot(c23, b10);
}

inlnfunc mat4_t 
mat4_inverse(mat4_t m) {
    
	vec3_t c01 = vec3_cross(m.columns[0].xyz, m.columns[1].xyz);
	vec3_t c23 = vec3_cross(m.columns[2].xyz, m.columns[3].xyz);
	vec3_t b10 = vec3_sub(vec3_mul(m.columns[0].xyz, m.columns[1].w), vec3_mul(m.columns[1].xyz, m.columns[0].w));
	vec3_t b32 = vec3_sub(vec3_mul(m.columns[2].xyz, m.columns[3].w), vec3_mul(m.columns[3].xyz, m.columns[2].w));
    
	f32 inv_determinant = 1.0f / (vec3_dot(c01, b32) + vec3_dot(c23, b10));
	c01 = vec3_mul(c01, inv_determinant);
	c23 = vec3_mul(c23, inv_determinant);
	b10 = vec3_mul(b10, inv_determinant);
	b32 = vec3_mul(b32, inv_determinant);
    
	mat4_t result;
	result.columns[0] = vec4(vec3_add(vec3_cross(m.columns[1].xyz, b32), vec3_mul(c23, m.columns[1].w)), -vec3_dot(m.columns[1].xyz, c23));
	result.columns[1] = vec4(vec3_sub(vec3_cross(b32, m.columns[0].xyz), vec3_mul(c23, m.columns[0].w)), +vec3_dot(m.columns[0].xyz, c23));
	result.columns[2] = vec4(vec3_add(vec3_cross(m.columns[3].xyz, b10), vec3_mul(c01, m.columns[3].w)), -vec3_dot(m.columns[3].xyz, c01));
	result.columns[3] = vec4(vec3_sub(vec3_cross(b10, m.columns[2].xyz), vec3_mul(c01, m.columns[2].w)), +vec3_dot(m.columns[2].xyz, c01));
    
	return mat4_transpose(result);
}

inlnfunc mat4_t 
mat4_inv_perspective(mat4_t m) {
    
	mat4_t result = { 0 };
    
	result[0][0] = 1.0f / m[0][0];
	result[1][1] = 1.0f / m[1][1];
	result[2][2] = 0.0f;
    
	result[2][3] = 1.0f / m[3][2];
	result[3][3] = m[2][2] * -result[2][3];
	result[3][2] = m[2][3];
    
	return result;
}

inlnfunc mat4_t
mat4_from_quat(quat_t q) {
	mat4_t result;
    
	quat_t normalizedq = quat_normalize(q);
    
	f32 xx, yy, zz,
    xy, xz, yz,
    wx, wy, wz;
    
	xx = normalizedq.x * normalizedq.x;
	yy = normalizedq.y * normalizedq.y;
	zz = normalizedq.z * normalizedq.z;
	xy = normalizedq.x * normalizedq.y;
	xz = normalizedq.x * normalizedq.z;
	yz = normalizedq.y * normalizedq.z;
	wx = normalizedq.w * normalizedq.x;
	wy = normalizedq.w * normalizedq.y;
	wz = normalizedq.w * normalizedq.z;
    
	result[0][0] = 1.0f - 2.0f * (yy + zz);
	result[0][1] = 2.0f * (xy + wz);
	result[0][2] = 2.0f * (xz - wy);
	result[0][3] = 0.0f;
    
	result[1][0] = 2.0f * (xy - wz);
	result[1][1] = 1.0f - 2.0f * (xx + zz);
	result[1][2] = 2.0f * (yz + wx);
	result[1][3] = 0.0f;
    
	result[2][0] = 2.0f * (xz + wy);
	result[2][1] = 2.0f * (yz - wx);
	result[2][2] = 1.0f - 2.0f * (xx + yy);
	result[2][3] = 0.0f;
    
	result[3][0] = 0.0f;
	result[3][1] = 0.0f;
	result[3][2] = 0.0f;
	result[3][3] = 1.0f;
    
	return result;
}

inlnfunc mat4_t
mat4_translate(vec3_t translate) {
	mat4_t result = mat4(1.0f);
	result[3][0] = translate.x;
	result[3][1] = translate.y;
	result[3][2] = translate.z;
	return result;
}

inlnfunc mat4_t
mat4_translate(mat4_t m, vec3_t t) {
	mat4_t result = m;
	result[0][3] = t.x;
	result[1][3] = t.y;
	result[2][3] = t.z;
	return result;
}

inlnfunc mat4_t
mat4_scale(vec3_t scale) {
	mat4_t result = mat4(1.0f);
	result[0][0] = scale.x;
	result[1][1] = scale.y;
	result[2][2] = scale.z;
	return result;
}

inlnfunc mat4_t
mat4_orthographic(f32 left, f32 right, f32 top, f32 bottom, f32 z_near, f32 z_far) {
	mat4_t r = { 0 };
	return r;
}

inlnfunc mat4_t
mat4_perspective(f32 fov, f32 ar, f32 n, f32 f) {
    
	mat4_t result = {0};
	
	f32 theta = tanf(radians(fov) / 2.0f);
	result[0][0] = 1.0f / theta;
	result[1][1] = ar / theta;
	result[2][2] = -(n + f) / (n - f);
	result[2][3] = 1.0f;
	result[3][2] = (2.0f * n * f) / (n - f);
    
	return result;
}

inlnfunc mat4_t
mat4_lookat(vec3_t from, vec3_t to, vec3_t up) {
    
	vec3_t f = vec3_normalize(vec3_sub(to, from));
	vec3_t r = vec3_normalize(vec3_cross(f, up));
	vec3_t u = vec3_cross(r, f);
    
	mat4_t v = { 0 };
    
	return v;
}

//- rect

inlnfunc rect_t
rect(f32 x0, f32 y0, f32 x1, f32 y1) {
	return { x0, y0, x1, y1 };
}

inlnfunc rect_t
rect(vec2_t p0, vec2_t p1) {
	return { p0.x, p0.y, p1.x, p1.y };
}

inlnfunc b8 
rect_equals(rect_t a, rect_t b) {
	return (a.x0 == b.x0 && a.y0 == b.y0 && a.x1 == b.x1 && a.y1 == b.y1);
}

inlnfunc void
rect_validate(rect_t& r) {
	if (r.x0 > r.x1) {
		f32 temp = r.x0;
		r.x0 = r.x1;
		r.x1 = temp;
	}
	if (r.y0 > r.y1) {
		f32 temp = r.y0;
		r.y0 = r.y1;
		r.y1 = temp;
	}
}

inlnfunc b8
rect_contains(rect_t r, vec2_t p) {
	return (p.x > r.x0 && p.x < r.x1 && p.y > r.y0 && p.y < r.y1);
}

inlnfunc b8
rect_contains(rect_t a, rect_t b) {
	return (a.x0 <= b.x0 && a.x1 >= b.x1 && a.y0 <= b.y0 && a.y1 >= b.y1);
}

inlnfunc rect_t
rect_intersection(rect_t a, rect_t b) {
	return { max(a.x0, b.x0), max(a.y0, b.y0), min(a.x1, b.x1), min(a.y1, b.y1) };
}

inlnfunc f32
rect_width(rect_t r) {
	return fabsf(r.x1 - r.x0);
}

inlnfunc f32
rect_height(rect_t r) {
	return fabsf(r.y1 - r.y0);
}

inlnfunc vec2_t
rect_size(rect_t r) {
	return vec2(rect_width(r), rect_height(r));
}

inlnfunc vec2_t
rect_center(rect_t r) {
	return { (r.x0 + r.x1) * 0.5f, (r.y0 + r.y1) * 0.5f };
}

inlnfunc rect_t
rect_center(rect_t a, rect_t b) {
	f32 a_width = a.x1 - a.x0;
	f32 a_height = a.y1 - a.y0;
	f32 b_width = b.x1 - b.x0;
	f32 b_height = b.y1 - b.y0;
    
	f32 a_center_x = a.x0 + a_width / 2.0f;
	f32 a_center_y = a.y0 + a_height / 2.0f;
    
	b.x0 = a_center_x - b_width / 2.0f;
	b.y0 = a_center_y - b_height / 2.0f;
	b.x1 = b.x0 + b_width;
	b.y1 = b.y0 + b_height;
    
	return b;
}

inlnfunc rect_t
rect_grow(rect_t r, f32 a) {
	return { r.x0 - a, r.y0 - a, r.x1 + a, r.y1 + a };
}

inlnfunc rect_t
rect_grow(rect_t r, vec2_t a) {
	return { r.x0 - a.x, r.y0 - a.y, r.x1 + a.x, r.y1 + a.y };
}

inlnfunc rect_t
rect_shrink(rect_t r, f32 a) {
	return { r.x0 + a, r.y0 + a, r.x1 - a, r.y1 - a };
}

inlnfunc rect_t
rect_shrink(rect_t r, vec2_t a) {
	return { r.x0 + a.x, r.y0 + a.y, r.x1 - a.x, r.y1 - a.y };
}

inlnfunc rect_t
rect_translate(rect_t r, f32 a) {
	return { r.x0 + a, r.y0 + a, r.x1 + a, r.y1 + a };
}

inlnfunc rect_t
rect_translate(rect_t r, vec2_t a) {
	return { r.x0 + a.x, r.y0 + a.y, r.x1 + a.x, r.y1 + a.y };
}

inlnfunc rect_t
rect_bbox(vec2_t* points, u32 count) {
	rect_t result = { f32_max, f32_max, f32_min, f32_min };
    
	for (i32 i = 0; i < count; i++) {
		vec2_t p = points[i];
        
		if (p.x < result.x0) { result.x0 = p.x; }
		if (p.y < result.y0) { result.y0 = p.y; }
		if (p.x > result.x1) { result.x1 = p.x; }
		if (p.y > result.y1) { result.y1 = p.y; }
	}
    
	return result;
}

inlnfunc rect_t
rect_round(rect_t r) {
	return { roundf(r.x0), roundf(r.y0), roundf(r.x1), roundf(r.y1) };
}

inlnfunc rect_t
rect_lerp(rect_t a, rect_t b, f32 t) {
    vec4_t v_a = vec4(a.x0, a.y0, a.x1, a.y1);
    vec4_t v_b = vec4(b.x0, b.y0, b.x1, b.y1);
    vec4_t v_l = vec4_lerp(v_a, v_b, t);
    return {v_l.x, v_l.y, v_l.z, v_l.w};
}

inlnfunc rect_t
rect_cut_top(rect_t r, f32 a) {
    return { r.x0, r.y0, r.x1, min(r.y1, r.y0 + a) };
}

inlnfunc rect_t
rect_cut_bottom(rect_t r, f32 a) {
    return { r.x0, max(r.y0, r.y1 - a), r.x1, r.y1 };
}

inlnfunc rect_t
rect_cut_left(rect_t r, f32 a) {
    return { r.x0, r.y0, min(r.x1, r.x0 + a), r.y1 };
}

inlnfunc rect_t
rect_cut_right(rect_t r, f32 a) {
    return { max(r.x0, r.x1 - a), r.y0, r.x1, r.y1 };
}


//- color 

inlnfunc color_t
color(u32 hex) {
	color_t result = { 0 };
	result.r = (f32)((hex & 0xff000000) >> 24) / 255.0f;
	result.g = (f32)((hex & 0x00ff0000) >> 16) / 255.0f;
	result.b = (f32)((hex & 0x0000ff00) >> 8) / 255.0f;
	result.a = (f32)((hex & 0x000000ff) >> 0) / 255.0f;
	return result;
}

inlnfunc color_t
color(f32 r, f32 g, f32 b, f32 a) {
	color_t col;
	col.r = r;
	col.g = g;
	col.b = b;
	col.a = a;
	return col;
}

inlnfunc color_t
color_add(color_t a, f32 b) {
	return { clamp_01(a.r + b), clamp_01(a.g + b), clamp_01(a.b + b), clamp_01(a.a + b) };
}

inlnfunc color_t
color_add(color_t a, color_t b) {
	return { clamp_01(a.r + b.r), clamp_01(a.g + b.g), clamp_01(a.b + b.b), clamp_01(a.a + b.a) };
}

inlnfunc color_t
color_lerp(color_t a, color_t b, f32 t) {
	return {
		lerp(a.r, b.r, t),
		lerp(a.g, b.g, t),
		lerp(a.b, b.b, t),
		lerp(a.a, b.a, t)
	};
}

inlnfunc color_t
color_rgb_to_hsv(color_t rgb) {
    
	f32 c_max = max(rgb.r, max(rgb.g, rgb.b));
	f32 c_min = min(rgb.r, min(rgb.g, rgb.b));
	f32 delta = c_max - c_min;
    
	f32 h = (
             (delta == 0.0f) ? 0.0f :
             (c_max == rgb.r) ? fmodf((rgb.g - rgb.b) / delta + 6.0f, 6.0f) :
             (c_max == rgb.g) ? (rgb.b - rgb.r) / delta + 2.0f :
             (c_max == rgb.b) ? (rgb.r - rgb.g) / delta + 4.0f :
             0.0f
             );
	f32 s = (c_max == 0.0f) ? 0.0f : (delta / c_max);
	f32 v = c_max;
    
	color_t hsv_color;
	hsv_color.h = h / 6.0f;
	hsv_color.s = s;
	hsv_color.v = v;
	hsv_color.a = rgb.a;
	return hsv_color;
    
}

inlnfunc color_t
color_hsv_to_rgb(color_t hsv) {
	
	f32 h = fmodf(hsv.h * 360.0f, 360.0f);
	f32 s = hsv.s;
	f32 v = hsv.v;
    
	f32 c = v * s;
	f32 x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
	f32 m = v - c;
    
	f32 r = 0.0f;
	f32 g = 0.0f;
	f32 b = 0.0f;
    
	if ((h >= 0.0f && h < 60.0f) || (h >= 360.0f && h < 420.0f)) {
		r = c;
		g = x;
		b = 0.0f;
	} else if (h >= 60.0f && h < 120.0f) {
		r = x;
		g = c;
		b = 0.0f;
	} else if (h >= 120.0f && h < 180.0f) {
		r = 0.0f;
		g = c;
		b = x;
	} else if (h >= 180.0f && h < 240.0f) {
		r = 0.0f;
		g = x;
		b = c;
	} else if (h >= 240.0f && h < 300.0f) {
		r = x;
		g = 0.0f;
		b = c;
	} else if ((h >= 300.0f && h <= 360.0f) || (h >= -60.0f && h <= 0.0f)) {
		r = c;
		g = 0.0f;
		b = x;
	}
    
	color_t rgb_color;
	rgb_color.r = clamp_01(r + m);
	rgb_color.g = clamp_01(g + m);
	rgb_color.b = clamp_01(b + m);
	rgb_color.a = hsv.a;
    
	return rgb_color;
}

function color_t 
color_blend(color_t src, color_t dst, color_blend_mode mode) {
	color_t result = { 0 };
    
	switch (mode) {
		case color_blend_mode_normal: {
			f32 result_a = dst.a + (1 - dst.a) * src.a;
            
			result.r = (dst.r * dst.a + src.r * src.a * (1.0f - dst.a)) / result_a;
			result.g = (dst.g * dst.a + src.g * src.a * (1.0f - dst.a)) / result_a;
			result.b = (dst.b * dst.a + src.b * src.a * (1.0f - dst.a)) / result_a;
			result.a = result_a;
            
			break;
		}
        
		case color_blend_mode_mul: {
			result.r = src.r * dst.r;
			result.g = src.g * dst.g;
			result.b = src.b * dst.b;
			result.a = src.a * dst.a;
			break;
		}
        
		case color_blend_mode_add: {
			result.r = src.r + dst.r;
			result.g = src.g + dst.g;
			result.b = src.b + dst.b;
			result.a = src.a + dst.a;
			break;
		}
        
		case color_blend_mode_overlay: {
			// TODO: 
			break;
		}
        
	}
    
	return result;
    
}

inlnfunc u32 
color_to_hex(color_t color) {
	u32 hex = 0;
	hex |= ((u32)(color.r * 255.0f) & 0xFF) << 24;
	hex |= ((u32)(color.g * 255.0f) & 0xFF) << 16;
	hex |= ((u32)(color.b * 255.0f) & 0xFF) << 8;
	hex |= ((u32)(color.a * 255.0f) & 0xFF);
	return hex;
}

//- complex 

inlnfunc complex_t
complex(f32 real, f32 imag) {
    complex_t result;
    result.real = real;
    result.imag = imag;
    return result;
}

inlnfunc complex_t 
complex_add(complex_t a, complex_t b) {
    complex_t result;
    result.real = a.real + b.real;
    result.imag = a.imag + b.imag;
	return result;
}

inlnfunc complex_t 
complex_sub(complex_t a, complex_t b) {
    complex_t result;
    result.real = a.real - b.real;
    result.imag = a.imag - b.imag;
	return result;
}

inlnfunc complex_t 
complex_mul(complex_t a, complex_t b) {
    complex_t result;
    result.real = a.real * b.real - a.imag * b.imag;
    result.imag =  a.imag * b.real + a.real * b.imag;
	return result;
}

inlnfunc complex_t 
complex_div(complex_t a, complex_t b) {
    complex_t result;
	f32 denominator = (b.real * b.real) + (b.imag * b.imag);
    result.real = ((a.real * b.real) + (a.imag * b.imag)) / denominator;
    result.imag = ((a.imag * b.real) - (a.real * b.imag)) / denominator;
	return result;
}

inlnfunc f32 
complex_modulus(complex_t a) {
    f32 result = sqrtf(a.real * a.real + a.imag * a.imag);
	return result;
}

inlnfunc complex_t 
complex_conjugate(complex_t a) {
    complex_t result;
    result.real = a.real;
    result.imag = -a.imag;
    return result;
}

inlnfunc f32 
complex_argument(complex_t a) {
    f32 result = atan2f(a.imag, a.real);
	return result;
}

inlnfunc complex_t 
complex_exponential(f32 angle) {
    complex_t result;
    result.real = cosf(angle);        
    result.imag = sinf(angle);        
    return result;
}

//- misc functions

function vec3_t
barycentric(vec2_t p, vec2_t a, vec2_t b, vec2_t c) {
    
	vec2_t v0 = vec2_sub(b, a);
	vec2_t v1 = vec2_sub(c, a);
	vec2_t v2 = vec2_sub(p, a);
    
	f32 denom = v0.x * v1.y - v1.x * v0.y;
    
	f32 v = (v2.x * v1.y - v1.x * v2.y) / denom;
	f32 w = (v0.x * v2.y - v2.x * v0.y) / denom;
	f32 u = 1.0f - v - w;
    
	return vec3(u, v, w);
    
}

function b8
tri_contains(vec2_t a, vec2_t b, vec2_t c, vec2_t p) {
    
	vec2_t v0 = vec2_sub(b, a);
	vec2_t v1 = vec2_sub(c, a);
	vec2_t v2 = vec2_sub(p, a);
    
	f32 d00 = vec2_dot(v0, v0);
	f32 d01 = vec2_dot(v0, v1);
	f32 d11 = vec2_dot(v1, v1);
	f32 d20 = vec2_dot(v2, v0);
	f32 d21 = vec2_dot(v2, v1);
    
	f32 denom = d00 * d11 - d01 * d01;
    
	f32 u = (d11 * d20 - d01 * d21) / denom;
	f32 v = (d00 * d21 - d01 * d20) / denom;
    
	return (u >= 0) && (v >= 0) && (u + v <= 1);
}


function void 
fft(f32* in, complex_t* out, u32 num) {
    
    // bit-reverse the input
    i32 r = (i32)floorf(log2(num));   
    for (i32 k = 0; k < num; k++) {
        i32 l = 0;
        i32 j = k;
        for(i32 i = 0; i < r; i++) {
            l = (l << 1) + (j & 1);
            j >>= 1;
        }
        out[l].real = in[k];
        out[l].imag = 0.0f;
    }
    
    i32 step = 1;
    i32 p;
    i32 q;
    complex_t w;
    complex_t wkn;
    complex_t w_;
    for (i32 k = 0; k < r; k++) {
        for (i32 l = 0; l < num; l += 2 * step) {
            
            w = complex_exponential(-f32_pi / step);
            wkn = complex(1.0f, 0.0f);
            
            for (i32 n = 0; n < step; n++) {
                p = l + n;
                q = p + step;
                w_ = complex_mul(out[q], wkn);
                out[q] = complex_sub(out[p], w_);
                w_ = complex_mul(out[p], complex(2.0f, 0.0f));
                out[p] = complex_sub(w_, out[q]);
                wkn = complex_mul(wkn, w);
            }
        }
        step <<= 1;
    }
}



#endif // SORA_BASE_CPP