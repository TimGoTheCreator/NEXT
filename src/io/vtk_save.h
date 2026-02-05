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
#include "../struct/particle.h"

inline void SaveVTK(const std::vector<Particle>& p, const std::string& filename)
{
    std::ofstream out(filename);
    if (!out) return;

    const size_t N = p.size();
    out << "# vtk DataFile Version 3.0\n";
    out << "NEXT snapshot\n";
    out << "ASCII\n";
    out << "DATASET POLYDATA\n";

    #ifdef NEXT_FP64
    constexpr const char* vtkType = "double";
    #elif defined(NEXT_FP32)
    constexpr const char* vtkType = "float";
    #endif

    // draw points
    out << "POINTS " << N << " " << vtkType << "\n";
    for (const auto& a : p)
        out << a.x << " " << a.y << " " << a.z << "\n";

    // draw vertices
    out << "VERTICES " << N << " " << N*2 << "\n";
    for (size_t i = 0; i < N; i++)
        out << "1 " << i << "\n";

    // that too
    out << "POINT_DATA " << N << "\n";
    out << "VECTORS velocity " << vtkType << "\n";
    for (const auto& a : p)
        out << a.vx << " " << a.vy << " " << a.vz << "\n";

    out << "SCALARS mass " << vtkType << " 1\n";
    out << "LOOKUP_TABLE default\n";
    for (const auto& a : p)
        out << a.m << "\n";

    out.close();
}
