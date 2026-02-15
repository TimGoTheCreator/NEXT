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

#include "../argparse/argparse.hpp"
#include "dt/adaptive.h"
#include "floatdef.h"
#include "gravity/step.h"
#include "io/load_particle.hpp"
#include "io/vtk_save.h"
#include "io/vtu_save.h"
#include "io/hdf5_save.h"
#include <fstream>
#include <iostream>
#include <omp.h>
#include <string>
#include <vector>
#ifdef NEXT_MPI
  #include <mpi.h>
#endif
#include "hdf5.h"


using next::OutputFormat;

// Helper: only rank 0, thread 0 prints
inline void log_once(int rank, const std::string &msg) {
    if (rank == 0 && omp_get_thread_num() == 0) {
        std::cout << msg << std::endl;
    }
}

int main(int argc, char **argv) {
    int rank = 0;
    int size = 1;

#ifdef NEXT_MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    log_once(rank, "MPI mode enabled (" + std::to_string(size) + " ranks)");
#endif

    H5Eset_auto(H5E_DEFAULT, nullptr, nullptr);

    auto args = next::parse_arguments(argc, argv, rank);

    static constexpr const char *BANNER = R"NEXTBANNER(
 _  _ ________   _________ 
| \ | |  ____\ \ / /__   __|
|  \| | |__   \ V /   | |   
| . ` |  __|   > <    | |   
| |\  | |____ / . \   | |   
|_| \_|______/_/ \_\  |_|   
)NEXTBANNER";

    omp_set_num_threads(args.threads);

    log_once(rank, BANNER);
    log_once(rank, " Threads:   " + std::to_string(args.threads));
#ifdef NEXT_FP64
    log_once(rank, " Precision: FP64");
#elif defined(NEXT_FP32)
    log_once(rank, " Precision: FP32");
#endif

    // Load particles
    Particle particles = LoadParticlesFromFile(args.input_file);
    log_once(rank, " Particles: " + std::to_string(particles.size()));

    real simTime = 0;
    real nextDump = 0;
    int step = 0;
    char command;

    while (true) {
        real dtAdaptive = computeAdaptiveDt(particles, args.dt);
        Step(particles, dtAdaptive);
        simTime += dtAdaptive;

        if (simTime >= nextDump) {
            std::string out = "dump_" + std::to_string(step);

            switch (args.format) {
                case OutputFormat::VTK:  out += ".vtk";  SaveVTK(particles, out);  break;
                case OutputFormat::VTU:  out += ".vtu";  SaveVTU(particles, out);  break;
                case OutputFormat::HDF5: out += ".hdf5"; SaveHDF5(particles, out); break;
            }

            log_once(rank, "[Dump " + std::to_string(step) +
                           "] t = " + std::to_string(simTime) +
                           ", file: " + out);

            nextDump += args.dump_interval;
            step++;
        }

        // Non-blocking exit check
        if (std::cin.rdbuf()->in_avail() > 0) {
            std::cin >> command;
            if (command == 'q' || command == 'Q') {
                log_once(rank, "Exiting...");
                break;
            }
        }
    }

#ifdef NEXT_MPI
    MPI_Finalize();
#endif

    return 0;
}
