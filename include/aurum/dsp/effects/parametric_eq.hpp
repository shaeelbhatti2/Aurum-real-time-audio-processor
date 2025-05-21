#pragma once

#include <aurum/dsp/effect.hpp>
#include <aurum/dsp/parameter.hpp>
#include <aurum/dsp/utils/biquad.hpp>
#include <aurum/engine/audio_buffer.hpp>

#include <array>
#include <atomic>
#include <cstring>
#include <string>
#include <vector>

namespace aurum::dsp {

class ParametricEqEffect : public Effect {
public:
    ParametricEqEffect() {
        for (auto& n : normals_) {
            n.store(0.5f);
        }
        normals_[0].store(map_value_to_normalized(parameters()[0], 0.0f));
        normals_[1].store(map_value_to_normalized(parameters()[1], 0.0f));
        normals_[2].store(map_value_to_normalized(parameters()[2], 0.0f));
        normals_[3].store(map_value_to_normalized(parameters()[3], 120.0f));
        normals_[4].store(map_value_to_normalized(parameters()[4], 1000.0f));
        normals_[5].store(map_value_to_normalized(parameters()[5], 8000.0f));
    }
    std::string name() const override { return "Parametric EQ"; }

    std::vector<ParameterDescriptor> parameters() const override {
        return {
            {0, "low_gain", ParameterType::Float, -24.0f, 24.0f, 0.0f, 1.0f, "dB"},
            {1, "mid_gain", ParameterType::Float, -24.0f, 24.0f, 0.0f, 1.0f, "dB"},
            {2, "high_gain", ParameterType::Float, -24.0f, 24.0f, 0.0f, 1.0f, "dB"},
            {3, "low_freq", ParameterType::Float, 40.0f, 400.0f, 120.0f, 0.5f, "Hz"},
            {4, "mid_freq", ParameterType::Float, 200.0f, 5000.0f, 1000.0f, 0.5f, "Hz"},
            {5, "high_freq", ParameterType::Float, 2000.0f, 16000.0f, 8000.0f, 0.5f, "Hz"},
        };
    }

    void prepare(int sample_rate, int max_block_size) override {
        sample_rate_ = sample_rate;
        params_.configure(parameters().front(), 20.0f, sample_rate);
        update_filters();
        scratch_.resize(max_block_size, 2);
    }

    void reset() override {
        for (auto& state : states_) {
            state.fill(BiquadState{});
        }
    }

    void set_parameter_normalized(int index, float value) override {
        if (index >= 0 && index < static_cast<int>(normals_.size())) {
            normals_[static_cast<std::size_t>(index)].store(value);
            update_filters();
        }
    }

    float parameter_normalized(int index) const override {
        if (index >= 0 && index < static_cast<int>(normals_.size())) {
            return normals_[static_cast<std::size_t>(index)].load();
        }
        return 0.0f;
    }

    void process(const float* input, float* output, int frames, int channels) override {
        std::memcpy(output, input, static_cast<std::size_t>(frames * channels) * sizeof(float));
        const float gains[3] = {
            map_normalized_to_value(parameters()[0], normals_[0].load()),
            map_normalized_to_value(parameters()[1], normals_[1].load()),
            map_normalized_to_value(parameters()[2], normals_[2].load()),
        };
        const float freqs[3] = {
            map_normalized_to_value(parameters()[3], normals_[3].load()),
            map_normalized_to_value(parameters()[4], normals_[4].load()),
            map_normalized_to_value(parameters()[5], normals_[5].load()),
        };

        for (int band = 0; band < 3; ++band) {
            const BiquadCoeffs coeffs = make_peaking(sample_rate_, freqs[band], 1.0f, gains[band]);
            for (int ch = 0; ch < channels; ++ch) {
                BiquadState& state = states_[band][ch % 2];
                for (int i = 0; i < frames; ++i) {
                    float& sample = output[i * channels + ch];
                    sample = process_biquad(coeffs, state, sample);
                }
            }
        }
        mix_dry_wet(input, output, frames, channels);
    }

private:
    void update_filters() {}

    int sample_rate_ = 48000;
    SmoothedParameter params_{};
    std::array<std::atomic<float>, 6> normals_{};
    std::array<std::array<BiquadState, 2>, 3> states_{};
    AudioBuffer scratch_;
};

}  // namespace aurum::dsp
