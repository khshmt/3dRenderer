#pragma once
#include <chrono>
#include <fstream>
#include <filesystem>
#include <string>

using namespace std::string_literals;

class Timer {   
public:
    Timer() = default;

    ~Timer() = default;

    void startWatch() { 
        _startWatchTime = std::chrono::steady_clock::now(); 
    }

    void endWatch() { 
        _endWatchTime = std::chrono::steady_clock::now(); 
        auto _time =
            std::chrono::duration_cast<std::chrono::microseconds>(_endWatchTime - _startWatchTime)
                .count();
        _fps = FPS();
    }

    double getFPS() {
        return _fps;
    }

private:
    double FPS() {
        const auto& _start =
            std::chrono::time_point_cast<std::chrono::microseconds>(_startWatchTime)
                .time_since_epoch()
                .count();
        const auto& _end = std::chrono::time_point_cast<std::chrono::microseconds>(_endWatchTime)
                               .time_since_epoch()
                               .count();
        auto fps = 1.0 / ((_end - _start) * 1.0e-6);
        return fps;
    }

private:
    std::chrono::time_point<std::chrono::steady_clock> _startWatchTime;
    std::chrono::time_point<std::chrono::steady_clock> _endWatchTime;
    double _fps{};
};
