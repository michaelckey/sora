// sora_audio_wasapi.cpp

#ifndef SORA_AUDIO_WASAPI_CPP
#define SORA_AUDIO_WASAPI_CPP

//~ libs

#pragma comment(lib, "ole32")

//~ implementation

//- state functions

function void
audio_init() {
    //audio_state.params = params;
    
    audio_wasapi_state.arena = arena_create(gigabytes(1));
    
    // initalize COM
    CoInitializeEx(0, COINIT_MULTITHREADED);
    
    // create enumerator
    CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&audio_wasapi_state.device_enumerator);
    
    // set format
    //WAVEFORMATEXTENSIBLE mix_format_ex;
    //mix_format_ex.Format.nChannels = params.channels;
    //mix_format_ex.Format.nSamplesPerSec = params.frequency;
    //mix_format_ex.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    //mix_format_ex.Format.wBitsPerSample = 32;
    //mix_format_ex.Format.nBlockAlign = (mix_format_ex.Format.nChannels * mix_format_ex.Format.wBitsPerSample) / 8;
    //mix_format_ex.Format.nAvgBytesPerSec = mix_format_ex.Format.nSamplesPerSec * mix_format_ex.Format.nBlockAlign;
    //mix_format_ex.Format.cbSize = 22;
    //mix_format_ex.Samples.wValidBitsPerSample = 32;
    //if (params.channels == 1) {
    //mix_format_ex.dwChannelMask = SPEAKER_FRONT_CENTER;
    //} else {
    //mix_format_ex.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
    //}
    //mix_format_ex.SubFormat = { 0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };
    
    //REFERENCE_TIME dur = (REFERENCE_TIME)(((f64)params.samples) / (((f64)params.frequency) * (1.0 / 10000000.0)));
    
    //audio_state.audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY, dur, 0, (WAVEFORMATEX*)&mix_format_ex, 0);
    //audio_state.audio_client->GetBufferSize(&audio_state.buffer_size);
    //audio_state.audio_client->GetService(__uuidof(IAudioRenderClient), (void**)(&audio_state.audio_render_client));
    
    //audio_state.buffer_ready = CreateEventA(nullptr, FALSE, FALSE, nullptr);
    //audio_state.audio_client->SetEventHandle(audio_state.buffer_ready);
    
    // start audio thread
    atomic_i32_assign(&audio_wasapi_state.thread_state, 1);
    audio_wasapi_state.audio_thread = os_thread_create(audio_wasapi_thread_entry_point, nullptr);
    os_thread_set_name(audio_wasapi_state.audio_thread, str("audio_thread"));
    
}

function void
audio_release() {
    
    // stop audio thread
    atomic_i32_assign(&audio_wasapi_state.thread_state, 0);
    os_thread_join(audio_wasapi_state.audio_thread);
    
    // release 
    //if (audio_state.audio_render_client) { audio_state.audio_render_client->Release(); }
    //if (audio_state.audio_client) { audio_state.audio_client->Release(); }
    //if (audio_state.audio_device) { audio_state.audio_device->Release(); }
    //if (audio_state.device_enumerator) { audio_state.device_enumerator->Release(); }
    
    arena_release(audio_wasapi_state.arena);
    
}

function u32
audio_device_get_count(audio_device_type type) {
    
    // determine device flow
    EDataFlow wasapi_data_flow;
    switch (type) {
        case audio_device_type_input: { wasapi_data_flow = eCapture; break; }
        case audio_device_type_output: { wasapi_data_flow = eRender; break; }
        case audio_device_type_duplex: { wasapi_data_flow = eAll; break; }
    }
    
    IMMDeviceCollection* device_collection = nullptr;
    audio_wasapi_state.device_enumerator->EnumAudioEndpoints(wasapi_data_flow, DEVICE_STATE_ACTIVE, &device_collection);
    
    UINT count;
    device_collection->GetCount(&count);
    device_collection->Release();
    
    return count;
}









//- device functions 

function audio_device_t 
audio_device_get_default(audio_device_type type) {
    temp_t scratch = scratch_begin();
    
    // allocate device
    audio_wasapi_device_t* device = audio_wasapi_device_alloc();
    device->type = type;
    
    EDataFlow wasapi_data_flow;
    switch (type) {
        case audio_device_type_input: { wasapi_data_flow = eCapture; break; }
        case audio_device_type_output: { wasapi_data_flow = eRender; break; }
        case audio_device_type_duplex: { wasapi_data_flow = eAll; break; }
    }
    
    // get default device
    audio_wasapi_state.device_enumerator->GetDefaultAudioEndpoint(wasapi_data_flow, eConsole, &device->device);
    
    // get name
    IPropertyStore *props = NULL;
    PROPVARIANT var_name;
    PropVariantInit(&var_name);
    device->device->OpenPropertyStore(STGM_READ, &props);
    props->GetValue(PKEY_Device_FriendlyName, &var_name);
    str16_t wide_name = str16((u16*)var_name.pwszVal);
    str_t name = str_from_str16(scratch.arena, wide_name);
    memcpy(device->name, name.data, name.size);
    PropVariantClear(&var_name);
    
    
    scratch_end(scratch);
    
    audio_device_t device_handle = { (u64)device };
    return device_handle;
}

function audio_device_t
audio_device_get(audio_device_type type, u32 index) {
    
    
    
}




//~ wasapi specific functions

//- entry point

function i32
audio_wasapi_thread_entry_point(void* params) {
    
    //audio_state.audio_client->Start();
    //log_infof("[audio_thread] starting audio thread.");
    
    while (atomic_i32_load(&audio_wasapi_state.thread_state) == 1) {
        
        // yield for now
        os_thread_yield();
        
        //u32 buffer_padding;
        //audio_state.audio_client->GetCurrentPadding(&buffer_padding);
        
        //u32 frame_count = audio_state.buffer_size - buffer_padding;
        //BYTE* buffer = 0;
        
        //audio_state.audio_render_client->GetBuffer(frame_count, &buffer);
        //audio_state.params.callback_function((f32*)buffer, frame_count);
        //audio_state.audio_render_client->ReleaseBuffer(frame_count, 0);
    }
    
    //log_infof("[audio_thread] ending audio thread.");
    //audio_state.audio_client->Stop();
    
    return 0;
}


//- device functions

global audio_wasapi_device_t* 
audio_wasapi_device_alloc() {
    audio_wasapi_device_t* device = audio_wasapi_state.device_free;
    if (device != nullptr) {
        stack_pop(audio_wasapi_state.device_free);
    } else {
        device = (audio_wasapi_device_t*)arena_alloc(audio_wasapi_state.arena, sizeof(audio_wasapi_device_t));
    }
    memset(device, 0, sizeof(audio_wasapi_device_t));
    dll_push_back(audio_wasapi_state.device_first, audio_wasapi_state.device_last, device);
    return device;
}

global void 
audio_wasapi_device_release( audio_wasapi_device_t* device) {
    dll_remove(audio_wasapi_state.device_first, audio_wasapi_state.device_last, device);
    stack_push(audio_wasapi_state.device_free, device);
}


#endif // SORA_AUDIO_WASAPI_CPP