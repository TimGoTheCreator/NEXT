#include "hdf5_save.h"
#include <hdf5.h>
#include <vector>
#include <string>
#include <fstream>

void SaveHDF5(const std::vector<Particle>& p, const std::string& filename)
{
    hid_t file = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    // Group: PartType1 (DM)
    hid_t group = H5Gcreate(file, "PartType1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    size_t N = p.size();

    // Prepare arrays
    std::vector<float> coords(N * 3);
    std::vector<float> vels(N * 3);
    std::vector<float> masses(N);
    std::vector<int> ids(N);

    for (size_t i = 0; i < N; i++) {
        coords[3*i+0] = p[i].x;
        coords[3*i+1] = p[i].y;
        coords[3*i+2] = p[i].z;

        vels[3*i+0] = p[i].vx;
        vels[3*i+1] = p[i].vy;
        vels[3*i+2] = p[i].vz;

        masses[i] = p[i].m;
        ids[i] = i + 1; // Gadget requires IDs
    }

    // Dataspace for Nx3
    hsize_t dims3[2] = { N, 3 };
    hid_t space3 = H5Screate_simple(2, dims3, NULL);

    // Dataspace for N
    hsize_t dims1[1] = { N };
    hid_t space1 = H5Screate_simple(1, dims1, NULL);

    // Write datasets
    H5Dcreate(group, "Coordinates", H5T_NATIVE_FLOAT, space3, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(H5Dopen(group, "Coordinates", H5P_DEFAULT), H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, coords.data());

    H5Dcreate(group, "Velocities", H5T_NATIVE_FLOAT, space3, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(H5Dopen(group, "Velocities", H5P_DEFAULT), H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, vels.data());

    H5Dcreate(group, "Masses", H5T_NATIVE_FLOAT, space1, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(H5Dopen(group, "Masses", H5P_DEFAULT), H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, masses.data());

    H5Dcreate(group, "ParticleIDs", H5T_NATIVE_INT, space1, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(H5Dopen(group, "ParticleIDs", H5P_DEFAULT), H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, ids.data());

    // Cleanup
    H5Sclose(space3);
    H5Sclose(space1);
    H5Gclose(group);
    H5Fclose(file);

        // ===============================
    // Write XDMF sidecar for ParaView
    // ===============================
    std::string xdmf = filename.substr(0, filename.find_last_of('.')) + ".xdmf";
    std::ofstream xmf(xdmf);

    xmf << "<?xml version=\"1.0\" ?>\n";
    xmf << "<Xdmf Version=\"3.0\">\n";
    xmf << "  <Domain>\n";
    xmf << "    <Grid Name=\"Particles\" GridType=\"Uniform\">\n";
    xmf << "      <Topology TopologyType=\"Polyvertex\" NumberOfElements=\"" << N << "\"/>\n";
    xmf << "      <Geometry GeometryType=\"XYZ\">\n";
    xmf << "        <DataItem Dimensions=\"" << N << " 3\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n";
    xmf << "          " << filename << ":/PartType1/Coordinates\n";
    xmf << "        </DataItem>\n";
    xmf << "      </Geometry>\n";

    xmf << "      <Attribute Name=\"Velocity\" AttributeType=\"Vector\" Center=\"Node\">\n";
    xmf << "        <DataItem Dimensions=\"" << N << " 3\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n";
    xmf << "          " << filename << ":/PartType1/Velocities\n";
    xmf << "        </DataItem>\n";
    xmf << "      </Attribute>\n";

    xmf << "      <Attribute Name=\"Mass\" AttributeType=\"Scalar\" Center=\"Node\">\n";
    xmf << "        <DataItem Dimensions=\"" << N << "\" NumberType=\"Float\" Precision=\"4\" Format=\"HDF\">\n";
    xmf << "          " << filename << ":/PartType1/Masses\n";
    xmf << "        </DataItem>\n";
    xmf << "      </Attribute>\n";

    xmf << "      <Attribute Name=\"ID\" AttributeType=\"Scalar\" Center=\"Node\">\n";
    xmf << "        <DataItem Dimensions=\"" << N << "\" NumberType=\"Int\" Precision=\"4\" Format=\"HDF\">\n";
    xmf << "          " << filename << ":/PartType1/ParticleIDs\n";
    xmf << "        </DataItem>\n";
    xmf << "      </Attribute>\n";

    xmf << "    </Grid>\n";
    xmf << "  </Domain>\n";
    xmf << "</Xdmf>\n";

    xmf.close();

}
