#pragma once

#include <aurum/dsp/utils/biquad.hpp>

#include <cmath>

namespace aurum::dsp {

enum class BiquadMode { LowPass, HighPass, BandPass, Notch };

class BiquadFilterEffect {
public:
    void prepare(int sample_rate, int /*max_block_size*/) { sample_rate_ = sample_rate; }
    void reset() { for (auto& state : states_) { state.reset(); } }

    void set_mode(BiquadMode mode) { mode_ = mode; update_coefficients(); }
    void set_frequency(float hz) {
        frequency_ = hz;
        update_coefficients();
    }
    void set_q(float q) {
        q_ = q;
        update_coefficients();
    }

    void process(const float* input, float* output, int frames, int channels) {
        for (int ch = 0; ch < channels; ++ch) {
            BiquadState& state = states_[ch % 2];
            for (int i = 0; i < frames; ++i) {
                const float sample = input[i * channels + ch];
                output[i * channels + ch] = process_biquad(coeffs_, state, sample);
            }
        }
    }

private:
    void update_coefficients() {
        switch (mode_) {
            case BiquadMode::LowPass:
                coeffs_ = make_lowpass(sample_rate_, frequency_, q_);
                break;
            case BiquadMode::HighPass:
                coeffs_ = make_highpass(sample_rate_, frequency_, q_);
                break;
            case BiquadMode::BandPass:
                coeffs_ = make_bandpass(sample_rate_, frequency_, q_);
                break;
            case BiquadMode::Notch:
                coeffs_ = make_notch(sample_rate_, frequency_, q_);
                break;
        }
    }

    int sample_rate_ = 48000;
    BiquadMode mode_ = BiquadMode::LowPass;
    float frequency_ = 1000.0f;
    float q_ = 0.707f;
    BiquadCoeffs coeffs_{};
    BiquadState states_[2]{};
};

}  // namespace aurum::dsp
