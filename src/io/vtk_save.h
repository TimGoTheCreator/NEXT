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

    // draw points
    out << "POINTS " << N << " float\n";
    for (const auto& a : p)
        out << a.x << " " << a.y << " " << a.z << "\n";

    // draw vertices
    out << "VERTICES " << N << " " << N*2 << "\n";
    for (size_t i = 0; i < N; i++)
        out << "1 " << i << "\n";

    // that too
    out << "POINT_DATA " << N << "\n";
    out << "VECTORS velocity float\n";
    for (const auto& a : p)
        out << a.vx << " " << a.vy << " " << a.vz << "\n";

    out << "SCALARS mass float 1\n";
    out << "LOOKUP_TABLE default\n";
    for (const auto& a : p)
        out << a.m << "\n";
}
