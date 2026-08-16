#pragma once
#include <string>

namespace next {

enum class OutputFormat { VTK, VTU, HDF5, HDF5_SINGLE };

struct Arguments {
    std::string input_file;
    int threads;
    double dt;
    double dump_interval;
    OutputFormat format;
    int max_steps = -1;
    bool restart = false;
    std::string output_name = "";
    bool cuda = false;
};

Arguments parse_arguments(int argc, char** argv, int rank);

}  // namespace next
