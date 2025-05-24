#pragma once

#include <aurum/dsp/effect.hpp>
#include <aurum/dsp/parameter.hpp>

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <string>
#include <vector>

namespace aurum::dsp {

class CompressorEffect : public Effect {
public:
    CompressorEffect() {
        normals_[0].store(map_value_to_normalized(parameters()[0], -18.0f));
        normals_[1].store(map_value_to_normalized(parameters()[1], 4.0f));
        normals_[2].store(map_value_to_normalized(parameters()[2], 10.0f));
        normals_[3].store(map_value_to_normalized(parameters()[3], 100.0f));
        normals_[4].store(map_value_to_normalized(parameters()[4], 0.0f));
    }
    std::string name() const override { return "Compressor"; }

    std::vector<ParameterDescriptor> parameters() const override {
        return {
            {0, "threshold", ParameterType::Float, -60.0f, 0.0f, -18.0f, 1.0f, "dB"},
            {1, "ratio", ParameterType::Float, 1.0f, 20.0f, 4.0f, 1.0f, ":1"},
            {2, "attack", ParameterType::Float, 0.1f, 100.0f, 10.0f, 0.5f, "ms"},
            {3, "release", ParameterType::Float, 10.0f, 1000.0f, 100.0f, 0.5f, "ms"},
            {4, "makeup", ParameterType::Float, 0.0f, 24.0f, 0.0f, 1.0f, "dB"},
        };
    }

    void prepare(int sample_rate, int /*max_block_size*/) override {
        sample_rate_ = sample_rate;
        update_coefficients();
        envelope_ = 0.0f;
    }

    void reset() override { envelope_ = 0.0f; }

    void set_parameter_normalized(int index, float value) override {
        if (index >= 0 && index < static_cast<int>(normals_.size())) {
            normals_[static_cast<std::size_t>(index)].store(value);
            update_coefficients();
        }
    }

    float parameter_normalized(int index) const override {
        return index >= 0 && index < static_cast<int>(normals_.size())
                   ? normals_[static_cast<std::size_t>(index)].load()
                   : 0.0f;
    }

    void process(const float* input, float* output, int frames, int channels) override {
        const float threshold = map_normalized_to_value(parameters()[0], normals_[0].load());
        const float ratio = map_normalized_to_value(parameters()[1], normals_[1].load());
        const float makeup = map_normalized_to_value(parameters()[4], normals_[4].load());
        const float makeup_lin = std::pow(10.0f, makeup / 20.0f);
        const float threshold_lin = std::pow(10.0f, threshold / 20.0f);

        for (int i = 0; i < frames; ++i) {
            float peak = 0.0f;
            for (int ch = 0; ch < channels; ++ch) {
                peak = std::max(peak, std::abs(input[i * channels + ch]));
            }

            const float coeff = peak > envelope_ ? attack_coeff_ : release_coeff_;
            envelope_ = peak + coeff * (envelope_ - peak);

            float gain = 1.0f;
            if (envelope_ > threshold_lin && envelope_ > 1.0e-9f) {
                const float env_db = 20.0f * std::log10(envelope_);
                const float over_db = env_db - threshold;
                const float compressed = over_db / ratio;
                const float target_db = threshold + compressed;
                gain = std::pow(10.0f, (target_db - env_db) / 20.0f);
            }

            gain *= makeup_lin;
            for (int ch = 0; ch < channels; ++ch) {
                output[i * channels + ch] = input[i * channels + ch] * gain;
            }
        }
        mix_dry_wet(input, output, frames, channels);
    }

private:
    void update_coefficients() {
        const float attack_ms = map_normalized_to_value(parameters()[2], normals_[2].load());
        const float release_ms = map_normalized_to_value(parameters()[3], normals_[3].load());
        attack_coeff_ = std::exp(-1.0f / (attack_ms * 0.001f * static_cast<float>(sample_rate_)));
        release_coeff_ = std::exp(-1.0f / (release_ms * 0.001f * static_cast<float>(sample_rate_)));
    }

    int sample_rate_ = 48000;
    float envelope_ = 0.0f;
    float attack_coeff_ = 0.0f;
    float release_coeff_ = 0.0f;
    std::array<std::atomic<float>, 5> normals_{};
};

}  // namespace aurum::dsp
