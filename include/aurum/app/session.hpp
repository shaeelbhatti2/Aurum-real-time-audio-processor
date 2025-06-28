#pragma once

#include <aurum/dsp/effect_chain.hpp>
#include <aurum/engine/audio_engine.hpp>
#include <aurum/engine/device_manager.hpp>
#include <aurum/engine/transport.hpp>
#include <aurum/io/offline_renderer.hpp>
#include <aurum/io/wav_file.hpp>
#include <aurum/preset/preset_manager.hpp>
#include <aurum/plugin/plugin_host.hpp>

#include <memory>
#include <string>

namespace aurum {

class Session {
public:
    Session();

    dsp::EffectChain& chain() { return chain_; }
    engine::Transport& transport() { return transport_; }
    engine::AudioEngine& engine() { return engine_; }
    engine::DeviceManager& devices() { return devices_; }
    preset::PresetManager& presets() { return presets_; }
    plugin::PluginHost& plugins() { return plugins_; }
    io::OfflineRenderer& offline_renderer() { return offline_renderer_; }

    bool load_preset_file(const std::string& path, std::string& error);
    bool load_wav_file(const std::string& path, std::string& error);
    bool render_offline(const std::string& output_path, std::string& error);

private:
    dsp::EffectChain chain_;
    engine::Transport transport_;
    engine::AudioEngine engine_;
    engine::DeviceManager devices_;
    preset::PresetManager presets_;
    plugin::PluginHost plugins_;
    io::OfflineRenderer offline_renderer_;
    io::WavData loaded_wav_;
};

}  // namespace aurum
