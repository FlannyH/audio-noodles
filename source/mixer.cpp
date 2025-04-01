#include "mixer.hpp"

#include <cmath>
#include <cstdio>
#include <portaudio.h>

namespace Mixer {
    PaStream* stream = NULL;
    const int output_sample_rate = 44100;
    double time = 0.0;
    double global_volume = 0.8f;

    int pa_callback(const void*, void* output_buffer, unsigned long frames_per_buffer, const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags flags, void* user_data) {
        (void)user_data;
        (void)flags;
        (void)time_info;
        
        const double sample_length_sec = (double)1.0 / (double)output_sample_rate;
        float* output = (float*)output_buffer;

        for (size_t i = 0; i < frames_per_buffer; ++i) {
            double sample_l = sin(time * 440.0 * 2.0 * 3.14159265);
            double sample_r = sin(time * 440.0 * 2.0 * 3.14159265);
            output[2*i + 0] = (float)(sample_l * global_volume);
            output[2*i + 1] = (float)(sample_r * global_volume);
            time += sample_length_sec;
        } 

        return paContinue;
    }
    
    void pa_stream_finished(void* user_data) {
        (void)user_data;
        return;
    }

    void init() {
        Pa_Initialize();

        const int device_index = Pa_GetDefaultOutputDevice();
        if (device_index == paNoDevice) { 
            printf("Failed to get audio output device\n");
            return; 
        }

        const PaStreamParameters output_parameters = {
            device_index,
            2,
            paFloat32,
            Pa_GetDeviceInfo(device_index)->defaultLowOutputLatency,
            NULL,
        };

        const PaDeviceInfo* device_info = Pa_GetDeviceInfo(device_index);
        if (device_info != NULL) {
            printf("Audio output device: \"%s\"\n", device_info->name);
        }

        PaError error = Pa_OpenStream(
            &stream,
            NULL,
            &output_parameters,
            output_sample_rate,
            paFramesPerBufferUnspecified,
            paClipOff,
            &pa_callback,
            NULL // todo: userdata?
        );
        if (error != paNoError || stream == NULL) {
            printf("Failed to open audio stream!\n");
            return;
        }

        //Set stream finished callback
        error = Pa_SetStreamFinishedCallback(stream, &pa_stream_finished);
        if (error != paNoError) {
            printf("Error setting up audio stream!\n");
            Pa_CloseStream(stream);
            stream = NULL;
            return;
        }

        error = Pa_StartStream(stream);
        if (error != paNoError) {
            printf("Error setting up audio stream!\n");
            Pa_CloseStream(stream);
            stream = NULL;
            return;
        }
    }
}
