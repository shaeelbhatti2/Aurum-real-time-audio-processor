#include <aurum/engine/device_manager.hpp>

#if AURUM_HAS_PORTAUDIO
#include <portaudio.h>
#endif

namespace aurum::engine {

#if AURUM_HAS_PORTAUDIO
bool DeviceManager::refresh(std::string& error) {
    devices_.clear();
    default_output_ = Pa_GetDefaultOutputDevice();
    default_input_ = Pa_GetDefaultInputDevice();

    const int count = Pa_GetDeviceCount();
    if (count < 0) {
        error = Pa_GetErrorText(count);
        return false;
    }

    for (int i = 0; i < count; ++i) {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
        if (info == nullptr) {
            continue;
        }

        AudioDeviceInfo device;
        device.index = i;
        device.name = info->name != nullptr ? info->name : "Unknown";
        device.max_input_channels = static_cast<int>(info->maxInputChannels);
        device.max_output_channels = static_cast<int>(info->maxOutputChannels);
        device.default_sample_rate = info->defaultSampleRate;
        device.is_default_input = i == default_input_;
        device.is_default_output = i == default_output_;
        devices_.push_back(device);
    }
    return true;
}
#else
bool DeviceManager::refresh(std::string& error) {
    error = "PortAudio not available";
    return false;
}
#endif

AudioDeviceInfo DeviceManager::find_by_index(int index) const {
    for (const auto& device : devices_) {
        if (device.index == index) {
            return device;
        }
    }
    return {};
}

}  // namespace aurum::engine
