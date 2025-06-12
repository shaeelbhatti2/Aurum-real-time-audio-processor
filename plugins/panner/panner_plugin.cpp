#include <aurum/plugin/plugin_abi.h>

namespace {

struct PannerState {
    float pan = 0.5f;
};

PannerState* as_state(void* instance) { return static_cast<PannerState*>(instance); }

void* create() { return new PannerState(); }

void destroy(void* instance) { delete as_state(instance); }

const AurumPluginInfo* info() {
    static AurumPluginInfo plugin_info{"Panner", 1};
    return &plugin_info;
}

void set_param(void* instance, int index, float normalized) {
    if (index == 0) {
        as_state(instance)->pan = normalized;
    }
}

float get_param(void* instance, int index) {
    return index == 0 ? as_state(instance)->pan : 0.0f;
}

void process(void* instance, const float* input, float* output, int frames, int /*channels*/) {
    const float pan = as_state(instance)->pan;
    const float left_gain = 1.0f - pan;
    const float right_gain = pan;

    for (int i = 0; i < frames; ++i) {
        const float mono = (input[i * 2] + input[i * 2 + 1]) * 0.5f;
        output[i * 2] = mono * left_gain;
        output[i * 2 + 1] = mono * right_gain;
    }
}

AurumPluginEntry entry{create, destroy, process, info, set_param, get_param};

}  // namespace

extern "C" AURUM_PLUGIN_EXPORT AurumPluginEntry* aurum_plugin_entry() { return &entry; }
