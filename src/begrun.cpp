#include "gravity/step.h"
#include "floatdef.h"
#include "dt/adaptive.h"
#include "io/vtk_save.h"
#include "io/vtu_save.h"
#include "io/load_particle.hpp"
#include "../argparse/argparse.hpp"
#include <iostream>
#include <fstream>
#include <omp.h>
#include <vector>
#include <string>

int main(int argc, char** argv)
{
    auto args = next::parse_arguments(argc, argv);

    // ASCII banner (raw string literal preserves backslashes and spacing)
    static constexpr const char* BANNER = R"NEXTBANNER(
_   _ ________   _________ 
| \ | |  ____\ \ / /__   __|
|  \| | |__   \ V /   | |   
| . ` |  __|   > <    | |   
| |\  | |____ / . \   | |   
|_| \_|______/_/ \_\  |_|  
)NEXTBANNER";

    // Print banner once at startup
    std::cout << BANNER << '\n';

    // Set threads and report
    omp_set_num_threads(args.threads);
    
    std::cout << " Threads: " << args.threads << "\n";

    #ifdef NEXT_FP64
    std::cout << " Precision: FP64\n";
    #elif defined(NEXT_FP32) 
    std::cout << " Precision: FP32\n";
    #endif

    // Load particles
    std::vector<Particle> particles = LoadParticlesFromFile(args.input_file);

    real simTime = 0;
    real nextDump = 0;
    int step = 0;

    while (true)
    {
        real dtAdaptive = computeAdaptiveDt(particles, args.dt);
        Step(particles, dtAdaptive);
        simTime += dtAdaptive;

        if (simTime >= nextDump)
        {
            std::string out = "dump_" + std::to_string(step) + (args.use_vtu ? ".vtu" : ".vtk");

            if (args.use_vtu)
                SaveVTU(particles, out);
            else
                SaveVTK(particles, out);

            std::cout << "Wrote: " << out << "\n";

            nextDump += args.dump_interval;
            step++;
        }
    }

    return 0;
}