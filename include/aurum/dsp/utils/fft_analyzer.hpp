#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

namespace aurum::dsp {

class FftAnalyzer {
public:
    explicit FftAnalyzer(std::size_t size);

    void analyze(const float* samples, std::size_t count);
    const std::vector<float>& magnitudes() const { return magnitudes_; }

private:
    void bit_reverse_permute(std::vector<float>& real, std::vector<float>& imag);
    void compute_fft(std::vector<float>& real, std::vector<float>& imag);

    std::size_t size_ = 0;
    std::vector<float> window_;
    std::vector<float> magnitudes_;
    std::vector<float> real_;
    std::vector<float> imag_;
};

inline FftAnalyzer::FftAnalyzer(std::size_t size) : size_(size) {
    window_.resize(size_);
    magnitudes_.resize(size_ / 2);
    real_.resize(size_);
    imag_.resize(size_);
    for (std::size_t i = 0; i < size_; ++i) {
        window_[i] = 0.5f * (1.0f - std::cos(2.0f * static_cast<float>(M_PI) * static_cast<float>(i) /
                                                     static_cast<float>(size_ - 1)));
    }
}

inline void FftAnalyzer::bit_reverse_permute(std::vector<float>& real, std::vector<float>& imag) {
    std::size_t j = 0;
    for (std::size_t i = 0; i < size_; ++i) {
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
        std::size_t m = size_ >> 1;
        while (m >= 1 && j >= m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    }
}

inline void FftAnalyzer::compute_fft(std::vector<float>& real, std::vector<float>& imag) {
    bit_reverse_permute(real, imag);
    for (std::size_t step = 2; step <= size_; step <<= 1) {
        const float angle = -2.0f * static_cast<float>(M_PI) / static_cast<float>(step);
        const float w_step_re = std::cos(angle);
        const float w_step_im = std::sin(angle);
        for (std::size_t i = 0; i < size_; i += step) {
            float w_re = 1.0f;
            float w_im = 0.0f;
            for (std::size_t j = 0; j < step / 2; ++j) {
                const std::size_t even = i + j;
                const std::size_t odd = i + j + step / 2;
                const float odd_re = w_re * real[odd] - w_im * imag[odd];
                const float odd_im = w_re * imag[odd] + w_im * real[odd];
                real[odd] = real[even] - odd_re;
                imag[odd] = imag[even] - odd_im;
                real[even] += odd_re;
                imag[even] += odd_im;
                const float next_w_re = w_re * w_step_re - w_im * w_step_im;
                w_im = w_re * w_step_im + w_im * w_step_re;
                w_re = next_w_re;
            }
        }
    }
}

inline void FftAnalyzer::analyze(const float* samples, std::size_t count) {
    const std::size_t n = std::min(count, size_);
    for (std::size_t i = 0; i < size_; ++i) {
        const float sample = i < n ? samples[i] : 0.0f;
        real_[i] = sample * window_[i];
        imag_[i] = 0.0f;
    }
    compute_fft(real_, imag_);
    for (std::size_t i = 0; i < size_ / 2; ++i) {
        magnitudes_[i] = std::sqrt(real_[i] * real_[i] + imag_[i] * imag_[i]);
    }
}

}  // namespace aurum::dsp
