#pragma once

#include <aurum/preset/preset_manager.hpp>

#include <vector>

namespace aurum::dsp {

class AutomationEngine {
public:
    void set_points(const std::vector<preset::AutomationPoint>& points) {
        points_ = points;
    }

    float value_at(double time_seconds) const {
        if (points_.empty()) {
            return 0.0f;
        }
        if (time_seconds <= points_.front().time_seconds) {
            return points_.front().normalized_value;
        }
        if (time_seconds >= points_.back().time_seconds) {
            return points_.back().normalized_value;
        }

        for (std::size_t i = 1; i < points_.size(); ++i) {
            const auto& prev = points_[i - 1];
            const auto& next = points_[i];
            if (time_seconds <= next.time_seconds) {
                const double span = next.time_seconds - prev.time_seconds;
                if (span <= 0.0) {
                    return next.normalized_value;
                }
                const double t = (time_seconds - prev.time_seconds) / span;
                return prev.normalized_value +
                       static_cast<float>(t) *
                           (next.normalized_value - prev.normalized_value);
            }
        }
        return points_.back().normalized_value;
    }

    void clear() { points_.clear(); }

private:
    std::vector<preset::AutomationPoint> points_;
};

class AutomationTrack {
public:
    void set_lane(int effect_index, int parameter_index,
                  const std::vector<preset::AutomationPoint>& points) {
        effect_index_ = effect_index;
        parameter_index_ = parameter_index;
        engine_.set_points(points);
    }

    void apply(dsp::Effect* effect, double time_seconds) const {
        if (effect == nullptr) {
            return;
        }
        effect->set_parameter_normalized(parameter_index_, engine_.value_at(time_seconds));
    }

    int effect_index() const { return effect_index_; }
    int parameter_index() const { return parameter_index_; }

private:
    int effect_index_ = 0;
    int parameter_index_ = 0;
    AutomationEngine engine_;
};

class AutomationScheduler {
public:
    void clear() { tracks_.clear(); }

    void add_track(AutomationTrack track) { tracks_.push_back(std::move(track)); }

    void apply_at(dsp::EffectChain& chain, double time_seconds) const {
        for (const auto& track : tracks_) {
            if (track.effect_index() < 0 ||
                static_cast<std::size_t>(track.effect_index()) >= chain.size()) {
                continue;
            }
            track.apply(chain.at(static_cast<std::size_t>(track.effect_index())), time_seconds);
        }
    }

    const std::vector<AutomationTrack>& tracks() const { return tracks_; }

private:
    std::vector<AutomationTrack> tracks_;
};

}  // namespace aurum::dsp
