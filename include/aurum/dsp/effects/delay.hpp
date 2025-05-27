#pragma once

#include <aurum/dsp/effect.hpp>
#include <aurum/dsp/parameter.hpp>

#include <array>
#include <atomic>
#include <cmath>
#include <string>
#include <vector>

namespace aurum::dsp {

class DelayEffect : public Effect {
public:
    DelayEffect() {
        normals_[0].store(map_value_to_normalized(parameters()[0], 250.0f));
        normals_[1].store(map_value_to_normalized(parameters()[1], 0.35f));
        normals_[2].store(map_value_to_normalized(parameters()[2], 0.5f));
        normals_[3].store(map_value_to_normalized(parameters()[3], 120.0f));
    }

    std::string name() const override { return "Delay"; }

    std::vector<ParameterDescriptor> parameters() const override {
        return {
            {0, "time_ms", ParameterType::Float, 1.0f, 2000.0f, 250.0f, 0.5f, "ms"},
            {1, "feedback", ParameterType::Float, 0.0f, 0.95f, 0.35f, 1.0f, ""},
            {2, "mix", ParameterType::Float, 0.0f, 1.0f, 0.5f, 1.0f, ""},
            {3, "bpm", ParameterType::Float, 40.0f, 240.0f, 120.0f, 1.0f, "BPM"},
        };
    }

    void prepare(int sample_rate, int /*max_block_size*/) override {
        sample_rate_ = sample_rate;
        const int max_delay = static_cast<int>(2.0f * sample_rate_);
        buffer_.assign(static_cast<std::size_t>(max_delay), 0.0f);
        write_index_ = 0;
    }

    void reset() override { std::fill(buffer_.begin(), buffer_.end(), 0.0f); }

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
        const float time_ms = map_normalized_to_value(parameters()[0], normals_[0].load());
        const float feedback = map_normalized_to_value(parameters()[1], normals_[1].load());
        const float mix = map_normalized_to_value(parameters()[2], normals_[2].load());
        const int delay_samples =
            std::max(1, static_cast<int>(time_ms * 0.001f * static_cast<float>(sample_rate_)));
        const int buf_size = static_cast<int>(buffer_.size());

        for (int i = 0; i < frames; ++i) {
            for (int ch = 0; ch < channels; ++ch) {
                const float in = input[i * channels + ch];
                const int read_index = (write_index_ - delay_samples + buf_size) % buf_size;
                const float delayed = buffer_[static_cast<std::size_t>(read_index)];
                const float wet = in + delayed * feedback;
                buffer_[static_cast<std::size_t>(write_index_)] = wet;
                output[i * channels + ch] = in * (1.0f - mix) + delayed * mix;
            }
            write_index_ = (write_index_ + 1) % buf_size;
        }
    }

private:
    int sample_rate_ = 48000;
    std::vector<float> buffer_;
    int write_index_ = 0;
    std::array<std::atomic<float>, 4> normals_{};
};

}  // namespace aurum::dsp
