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

#include <hdf5.h>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "floatdef.h"
#include "struct/particle.h"

/**
 * @brief Helper: load one PartType group from HDF5 into the SoA Particle database.
 */
void LoadPartType(hid_t file, const std::string& group, int internalType,
                  Particle& p)  // p is now the SoA container
{
    // Coordinates Check
    if (H5Lexists(file, (group + "/Coordinates").c_str(), H5P_DEFAULT) <= 0) return;

    hid_t dset_coords = H5Dopen(file, (group + "/Coordinates").c_str(), H5P_DEFAULT);
    hid_t space_coords = H5Dget_space(dset_coords);

    hsize_t dims[2];
    H5Sget_simple_extent_dims(space_coords, dims, NULL);
    size_t N = dims[0];

    // Temporary buffers for HDF5 read
    std::vector<float> coords(N * 3);
    std::vector<float> vels(N * 3);
    std::vector<float> masses(N);

    // Read everything from HDF5
    H5Dread(dset_coords, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, coords.data());

    hid_t dset_vels = H5Dopen(file, (group + "/Velocities").c_str(), H5P_DEFAULT);
    H5Dread(dset_vels, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, vels.data());

    hid_t dset_masses = H5Dopen(file, (group + "/Masses").c_str(), H5P_DEFAULT);
    H5Dread(dset_masses, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, masses.data());

    // --- RE-FILL SoA LANES ---
    // We reserve space to avoid multiple reallocations
    size_t current_size = p.x.size();
    p.x.resize(current_size + N);
    p.y.resize(current_size + N);
    p.z.resize(current_size + N);
    p.vx.resize(current_size + N);
    p.vy.resize(current_size + N);
    p.vz.resize(current_size + N);
    p.ax.resize(current_size + N, 0);
    p.ay.resize(current_size + N, 0);
    p.az.resize(current_size + N, 0);
    p.m.resize(current_size + N);
    p.type.resize(current_size + N);

    for (size_t i = 0; i < N; i++) {
        size_t idx = current_size + i;
        p.x[idx] = coords[3 * i + 0];
        p.y[idx] = coords[3 * i + 1];
        p.z[idx] = coords[3 * i + 2];
        p.vx[idx] = vels[3 * i + 0];
        p.vy[idx] = vels[3 * i + 1];
        p.vz[idx] = vels[3 * i + 2];
        p.m[idx] = masses[i];
        p.type[idx] = internalType;
    }

    // Cleanup
    H5Dclose(dset_coords);
    H5Dclose(dset_vels);
    H5Dclose(dset_masses);
    H5Sclose(space_coords);
}

/**
 * @brief Loads the Particle database from file.
 * Optionally extracts simulation time and next dump count if restarting.
 */
