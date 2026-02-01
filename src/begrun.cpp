#include "gravity/step.h"
#include<iostream>
#include<fstream>
#include"floatdef.h"
#include"dt/adaptive.h"

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

    if (argc != 3)
    {
        std::cerr << "Usage: next <initial.txt> <dt>\n";
        return 1;
    }

    const char* filename = argv[1];
    real dt = std::stod(argv[2]);

    std::vector<Particle> particles = LoadParticlesFromFile(filename);

    while (true)
    {
        real dtAdaptive = computeAdaptiveDt(particles, dt);
        Step(particles, dtAdaptive);
    }

    return 0;
    
    
}

