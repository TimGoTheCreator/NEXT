#pragma once
#include <vector>
#include "struct/particle.h"
#include <string>

void SaveHDF5(const std::vector<Particle>& p, const std::string& filename);
