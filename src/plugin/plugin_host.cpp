#include <aurum/plugin/plugin_host.hpp>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <cstring>
#include <sstream>

namespace aurum::plugin {

PluginEffect::PluginEffect(AurumPluginEntry* entry, void* handle)
    : entry_(entry), handle_(handle) {
    if (entry_ != nullptr && entry_->create != nullptr) {
        instance_ = entry_->create();
    }
    if (entry_ != nullptr && entry_->info != nullptr) {
        const AurumPluginInfo* info = entry_->info();
        if (info != nullptr) {
            param_count_ = info->num_params;
        }
    }
}

PluginEffect::~PluginEffect() {
    if (entry_ != nullptr && entry_->destroy != nullptr && instance_ != nullptr) {
        entry_->destroy(instance_);
    }
}

std::string PluginEffect::name() const {
    if (entry_ != nullptr && entry_->info != nullptr) {
        const AurumPluginInfo* info = entry_->info();
        if (info != nullptr && info->name != nullptr) {
            return info->name;
        }
    }
    return "Plugin";
}

std::vector<dsp::ParameterDescriptor> PluginEffect::parameters() const {
    std::vector<dsp::ParameterDescriptor> params;
    for (int i = 0; i < param_count_; ++i) {
        params.push_back({i, "param_" + std::to_string(i), dsp::ParameterType::Float, 0.0f, 1.0f,
                          0.5f, 1.0f, ""});
    }
    return params;
}

void PluginEffect::prepare(int /*sample_rate*/, int /*max_block_size*/) {}

void PluginEffect::reset() {}

void PluginEffect::set_parameter_normalized(int index, float value) {
    if (entry_ != nullptr && entry_->set_param != nullptr && instance_ != nullptr) {
        entry_->set_param(instance_, index, value);
    }
}

float PluginEffect::parameter_normalized(int index) const {
    if (entry_ != nullptr && entry_->get_param != nullptr && instance_ != nullptr) {
        return entry_->get_param(instance_, index);
    }
    return 0.0f;
}

void PluginEffect::process(const float* input, float* output, int frames, int channels) {
    if (entry_ != nullptr && entry_->process != nullptr && instance_ != nullptr) {
        entry_->process(instance_, input, output, frames, channels);
    } else if (output != input) {
        std::memcpy(output, input, static_cast<std::size_t>(frames * channels) * sizeof(float));
    }
}

std::unique_ptr<PluginEffect> PluginHost::load(const std::string& path, std::string& error) {
#if defined(_WIN32)
    void* handle = LoadLibraryA(path.c_str());
    if (handle == nullptr) {
        error = "failed to load plugin library";
        return nullptr;
    }
    auto* symbol = reinterpret_cast<AurumPluginEntry* (*)()>(
        GetProcAddress(static_cast<HMODULE>(handle), "aurum_plugin_entry"));
#else
    void* handle = dlopen(path.c_str(), RTLD_NOW);
    if (handle == nullptr) {
        error = dlerror() != nullptr ? dlerror() : "failed to load plugin library";
        return nullptr;
    }
    auto* symbol = reinterpret_cast<AurumPluginEntry* (*)()>(
        dlsym(handle, "aurum_plugin_entry"));
#endif

    if (symbol == nullptr) {
        error = "aurum_plugin_entry symbol missing";
        return nullptr;
    }

    AurumPluginEntry* entry = symbol();
    if (entry == nullptr || entry->create == nullptr || entry->process == nullptr) {
        error = "invalid plugin entry table";
        return nullptr;
    }

    libraries_.push_back({handle, entry});
    return std::make_unique<PluginEffect>(entry, handle);
}

}  // namespace aurum::plugin
