#pragma once

#include <aurum/dsp/effect.hpp>
#include <aurum/dsp/parameter.hpp>

#include <array>
#include <atomic>
#include <cmath>
#include <string>
#include <vector>

namespace aurum::dsp {

class DistortionEffect : public Effect {
public:
    DistortionEffect() {
        normals_[0].store(map_value_to_normalized(parameters()[0], 0.4f));
        normals_[1].store(map_value_to_normalized(parameters()[1], 0.5f));
        normals_[2].store(map_value_to_normalized(parameters()[2], 0.5f));
    }

    std::string name() const override { return "Distortion"; }

    std::vector<ParameterDescriptor> parameters() const override {
        return {
            {0, "drive", ParameterType::Float, 0.0f, 1.0f, 0.4f, 1.0f, ""},
            {1, "tone", ParameterType::Float, 0.0f, 1.0f, 0.5f, 1.0f, ""},
            {2, "mix", ParameterType::Float, 0.0f, 1.0f, 0.5f, 1.0f, ""},
        };
    }

    void prepare(int sample_rate, int /*max_block_size*/) override { sample_rate_ = sample_rate; }

    void reset() override {}

    void set_parameter_normalized(int index, float value) override {
        if (index >= 0 && index < static_cast<int>(normals_.size())) {
            normals_[static_cast<std::size_t>(index)].store(value);
        }
    }

    float parameter_normalized(int index) const override {
        return index >= 0 && index < static_cast<int>(normals_.size())
                   ? normals_[static_cast<std::size_t>(index)].load()
                   : 0.0f;
    }

    void process(const float* input, float* output, int frames, int channels) override {
        const float drive = map_normalized_to_value(parameters()[0], normals_[0].load());
        const float mix = map_normalized_to_value(parameters()[2], normals_[2].load());
        const float gain = 1.0f + drive * 24.0f;

        for (int i = 0; i < frames * channels; ++i) {
            const float dry = input[i];
            const float shaped = std::tanh(dry * gain);
            output[i] = dry * (1.0f - mix) + shaped * mix;
        }
    }

private:
    int sample_rate_ = 48000;
    std::array<std::atomic<float>, 3> normals_{};
};

}  // namespace aurum::dsp
