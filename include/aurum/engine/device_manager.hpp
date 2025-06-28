#pragma once

#include <aurum/engine/audio_config.hpp>

#include <string>
#include <vector>

namespace aurum::engine {

struct AudioDeviceInfo {
    int index = -1;
    std::string name;
    int max_input_channels = 0;
    int max_output_channels = 0;
    double default_sample_rate = 48000.0;
    bool is_default_input = false;
    bool is_default_output = false;
};

class DeviceManager {
public:
    bool refresh(std::string& error);
    const std::vector<AudioDeviceInfo>& devices() const { return devices_; }

    int default_output_index() const { return default_output_; }
    int default_input_index() const { return default_input_; }

    AudioDeviceInfo find_by_index(int index) const;

private:
    std::vector<AudioDeviceInfo> devices_;
    int default_output_ = -1;
    int default_input_ = -1;
};

}  // namespace aurum::engine
