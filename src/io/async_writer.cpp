// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "async_writer.h"

#include <iostream>

#include "hdf5_save.h"
#include "vtk_save.h"
#include "vtu_save.h"

namespace next {

AsyncWriter::AsyncWriter() {
    worker_thread_ = std::thread(&AsyncWriter::worker_loop, this);
}

AsyncWriter::~AsyncWriter() {
    flush();
    stop_ = true;
    cv_.notify_all();
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

void AsyncWriter::enqueue(const ParticleSystem& ps, const std::string& filename, OutputFormat format, int step, double time) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push({ps, filename, format, step, time});
        pending_count_++;
    }
    cv_.notify_one();
}

void AsyncWriter::flush() {
    std::unique_lock<std::mutex> lock(mutex_);
    flush_cv_.wait(lock, [this]() { return queue_.empty() && pending_count_ == 0; });
}

void AsyncWriter::worker_loop() {
    while (true) {
        WriteTask task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this]() { return stop_ || !queue_.empty(); });

            if (stop_ && queue_.empty()) {
                break;
            }

            task = std::move(queue_.front());
            queue_.pop();
        }

        switch (task.format) {
            case OutputFormat::VTK:
                SaveVTK(task.ps, task.filename);
                break;
            case OutputFormat::VTU:
                SaveVTU(task.ps, task.filename);
                break;
            case OutputFormat::HDF5:
                SaveHDF5(task.ps, task.filename);
                break;
            case OutputFormat::HDF5_SINGLE:
                SaveHDF5SingleStep(task.ps, task.filename, task.step, task.time);
                break;
        }

        {
            std::unique_lock<std::mutex> lock(mutex_);
            pending_count_--;
            if (queue_.empty() && pending_count_ == 0) {
                flush_cv_.notify_all();
            }
        }
    }
}

}  // namespace next
