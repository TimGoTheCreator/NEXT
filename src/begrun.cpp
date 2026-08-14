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

#include <omp.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "../argparse/argparse.hpp"
#include "dt/adaptive.h"
#include "floatdef.h"
#include "gravity/step.h"
#include "io/async_writer.h"
#include "io/hdf5_save.h"
#include "io/load_particle.hpp"
#include "io/vtk_save.h"
#include "io/vtu_save.h"
#ifdef NEXT_MPI
#include <mpi.h>
#endif
#include "hdf5.h"

using next::OutputFormat;

// Helper: only rank 0, thread 0 prints
inline void log_once(int rank, const std::string& msg) {
    if (rank == 0 && omp_get_thread_num() == 0) {
        std::cout << msg << std::endl;
    }
}

int main(int argc, char** argv) {
    int rank = 0;
    int size = 1;

#ifdef NEXT_MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Silence stdout/stderr for all ranks except 0
    if (rank != 0) {
        fclose(stdout);
    }

#else
    (void)rank;
    (void)size;
#endif

    H5Eset_auto(H5E_DEFAULT, nullptr, nullptr);

    auto args = next::parse_arguments(argc, argv, rank);

    static constexpr const char* BANNER = R"NEXTBANNER(
 _  _ ________   _________ 
| \ | |  ____\ \ / /__   __|
|  \| | |__   \ V /   | |   
| . ` |  __|   > <    | |   
| |\  | |____ / . \   | |   
|_| \_|______/_/ \_\  |_|   
)NEXTBANNER";

    omp_set_num_threads(args.threads);

    // Only rank 0 prints startup info
    if (rank == 0 && omp_get_thread_num() == 0) {
#ifdef NEXT_MPI
        std::cout << "MPI mode enabled (" << size << " ranks)" << std::endl;
#endif
        std::cout << BANNER << std::endl;
        std::cout << " Threads:   " << args.threads << std::endl;
#ifdef NEXT_FP64
        std::cout << " Precision: FP64" << std::endl;
#elif defined(NEXT_FP32)
        std::cout << " Precision: FP32" << std::endl;
#endif
        if (args.max_steps > 0) {
            std::cout << " Max Steps: " << args.max_steps << std::endl;
        }
    }

    // Load particles
    Particle particles = LoadParticlesFromFile(args.input_file);
    log_once(rank, " Particles: " + std::to_string(particles.size()));

    real simTime = 0;
    real nextDump = 0;
    int force_step = 0;
    int dump_count = 0;
    char command;

    next::AsyncWriter async_writer;

    while (true) {
        if (args.max_steps > 0 && dump_count >= args.max_steps) {
            log_once(rank, "Reached maximum dumps (" + std::to_string(args.max_steps) + "). Exiting...");
            break;
        }

        real dtAdaptive = computeAdaptiveDt(particles, args.dt);
        Step(particles, dtAdaptive);
        simTime += dtAdaptive;
        force_step++;

        bool should_dump = false;
        if (args.dump_interval >= 1.0) {
            int step_freq = static_cast<int>(args.dump_interval);
            if (force_step % step_freq == 0 || force_step == 1) {
                should_dump = true;
            }
        } else {
            if (simTime >= nextDump) {
                should_dump = true;
                nextDump += args.dump_interval;
            }
        }

        if (should_dump) {
            std::string out = "dump_" + std::to_string(dump_count);

            switch (args.format) {
                case OutputFormat::VTK:
                    out += ".vtk";
                    break;
                case OutputFormat::VTU:
                    out += ".vtu";
                    break;
                case OutputFormat::HDF5:
                    out += ".hdf5";
                    break;
                case OutputFormat::HDF5_SINGLE:
                    out = "simulation.h5";
                    break;
            }

            async_writer.enqueue(particles, out, args.format, dump_count, simTime);

            log_once(rank, "[Dump " + std::to_string(dump_count) + "] step = " + std::to_string(force_step) + ", t = " + std::to_string(simTime) + ", file: " + out);

            dump_count++;
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

    async_writer.flush();


#ifdef NEXT_MPI
    MPI_Finalize();
#endif

    return 0;
}
