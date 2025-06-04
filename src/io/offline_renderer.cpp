#include <aurum/io/offline_renderer.hpp>

#include <aurum/engine/audio_buffer.hpp>

#include <algorithm>
#include <cstring>

namespace aurum::io {

bool OfflineRenderer::render(const io::WavData& input, dsp::EffectChain& chain,
                             io::WavData& output, ProgressCallback progress,
                             std::string& error) {
    if (input.channels <= 0 || input.sample_rate <= 0 || input.samples.empty()) {
        error = "empty input wav";
        return false;
    }

    chain.prepare(input.sample_rate, block_size_);
    output.sample_rate = input.sample_rate;
    output.channels = input.channels;
    output.samples.assign(input.samples.size(), 0.0f);

    aurum::AudioBuffer in_buf;
    aurum::AudioBuffer out_buf;
    in_buf.resize(block_size_, input.channels);
    out_buf.resize(block_size_, input.channels);

    const int total_frames =
        static_cast<int>(input.samples.size() / static_cast<std::size_t>(input.channels));
    int processed = 0;

    while (processed < total_frames) {
        const int frames =
            std::min(block_size_, total_frames - processed);
        for (int i = 0; i < frames; ++i) {
            for (int ch = 0; ch < input.channels; ++ch) {
                in_buf.at(i, ch) =
                    input.samples[static_cast<std::size_t>(processed + i) *
                                      static_cast<std::size_t>(input.channels) +
                                  static_cast<std::size_t>(ch)];
            }
        }

        chain.process(in_buf.data(), out_buf.data(), frames, input.channels);

        for (int i = 0; i < frames; ++i) {
            for (int ch = 0; ch < input.channels; ++ch) {
                output.samples[static_cast<std::size_t>(processed + i) *
                                   static_cast<std::size_t>(input.channels) +
                               static_cast<std::size_t>(ch)] = out_buf.at(i, ch);
            }
        }

        processed += frames;
        if (progress) {
            progress(static_cast<float>(processed) / static_cast<float>(total_frames));
        }
    }

    return true;
}

}  // namespace aurum::io
