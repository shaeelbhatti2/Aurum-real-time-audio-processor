#include <aurum/plugin/plugin_abi.h>

#include <algorithm>
#include <cmath>

namespace {

struct GainState {
    float gain = 0.5f;
};

GainState* as_state(void* instance) { return static_cast<GainState*>(instance); }

void* create() { return new GainState(); }

void destroy(void* instance) { delete as_state(instance); }

const AurumPluginInfo* info() {
    static AurumPluginInfo plugin_info{"Gain", 1};
    return &plugin_info;
}

void set_param(void* instance, int index, float normalized) {
    if (index == 0) {
        as_state(instance)->gain = normalized;
    }
}

float get_param(void* instance, int index) {
    return index == 0 ? as_state(instance)->gain : 0.0f;
}

void process(void* instance, const float* input, float* output, int frames, int channels) {
    const float gain = as_state(instance)->gain * 2.0f;
    const int count = frames * channels;
    for (int i = 0; i < count; ++i) {
        output[i] = input[i] * gain;
    }
}

AurumPluginEntry entry{create, destroy, process, info, set_param, get_param};

}  // namespace

extern "C" AURUM_PLUGIN_EXPORT AurumPluginEntry* aurum_plugin_entry() { return &entry; }
