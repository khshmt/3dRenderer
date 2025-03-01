#pragma once
#include <chrono>
#include <iostream>

class Timer {
   public:
    Timer() { start_time = std::chrono::steady_clock::now(); }

    ~Timer() {
        end_time = std::chrono::steady_clock::now();

        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(start_time)
                         .time_since_epoch()
                         .count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(end_time)
                       .time_since_epoch()
                       .count();

        std::cout << (end - start) << "us\n";
    }

   private:
    std::chrono::time_point<std::chrono::steady_clock> start_time;
    std::chrono::time_point<std::chrono::steady_clock> end_time;
};
