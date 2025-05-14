#pragma once

#include <aurum/dsp/utils/denormal.hpp>
#include <aurum/engine/audio_buffer.hpp>
#include <aurum/engine/audio_config.hpp>

#include <atomic>
#include <functional>
#include <memory>
#include <string>

namespace aurum {

using AudioProcessCallback = std::function<void(const float* input, float* output, int frames)>;

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    bool initialize(const AudioConfig& config);
    void shutdown();

    bool start();
    void stop();

    bool is_running() const { return running_.load(); }

    void set_process_callback(AudioProcessCallback callback);

    const AudioConfig& config() const { return config_; }
    std::uint64_t processed_frames() const { return processed_frames_.load(); }
    std::uint64_t xrun_count() const { return xrun_count_.load(); }

    std::string last_error() const { return last_error_; }

private:
    static int portaudio_callback(const void* input, void* output, unsigned long frame_count,
                                  const void* time_info, unsigned long status_flags,
                                  void* user_data);

    void process_block(const float* input, float* output, int frames);

    AudioConfig config_{};
    AudioProcessCallback callback_;
    AudioBuffer scratch_;

    std::atomic<bool> running_{false};
    std::atomic<std::uint64_t> processed_frames_{0};
    std::atomic<std::uint64_t> xrun_count_{0};

    void* stream_ = nullptr;
    bool portaudio_initialized_ = false;
    std::string last_error_;
};

}  // namespace aurum
