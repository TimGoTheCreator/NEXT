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

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--restart" || arg == "-r" || arg == "--continue" || arg == "-c") {
            restart_flag = true;
        } else if (arg == "--help" || arg == "-h") {
            if (rank == 0) {
                std::cout << "Usage: next <input_file> <threads> <dt> <dump_interval> <format> [max_steps] [--restart]\n"
                          << "Formats: vtk, vtu, hdf5, hdf5-single\n"
                          << "Options:\n"
                          << "  --restart, -r, --continue   Resume simulation from checkpoint/snapshot\n";
            }
#ifdef NEXT_MPI
            MPI_Finalize();
#endif
            std::exit(0);
        } else {
            pos_args.push_back(arg);
        }
    }

    if (pos_args.size() < 5 || pos_args.size() > 6) {
        // Only rank 0 prints usage
        if (rank == 0) {
            std::cerr << "Usage: next <input_file> <threads> <dt> <dump_interval> <vtk|vtu|hdf5|hdf5-single> "
                         "[max_steps] [--restart]\n";
        }

#ifdef NEXT_MPI
        MPI_Finalize();
#endif
        std::exit(1);
    }

    Arguments args;
    args.restart = restart_flag;

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

    if (pos_args.size() == 6) {
        args.max_steps = std::stoi(pos_args[5]);
    }

    return args;
}

}  // namespace next
