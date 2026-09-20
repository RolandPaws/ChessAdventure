#pragma once

#include <SDL3/SDL.h>

namespace chessadventure {

// Tracks frame-to-frame delta time in seconds using SDL's monotonic tick counter.
class Clock {
public:
    Clock() : last_ticks_(SDL_GetTicks()) {}

    // Call once per frame. Returns the elapsed time since the previous call, in seconds.
    float Tick() {
        const Uint64 now = SDL_GetTicks();
        const float delta_seconds = static_cast<float>(now - last_ticks_) / 1000.0f;
        last_ticks_ = now;
        return delta_seconds;
    }

private:
    Uint64 last_ticks_;
};

}  // namespace chessadventure
