# Aurum

Real-time audio effects processor and plugin host built in modern C++20.

Aurum loads audio, runs it through a chain of built-in DSP effects and optional native plugins, and outputs to speakers or disk. A desktop GUI exposes parameter controls, preset management, and level/spectrum visualization.

## Features

- Low-latency real-time audio engine
- Built-in effects: EQ, compressor, reverb, delay, distortion, filters
- Serial effect chain with bypass and dry/wet mix
- JSON preset save/load with factory presets
- WAV playback and offline render
- Native plugin host with bundled gain and panner plugins
- ImGui-based control surface with VU meters and spectrum analyzer

## Architecture

```
[File / Input] -> [Transport] -> [Effect Chain] -> [Master Limiter] -> [Output]
                                        ^
                                   [Plugin Host]
```

The audio callback runs on a dedicated real-time thread. Parameter updates from the UI travel through lock-free atomic storage. Visualization taps the signal via a lock-free ring buffer read at 30fps on the UI thread.

## Build

Requirements: CMake 3.25+, C++20 compiler, PortAudio, GLFW, OpenGL.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/aurum
```

On macOS with Homebrew:

```bash
brew install portaudio glfw
```

## Project layout

```
src/
  app/        entry point and application shell
  engine/     audio engine and transport
  dsp/        effects and math utilities
  io/         WAV load/save and offline render
  plugin/     plugin ABI and host loader
  gui/        ImGui panels and visualization
plugins/
  gain/       example gain plugin
  panner/     example stereo panner plugin
presets/      factory preset JSON files
```

## License

MIT — see [LICENSE](LICENSE).
