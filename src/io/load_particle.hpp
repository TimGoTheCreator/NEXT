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

#include "struct/particle.h"
#include "floatdef.h"
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>

// HDF5 C API
#include <hdf5.h>

// Helper: load one PartType group into particles
void LoadPartType(hid_t file,
                  const std::string& group,
                  int internalType,
                  std::vector<Particle>& particles)
{
    // Coordinates
    if (H5Lexists(file, (group + "/Coordinates").c_str(), H5P_DEFAULT) <= 0)
        return;

    hid_t dset_coords = H5Dopen(file, (group + "/Coordinates").c_str(), H5P_DEFAULT);
    hid_t space_coords = H5Dget_space(dset_coords);

    hsize_t dims[2];
    H5Sget_simple_extent_dims(space_coords, dims, NULL);
    size_t N = dims[0]; // number of particles

    std::vector<float> coords(N*3);
    H5Dread(dset_coords, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, coords.data());

    H5Dclose(dset_coords);
    H5Sclose(space_coords);

    // Velocities
    hid_t dset_vels = H5Dopen(file, (group + "/Velocities").c_str(), H5P_DEFAULT);
    std::vector<float> vels(N*3);
    H5Dread(dset_vels, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, vels.data());
    H5Dclose(dset_vels);

    // Masses
    hid_t dset_masses = H5Dopen(file, (group + "/Masses").c_str(), H5P_DEFAULT);
    std::vector<float> masses(N);
    H5Dread(dset_masses, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, masses.data());
    H5Dclose(dset_masses);

    // Convert into Particle structs
    for (size_t i = 0; i < N; i++) {
        Particle p;
        p.x = coords[3*i+0];
        p.y = coords[3*i+1];
        p.z = coords[3*i+2];
        p.vx = vels[3*i+0];
        p.vy = vels[3*i+1];
        p.vz = vels[3*i+2];
        p.m = masses[i];
        p.type = internalType;
        particles.push_back(p);
    }
}

std::vector<Particle> LoadParticlesFromFile(const std::string& filename)
{
    std::vector<Particle> p;

    // --- Try HDF5 first ---
    hid_t file = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file >= 0) {
        // Load PartType1 (DM → internal type=1)
        LoadPartType(file, "PartType1", 1, p);

        // Load PartType4 (Stars → internal type=0)
        LoadPartType(file, "PartType4", 0, p);

        H5Fclose(file);
        return p;
    }

    // --- Fallback: plain text loader ---
    std::ifstream in(filename);
    Particle temp;
    while (in >> temp.x >> temp.y >> temp.z >> temp.vx >> temp.vy >> temp.vz >> temp.m >> temp.type) {
        p.push_back(temp);
    }
    return p;
}
