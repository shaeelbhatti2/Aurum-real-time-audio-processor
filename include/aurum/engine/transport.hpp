#pragma once

#include <aurum/io/wav_file.hpp>

namespace aurum::engine {

enum class TransportState { Stopped, Playing, Paused };

class Transport {
public:
    void load(io::WavData data) {
        data_ = std::move(data);
        position_ = 0;
        state_ = TransportState::Stopped;
    }

    const io::WavData& data() const { return data_; }
    bool has_source() const { return !data_.samples.empty(); }

    TransportState state() const { return state_; }
    void play() {
        if (has_source()) {
            state_ = TransportState::Playing;
        }
    }
    void pause() { state_ = TransportState::Paused; }
    void stop() {
        state_ = TransportState::Stopped;
        position_ = 0;
    }

    void seek_frames(int frame) {
        const int max_frame = total_frames();
        position_ = frame < 0 ? 0 : (frame > max_frame ? max_frame : frame);
    }

    int position_frames() const { return position_; }
    int total_frames() const {
        if (data_.channels <= 0) {
            return 0;
        }
        return static_cast<int>(data_.samples.size() / data_.channels);
    }

    int read_frames(float* output, int frames, int channels) {
        if (state_ != TransportState::Playing || !has_source()) {
            return 0;
        }

        int produced = 0;
        const int total = total_frames();
        while (produced < frames && position_ < total) {
            for (int ch = 0; ch < channels; ++ch) {
                const int src_ch = ch < data_.channels ? ch : data_.channels - 1;
                const float sample =
                    data_.samples[static_cast<std::size_t>(position_) *
                                      static_cast<std::size_t>(data_.channels) +
                                  static_cast<std::size_t>(src_ch)];
                output[static_cast<std::size_t>(produced) *
                           static_cast<std::size_t>(channels) +
                       static_cast<std::size_t>(ch)] = sample;
            }
            ++position_;
            ++produced;
        }

        if (position_ >= total) {
            stop();
        }
        return produced;
    }

private:
    io::WavData data_{};
    int position_ = 0;
    TransportState state_ = TransportState::Stopped;
};

}  // namespace aurum::engine
