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
 * @brief Saves the SoA Particle database to a VTK Legacy file for ParaView.
 */
inline void SaveVTK(const Particle& p, const std::string& filename)
{
    std::ofstream out(filename);
    if (!out) return;

    const size_t N = p.size(); // p is the SoA database
    out << "# vtk DataFile Version 3.0\n";
    out << "NEXT snapshot\n";
    out << "ASCII\n";
    out << "DATASET POLYDATA\n";

    // Detect precision based on your build macros
    #ifdef NEXT_FP64
    constexpr const char* vtkType = "double";
    #else
    constexpr const char* vtkType = "float";
    #endif

    // --- POINTS (Coordinates) ---
    out << "POINTS " << N << " " << vtkType << "\n";
    for (size_t i = 0; i < N; i++) {
        out << p.x[i] << " " << p.y[i] << " " << p.z[i] << "\n";
    }

    // --- VERTICES ---
    // VTK needs topology to render points as actual pixels
    out << "VERTICES " << N << " " << N * 2 << "\n";
    for (size_t i = 0; i < N; i++) {
        out << "1 " << i << "\n";
    }

    out << "POINT_DATA " << N << "\n";

    // --- Type (0 for Star, 1 for DM) ---
    out << "SCALARS type int 1\n";
    out << "LOOKUP_TABLE default\n";
    for (size_t i = 0; i < N; i++) {
        out << p.type[i] << "\n";
    }

    // --- Velocity Vectors ---
    out << "VECTORS velocity " << vtkType << "\n";
    for (size_t i = 0; i < N; i++) {
        out << p.vx[i] << " " << p.vy[i] << " " << p.vz[i] << "\n";
    }

    // --- Mass ---
    out << "SCALARS mass " << vtkType << " 1\n";
    out << "LOOKUP_TABLE default\n";
    for (size_t i = 0; i < N; i++) {
        out << p.m[i] << "\n";
    }

    out.close();
}
