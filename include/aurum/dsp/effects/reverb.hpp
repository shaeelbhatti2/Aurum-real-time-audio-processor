#pragma once

#include <aurum/dsp/effect.hpp>
#include <aurum/dsp/parameter.hpp>

#include <array>
#include <atomic>
#include <cmath>
#include <string>
#include <vector>

namespace aurum::dsp {

class ReverbEffect : public Effect {
public:
    ReverbEffect() {
        normals_[0].store(map_value_to_normalized(parameters()[0], 0.6f));
        normals_[1].store(map_value_to_normalized(parameters()[1], 0.5f));
        normals_[2].store(map_value_to_normalized(parameters()[2], 0.35f));
    }

    std::string name() const override { return "Reverb"; }

    std::vector<ParameterDescriptor> parameters() const override {
        return {
            {0, "room", ParameterType::Float, 0.0f, 1.0f, 0.6f, 1.0f, ""},
            {1, "damping", ParameterType::Float, 0.0f, 1.0f, 0.5f, 1.0f, ""},
            {2, "mix", ParameterType::Float, 0.0f, 1.0f, 0.35f, 1.0f, ""},
        };
    }

    void prepare(int sample_rate, int /*max_block_size*/) override {
        sample_rate_ = sample_rate;
        for (auto& comb : combs_) {
            comb.buffer.assign(static_cast<std::size_t>(sample_rate_ / 4), 0.0f);
            comb.index = 0;
        }
        for (auto& ap : allpasses_) {
            ap.buffer.assign(static_cast<std::size_t>(sample_rate_ / 16), 0.0f);
            ap.index = 0;
        }
    }

    void reset() override {
        for (auto& comb : combs_) {
            std::fill(comb.buffer.begin(), comb.buffer.end(), 0.0f);
        }
        for (auto& ap : allpasses_) {
            std::fill(ap.buffer.begin(), ap.buffer.end(), 0.0f);
        }
    }

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
        const float room = map_normalized_to_value(parameters()[0], normals_[0].load());
        const float damping = map_normalized_to_value(parameters()[1], normals_[1].load());
        const float mix = map_normalized_to_value(parameters()[2], normals_[2].load());
        const float feedback = 0.55f + room * 0.35f;

        for (int i = 0; i < frames; ++i) {
            float mono = 0.0f;
            for (int ch = 0; ch < channels; ++ch) {
                mono += input[i * channels + ch];
            }
            mono /= static_cast<float>(channels);

            float reverb = 0.0f;
            for (auto& comb : combs_) {
                const float delayed = comb.buffer[static_cast<std::size_t>(comb.index)];
                const float filtered = delayed * (1.0f - damping) + comb.last * damping;
                comb.last = filtered;
                const float next = mono + filtered * feedback;
                comb.buffer[static_cast<std::size_t>(comb.index)] = next;
                comb.index = (comb.index + 1) % static_cast<int>(comb.buffer.size());
                reverb += delayed;
            }

            for (auto& ap : allpasses_) {
                const float buf_out = ap.buffer[static_cast<std::size_t>(ap.index)];
                const float next = reverb + buf_out * 0.5f;
                ap.buffer[static_cast<std::size_t>(ap.index)] = next;
                reverb = buf_out - next * 0.5f;
                ap.index = (ap.index + 1) % static_cast<int>(ap.buffer.size());
            }

            for (int ch = 0; ch < channels; ++ch) {
                const float dry = input[i * channels + ch];
                output[i * channels + ch] = dry * (1.0f - mix) + reverb * mix;
            }
        }
    }

private:
    struct Comb {
        std::vector<float> buffer;
        int index = 0;
        float last = 0.0f;
    };

    struct Allpass {
        std::vector<float> buffer;
        int index = 0;
    };

    int sample_rate_ = 48000;
    std::array<Comb, 4> combs_{};
    std::array<Allpass, 2> allpasses_{};
    std::array<std::atomic<float>, 3> normals_{};
};

}  // namespace aurum::dsp
