// sora_audio.h

#ifndef SORA_AUDIO_H
#define SORA_AUDIO_H

//~ typedefs

typedef void audio_callback_func(void*, u32);

//~ enums

enum audio_format {
	audio_format_f32,
	audio_format_i16,
};

enum audio_device_type {
    audio_device_type_output,
    audio_device_type_input,
    audio_device_type_duplex,
    audio_device_type_loopback,
};

enum audio_share_mode {
    audio_share_mode_shared,
    audio_share_mode_exclusive,
};

//~ structs

// handles
struct audio_device_t { u64 id; };
struct audio_stream_t { u64 id; };

struct audio_stream_params_t {
	u32 frequency; // 44100
	audio_format format; // audio_format_f32
	u32 channels; // 2
	u32 samples; // 256
	audio_callback_func* callback_function;
};


//~ functions

// state (implemented per backend)
function void audio_init();
function void audio_release();

// devices (implemented per backend)
function audio_device_t audio_device_get_default(audio_device_type type = audio_device_type_output);
function audio_device_t audio_device_get(audio_device_type type, u32 index);

function u32 audio_device_get_count(audio_device_type type = audio_device_type_output);
function str_t audio_device_get_name(audio_device_t device_handle);


//~ per backend includes

#if OS_BACKEND_WIN32
#    define AUD_BACKEND_WASAPI 1
#    include "backends/audio/sora_audio_wasapi.h"
#elif OS_BACKEND_MACOS
#    define AUD_BACKEND_CORE_AUDIO 1
#    error audio backend not implemented.
#elif OS_BACKEND_LINUX
#    define AUD_BACKEND_ALSA 1
#    error audio backend not implemented.
#endif 

#endif // SORA_AUDIO_H