#include <aurum/engine/audio_engine.hpp>

#include <aurum/dsp/utils/denormal.hpp>

#include <algorithm>
#include <cstring>

#if AURUM_HAS_PORTAUDIO
#include <portaudio.h>
#endif

namespace aurum {

AudioEngine::AudioEngine() = default;

AudioEngine::~AudioEngine() {
    shutdown();
}

bool AudioEngine::initialize(const AudioConfig& config) {
    shutdown();
    config_ = config;
    scratch_.resize(config_.block_size, config_.output_channels);

#if AURUM_HAS_PORTAUDIO
    const PaError err = Pa_Initialize();
    if (err != paNoError) {
        last_error_ = Pa_GetErrorText(err);
        return false;
    }
    portaudio_initialized_ = true;
    return true;
#else
    last_error_ = "PortAudio not available";
    return false;
#endif
}

void AudioEngine::shutdown() {
    stop();
#if AURUM_HAS_PORTAUDIO
    if (portaudio_initialized_) {
        Pa_Terminate();
        portaudio_initialized_ = false;
    }
#endif
    stream_ = nullptr;
}

void AudioEngine::set_process_callback(AudioProcessCallback callback) {
    callback_ = std::move(callback);
}

bool AudioEngine::start() {
#if !AURUM_HAS_PORTAUDIO
    last_error_ = "PortAudio not available";
    return false;
#else
    if (!portaudio_initialized_) {
        last_error_ = "PortAudio not initialized";
        return false;
    }
    if (running_.load()) {
        return true;
    }

    PaStreamParameters output{};
    output.device = Pa_GetDefaultOutputDevice();
    if (output.device == paNoDevice) {
        last_error_ = "No default output device";
        return false;
    }

    output.channelCount = config_.output_channels;
    output.sampleFormat = paFloat32;
    output.suggestedLatency = Pa_GetDeviceInfo(output.device)->defaultLowOutputLatency;
    output.hostApiSpecificStreamInfo = nullptr;

    const PaStreamParameters* input_params = nullptr;
    PaStreamParameters input{};
    if (config_.input_channels > 0) {
        input.device = Pa_GetDefaultInputDevice();
        if (input.device == paNoDevice) {
            last_error_ = "No default input device";
            return false;
        }
        input.channelCount = config_.input_channels;
        input.sampleFormat = paFloat32;
        input.suggestedLatency = Pa_GetDeviceInfo(input.device)->defaultLowInputLatency;
        input.hostApiSpecificStreamInfo = nullptr;
        input_params = &input;
    }

    PaStream* stream = nullptr;
    const PaError open_err =
        Pa_OpenStream(&stream, input_params, &output, static_cast<double>(config_.sample_rate),
                      static_cast<unsigned long>(config_.block_size), paClipOff, portaudio_callback,
                      this);
    if (open_err != paNoError) {
        last_error_ = Pa_GetErrorText(open_err);
        return false;
    }

    stream_ = stream;
    const PaError start_err = Pa_StartStream(stream);
    if (start_err != paNoError) {
        last_error_ = Pa_GetErrorText(start_err);
        Pa_CloseStream(stream);
        stream_ = nullptr;
        return false;
    }

    running_.store(true);
    return true;
#endif
}

void AudioEngine::stop() {
#if AURUM_HAS_PORTAUDIO
    if (stream_ != nullptr) {
        PaStream* stream = static_cast<PaStream*>(stream_);
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        stream_ = nullptr;
    }
#endif
    running_.store(false);
}

#if AURUM_HAS_PORTAUDIO
int AudioEngine::portaudio_callback(const void* input, void* output, unsigned long frame_count,
                                    const void* /*time_info*/, unsigned long status_flags,
                                    void* user_data) {
    auto* engine = static_cast<AudioEngine*>(user_data);
    if ((status_flags & paInputOverflow) != 0U || (status_flags & paOutputUnderflow) != 0U) {
        engine->xrun_count_.fetch_add(1);
    }

    const auto* in = static_cast<const float*>(input);
    auto* out = static_cast<float*>(output);
    engine->process_block(in, out, static_cast<int>(frame_count));
    return paContinue;
}
#endif

void AudioEngine::process_block(const float* input, float* output, int frames) {
    dsp::flush_denormals_to_zero();

    if (callback_) {
        callback_(input, output, frames);
    } else if (input != nullptr && output != nullptr && input != output) {
        const int channels = config_.output_channels;
        const std::size_t samples =
            static_cast<std::size_t>(frames) * static_cast<std::size_t>(channels);
        std::memcpy(output, input, samples * sizeof(float));
    } else if (output != nullptr) {
        const int channels = config_.output_channels;
        const std::size_t samples =
            static_cast<std::size_t>(frames) * static_cast<std::size_t>(channels);
        std::fill(output, output + samples, 0.0f);
    }

    processed_frames_.fetch_add(static_cast<std::uint64_t>(frames));
}

}  // namespace aurum
