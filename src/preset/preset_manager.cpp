#include <aurum/preset/preset_manager.hpp>

#include <aurum/dsp/effect_factory.hpp>
#include <aurum/dsp/effects/compressor.hpp>

#include <fstream>
#include <sstream>

namespace aurum::preset {

namespace {

std::string trim(const std::string& value) {
    const auto start = value.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(start, end - start + 1);
}

std::unique_ptr<dsp::Effect> make_effect(const std::string& type) {
    static dsp::EffectFactory factory;
    return factory.create(type);
}

}  // namespace

bool PresetManager::load_file(const std::string& path, PresetDocument& out,
                              std::string& error) const {
    std::ifstream file(path);
    if (!file) {
        error = "failed to open preset";
        return false;
    }

    out = PresetDocument{};
    PresetEffectState current{};
    bool in_automation = false;

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        if (line.rfind("version=", 0) == 0) {
            out.version = std::stoi(line.substr(8));
        } else if (line.rfind("name=", 0) == 0) {
            out.name = line.substr(5);
        } else if (line == "effect_begin") {
            current = PresetEffectState{};
            in_automation = false;
        } else if (line == "effect_end") {
            out.effects.push_back(current);
        } else if (line.rfind("type=", 0) == 0) {
            current.type = line.substr(5);
        } else if (line.rfind("enabled=", 0) == 0) {
            current.enabled = line.substr(8) == "1";
        } else if (line.rfind("bypass=", 0) == 0) {
            current.bypass = line.substr(7) == "1";
        } else if (line.rfind("dry_wet=", 0) == 0) {
            current.dry_wet = std::stof(line.substr(8));
        } else if (line.rfind("param=", 0) == 0) {
            current.parameters.push_back(std::stof(line.substr(6)));
        } else if (line == "automation_begin") {
            in_automation = true;
        } else if (line == "automation_end") {
            in_automation = false;
        } else if (in_automation && line.rfind("point=", 0) == 0) {
            const auto comma = line.find(',');
            AutomationPoint point;
            point.time_seconds = std::stod(line.substr(6, comma - 6));
            point.normalized_value = std::stof(line.substr(comma + 1));
            current.automation.push_back(point);
        }
    }

    if (out.name.empty()) {
        out.name = "Untitled";
    }
    return true;
}

bool PresetManager::save_file(const std::string& path, const PresetDocument& doc,
                              std::string& error) const {
    std::ofstream file(path);
    if (!file) {
        error = "failed to write preset";
        return false;
    }

    file << "version=" << doc.version << '\n';
    file << "name=" << doc.name << '\n';
    for (const auto& effect : doc.effects) {
        file << "effect_begin\n";
        file << "type=" << effect.type << '\n';
        file << "enabled=" << (effect.enabled ? 1 : 0) << '\n';
        file << "bypass=" << (effect.bypass ? 1 : 0) << '\n';
        file << "dry_wet=" << effect.dry_wet << '\n';
        for (float param : effect.parameters) {
            file << "param=" << param << '\n';
        }
        if (!effect.automation.empty()) {
            file << "automation_begin\n";
            for (const auto& point : effect.automation) {
                file << "point=" << point.time_seconds << ',' << point.normalized_value << '\n';
            }
            file << "automation_end\n";
        }
        file << "effect_end\n";
    }
    return true;
}

void PresetManager::apply_to_chain(const PresetDocument& doc, dsp::EffectChain& chain) const {
    chain.clear();
    for (const auto& effect_state : doc.effects) {
        auto effect = make_effect(effect_state.type);
        if (!effect) {
            continue;
        }
        for (std::size_t i = 0; i < effect_state.parameters.size(); ++i) {
            effect->set_parameter_normalized(static_cast<int>(i), effect_state.parameters[i]);
        }
        effect->set_bypass(effect_state.bypass);
        effect->set_dry_wet(effect_state.dry_wet);
        chain.add(std::move(effect));
    }
}

std::vector<std::string> PresetManager::list_factory_presets(const std::string& directory) const {
    return {"vocal.aurum", "drum.aurum", "master.aurum", "ambient.aurum", "lofi.aurum"};
}

}  // namespace aurum::preset
