#include "gravity/step.h"
#include<iostream>
#include<fstream>
#include"floatdef.h"

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

    if (argc != 2)
    {
        std::cerr << "Usage: next <initial.txt>\n";
        return 1;
    }

    const char* filename = argv[1];

    std::vector<Particle> particles = LoadParticlesFromFile(filename);

    real dt = 0.01;
    while (true)
    {
        Step(particles, dt);
    }

    return 0;
    
    
}