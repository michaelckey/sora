// sora_audio.h

#ifndef SORA_AUDIO_H
#define SORA_AUDIO_H

//- typedefs

typedef void audio_callback_function(f32*, u32);

//- enums

enum audio_format {
	audio_format_f32
};

//- structs

struct audio_params_t {
	u32 frequency = 44100;
	audio_format format = audio_format_f32;
	u32 channels = 2;
	u32 samples = 256;
	audio_callback_function* callback_function;
};

//- functions

// state (implementate per backend)
function void audio_init(audio_params_t params);
function void audio_release();
function void audio_thread_function(void* params);

//- per backend includes

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