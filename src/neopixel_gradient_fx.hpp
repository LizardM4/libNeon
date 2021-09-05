//
// Created by spak on 6/6/21.
//

#ifndef PICOSKATE_NEOPIXEL_GRADIENT_FX_HPP
#define PICOSKATE_NEOPIXEL_GRADIENT_FX_HPP

#include "neopixel_gradient.hpp"
#include "neopixel_strip.hpp"
#include <chrono>
#include <functional>

namespace neo {
    namespace {
        using namespace std::chrono_literals;
    }

    class gradient_fx {
        neo::gradient const &_gradient;
        std::chrono::milliseconds _duration;
        float _repeats;
    public:
        inline explicit gradient_fx(neo::gradient const &g, std::chrono::milliseconds duration = 1s, float repeats = 1.f);

        [[nodiscard]] inline neo::gradient const &gradient() const;
        [[nodiscard]] inline float repeats() const;
        [[nodiscard]] inline std::chrono::milliseconds duration() const;

        inline void set_repeats(float n);
        inline void set_duration(std::chrono::milliseconds d);

        [[nodiscard]] std::vector<rgb> sample(std::size_t n_leds, std::chrono::milliseconds time_since_start, std::vector<rgb> recycle_buffer = {}, blending_method method = blend_linear) const;

        [[nodiscard]] std::function<void(std::chrono::milliseconds)> make_steady_timer_callback(transmittable_rgb_strip &strip, rmt_channel_t channel, blending_method method = blend_linear) const;
    };
}

namespace neo {
    gradient_fx::gradient_fx(const neo::gradient &g, std::chrono::milliseconds duration, float repeats) :
        _gradient{g},
        _duration{duration},
        _repeats{repeats}
    {}

    std::chrono::milliseconds gradient_fx::duration() const {
        return _duration;
    }

    float gradient_fx::repeats() const {
        return _repeats;
    }

    const neo::gradient & gradient_fx::gradient() const {
        return _gradient;
    }

    void gradient_fx::set_duration(std::chrono::milliseconds d) {
        _duration = d;
    }

    void gradient_fx::set_repeats(float n) {
        _repeats = n;
    }

    std::function<void(std::chrono::milliseconds)> gradient_fx::make_steady_timer_callback(transmittable_rgb_strip &strip, rmt_channel_t channel, blending_method method) const {
        return [buffer = std::vector<rgb>{}, &strip, channel, method, *this] (std::chrono::milliseconds elapsed) mutable {
            // Use lambda initialization syntax and mutability to always recycle the buffer
            buffer = sample(strip.size(), elapsed, std::move(buffer), method);
            ESP_ERROR_CHECK_WITHOUT_ABORT(strip.update(buffer, channel, false));
        };
    }
}

#endif //PICOSKATE_NEOPIXEL_GRADIENT_FX_HPP
