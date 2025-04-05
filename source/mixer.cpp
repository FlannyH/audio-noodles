#include "mixer.hpp"
#include "processor.hpp"
#include "processors/wav_osc.hpp"

#include <cmath>
#include <cstdio>
#include <vector>
#include <memory>
#include <portaudio.h>

namespace Mixer {
    PaStream* stream = NULL;
    const double output_sample_rate = 44100;
    double block_start_time_value = 0.0;
    double global_volume_value = 0.8;

    std::vector<std::shared_ptr<Processor>> processors;

    int pa_callback(const void*, void* output_buffer, unsigned long frames_per_buffer, const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags flags, void* user_data) {
        (void)user_data;
        (void)flags;
        (void)time_info;

        memset(output_buffer, 0, sizeof(float) * 2 * frames_per_buffer);
        
        for (auto& processor : processors) {
            processor->process_block(frames_per_buffer, (float*)output_buffer);
        }

        block_start_time_value += (1.0 / output_sample_rate) * frames_per_buffer;

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

    void register_processor(std::shared_ptr<Processor> processor) {
        processors.push_back(processor);
    }

    double sample_rate() {
        return output_sample_rate;
    }
    
    double block_start_time() {
        return block_start_time_value;
    }
    
    double global_volume() {
        return global_volume_value;
    }
}
