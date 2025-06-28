#pragma once

#include <aurum/dsp/effect_chain.hpp>
#include <aurum/engine/transport.hpp>
#include <aurum/gui/ring_buffer.hpp>
#include <aurum/preset/preset_manager.hpp>

#include <memory>
#include <string>
#include <vector>

namespace aurum::gui {

class AurumGui {
public:
    AurumGui();
    ~AurumGui();

    int run();

private:
    bool init_window();
    void shutdown_window();
    void render_frame();
    void draw_chain_panel();
    void draw_parameter_panel();
    void draw_transport_panel();
    void draw_meter_panel();

    dsp::EffectChain chain_;
    engine::Transport transport_;
    preset::PresetManager presets_;
    LockFreeRingBuffer meter_buffer_{2048};

    void* window_ = nullptr;
    void* imgui_context_ = nullptr;
    bool running_ = true;
    int selected_effect_ = 0;
    std::vector<float> spectrum_;
    std::vector<float> waveform_;
};

}  // namespace aurum::gui
