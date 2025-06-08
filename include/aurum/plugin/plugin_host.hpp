#pragma once

#include <aurum/dsp/effect.hpp>
#include <aurum/plugin/plugin_abi.h>

#include <memory>
#include <string>
#include <vector>

namespace aurum::plugin {

class PluginEffect : public dsp::Effect {
public:
    explicit PluginEffect(AurumPluginEntry* entry, void* handle);
    ~PluginEffect() override;

    std::string name() const override;
    std::vector<dsp::ParameterDescriptor> parameters() const override;

    void prepare(int sample_rate, int max_block_size) override;
    void reset() override;

    void set_parameter_normalized(int index, float value) override;
    float parameter_normalized(int index) const override;

    void process(const float* input, float* output, int frames, int channels) override;

private:
    AurumPluginEntry* entry_ = nullptr;
    void* handle_ = nullptr;
    void* instance_ = nullptr;
    int param_count_ = 0;
};

class PluginHost {
public:
    std::unique_ptr<PluginEffect> load(const std::string& path, std::string& error);

private:
    struct LoadedLibrary {
        void* handle = nullptr;
        AurumPluginEntry* entry = nullptr;
    };

    std::vector<LoadedLibrary> libraries_;
};

}  // namespace aurum::plugin
