#include <aurum/app/session.hpp>

namespace aurum {

Session::Session() {
    engine::AudioConfig config;
    config.sample_rate = 48000;
    config.block_size = 512;
    config.output_channels = 2;
    engine_.initialize(config);
    chain_.prepare(config.sample_rate, config.block_size);
}

bool Session::load_preset_file(const std::string& path, std::string& error) {
    preset::PresetDocument doc;
    if (!presets_.load_file(path, doc, error)) {
        return false;
    }
    presets_.apply_to_chain(doc, chain_);
    return true;
}

bool Session::load_wav_file(const std::string& path, std::string& error) {
    if (!io::load_wav(path, loaded_wav_, error)) {
        return false;
    }
    transport_.load(loaded_wav_);
    return true;
}

bool Session::render_offline(const std::string& output_path, std::string& error) {
    if (loaded_wav_.samples.empty()) {
        error = "no wav loaded";
        return false;
    }
    io::WavData rendered;
    if (!offline_renderer_.render(loaded_wav_, chain_, rendered,
                                  [](float) {}, error)) {
        return false;
    }
    return io::save_wav(output_path, rendered, error);
}

}  // namespace aurum
