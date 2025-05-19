#pragma once

#include <aurum/dsp/effect.hpp>
#include <aurum/engine/audio_buffer.hpp>

#include <vector>

namespace aurum::dsp {

class EffectChain {
public:
    void clear();
    void add(EffectPtr effect);
    void remove(std::size_t index);
    void move(std::size_t from, std::size_t to);

    std::size_t size() const { return effects_.size(); }
    Effect* at(std::size_t index) { return effects_[index].get(); }
    const Effect* at(std::size_t index) const { return effects_[index].get(); }

    void prepare(int sample_rate, int max_block_size);
    void reset();

    void set_chain_bypass(bool value) { chain_bypass_.store(value); }
    bool chain_bypass() const { return chain_bypass_.load(); }

    void process(const float* input, float* output, int frames, int channels);

private:
    std::vector<EffectPtr> effects_;
    AudioBuffer scratch_a_;
    AudioBuffer scratch_b_;
    std::atomic<bool> chain_bypass_{false};
    int sample_rate_ = 48000;
    int block_size_ = 512;
};

}  // namespace aurum::dsp
