#pragma once
#include <string>

#include "../struct/particle.h"

void SaveHDF5(const ParticleSystem& ps, const std::string& filename);
void SaveHDF5SingleStep(const ParticleSystem& ps, const std::string& filename, int step, double time);
