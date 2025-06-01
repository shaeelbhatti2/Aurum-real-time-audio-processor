#include <aurum/io/wav_file.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <string>

namespace aurum::io {

namespace {

struct RiffHeader {
    char riff[4];
    std::uint32_t chunk_size;
    char wave[4];
};

struct FmtChunk {
    char id[4];
    std::uint32_t size;
    std::uint16_t audio_format;
    std::uint16_t num_channels;
    std::uint32_t sample_rate;
    std::uint32_t byte_rate;
    std::uint16_t block_align;
    std::uint16_t bits_per_sample;
};

}  // namespace

bool load_wav(const std::string& path, WavData& out, std::string& error) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        error = "failed to open wav file";
        return false;
    }

    RiffHeader header{};
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (std::string(header.riff, 4) != "RIFF" || std::string(header.wave, 4) != "WAVE") {
        error = "invalid wav header";
        return false;
    }

    FmtChunk fmt{};
    file.read(reinterpret_cast<char*>(&fmt), sizeof(fmt));
    if (std::string(fmt.id, 4) != "fmt ") {
        error = "missing fmt chunk";
        return false;
    }

    char data_id[4];
    std::uint32_t data_size = 0;
    while (file.read(data_id, 4)) {
        file.read(reinterpret_cast<char*>(&data_size), sizeof(data_size));
        if (std::string(data_id, 4) == "data") {
            break;
        }
        file.seekg(static_cast<std::streamoff>(data_size), std::ios::cur);
    }

    if (std::string(data_id, 4) != "data") {
        error = "missing data chunk";
        return false;
    }

    out.sample_rate = static_cast<int>(fmt.sample_rate);
    out.channels = static_cast<int>(fmt.num_channels);
    const int bytes_per_sample = fmt.bits_per_sample / 8;
    const int frame_count =
        static_cast<int>(data_size / (bytes_per_sample * out.channels));
    out.samples.resize(static_cast<std::size_t>(frame_count * out.channels));

    if (fmt.bits_per_sample == 16) {
        for (int i = 0; i < frame_count * out.channels; ++i) {
            std::int16_t sample = 0;
            file.read(reinterpret_cast<char*>(&sample), sizeof(sample));
            out.samples[static_cast<std::size_t>(i)] =
                static_cast<float>(sample) / 32768.0f;
        }
    } else if (fmt.bits_per_sample == 32 && fmt.audio_format == 3) {
        file.read(reinterpret_cast<char*>(out.samples.data()),
                  static_cast<std::streamsize>(data_size));
    } else {
        error = "unsupported wav format";
        return false;
    }

    return true;
}

bool save_wav(const std::string& path, const WavData& data, std::string& error) {
    if (data.channels <= 0 || data.sample_rate <= 0) {
        error = "invalid wav metadata";
        return false;
    }

    std::ofstream file(path, std::ios::binary);
    if (!file) {
        error = "failed to open output wav";
        return false;
    }

    const std::uint32_t data_bytes =
        static_cast<std::uint32_t>(data.samples.size() * sizeof(float));
    const std::uint32_t riff_size = 36 + data_bytes;

    file.write("RIFF", 4);
    file.write(reinterpret_cast<const char*>(&riff_size), sizeof(riff_size));
    file.write("WAVE", 4);

    const std::uint32_t fmt_size = 16;
    const std::uint16_t audio_format = 3;
    const std::uint16_t channels = static_cast<std::uint16_t>(data.channels);
    const std::uint32_t sample_rate = static_cast<std::uint32_t>(data.sample_rate);
    const std::uint16_t bits = 32;
    const std::uint16_t block_align =
        static_cast<std::uint16_t>(channels * bits / 8);
    const std::uint32_t byte_rate = sample_rate * block_align;

    file.write("fmt ", 4);
    file.write(reinterpret_cast<const char*>(&fmt_size), sizeof(fmt_size));
    file.write(reinterpret_cast<const char*>(&audio_format), sizeof(audio_format));
    file.write(reinterpret_cast<const char*>(&channels), sizeof(channels));
    file.write(reinterpret_cast<const char*>(&sample_rate), sizeof(sample_rate));
    file.write(reinterpret_cast<const char*>(&byte_rate), sizeof(byte_rate));
    file.write(reinterpret_cast<const char*>(&block_align), sizeof(block_align));
    file.write(reinterpret_cast<const char*>(&bits), sizeof(bits));

    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&data_bytes), sizeof(data_bytes));
    file.write(reinterpret_cast<const char*>(data.samples.data()),
               static_cast<std::streamsize>(data_bytes));
    return true;
}

}  // namespace aurum::io
