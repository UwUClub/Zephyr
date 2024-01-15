#include "Clock.hpp"

namespace Engine {
    const constexpr int time = 1000;

    double Clock::getElapsedTime() const
    {
        return _delta.count() * time;
    }

    double Clock::restart()
    {
        auto currentTime = std::chrono::high_resolution_clock::now();

        _delta = currentTime - _lastTime;
        _lastTime = currentTime;
        return getElapsedTime();
    }
} // namespace Engine
