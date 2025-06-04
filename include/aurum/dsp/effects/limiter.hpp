#pragma once

#include <aurum/dsp/effect.hpp>
#include <aurum/dsp/parameter.hpp>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <string>
#include <vector>

namespace aurum::dsp {

class LimiterEffect : public Effect {
public:
    std::string name() const override { return "Limiter"; }

    std::vector<ParameterDescriptor> parameters() const override {
        return {{0, "ceiling", ParameterType::Float, -12.0f, 0.0f, -0.3f, 1.0f, "dB"}};
    }

    void prepare(int /*sample_rate*/, int /*max_block_size*/) override {}

    void reset() override {}

    void set_parameter_normalized(int index, float value) override {
        if (index == 0) {
            ceiling_norm_.store(value);
        }
    }

    float parameter_normalized(int index) const override {
        return index == 0 ? ceiling_norm_.load() : 0.0f;
    }

    void process(const float* input, float* output, int frames, int channels) override {
        const float ceiling_db = map_normalized_to_value(parameters()[0], ceiling_norm_.load());
        const float ceiling = std::pow(10.0f, ceiling_db / 20.0f);
        for (int i = 0; i < frames * channels; ++i) {
            output[i] = std::clamp(input[i], -ceiling, ceiling);
        }
    }

private:
    std::atomic<float> ceiling_norm_{0.95f};
};

}  // namespace aurum::dsp
