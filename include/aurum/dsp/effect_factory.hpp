#pragma once

#include <aurum/dsp/effect.hpp>
#include <aurum/dsp/effects/biquad_filter.hpp>
#include <aurum/dsp/effects/compressor.hpp>
#include <aurum/dsp/effects/delay.hpp>
#include <aurum/dsp/effects/distortion.hpp>
#include <aurum/dsp/effects/limiter.hpp>
#include <aurum/dsp/effects/parametric_eq.hpp>
#include <aurum/dsp/effects/reverb.hpp>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace aurum::dsp {

class BiquadFilterEffectWrapper : public Effect {
public:
    std::string name() const override { return "Biquad Filter"; }

    std::vector<ParameterDescriptor> parameters() const override {
        return {
            {0, "frequency", ParameterType::Float, 20.0f, 20000.0f, 1000.0f, 0.5f, "Hz"},
            {1, "q", ParameterType::Float, 0.1f, 10.0f, 0.707f, 1.0f, ""},
            {2, "mode", ParameterType::Float, 0.0f, 3.0f, 0.0f, 1.0f, ""},
        };
    }

    void prepare(int sample_rate, int max_block_size) override {
        filter_.prepare(sample_rate, max_block_size);
    }

    void reset() override { filter_.reset(); }

    void set_parameter_normalized(int index, float value) override {
        if (index == 0) {
            frequency_ = map_normalized_to_value(parameters()[0], value);
        } else if (index == 1) {
            q_ = map_normalized_to_value(parameters()[1], value);
        } else if (index == 2) {
            mode_index_ = static_cast<int>(map_normalized_to_value(parameters()[2], value));
        }
        apply_params();
    }

    float parameter_normalized(int index) const override {
        if (index == 0) {
            return map_value_to_normalized(parameters()[0], frequency_);
        }
        if (index == 1) {
            return map_value_to_normalized(parameters()[1], q_);
        }
        if (index == 2) {
            return map_value_to_normalized(parameters()[2], static_cast<float>(mode_index_));
        }
        return 0.0f;
    }

    void process(const float* input, float* output, int frames, int channels) override {
        filter_.process(input, output, frames, channels);
        mix_dry_wet(input, output, frames, channels);
    }

private:
    void apply_params() {
        filter_.set_frequency(frequency_);
        filter_.set_q(q_);
        switch (mode_index_) {
            case 0:
                filter_.set_mode(BiquadMode::LowPass);
                break;
            case 1:
                filter_.set_mode(BiquadMode::HighPass);
                break;
            case 2:
                filter_.set_mode(BiquadMode::BandPass);
                break;
            default:
                filter_.set_mode(BiquadMode::Notch);
                break;
        }
    }

    BiquadFilterEffect filter_;
    float frequency_ = 1000.0f;
    float q_ = 0.707f;
    int mode_index_ = 0;
};

class EffectFactory {
public:
    using Creator = std::function<EffectPtr()>;

    EffectFactory() {
        register_type("eq", [] { return std::make_unique<ParametricEqEffect>(); });
        register_type("biquad", [] { return std::make_unique<BiquadFilterEffectWrapper>(); });
        register_type("compressor", [] { return std::make_unique<CompressorEffect>(); });
        register_type("delay", [] { return std::make_unique<DelayEffect>(); });
        register_type("reverb", [] { return std::make_unique<ReverbEffect>(); });
        register_type("distortion", [] { return std::make_unique<DistortionEffect>(); });
        register_type("limiter", [] { return std::make_unique<LimiterEffect>(); });
    }

    void register_type(const std::string& type, Creator creator) {
        creators_[type] = std::move(creator);
    }

    EffectPtr create(const std::string& type) const {
        const auto it = creators_.find(type);
        if (it == creators_.end()) {
            return nullptr;
        }
        return it->second();
    }

    std::vector<std::string> types() const {
        std::vector<std::string> names;
        names.reserve(creators_.size());
        for (const auto& entry : creators_) {
            names.push_back(entry.first);
        }
        return names;
    }

private:
    std::unordered_map<std::string, Creator> creators_;
};

}  // namespace aurum::dsp
