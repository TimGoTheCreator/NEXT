// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "argparse.hpp"

#include <iostream>
#include <stdexcept>
#include <vector>
#ifdef NEXT_MPI
#include <mpi.h>
#endif

namespace next {

Arguments parse_arguments(int argc, char** argv, int rank) {
    std::vector<std::string> pos_args;
    bool restart_flag = false;
    bool cuda_flag = false;
    bool treepm_flag = false;
    int pm_grid_val = 128;
    double r_split_val = 0.0;
    std::string custom_output = "";

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--restart" || arg == "-r" || arg == "--continue" || arg == "-c") {
            restart_flag = true;
        } else if (arg == "--cuda" || arg == "-g" || arg == "--gpu") {
            cuda_flag = true;
        } else if (arg == "--treepm" || arg == "-tpm" || arg == "--pm") {
            treepm_flag = true;
        } else if (arg == "--pm-grid" || arg == "--grid") {
            if (i + 1 < argc) pm_grid_val = std::stoi(argv[++i]);
        } else if (arg == "--r-split" || arg == "--rsplit") {
            if (i + 1 < argc) r_split_val = std::stod(argv[++i]);
        } else if (arg == "--output" || arg == "-o" || arg == "--out") {
            if (i + 1 < argc) {
                custom_output = argv[++i];
            }
        } else if (arg.rfind("--output=", 0) == 0) {
            custom_output = arg.substr(9);
        } else if (arg.rfind("-o=", 0) == 0) {
            custom_output = arg.substr(3);
        } else if (arg == "--help" || arg == "-h") {
            if (rank == 0) {
                std::cout << "Usage: next <input_file> <threads> <dt> <dump_interval> <format> [max_steps] [output_name] [--restart] [--cuda] [--treepm] [--pm-grid <N>] [--output <name>]\n"
                          << "Formats: vtk, vtu, hdf5, hdf5-single\n"
                          << "Options:\n"
                          << "  --cuda, -g, --gpu           Enable NVIDIA CUDA GPU Acceleration\n"
                          << "  --treepm, -tpm, --pm        Enable Hybrid TreePM (Tree-Particle-Mesh) Poisson Solver\n"
                          << "  --pm-grid <N>               Set Particle-Mesh 3D Grid Dimension (default: 128)\n"
                          << "  --r-split <R>               Set TreePM potential splitting radius r_s\n"
                          << "  --restart, -r, --continue   Resume simulation from checkpoint/snapshot\n"
                          << "  --output, -o <name>         Custom output file name (e.g. whirlpool.h5, galaxy.h5)\n";
            }
#ifdef NEXT_MPI
            MPI_Finalize();
#endif
            std::exit(0);
        } else {
            pos_args.push_back(arg);
        }
    }

    if (pos_args.size() < 5 || pos_args.size() > 7) {
        // Only rank 0 prints usage
        if (rank == 0) {
            std::cerr << "Usage: next <input_file> <threads> <dt> <dump_interval> <vtk|vtu|hdf5|hdf5-single> "
                         "[max_steps] [output_name] [--restart] [--cuda] [--output <name>]\n";
        }

#ifdef NEXT_MPI
        MPI_Finalize();
#endif
        std::exit(1);
    }

    Arguments args;
    args.restart = restart_flag;
    args.cuda = cuda_flag;
    args.treepm = treepm_flag;
    args.pm_grid = pm_grid_val;
    args.r_split = r_split_val;
    args.output_name = custom_output;

    args.input_file = pos_args[0];
    args.threads = std::stoi(pos_args[1]);
    args.dt = std::stod(pos_args[2]);
    args.dump_interval = std::stod(pos_args[3]);

    std::string fmt = pos_args[4];

    if (fmt == "vtk") {
        args.format = OutputFormat::VTK;
    } else if (fmt == "vtu") {
        args.format = OutputFormat::VTU;
    } else if (fmt == "hdf5") {
        args.format = OutputFormat::HDF5;
    } else if (fmt == "hdf5-single" || fmt == "hdf5_single" || fmt == "h5single" || fmt == "h5") {
        args.format = OutputFormat::HDF5_SINGLE;
    } else {
        if (rank == 0) {
            std::cerr << "Choose a file format: vtk, vtu, hdf5, or hdf5-single\n";
        }
#ifdef NEXT_MPI
        MPI_Finalize();
#endif
        std::exit(1);
    }

    if (pos_args.size() >= 6) {
        // Check if 6th arg is a number (max_steps) or a string (output_name)
        try {
            args.max_steps = std::stoi(pos_args[5]);
        } catch (const std::exception&) {
            if (args.output_name.empty()) {
                args.output_name = pos_args[5];
            }
        }
    }

    if (pos_args.size() == 7) {
        if (args.output_name.empty()) {
            args.output_name = pos_args[6];
        }
    }

    return args;
}

}  // namespace next
