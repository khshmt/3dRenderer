#pragma once
#include <chrono>
#include <fstream>
#include <filesystem>
#include <string>

using namespace std::string_literals;
class Timer {   
public:
    Timer() {
        auto file_name = "log"s + std::to_string(count) + ".txt"s;
        _file = std::fstream(file_name, std::ios::out);
        count++;
    }

    ~Timer() { 
        _file.close();
    }

    void startWatch(std::string_view functionName) { 
        _startWatchTime = std::chrono::steady_clock::now(); 
        _functionName = functionName;
    }

    void endWatch() { 
        _endWatchTime = std::chrono::steady_clock::now(); 
        auto _time =
            std::chrono::duration_cast<std::chrono::microseconds>(_endWatchTime - _startWatchTime)
                .count();
        _file << _functionName << " takes: " << _time << "us\n";
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
    std::fstream _file;
    std::string_view _functionName;
    double _fps{};
    inline static int count{0};

};
