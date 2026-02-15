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

#include "particle_system.h"
#include <hdf5.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

/**
 * @brief Saves the ParticleSystem to an HDF5 file and creates an XDMF sidecar.
 * Because we use SoA, we can pass buffer pointers directly to H5Dwrite.
 */
void SaveHDF5(const ParticleSystem& ps, const std::string& filename)
{
    const size_t N = ps.size();
    if (N == 0) return;

    hid_t file = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (file < 0) return;

    // Gadget format uses PartType1 for DM and PartType4 for Stars.
    // To keep it simple, we'll put all particles in PartType1, 
    // but you could split them using ps.type[i].
    hid_t group = H5Gcreate(file, "PartType1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    // 1. Prepare Coordinates (Nx3 interleaved)
    // Unfortunately, HDF5 Gadget format expects XYZXYZXYZ. 
    // Since our SoA is XXXXX, YYYYY, ZZZZZ, we DO need one interleaved buffer for Coords.
    std::vector<float> coords(N * 3);
    std::vector<float> vels(N * 3);
    std::vector<int> ids(N);

    #pragma omp parallel for
    for (size_t i = 0; i < N; i++) {
        coords[3*i+0] = (float)ps.x[i];
        coords[3*i+1] = (float)ps.y[i];
        coords[3*i+2] = (float)ps.z[i];

        vels[3*i+0] = (float)ps.vx[i];
        vels[3*i+1] = (float)ps.vy[i];
        vels[3*i+2] = (float)ps.vz[i];
        
        ids[i] = (int)i + 1;
    }

    // Dataspaces
    hsize_t dims3[2] = { N, 3 };
    hid_t space3 = H5Screate_simple(2, dims3, NULL);
    hsize_t dims1[1] = { N };
    hid_t space1 = H5Screate_simple(1, dims1, NULL);

    // Write Coordinates
    hid_t dset_coords = H5Dcreate(group, "Coordinates", H5T_NATIVE_FLOAT, space3, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(dset_coords, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, coords.data());
    H5Dclose(dset_coords);

    // Write Velocities
    hid_t dset_vels = H5Dcreate(group, "Velocities", H5T_NATIVE_FLOAT, space3, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(dset_vels, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, vels.data());
    H5Dclose(dset_vels);

    // --- DIRECT WRITE WIN ---
    // For Masses and Types, we don't need temporary vectors! 
    // We can pass the ps.m.data() pointer directly.
    hid_t dset_masses = H5Dcreate(group, "Masses", H5T_NATIVE_FLOAT, space1, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(dset_masses, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, ps.m.data());
    H5Dclose(dset_masses);

    hid_t dset_ids = H5Dcreate(group, "ParticleIDs", H5T_NATIVE_INT, space1, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(dset_ids, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, ids.data());
    H5Dclose(dset_ids);

    // Cleanup HDF5
    H5Sclose(space3);
    H5Sclose(space1);
    H5Gclose(group);
    H5Fclose(file);

    // ===============================
    // Write XDMF sidecar for ParaView
    // ===============================
    std::string xdmf_filename = filename.substr(0, filename.find_last_of('.')) + ".xdmf";
    std::ofstream xmf(xdmf_filename);
    
    // (XDMF content remains same as your original version)
    xmf << "<?xml version=\"1.0\" ?>\n<Xdmf Version=\"3.0\">\n  <Domain>\n";
    xmf << "    <Grid Name=\"Particles\" GridType=\"Uniform\">\n";
    xmf << "      <Topology TopologyType=\"Polyvertex\" NumberOfElements=\"" << N << "\"/>\n";
    xmf << "      <Geometry GeometryType=\"XYZ\">\n";
    xmf << "        <DataItem Dimensions=\"" << N << " 3\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n";
    xmf << "          " << filename << ":/PartType1/Coordinates\n";
    xmf << "        </DataItem>\n      </Geometry>\n";
    xmf << "      <Attribute Name=\"Mass\" AttributeType=\"Scalar\" Center=\"Node\">\n";
    xmf << "        <DataItem Dimensions=\"" << N << "\" NumberType=\"Float\" Precision=\"8\" Format=\"HDF\">\n";
    xmf << "          " << filename << ":/PartType1/Masses\n";
    xmf << "        </DataItem>\n      </Attribute>\n";
    xmf << "    </Grid>\n  </Domain>\n</Xdmf>\n";
    xmf.close();
}
