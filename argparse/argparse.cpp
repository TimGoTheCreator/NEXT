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
#ifdef NEXT_MPI
#include <mpi.h>
#endif

namespace next {

Arguments parse_arguments(int argc, char** argv, int rank) {
    if (argc < 6 || argc > 7) {
        // Only rank 0 prints usage
        if (rank == 0) {
            std::cerr << "Usage: next <input.txt> <threads> <dt> <dump_interval> <vtk|vtu|hdf5|hdf5-single> "
                         "[max_steps]\n";
        }

#ifdef NEXT_MPI
        MPI_Finalize();
#endif
        std::exit(1);
    }

    Arguments args;

    args.input_file = argv[1];
    args.threads = std::stoi(argv[2]);
    args.dt = std::stod(argv[3]);
    args.dump_interval = std::stod(argv[4]);

    std::string fmt = argv[5];

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

    if (argc == 7) {
        args.max_steps = std::stoi(argv[6]);
    }

    return args;
}

}  // namespace next
