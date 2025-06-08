#pragma once

#include <cstdint>

#ifdef _WIN32
#define AURUM_PLUGIN_EXPORT __declspec(dllexport)
#else
#define AURUM_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AurumPluginInfo {
    const char* name;
    int num_params;
} AurumPluginInfo;

typedef void* (*AurumCreateFn)();
typedef void (*AurumDestroyFn)(void* instance);
typedef void (*AurumProcessFn)(void* instance, const float* input, float* output, int frames,
                               int channels);
typedef const AurumPluginInfo* (*AurumInfoFn)();
typedef void (*AurumSetParamFn)(void* instance, int index, float normalized);
typedef float (*AurumGetParamFn)(void* instance, int index);

typedef struct AurumPluginEntry {
    AurumCreateFn create;
    AurumDestroyFn destroy;
    AurumProcessFn process;
    AurumInfoFn info;
    AurumSetParamFn set_param;
    AurumGetParamFn get_param;
} AurumPluginEntry;

AURUM_PLUGIN_EXPORT AurumPluginEntry* aurum_plugin_entry();

#ifdef __cplusplus
}
#endif
