#include <aurum/dsp/effect_chain.hpp>

#include <algorithm>
#include <cstring>

namespace aurum::dsp {

void EffectChain::clear() {
    effects_.clear();
}

void EffectChain::add(EffectPtr effect) {
    effects_.push_back(std::move(effect));
}

void EffectChain::remove(std::size_t index) {
    if (index < effects_.size()) {
        effects_.erase(effects_.begin() + static_cast<std::ptrdiff_t>(index));
    }
}

void EffectChain::move(std::size_t from, std::size_t to) {
    if (from >= effects_.size() || to >= effects_.size() || from == to) {
        return;
    }
    auto node = std::move(effects_[from]);
    effects_.erase(effects_.begin() + static_cast<std::ptrdiff_t>(from));
    effects_.insert(effects_.begin() + static_cast<std::ptrdiff_t>(to), std::move(node));
}

void EffectChain::prepare(int sample_rate, int max_block_size) {
    sample_rate_ = sample_rate;
    block_size_ = max_block_size;
    scratch_a_.resize(max_block_size, 2);
    scratch_b_.resize(max_block_size, 2);
    for (auto& effect : effects_) {
        effect->prepare(sample_rate, max_block_size);
    }
}

void EffectChain::reset() {
    for (auto& effect : effects_) {
        effect->reset();
    }
}

void EffectChain::process(const float* input, float* output, int frames, int channels) {
    const int samples = frames * channels;
    if (chain_bypass_.load() || effects_.empty()) {
        if (output != input) {
            std::memcpy(output, input, static_cast<std::size_t>(samples) * sizeof(float));
        }
        return;
    }

    scratch_a_.resize(frames, channels);
    scratch_b_.resize(frames, channels);
    std::memcpy(scratch_a_.data(), input, static_cast<std::size_t>(samples) * sizeof(float));

    float* current_in = scratch_a_.data();
    float* current_out = scratch_b_.data();

    for (auto& effect : effects_) {
        if (effect->bypass()) {
            continue;
        }
        effect->process(current_in, current_out, frames, channels);
        std::swap(current_in, current_out);
    }

    if (current_in != output) {
        std::memcpy(output, current_in, static_cast<std::size_t>(samples) * sizeof(float));
    }
}

}  // namespace aurum::dsp
