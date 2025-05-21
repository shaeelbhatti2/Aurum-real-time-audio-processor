#pragma once

#include <cmath>

namespace aurum::dsp {

struct BiquadCoeffs {
    float b0 = 1.0f;
    float b1 = 0.0f;
    float b2 = 0.0f;
    float a1 = 0.0f;
    float a2 = 0.0f;
};

struct BiquadState {
    float z1 = 0.0f;
    float z2 = 0.0f;

    void reset() {
        z1 = 0.0f;
        z2 = 0.0f;
    }
};

inline float process_biquad(const BiquadCoeffs& c, BiquadState& s, float x) {
    const float y = c.b0 * x + s.z1;
    s.z1 = c.b1 * x - c.a1 * y + s.z2;
    s.z2 = c.b2 * x - c.a2 * y;
    return y;
}

inline BiquadCoeffs make_lowpass(int sample_rate, float frequency, float q) {
    const float w0 = 2.0f * static_cast<float>(M_PI) * frequency /
                     static_cast<float>(sample_rate);
    const float cos_w0 = std::cos(w0);
    const float sin_w0 = std::sin(w0);
    const float alpha = sin_w0 / (2.0f * q);

    const float b0 = (1.0f - cos_w0) * 0.5f;
    const float b1 = 1.0f - cos_w0;
    const float b2 = (1.0f - cos_w0) * 0.5f;
    const float a0 = 1.0f + alpha;
    const float a1 = -2.0f * cos_w0;
    const float a2 = 1.0f - alpha;

    return {b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0};
}

inline BiquadCoeffs make_highpass(int sample_rate, float frequency, float q) {
    const float w0 = 2.0f * static_cast<float>(M_PI) * frequency /
                     static_cast<float>(sample_rate);
    const float cos_w0 = std::cos(w0);
    const float sin_w0 = std::sin(w0);
    const float alpha = sin_w0 / (2.0f * q);

    const float b0 = (1.0f + cos_w0) * 0.5f;
    const float b1 = -(1.0f + cos_w0);
    const float b2 = (1.0f + cos_w0) * 0.5f;
    const float a0 = 1.0f + alpha;
    const float a1 = -2.0f * cos_w0;
    const float a2 = 1.0f - alpha;

    return {b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0};
}

inline BiquadCoeffs make_bandpass(int sample_rate, float frequency, float q) {
    const float w0 = 2.0f * static_cast<float>(M_PI) * frequency /
                     static_cast<float>(sample_rate);
    const float cos_w0 = std::cos(w0);
    const float sin_w0 = std::sin(w0);
    const float alpha = sin_w0 / (2.0f * q);

    const float b0 = alpha;
    const float b1 = 0.0f;
    const float b2 = -alpha;
    const float a0 = 1.0f + alpha;
    const float a1 = -2.0f * cos_w0;
    const float a2 = 1.0f - alpha;

    return {b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0};
}

inline BiquadCoeffs make_notch(int sample_rate, float frequency, float q) {
    const float w0 = 2.0f * static_cast<float>(M_PI) * frequency /
                     static_cast<float>(sample_rate);
    const float cos_w0 = std::cos(w0);
    const float sin_w0 = std::sin(w0);
    const float alpha = sin_w0 / (2.0f * q);

    const float b0 = 1.0f;
    const float b1 = -2.0f * cos_w0;
    const float b2 = 1.0f;
    const float a0 = 1.0f + alpha;
    const float a1 = -2.0f * cos_w0;
    const float a2 = 1.0f - alpha;

    return {b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0};
}

inline BiquadCoeffs make_peaking(int sample_rate, float frequency, float q, float gain_db) {
    const float a = std::pow(10.0f, gain_db / 40.0f);
    const float w0 = 2.0f * static_cast<float>(M_PI) * frequency /
                     static_cast<float>(sample_rate);
    const float cos_w0 = std::cos(w0);
    const float sin_w0 = std::sin(w0);
    const float alpha = sin_w0 / (2.0f * q);

    const float b0 = 1.0f + alpha * a;
    const float b1 = -2.0f * cos_w0;
    const float b2 = 1.0f - alpha * a;
    const float a0 = 1.0f + alpha / a;
    const float a1 = -2.0f * cos_w0;
    const float a2 = 1.0f - alpha / a;

    return {b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0};
}

}  // namespace aurum::dsp
