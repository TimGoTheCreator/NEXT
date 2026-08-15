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
#include <iostream>
#include <string>
#include <vector>

#include "struct/particle.h"

// Helper to write a Gadget-style PartType group
static void WritePartTypeGroup(hid_t file, const std::string& group_name,
                               const ParticleSystem& ps,
                               const std::vector<size_t>& indices,
                               hid_t h5_real_type) {
    const size_t N = indices.size();
    if (N == 0) return;

    hid_t group = H5Gcreate(file, group_name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (group < 0) return;

    std::vector<float> coords(N * 3);
    std::vector<float> vels(N * 3);
    std::vector<real> masses(N);
    std::vector<int> ids(N);

    for (size_t i = 0; i < N; i++) {
        size_t idx = indices[i];
        coords[3 * i + 0] = static_cast<float>(ps.x[idx]);
        coords[3 * i + 1] = static_cast<float>(ps.y[idx]);
        coords[3 * i + 2] = static_cast<float>(ps.z[idx]);

        vels[3 * i + 0] = static_cast<float>(ps.vx[idx]);
        vels[3 * i + 1] = static_cast<float>(ps.vy[idx]);
        vels[3 * i + 2] = static_cast<float>(ps.vz[idx]);

        masses[i] = ps.m[idx];
        ids[i] = static_cast<int>(idx + 1);
    }

    hsize_t dims3[2] = {N, 3};
    hid_t space3 = H5Screate_simple(2, dims3, NULL);
    hsize_t dims1[1] = {N};
    hid_t space1 = H5Screate_simple(1, dims1, NULL);

    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_alloc_time(dcpl, H5D_ALLOC_TIME_EARLY);

    // Coordinates
    hid_t dset_coords = H5Dcreate(group, "Coordinates", H5T_NATIVE_FLOAT, space3, H5P_DEFAULT,
                                  dcpl, H5P_DEFAULT);
    H5Dwrite(dset_coords, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, coords.data());
    H5Dclose(dset_coords);

    // Velocities
    hid_t dset_vels = H5Dcreate(group, "Velocities", H5T_NATIVE_FLOAT, space3, H5P_DEFAULT,
                                dcpl, H5P_DEFAULT);
    H5Dwrite(dset_vels, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, vels.data());
    H5Dclose(dset_vels);

    // Masses
    hid_t dset_masses =
        H5Dcreate(group, "Masses", h5_real_type, space1, H5P_DEFAULT, dcpl, H5P_DEFAULT);
    H5Dwrite(dset_masses, h5_real_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, masses.data());
    H5Dclose(dset_masses);

    // ParticleIDs
    hid_t dset_ids = H5Dcreate(group, "ParticleIDs", H5T_NATIVE_INT, space1, H5P_DEFAULT,
                               dcpl, H5P_DEFAULT);
    H5Dwrite(dset_ids, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, ids.data());
    H5Dclose(dset_ids);

    H5Pclose(dcpl);
    H5Sclose(space3);
    H5Sclose(space1);
    H5Gclose(group);
}

/**
 * @brief Saves the ParticleSystem to Gadget-compliant HDF5.
 * Separates Baryons (PartType4) and Dark Matter (PartType1), and writes /Header metadata.
 */
void SaveHDF5(const ParticleSystem& ps, const std::string& filename) {
    const size_t totalN = ps.size();
    if (totalN == 0) return;

    std::vector<size_t> stars_indices;
    std::vector<size_t> dm_indices;
    stars_indices.reserve(totalN);
    dm_indices.reserve(totalN);

    for (size_t i = 0; i < totalN; i++) {
        if (ps.type[i] == 0) {
            stars_indices.push_back(i);
        } else {
            dm_indices.push_back(i);
        }
    }

    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_libver_bounds(fapl, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
    H5Pset_meta_block_size(fapl, 1024 * 1024);
    H5Pset_alignment(fapl, 4096, 4096);

    hid_t file = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, fapl);
    H5Pclose(fapl);
    if (file < 0) return;

    // --- Write Gadget Header ---
    hid_t header_group = H5Gcreate(file, "Header", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (header_group >= 0) {
        int num_part_this_file[6] = {0, static_cast<int>(dm_indices.size()), 0, 0,
                                      static_cast<int>(stars_indices.size()), 0};
        unsigned int num_part_total[6] = {0, static_cast<unsigned int>(dm_indices.size()), 0, 0,
                                           static_cast<unsigned int>(stars_indices.size()), 0};
        double mass_table[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        double time = 0.0;
        double redshift = 0.0;
        int num_files = 1;

        hsize_t dims6[1] = {6};
        hid_t space6 = H5Screate_simple(1, dims6, NULL);

        hid_t attr;
        attr = H5Acreate(header_group, "NumPart_ThisFile", H5T_NATIVE_INT, space6, H5P_DEFAULT, H5P_DEFAULT);
        H5Awrite(attr, H5T_NATIVE_INT, num_part_this_file);
        H5Aclose(attr);

        attr = H5Acreate(header_group, "NumPart_Total", H5T_NATIVE_UINT, space6, H5P_DEFAULT, H5P_DEFAULT);
        H5Awrite(attr, H5T_NATIVE_UINT, num_part_total);
        H5Aclose(attr);

        attr = H5Acreate(header_group, "MassTable", H5T_NATIVE_DOUBLE, space6, H5P_DEFAULT, H5P_DEFAULT);
        H5Awrite(attr, H5T_NATIVE_DOUBLE, mass_table);
        H5Aclose(attr);

        hsize_t dims1[1] = {1};
        hid_t space1 = H5Screate_simple(1, dims1, NULL);

        attr = H5Acreate(header_group, "Time", H5T_NATIVE_DOUBLE, space1, H5P_DEFAULT, H5P_DEFAULT);
        H5Awrite(attr, H5T_NATIVE_DOUBLE, &time);
        H5Aclose(attr);

        attr = H5Acreate(header_group, "Redshift", H5T_NATIVE_DOUBLE, space1, H5P_DEFAULT, H5P_DEFAULT);
        H5Awrite(attr, H5T_NATIVE_DOUBLE, &redshift);
        H5Aclose(attr);

        attr = H5Acreate(header_group, "NumFilesPerSnapshot", H5T_NATIVE_INT, space1, H5P_DEFAULT, H5P_DEFAULT);
        H5Awrite(attr, H5T_NATIVE_INT, &num_files);
        H5Aclose(attr);

        H5Sclose(space6);
        H5Sclose(space1);
        H5Gclose(header_group);
    }

    hid_t h5_real_type = (sizeof(real) == 4) ? H5T_NATIVE_FLOAT : H5T_NATIVE_DOUBLE;
    int precision = (sizeof(real) == 4) ? 4 : 8;

    // --- Write Groups ---
    WritePartTypeGroup(file, "PartType1", ps, dm_indices, h5_real_type);     // Dark Matter
    WritePartTypeGroup(file, "PartType4", ps, stars_indices, h5_real_type);  // Stars / Baryons

    H5Fclose(file);

    // --- XDMF SIDECAR ---
    std::string xdmf_filename = filename.substr(0, filename.find_last_of('.')) + ".xdmf";
    std::ofstream xmf(xdmf_filename);
    xmf << "<?xml version=\"1.0\" ?>\n<Xdmf Version=\"3.0\">\n  <Domain>\n";
    xmf << "    <Grid Name=\"ParticleCollection\" GridType=\"Collection\" CollectionType=\"Spatial\">\n";

    auto write_grid = [&](const std::string& name, const std::string& group, size_t count) {
        if (count == 0) return;
        xmf << "      <Grid Name=\"" << name << "\" GridType=\"Uniform\">\n";
        xmf << "        <Topology TopologyType=\"Polyvertex\" NumberOfElements=\"" << count << "\"/>\n";
        xmf << "        <Geometry GeometryType=\"XYZ\">\n";
        xmf << "          <DataItem Dimensions=\"" << count
            << " 3\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n";
        xmf << "            " << filename << ":/" << group << "/Coordinates\n";
        xmf << "          </DataItem>\n        </Geometry>\n";
        xmf << "        <Attribute Name=\"Mass\" AttributeType=\"Scalar\" Center=\"Node\">\n";
        xmf << "          <DataItem Dimensions=\"" << count << "\" NumberType=\"Float\" Precision=\""
            << precision << "\" Format=\"HDF\">\n";
        xmf << "            " << filename << ":/" << group << "/Masses\n";
        xmf << "          </DataItem>\n        </Attribute>\n";
        xmf << "        <Attribute Name=\"Velocity\" AttributeType=\"Vector\" Center=\"Node\">\n";
        xmf << "          <DataItem Dimensions=\"" << count << " 3\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n";
        xmf << "            " << filename << ":/" << group << "/Velocities\n";
        xmf << "          </DataItem>\n        </Attribute>\n";
        xmf << "        <Attribute Name=\"ParticleID\" AttributeType=\"Scalar\" Center=\"Node\">\n";
        xmf << "          <DataItem Dimensions=\"" << count << "\" NumberType=\"Int\" Precision=\"4\" Format=\"HDF\">\n";
        xmf << "            " << filename << ":/" << group << "/ParticleIDs\n";
        xmf << "          </DataItem>\n        </Attribute>\n";
        xmf << "      </Grid>\n";
    };

    write_grid("DarkMatter", "PartType1", dm_indices.size());
    write_grid("Stars", "PartType4", stars_indices.size());

    xmf << "    </Grid>\n  </Domain>\n</Xdmf>\n";
    xmf.close();
}

/**
 * @brief Saves the ParticleSystem into a single HDF5 file with step groups /Snap_N.
 * Extremely fast: opens file once and appends step snapshots.
 */
void SaveHDF5SingleStep(const ParticleSystem& ps, const std::string& filename, int step, double time) {
    const size_t totalN = ps.size();
    if (totalN == 0) return;

    std::vector<size_t> stars_indices;
    std::vector<size_t> dm_indices;
    stars_indices.reserve(totalN);
    dm_indices.reserve(totalN);

    for (size_t i = 0; i < totalN; i++) {
        if (ps.type[i] == 0) {
            stars_indices.push_back(i);
        } else {
            dm_indices.push_back(i);
        }
    }

    hid_t file = -1;
    if (step == 0) {
        file = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    } else {
        file = H5Fopen(filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
        if (file < 0) {
            file = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        }
    }

    if (file < 0) return;

    std::string snap_name = "Snap_" + std::to_string(step);
    // If group already exists, delete or open it safely
    if (H5Lexists(file, snap_name.c_str(), H5P_DEFAULT) > 0) {
        H5Ldelete(file, snap_name.c_str(), H5P_DEFAULT);
    }

    hid_t snap_group = H5Gcreate(file, snap_name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (snap_group >= 0) {
        hsize_t dims1[1] = {1};
        hid_t space1 = H5Screate_simple(1, dims1, NULL);
        hid_t attr = H5Acreate(snap_group, "Time", H5T_NATIVE_DOUBLE, space1, H5P_DEFAULT, H5P_DEFAULT);
        if (attr >= 0) {
            H5Awrite(attr, H5T_NATIVE_DOUBLE, &time);
            H5Aclose(attr);
        }
        H5Sclose(space1);

        hid_t h5_real_type = (sizeof(real) == 4) ? H5T_NATIVE_FLOAT : H5T_NATIVE_DOUBLE;
        WritePartTypeGroup(snap_group, "PartType1", ps, dm_indices, h5_real_type);
        WritePartTypeGroup(snap_group, "PartType4", ps, stars_indices, h5_real_type);
        H5Gclose(snap_group);
    }

    // --- TEMPORAL XDMF SIDECAR FOR PARAVIEW TIME SLIDER ---
    std::string xdmf_filename = filename.substr(0, filename.find_last_of('.')) + ".xdmf";
    static std::vector<std::pair<int, double>> time_history;
    if (step == 0) {
        time_history.clear();
    } else if (time_history.empty()) {
        // Recover prior snapshot times if restarting
        for (int s = 0; s < step; s++) {
            std::string prev_snap = "Snap_" + std::to_string(s);
            if (H5Lexists(file, prev_snap.c_str(), H5P_DEFAULT) > 0) {
                hid_t s_grp = H5Gopen(file, prev_snap.c_str(), H5P_DEFAULT);
                double t_prev = static_cast<double>(s);
                if (s_grp >= 0) {
                    if (H5Aexists(s_grp, "Time") > 0) {
                        hid_t attr = H5Aopen(s_grp, "Time", H5P_DEFAULT);
                        H5Aread(attr, H5T_NATIVE_DOUBLE, &t_prev);
                        H5Aclose(attr);
                    }
                    H5Gclose(s_grp);
                }
                time_history.push_back({s, t_prev});
            }
        }
    }
    time_history.push_back({step, time});

    H5Fclose(file);

    std::ofstream xmf(xdmf_filename);
    xmf << "<?xml version=\"1.0\" ?>\n<Xdmf Version=\"3.0\">\n  <Domain>\n";
    xmf << "    <Grid Name=\"TimeSeries\" GridType=\"Collection\" CollectionType=\"Temporal\">\n";

    for (const auto& entry : time_history) {
        int s = entry.first;
        double t = entry.second;
        std::string snap = "Snap_" + std::to_string(s);
        xmf << "      <Grid Name=\"Step_" << s << "\" GridType=\"Collection\" CollectionType=\"Spatial\">\n";
        xmf << "        <Time Value=\"" << t << "\"/>\n";

        auto write_grid = [&](const std::string& name, const std::string& group, size_t count) {
            if (count == 0) return;
            xmf << "        <Grid Name=\"" << name << "\" GridType=\"Uniform\">\n";
            xmf << "          <Topology TopologyType=\"Polyvertex\" NumberOfElements=\"" << count << "\"/>\n";
            xmf << "          <Geometry GeometryType=\"XYZ\">\n";
            xmf << "            <DataItem Dimensions=\"" << count
                << " 3\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n";
            xmf << "              " << filename << ":/" << snap << "/" << group << "/Coordinates\n";
            xmf << "            </DataItem>\n          </Geometry>\n";
            xmf << "          <Attribute Name=\"Mass\" AttributeType=\"Scalar\" Center=\"Node\">\n";
            xmf << "            <DataItem Dimensions=\"" << count << "\" NumberType=\"Float\" Precision=\""
                << ((sizeof(real) == 4) ? 4 : 8) << "\" Format=\"HDF\">\n";
            xmf << "              " << filename << ":/" << snap << "/" << group << "/Masses\n";
            xmf << "            </DataItem>\n          </Attribute>\n";
            xmf << "          <Attribute Name=\"Velocity\" AttributeType=\"Vector\" Center=\"Node\">\n";
            xmf << "            <DataItem Dimensions=\"" << count << " 3\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n";
            xmf << "              " << filename << ":/" << snap << "/" << group << "/Velocities\n";
            xmf << "            </DataItem>\n          </Attribute>\n";
            xmf << "          <Attribute Name=\"ParticleID\" AttributeType=\"Scalar\" Center=\"Node\">\n";
            xmf << "            <DataItem Dimensions=\"" << count << "\" NumberType=\"Int\" Precision=\"4\" Format=\"HDF\">\n";
            xmf << "              " << filename << ":/" << snap << "/" << group << "/ParticleIDs\n";
            xmf << "            </DataItem>\n          </Attribute>\n";
            xmf << "        </Grid>\n";
        };

        write_grid("DarkMatter", "PartType1", dm_indices.size());
        write_grid("Stars", "PartType4", stars_indices.size());
        xmf << "      </Grid>\n";
    }

    xmf << "    </Grid>\n  </Domain>\n</Xdmf>\n";
    xmf.close();
}

