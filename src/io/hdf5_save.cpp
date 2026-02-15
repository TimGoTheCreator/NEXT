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
#include <hdf5.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

/**
 * @brief Saves the ParticleSystem to HDF5. 
 * Detects real precision to match NEXT_FP32 or NEXT_FP64.
 */
void SaveHDF5(const ParticleSystem& ps, const std::string& filename)
{
    const size_t N = ps.size();
    if (N == 0) return;

    hid_t file = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (file < 0) return;

    hid_t group = H5Gcreate(file, "PartType1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    // 1. Handle Coordinate/Velocity zipping
    // We keep these as float for visualization efficiency (ParaView rarely needs double)
    std::vector<float> coords(N * 3);
    std::vector<float> vels(N * 3);
    std::vector<int> ids(N);

    #pragma omp parallel for
    for (size_t i = 0; i < N; i++) {
        coords[3*i+0] = (float)ps.x[i];
        coords[3*i+1] = (float)ps.y[i];
        coords[3*i+2] = (float)ps.z[i];
        vels[3*i+0]   = (float)ps.vx[i];
        vels[3*i+1]   = (float)ps.vy[i];
        vels[3*i+2]   = (float)ps.vz[i];
        ids[i] = (int)i + 1;
    }

    hsize_t dims3[2] = { N, 3 };
    hid_t space3 = H5Screate_simple(2, dims3, NULL);
    hsize_t dims1[1] = { N };
    hid_t space1 = H5Screate_simple(1, dims1, NULL);

    // Write Coords & Vels (storing as float32)
    hid_t dset_coords = H5Dcreate(group, "Coordinates", H5T_NATIVE_FLOAT, space3, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(dset_coords, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, coords.data());
    H5Dclose(dset_coords);

    hid_t dset_vels = H5Dcreate(group, "Velocities", H5T_NATIVE_FLOAT, space3, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(dset_vels, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, vels.data());
    H5Dclose(dset_vels);

    // --- TYPE-SAFE DIRECT WRITE ---
    // Detect if 'real' is float or double for Masses
    hid_t h5_real_type = (sizeof(real) == 4) ? H5T_NATIVE_FLOAT : H5T_NATIVE_DOUBLE;
    int precision = (sizeof(real) == 4) ? 4 : 8;

    hid_t dset_masses = H5Dcreate(group, "Masses", h5_real_type, space1, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(dset_masses, h5_real_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, ps.m.data());
    H5Dclose(dset_masses);

    hid_t dset_ids = H5Dcreate(group, "ParticleIDs", H5T_NATIVE_INT, space1, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(dset_ids, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, ids.data());
    H5Dclose(dset_ids);

    H5Sclose(space3);
    H5Sclose(space1);
    H5Gclose(group);
    H5Fclose(file);

    // --- XDMF SIDECAR ---
    std::string xdmf_filename = filename.substr(0, filename.find_last_of('.')) + ".xdmf";
    std::ofstream xmf(xdmf_filename);
    xmf << "<?xml version=\"1.0\" ?>\n<Xdmf Version=\"3.0\">\n  <Domain>\n";
    xmf << "    <Grid Name=\"Particles\" GridType=\"Uniform\">\n";
    xmf << "      <Topology TopologyType=\"Polyvertex\" NumberOfElements=\"" << N << "\"/>\n";
    xmf << "      <Geometry GeometryType=\"XYZ\">\n";
    xmf << "        <DataItem Dimensions=\"" << N << " 3\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n";
    xmf << "          " << filename << ":/PartType1/Coordinates\n";
    xmf << "        </DataItem>\n      </Geometry>\n";
    xmf << "      <Attribute Name=\"Mass\" AttributeType=\"Scalar\" Center=\"Node\">\n";
    // We dynamically set the precision here to match the file
    xmf << "        <DataItem Dimensions=\"" << N << "\" NumberType=\"Float\" Precision=\"" << precision << "\" Format=\"HDF\">\n";
    xmf << "          " << filename << ":/PartType1/Masses\n";
    xmf << "        </DataItem>\n      </Attribute>\n";
    xmf << "    </Grid>\n  </Domain>\n</Xdmf>\n";
    xmf.close();
}
