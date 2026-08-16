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
    bool treepm = false;
    int pm_grid = 128;
    double r_split = 0.0;
    bool cosmology = false;
    double omega_m = 0.3089;
    double omega_l = 0.6911;
    double hubble_h0 = 67.74;
    double z_start = 50.0;
};

Arguments parse_arguments(int argc, char** argv, int rank);

}  // namespace next
