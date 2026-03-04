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
#include <hdf5.h>

/**
 * @brief Helper: load one PartType group from HDF5 into the SoA Particle database.
 */
void LoadPartType(hid_t file,
                  const std::string& group,
                  int internalType,
                  Particle& p) // p is now the SoA container
{
    // Coordinates Check
    if (H5Lexists(file, (group + "/Coordinates").c_str(), H5P_DEFAULT) <= 0)
        return;

    hid_t dset_coords = H5Dopen(file, (group + "/Coordinates").c_str(), H5P_DEFAULT);
    hid_t space_coords = H5Dget_space(dset_coords);

    hsize_t dims[2];
    H5Sget_simple_extent_dims(space_coords, dims, NULL);
    size_t N = dims[0];

    // Temporary buffers
    std::vector<float> coords(N * 3);
    std::vector<float> vels(N * 3);

    // Read Coordinates
    H5Dread(dset_coords, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, coords.data());

    // Read Velocities
    hid_t dset_vels = H5Dopen(file, (group + "/Velocities").c_str(), H5P_DEFAULT);
    H5Dread(dset_vels, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, vels.data());
    H5Dclose(dset_vels);

    // Reserve space
    size_t current_size = p.x.size();
    p.x.resize(current_size + N);
    p.y.resize(current_size + N);
    p.z.resize(current_size + N);
    p.vx.resize(current_size + N);
    p.vy.resize(current_size + N);
    p.vz.resize(current_size + N);
    p.m.resize(current_size + N);
    p.type.resize(current_size + N);

    // Read Masses with type check
    hid_t dset_masses = H5Dopen(file, (group + "/Masses").c_str(), H5P_DEFAULT);
    hid_t dtype = H5Dget_type(dset_masses);

    if (H5Tequal(dtype, H5T_NATIVE_FLOAT)) {
        std::vector<float> masses_f(N);
        H5Dread(dset_masses, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, masses_f.data());
        for (size_t i = 0; i < N; ++i)
            p.m[current_size + i] = masses_f[i];
    } else if (H5Tequal(dtype, H5T_NATIVE_DOUBLE)) {
        std::vector<double> masses_d(N);
        H5Dread(dset_masses, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, masses_d.data());
        for (size_t i = 0; i < N; ++i)
            p.m[current_size + i] = static_cast<real>(masses_d[i]);
    } else {
        throw std::runtime_error("Unsupported Masses datatype");
    }

    H5Tclose(dtype);
    H5Dclose(dset_masses);

    // Fill SoA lanes
    for (size_t i = 0; i < N; ++i) {
        size_t idx = current_size + i;
        p.x[idx]  = coords[3*i+0];
        p.y[idx]  = coords[3*i+1];
        p.z[idx]  = coords[3*i+2];
        p.vx[idx] = vels[3*i+0];
        p.vy[idx] = vels[3*i+1];
        p.vz[idx] = vels[3*i+2];
        p.type[idx] = internalType;
    }

    // Cleanup
    H5Dclose(dset_coords);
    H5Sclose(space_coords);

}

/**
 * @brief Loads the Particle database from file.
 */
Particle LoadParticlesFromFile(const std::string& filename)
{
    Particle p; // The SoA container

    // --- Try HDF5 first ---
    hid_t file = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file >= 0) {
        LoadPartType(file, "PartType1", 1, p); // DM
        LoadPartType(file, "PartType4", 0, p); // Stars
        H5Fclose(file);
        return p;
    }

    // --- Fallback: plain text loader ---
    std::ifstream in(filename);
    real tx, ty, tz, tvx, tvy, tvz, tm;
    int tt;
    
    // Read column by column and push into SoA lanes
    while (in >> tx >> ty >> tz >> tvx >> tvy >> tvz >> tm >> tt) {
        p.x.push_back(tx);
        p.y.push_back(ty);
        p.z.push_back(tz);
        p.vx.push_back(tvx);
        p.vy.push_back(tvy);
        p.vz.push_back(tvz);
        p.m.push_back(tm);
        p.type.push_back(tt);
    }
    return p;
}
