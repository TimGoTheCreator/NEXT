#include "gravity/step.h"
#include<iostream>
#include<fstream>
#include"floatdef.h"
#include"dt/adaptive.h"
#include"io/vtk_save.h"

std::vector<Particle> LoadParticlesFromFile(const std::string& filename)
{
    std::vector<Particle> p;
    std::ifstream in(filename);

    Particle temp;
    while(in >> temp.x >> temp.y >> temp.z >> temp.vx >> temp.vy >> temp.vz >> temp.m)
    {
        p.push_back(temp);
    }
    return p;
}

int main(int argc, char** argv) {

    std::cout
    << "NN   NN  EEEEEEE  XX   XX  TTTTTTT\n"
    << "NNN  NN  EE        XX XX     TTT  \n"
    << "NN N NN  EEEEE      XXX      TTT  \n"
    << "NN  NNN  EE        XX XX     TTT  \n"
    << "NN   NN  EEEEEEE  XX   XX    TTT  \n"
    << "Newtonian EXact Trajectories\n";

    if (argc != 4)
    {
        std::cerr << "No parameter file specified\n";
        std::cerr << "Usage: next <initial.txt> <dt> <dump_interval>\n";
        return 1;
    }

    const char* filename = argv[1];
    real dt = std::stod(argv[2]);
    double cacheInterval = std::stod(argv[3]);
    real simTime = 0;
    real nextDump = 0;

    std::vector<Particle> particles = LoadParticlesFromFile(filename);

    int step = 0;

    while (true)
    {
        real dtAdaptive = computeAdaptiveDt(particles, dt);
        Step(particles, dtAdaptive);
        simTime += dtAdaptive;

        if (simTime >= nextDump)
        {
            std::string out = "dump_" + std::to_string(step) + ".vtk";
            SaveVTK(particles, out);
            std::cout << "Wrote: " << out << "\n";
            nextDump += cacheInterval;
        }

        step++;
    }

    return 0;
    
    
}
