#pragma once

#include <cmath>
#include <string>

namespace aurum::dsp {

enum class ParameterType { Float, Int, Bool, Choice };

struct ParameterDescriptor {
    int id = 0;
    std::string name;
    ParameterType type = ParameterType::Float;
    float min_value = 0.0f;
    float max_value = 1.0f;
    float default_value = 0.0f;
    float skew = 1.0f;
    std::string unit;
};

inline float map_normalized_to_value(const ParameterDescriptor& desc, float normalized) {
    const float clamped = normalized < 0.0f ? 0.0f : (normalized > 1.0f ? 1.0f : normalized);
    if (desc.skew != 1.0f && desc.min_value >= 0.0f && desc.max_value > 0.0f) {
        const float scaled = std::pow(clamped, desc.skew);
        return desc.min_value + scaled * (desc.max_value - desc.min_value);
    }
    return desc.min_value + clamped * (desc.max_value - desc.min_value);
}

inline float map_value_to_normalized(const ParameterDescriptor& desc, float value) {
    if (desc.max_value <= desc.min_value) {
        return 0.0f;
    }
    const float t = (value - desc.min_value) / (desc.max_value - desc.min_value);
    const float clamped = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
    if (desc.skew != 1.0f && desc.min_value >= 0.0f && desc.max_value > 0.0f) {
        return std::pow(clamped, 1.0f / desc.skew);
    }
    return clamped;
}

}  // namespace aurum::dsp
