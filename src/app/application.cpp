#include <aurum/app/application.hpp>
#include <aurum/engine/audio_config.hpp>
#include <aurum/engine/audio_engine.hpp>
#include <aurum/version.hpp>

#include <iostream>

namespace aurum {

int Application::run() {
    std::cout << kName << " v" << kVersion << '\n';

    AudioConfig config;
    config.sample_rate = 48000;
    config.block_size = 512;
    config.output_channels = 2;

    AudioEngine engine;
    if (!engine.initialize(config)) {
        std::cerr << "audio engine init failed: " << engine.last_error() << '\n';
        return 1;
    }

    engine.set_process_callback([](const float* /*input*/, float* output, int frames) {
        const int channels = 2;
        for (int i = 0; i < frames * channels; ++i) {
            output[i] = 0.0f;
        }
    });

    std::cout << "audio engine ready (" << config.sample_rate << " Hz, block "
              << config.block_size << ")\n";
    engine.shutdown();
    return 0;
}

}  // namespace aurum
