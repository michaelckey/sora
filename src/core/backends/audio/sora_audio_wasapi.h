// sora_audio_wasapi.h

#ifndef SORA_AUDIO_WASAPI_H
#define SORA_AUDIO_WASAPI_H

//~ includes

#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>

//~ typedefs 

//~ enums 

//~ structs

struct audio_wasapi_device_t {
    audio_wasapi_device_t* next;
    audio_wasapi_device_t* prev;
    
    char name[256];
    audio_device_type type;
    
    IMMDevice* device;
};

struct audio_wasapi_state_t {
    
    arena_t* arena;
    
    // device list
    audio_wasapi_device_t* device_first;
    audio_wasapi_device_t* device_last;
    audio_wasapi_device_t* device_free;
    
	IMMDeviceEnumerator* device_enumerator;
	IMMDevice* audio_device;
	IAudioClient3* audio_client;
	IAudioRenderClient* audio_render_client;
    
	u32 buffer_size;
	HANDLE buffer_ready;
    
    
    
    
    
    
    
    os_thread_t audio_thread;
    atomic_i32 thread_state; // 0 - exited, 1 - running
};


//~ globals

global audio_wasapi_state_t audio_wasapi_state;

//~ wasapi specific functions

// entry point
function i32 audio_wasapi_thread_entry_point(void* params);

// device
function audio_wasapi_device_t* audio_wasapi_device_alloc(); 
function void audio_wasapi_device_release(audio_wasapi_device_t* device);

#endif // SORA_AUDIO_WASAPI_H