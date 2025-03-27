// sora_audio_wasapi.cpp

#ifndef SORA_AUDIO_WASAPI_CPP
#define SORA_AUDIO_WASAPI_CPP

//- include libs

#pragma comment(lib, "ole32")

//- implementation

//- state functions

function void
audio_init(audio_params_t params) {
    audio_state.params = params;
    
    // initalize wasapi
    CoInitializeEx(0, COINIT_MULTITHREADED);
    CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&audio_state.device_enumerator);
    audio_state.device_enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &audio_state.audio_device);
    audio_state.audio_device->Activate(__uuidof(IAudioClient3), CLSCTX_INPROC_SERVER, nullptr, (void**)&audio_state.audio_client);
    
    // set format
    WAVEFORMATEXTENSIBLE mix_format_ex;
    mix_format_ex.Format.nChannels = params.channels;
    mix_format_ex.Format.nSamplesPerSec = params.frequency;
    mix_format_ex.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    mix_format_ex.Format.wBitsPerSample = 32;
    mix_format_ex.Format.nBlockAlign = (mix_format_ex.Format.nChannels * mix_format_ex.Format.wBitsPerSample) / 8;
    mix_format_ex.Format.nAvgBytesPerSec = mix_format_ex.Format.nSamplesPerSec * mix_format_ex.Format.nBlockAlign;
    mix_format_ex.Format.cbSize = 22;
    mix_format_ex.Samples.wValidBitsPerSample = 32;
    if (params.channels == 1) {
        mix_format_ex.dwChannelMask = SPEAKER_FRONT_CENTER;
    } else {
        mix_format_ex.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
    }
    mix_format_ex.SubFormat = { 0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };
    
    REFERENCE_TIME dur = (REFERENCE_TIME)(((f64)params.samples) / (((f64)params.frequency) * (1.0 / 10000000.0)));
    
    audio_state.audio_client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY, dur, 0, (WAVEFORMATEX*)&mix_format_ex, 0);
    audio_state.audio_client->GetBufferSize(&audio_state.buffer_size);
    audio_state.audio_client->GetService(__uuidof(IAudioRenderClient), (void**)(&audio_state.audio_render_client));
    
    audio_state.buffer_ready = CreateEventA(nullptr, FALSE, FALSE, nullptr);
    audio_state.audio_client->SetEventHandle(audio_state.buffer_ready);
    
    // start audio thread
    atomic_u32_assign(&audio_state.thread_running, 1);
    audio_state.audio_thread = os_thread_create(audio_thread_function, nullptr);
    os_thread_set_name(audio_state.audio_thread, str("audio_thread"));
    
}

function void
audio_release() {
    
    // stop audio thread
    atomic_u32_assign(&audio_state.thread_running, 0);
    os_thread_join(audio_state.audio_thread, u64_max);
    
    // release 
    if (audio_state.audio_render_client) { audio_state.audio_render_client->Release(); }
    if (audio_state.audio_client) { audio_state.audio_client->Release(); }
    if (audio_state.audio_device) { audio_state.audio_device->Release(); }
    if (audio_state.device_enumerator) { audio_state.device_enumerator->Release(); }
    
}

function void
audio_thread_function(void* params) {
    
    audio_state.audio_client->Start();
    log_infof("[audio_thread] starting audio thread.");
    
    while (atomic_u32_load(&audio_state.thread_running)) {
        
        u32 buffer_padding;
        audio_state.audio_client->GetCurrentPadding(&buffer_padding);
        
        u32 frame_count = audio_state.buffer_size - buffer_padding;
        BYTE* buffer = 0;
        
        audio_state.audio_render_client->GetBuffer(frame_count, &buffer);
        audio_state.params.callback_function((f32*)buffer, frame_count);
        audio_state.audio_render_client->ReleaseBuffer(frame_count, 0);
    }
    
    log_infof("[audio_thread] ending audio thread.");
    audio_state.audio_client->Stop();
}

#endif // SORA_AUDIO_WASAPI_CPP