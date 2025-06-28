#pragma once

#include <atomic>
#include <cstddef>
#include <vector>

namespace aurum::gui {

class LockFreeRingBuffer {
public:
    explicit LockFreeRingBuffer(std::size_t capacity);

    void push(float left, float right);
    bool pop(float& left, float& right);
    std::size_t size() const;

    void snapshot(std::vector<float>& mono_out, std::size_t max_samples) const;

private:
    struct Sample {
        float left = 0.0f;
        float right = 0.0f;
    };

    std::vector<Sample> buffer_;
    std::size_t capacity_ = 0;
    std::atomic<std::size_t> write_index_{0};
};

}  // namespace aurum::gui
