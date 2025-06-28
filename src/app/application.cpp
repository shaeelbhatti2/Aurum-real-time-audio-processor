#include <aurum/app/application.hpp>
#include <aurum/app/session.hpp>
#include <aurum/engine/audio_config.hpp>
#include <aurum/engine/audio_engine.hpp>
#include <aurum/gui/aurum_gui.hpp>
#include <aurum/version.hpp>

#include <iostream>
#include <string>

namespace aurum {

int Application::run(int argc, char** argv) {
    bool gui_mode = false;
    std::string preset_path;
    std::string wav_path;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--gui") {
            gui_mode = true;
        } else if (arg == "--preset" && i + 1 < argc) {
            preset_path = argv[++i];
        } else if (arg == "--wav" && i + 1 < argc) {
            wav_path = argv[++i];
        }
    }

    Session session;
    std::string error;

    if (!preset_path.empty()) {
        if (!session.load_preset_file(preset_path, error)) {
            std::cerr << "preset load failed: " << error << '\n';
            return 1;
        }
    }

    if (!wav_path.empty()) {
        if (!session.load_wav_file(wav_path, error)) {
            std::cerr << "wav load failed: " << error << '\n';
            return 1;
        }
    }

    if (gui_mode) {
        gui::AurumGui gui;
        return gui.run();
    }

    std::cout << kName << " v" << kVersion << '\n';

    AudioConfig config;
    config.sample_rate = 48000;
    config.block_size = 512;
    config.output_channels = 2;

    AudioEngine& engine = session.engine();
    if (!engine.initialize(config)) {
        std::cerr << "audio engine init failed: " << engine.last_error() << '\n';
        return 1;
    }

    if (!session.devices().refresh(error)) {
        std::cerr << "device scan failed: " << error << '\n';
    } else {
        std::cout << "found " << session.devices().devices().size() << " audio devices\n";
    }

    engine.set_process_callback([&session](const float* input, float* output, int frames) {
        session.chain().process(input, output, frames, 2);
    });

    std::cout << "audio engine ready (" << config.sample_rate << " Hz, block "
              << config.block_size << ")\n";
    engine.shutdown();
    return 0;
}

}  // namespace aurum
