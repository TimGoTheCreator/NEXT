#include "struct/particle.h"
#include "floatdef.h"

std::vector<Particle> LoadParticlesFromFile(const std::string& filename)
{
    std::vector<Particle> p;
    std::ifstream in(filename);

    Particle temp;
    while (in >> temp.x >> temp.y >> temp.z >> temp.vx >> temp.vy >> temp.vz >> temp.m)
        p.push_back(temp);

    return p;
}
