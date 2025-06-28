#include <aurum/gui/aurum_gui.hpp>

#include <aurum/dsp/effects/compressor.hpp>
#include <aurum/dsp/effects/delay.hpp>
#include <aurum/dsp/effects/distortion.hpp>
#include <aurum/dsp/effects/parametric_eq.hpp>
#include <aurum/dsp/effects/reverb.hpp>
#include <aurum/dsp/utils/fft_analyzer.hpp>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <algorithm>
#include <cmath>

namespace aurum::gui {

AurumGui::AurumGui() {
    chain_.add(std::make_unique<dsp::ParametricEqEffect>());
    chain_.add(std::make_unique<dsp::CompressorEffect>());
    chain_.prepare(48000, 512);
    spectrum_.assign(128, 0.0f);
    waveform_.assign(512, 0.0f);
}

AurumGui::~AurumGui() {
    shutdown_window();
}

bool AurumGui::init_window() {
    if (!glfwInit()) {
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window_ = glfwCreateWindow(1280, 720, "Aurum", nullptr, nullptr);
    if (window_ == nullptr) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(static_cast<GLFWwindow*>(window_));
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    imgui_context_ = ImGui::GetCurrentContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(window_), true);
    ImGui_ImplOpenGL3_Init("#version 330");
    return true;
}

void AurumGui::shutdown_window() {
    if (imgui_context_ != nullptr) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        imgui_context_ = nullptr;
    }
    if (window_ != nullptr) {
        glfwDestroyWindow(static_cast<GLFWwindow*>(window_));
        window_ = nullptr;
    }
    glfwTerminate();
}

void AurumGui::draw_chain_panel() {
    ImGui::Begin("Effect Chain");
    for (std::size_t i = 0; i < chain_.size(); ++i) {
        dsp::Effect* effect = chain_.at(i);
        if (effect == nullptr) {
            continue;
        }
        const bool selected = static_cast<int>(i) == selected_effect_;
        if (ImGui::Selectable(effect->name().c_str(), selected)) {
            selected_effect_ = static_cast<int>(i);
        }
    }
    ImGui::End();
}

void AurumGui::draw_parameter_panel() {
    ImGui::Begin("Parameters");
    if (selected_effect_ >= 0 && static_cast<std::size_t>(selected_effect_) < chain_.size()) {
        dsp::Effect* effect = chain_.at(static_cast<std::size_t>(selected_effect_));
        if (effect != nullptr) {
            const auto params = effect->parameters();
            for (std::size_t i = 0; i < params.size(); ++i) {
                float value = effect->parameter_normalized(static_cast<int>(i));
                if (ImGui::SliderFloat(params[i].name.c_str(), &value, 0.0f, 1.0f)) {
                    effect->set_parameter_normalized(static_cast<int>(i), value);
                }
            }
            bool bypass = effect->bypass();
            if (ImGui::Checkbox("Bypass", &bypass)) {
                effect->set_bypass(bypass);
            }
            float dry_wet = effect->dry_wet();
            if (ImGui::SliderFloat("Dry/Wet", &dry_wet, 0.0f, 1.0f)) {
                effect->set_dry_wet(dry_wet);
            }
        }
    }
    ImGui::End();
}

void AurumGui::draw_transport_panel() {
    ImGui::Begin("Transport");
    if (ImGui::Button("Play")) {
        transport_.play();
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause")) {
        transport_.pause();
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        transport_.stop();
    }
    const int pos = transport_.position_frames();
    const int total = transport_.total_frames();
    ImGui::Text("Position: %d / %d", pos, total);
    ImGui::End();
}

void AurumGui::draw_meter_panel() {
    ImGui::Begin("Meters");
    meter_buffer_.snapshot(waveform_, 512);
    if (!waveform_.empty()) {
        fft_.analyze(waveform_.data(), waveform_.size());
        const auto& mags = fft_.magnitudes();
        for (std::size_t i = 0; i < spectrum_.size() && i < mags.size(); ++i) {
            spectrum_[i] = spectrum_[i] * 0.7f + mags[i] * 0.3f;
        }
        ImGui::PlotLines("Waveform", waveform_.data(),
                         static_cast<int>(waveform_.size()), 0, nullptr, -1.0f, 1.0f,
                         ImVec2(0, 80));
    }

    ImGui::PlotHistogram("Spectrum", spectrum_.data(), static_cast<int>(spectrum_.size()), 0,
                         nullptr, 0.0f, 1.0f, ImVec2(0, 80));
    ImGui::End();
}

void AurumGui::render_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    draw_chain_panel();
    draw_parameter_panel();
    draw_transport_panel();
    draw_meter_panel();

    ImGui::Render();
    int display_w = 0;
    int display_h = 0;
    glfwGetFramebufferSize(static_cast<GLFWwindow*>(window_), &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.08f, 0.08f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(static_cast<GLFWwindow*>(window_));
}

int AurumGui::run() {
    if (!init_window()) {
        return 1;
    }

    while (running_ && !glfwWindowShouldClose(static_cast<GLFWwindow*>(window_))) {
        glfwPollEvents();
        if (glfwGetKey(static_cast<GLFWwindow*>(window_), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            running_ = false;
        }
        if (glfwGetKey(static_cast<GLFWwindow*>(window_), GLFW_KEY_SPACE) == GLFW_PRESS) {
            transport_.play();
        }
        render_frame();
    }

    shutdown_window();
    return 0;
}

}  // namespace aurum::gui
