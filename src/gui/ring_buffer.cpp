#include <aurum/gui/ring_buffer.hpp>

#include <algorithm>

namespace aurum::gui {

LockFreeRingBuffer::LockFreeRingBuffer(std::size_t capacity)
    : buffer_(capacity), capacity_(capacity) {}

void LockFreeRingBuffer::push(float left, float right) {
    if (capacity_ == 0) {
        return;
    }
    const std::size_t index = write_index_.fetch_add(1) % capacity_;
    buffer_[index] = {left, right};
}

bool LockFreeRingBuffer::pop(float& left, float& right) {
    static thread_local std::size_t read_index = 0;
    const std::size_t write = write_index_.load();
    if (write == 0 || capacity_ == 0) {
        return false;
    }
    read_index = (read_index + 1) % capacity_;
    left = buffer_[read_index].left;
    right = buffer_[read_index].right;
    return true;
}

std::size_t LockFreeRingBuffer::size() const {
    return std::min(write_index_.load(), capacity_);
}

void LockFreeRingBuffer::snapshot(std::vector<float>& mono_out, std::size_t max_samples) const {
    mono_out.clear();
    const std::size_t available = size();
    const std::size_t count = std::min(max_samples, available);
    if (count == 0 || capacity_ == 0) {
        return;
    }

    const std::size_t write = write_index_.load();
    mono_out.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        const std::size_t index = (write + capacity_ - count + i) % capacity_;
        const Sample& sample = buffer_[index];
        mono_out.push_back((sample.left + sample.right) * 0.5f);
    }
}

}  // namespace aurum::gui
