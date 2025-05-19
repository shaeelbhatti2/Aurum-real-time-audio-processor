#pragma once

#include <aurum/dsp/parameter.hpp>

#include <memory>
#include <string>
#include <vector>

namespace aurum::dsp {

class Effect {
public:
    virtual ~Effect() = default;

    virtual std::string name() const = 0;
    virtual std::vector<ParameterDescriptor> parameters() const = 0;

    virtual void prepare(int sample_rate, int max_block_size) = 0;
    virtual void reset() = 0;

    virtual void set_parameter_normalized(int index, float value) = 0;
    virtual float parameter_normalized(int index) const = 0;

    virtual void process(const float* input, float* output, int frames, int channels) = 0;

    bool bypass() const { return bypass_.load(); }
    void set_bypass(bool value) { bypass_.store(value); }

    float dry_wet() const { return dry_wet_.load(); }
    void set_dry_wet(float value) { dry_wet_.store(value); }

protected:
    void mix_dry_wet(const float* input, float* output, int frames, int channels) const {
        const float wet = dry_wet();
        const float dry = 1.0f - wet;
        const int samples = frames * channels;
        for (int i = 0; i < samples; ++i) {
            output[i] = input[i] * dry + output[i] * wet;
        }
    }

private:
    std::atomic<bool> bypass_{false};
    std::atomic<float> dry_wet_{1.0f};
};

using EffectPtr = std::unique_ptr<Effect>;

}  // namespace aurum::dsp
