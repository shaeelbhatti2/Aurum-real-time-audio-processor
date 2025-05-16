#pragma once

#include <aurum/dsp/parameter_descriptor.hpp>

#include <atomic>
#include <cmath>
#include <vector>

namespace aurum::dsp {

class SmoothedParameter {
public:
    SmoothedParameter() = default;

    void configure(const ParameterDescriptor& desc, float smoothing_ms, int sample_rate) {
        desc_ = desc;
        target_.store(map_value_to_normalized(desc, desc.default_value));
        current_ = target_.load();
        const float smoothing = smoothing_ms / 1000.0f;
        if (smoothing <= 0.0f) {
            coeff_ = 1.0f;
        } else {
            coeff_ = 1.0f - std::exp(-1.0f / (smoothing * static_cast<float>(sample_rate)));
        }
    }

    void set_normalized(float value) { target_.store(value); }

    void set_value(float value) { target_.store(map_value_to_normalized(desc_, value)); }

    float value() {
        const float target = target_.load();
        current_ += (target - current_) * coeff_;
        return map_normalized_to_value(desc_, current_);
    }

    float normalized() const { return current_; }

private:
    ParameterDescriptor desc_{};
    std::atomic<float> target_{0.0f};
    float current_ = 0.0f;
    float coeff_ = 1.0f;
};

class ParameterSet {
public:
    explicit ParameterSet(std::vector<ParameterDescriptor> descriptors)
        : descriptors_(std::move(descriptors)) {
        values_.resize(descriptors_.size());
        for (std::size_t i = 0; i < descriptors_.size(); ++i) {
            values_[i].store(map_value_to_normalized(descriptors_[i], descriptors_[i].default_value));
        }
    }

    const std::vector<ParameterDescriptor>& descriptors() const { return descriptors_; }

    void set_normalized(std::size_t index, float value) {
        if (index < values_.size()) {
            values_[index].store(value);
        }
    }

    float normalized(std::size_t index) const {
        return index < values_.size() ? values_[index].load() : 0.0f;
    }

    float value(std::size_t index) const {
        if (index >= descriptors_.size()) {
            return 0.0f;
        }
        return map_normalized_to_value(descriptors_[index], normalized(index));
    }

private:
    std::vector<ParameterDescriptor> descriptors_;
    std::vector<std::atomic<float>> values_;
};

}  // namespace aurum::dsp
