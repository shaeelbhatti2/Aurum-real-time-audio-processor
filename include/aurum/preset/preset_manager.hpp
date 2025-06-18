#pragma once

#include <aurum/dsp/effect_chain.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace aurum::preset {

struct AutomationPoint {
    double time_seconds = 0.0;
    float normalized_value = 0.0f;
};

struct PresetEffectState {
    std::string type;
    bool enabled = true;
    bool bypass = false;
    float dry_wet = 1.0f;
    std::vector<float> parameters;
    std::vector<AutomationPoint> automation;
};

struct PresetDocument {
    int version = 1;
    std::string name;
    std::vector<PresetEffectState> effects;
};

class PresetManager {
public:
    bool load_file(const std::string& path, PresetDocument& out, std::string& error) const;
    bool save_file(const std::string& path, const PresetDocument& doc, std::string& error) const;

    void apply_to_chain(const PresetDocument& doc, dsp::EffectChain& chain) const;

    std::vector<std::string> list_factory_presets(const std::string& directory) const;
};

}  // namespace aurum::preset