inline Particle LoadParticlesFromFile(const std::string& filename, double* out_sim_time = nullptr,
                                      int* out_dump_count = nullptr) {
    Particle p;  // The SoA container

    // --- Try HDF5 first ---
    hid_t file = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file >= 0) {
        // Check if root contains PartType groups (Standard single snapshot)
        bool has_root_part = (H5Lexists(file, "PartType0", H5P_DEFAULT) > 0 ||
                              H5Lexists(file, "PartType1", H5P_DEFAULT) > 0 ||
                              H5Lexists(file, "PartType4", H5P_DEFAULT) > 0);

        if (has_root_part) {
            LoadPartType(file, "PartType0", 0, p);  // Gas / Stars / Baryons
            LoadPartType(file, "PartType1", 1, p);  // Halo Dark Matter
            LoadPartType(file, "PartType2", 0, p);  // Disk Stars
            LoadPartType(file, "PartType3", 0, p);  // Bulge Stars
            LoadPartType(file, "PartType4", 0, p);  // Star Particles
            LoadPartType(file, "PartType5", 0, p);  // Black Holes / Sinks

            // Try reading /Header/Time
            if (H5Lexists(file, "Header", H5P_DEFAULT) > 0) {
                hid_t hgrp = H5Gopen(file, "Header", H5P_DEFAULT);
                if (hgrp >= 0) {
                    if (H5Aexists(hgrp, "Time") > 0) {
                        hid_t attr = H5Aopen(hgrp, "Time", H5P_DEFAULT);
                        double t_val = 0.0;
                        H5Aread(attr, H5T_NATIVE_DOUBLE, &t_val);
                        H5Aclose(attr);
                        if (out_sim_time) *out_sim_time = t_val;
                    }
                    H5Gclose(hgrp);
                }
            }

            // Extract dump number from filename if available (e.g. dump_50.hdf5)
            size_t pos = filename.find("dump_");
            if (pos != std::string::npos) {
                try {
                    int num = std::stoi(filename.substr(pos + 5));
                    if (out_dump_count) *out_dump_count = num + 1;
                } catch (...) {}
            }
        } else {
            // Check for multi-step groups /Snap_0, /Snap_1, ...
            int max_snap = -1;
            for (int s = 0; s < 1000000; s++) {
                std::string snap_name = "Snap_" + std::to_string(s);
                if (H5Lexists(file, snap_name.c_str(), H5P_DEFAULT) > 0) {
                    max_snap = s;
                } else {
                    break;
                }
            }

            if (max_snap >= 0) {
                std::string prefix = "Snap_" + std::to_string(max_snap);
                LoadPartType(file, prefix + "/PartType0", 0, p);
                LoadPartType(file, prefix + "/PartType1", 1, p);
                LoadPartType(file, prefix + "/PartType2", 0, p);
                LoadPartType(file, prefix + "/PartType3", 0, p);
                LoadPartType(file, prefix + "/PartType4", 0, p);
                LoadPartType(file, prefix + "/PartType5", 0, p);

                if (out_dump_count) *out_dump_count = max_snap + 1;

                // Read time attribute from Snap_K
                hid_t s_grp = H5Gopen(file, prefix.c_str(), H5P_DEFAULT);
                if (s_grp >= 0) {
                    if (H5Aexists(s_grp, "Time") > 0) {
                        hid_t attr = H5Aopen(s_grp, "Time", H5P_DEFAULT);
                        double t_val = 0.0;
                        H5Aread(attr, H5T_NATIVE_DOUBLE, &t_val);
                        H5Aclose(attr);
                        if (out_sim_time) *out_sim_time = t_val;
                    }
                    H5Gclose(s_grp);
                }

                // Fallback: if time is not in HDF5 attribute, read from .xdmf sidecar
                if (out_sim_time && *out_sim_time == 0.0) {
                    std::string xdmf_filename = filename.substr(0, filename.find_last_of('.')) + ".xdmf";
                    std::ifstream xf(xdmf_filename);
                    if (xf.is_open()) {
                        std::string line;
                        std::string target_step = "Step_" + std::to_string(max_snap);
                        bool in_step = false;
                        while (std::getline(xf, line)) {
                            if (line.find(target_step) != std::string::npos) {
                                in_step = true;
                            }
                            if (in_step && line.find("<Time Value=\"") != std::string::npos) {
                                size_t p1 = line.find("<Time Value=\"") + 13;
                                size_t p2 = line.find("\"", p1);
                                if (p2 != std::string::npos) {
                                    try {
                                        *out_sim_time = std::stod(line.substr(p1, p2 - p1));
                                    } catch (...) {}
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
        H5Fclose(file);
        return p;
    }

    // --- Fallback: plain text loader ---
    std::ifstream in(filename);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        real tx, ty, tz, tvx, tvy, tvz, tm;
        int tt = 0;  // Default to 0 (Stars/Baryons) if 8th column is omitted
        if (ss >> tx >> ty >> tz >> tvx >> tvy >> tvz >> tm) {
            ss >> tt;  // Try to read optional 8th column
            p.x.push_back(tx);
            p.y.push_back(ty);
            p.z.push_back(tz);
            p.vx.push_back(tvx);
            p.vy.push_back(tvy);
            p.vz.push_back(tvz);
            p.m.push_back(tm);
            p.type.push_back(tt);
        }
    }
    return p;
}

