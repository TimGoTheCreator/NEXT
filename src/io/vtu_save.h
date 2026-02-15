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

#pragma once
#include <vector>
#include <string>
#include <fstream>
#include "struct/particle.h"

/**
 * @brief Saves the SoA Particle database to a VTU (XML) file.
 * This is the modern VTK format preferred by newer versions of ParaView.
 */
inline void SaveVTU(const Particle& p, const std::string& filename)
{
    std::ofstream out(filename);
    if (!out) return;

    const size_t N = p.size(); // Total particles in the database

    out << "<?xml version=\"1.0\"?>\n";
    out << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\">\n";
    out << "  <UnstructuredGrid>\n";
    out << "    <Piece NumberOfPoints=\"" << N 
        << "\" NumberOfCells=\"" << N << "\">\n";

    // --- POINTS (Coordinates) ---
    out << "      <Points>\n";
    out << "        <DataArray type=\"Float32\" NumberOfComponents=\"3\" format=\"ascii\">\n          ";
    for (size_t i = 0; i < N; i++)
        out << p.x[i] << " " << p.y[i] << " " << p.z[i] << " ";
    out << "\n        </DataArray>\n      </Points>\n";

    // --- CELLS (Topology) ---
    out << "      <Cells>\n";
    out << "        <DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">\n          ";
    for (size_t i = 0; i < N; i++) out << i << " ";
    out << "\n        </DataArray>\n";

    out << "        <DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">\n          ";
    for (size_t i = 1; i <= N; i++) out << i << " ";
    out << "\n        </DataArray>\n";

    out << "        <DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n          ";
    for (size_t i = 0; i < N; i++) out << "1 "; // 1 = VTK_VERTEX
    out << "\n        </DataArray>\n      </Cells>\n";

    // --- POINT DATA (Attributes) ---
    out << "      <PointData>\n";

    // Particle Type (0 = Star, 1 = DM)
    out << "        <DataArray type=\"Int32\" Name=\"type\" format=\"ascii\">\n          ";
    for (size_t i = 0; i < N; i++)
        out << p.type[i] << " ";
    out << "\n        </DataArray>\n";

    // Velocity
    out << "        <DataArray type=\"Float32\" Name=\"velocity\" NumberOfComponents=\"3\" format=\"ascii\">\n          ";
    for (size_t i = 0; i < N; i++)
        out << p.vx[i] << " " << p.vy[i] << " " << p.vz[i] << " ";
    out << "\n        </DataArray>\n";

    // Mass
    out << "        <DataArray type=\"Float32\" Name=\"mass\" format=\"ascii\">\n          ";
    for (size_t i = 0; i < N; i++)
        out << p.m[i] << " ";
    out << "\n        </DataArray>\n";

    out << "      </PointData>\n    </Piece>\n  </UnstructuredGrid>\n</VTKFile>\n";
}
