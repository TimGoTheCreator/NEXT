#pragma once
#include <string>

namespace next {

enum class OutputFormat {
    VTK,
    VTU,
    HDF5
};

struct Arguments {
    std::string input_file;
    int threads;
    double dt;
    double dump_interval;
    OutputFormat format;
};

Arguments parse_arguments(int argc, char** argv);

}
