// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <atomic>

#include "../../argparse/argparse.hpp"
#include "../struct/particle.h"

namespace next {

struct WriteTask {
    ParticleSystem ps;
    std::string filename;
    OutputFormat format;
    int step = 0;
    double time = 0.0;
};

class AsyncWriter {
public:
    AsyncWriter();
    ~AsyncWriter();

    void enqueue(const ParticleSystem& ps, const std::string& filename, OutputFormat format, int step = 0, double time = 0.0);
    void flush();

private:
    void worker_loop();

    std::queue<WriteTask> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::condition_variable flush_cv_;
    std::thread worker_thread_;
    std::atomic<bool> stop_{false};
    size_t pending_count_{0};
};

}  // namespace next
