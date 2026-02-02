#pragma once
#include <vector>
#include <string>
#include <fstream>
#include "../struct/particle.h"

inline void SaveVTU(const std::vector<Particle>& p, const std::string& filename)
{
    std::ofstream out(filename);
    if (!out) return;

    const size_t N = p.size();

    out << "<?xml version=\"1.0\"?>\n";
    out << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\">\n";
    out << "  <UnstructuredGrid>\n";
    out << "    <Piece NumberOfPoints=\"" << N 
        << "\" NumberOfCells=\"" << N << "\">\n";

    // --- POINTS ---
    out << "      <Points>\n";
    out << "        <DataArray type=\"Float32\" NumberOfComponents=\"3\" format=\"ascii\">\n          ";
    for (const auto& a : p)
        out << a.x << " " << a.y << " " << a.z << " ";
    out << "\n        </DataArray>\n";
    out << "      </Points>\n";

    // --- CELLS ---
    out << "      <Cells>\n";

    // connectivity
    out << "        <DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">\n          ";
    for (size_t i = 0; i < N; i++)
        out << i << " ";
    out << "\n        </DataArray>\n";

    // offsets
    out << "        <DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">\n          ";
    for (size_t i = 1; i <= N; i++)
        out << i << " ";
    out << "\n        </DataArray>\n";

    // types (1 = VTK_VERTEX)
    out << "        <DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n          ";
    for (size_t i = 0; i < N; i++)
        out << "1 ";
    out << "\n        </DataArray>\n";

    out << "      </Cells>\n";

    // --- POINT DATA ---
    out << "      <PointData>\n";

    // velocity vector
    out << "        <DataArray type=\"Float32\" Name=\"velocity\" NumberOfComponents=\"3\" format=\"ascii\">\n          ";
    for (const auto& a : p)
        out << a.vx << " " << a.vy << " " << a.vz << " ";
    out << "\n        </DataArray>\n";

    // mass scalar
    out << "        <DataArray type=\"Float32\" Name=\"mass\" format=\"ascii\">\n          ";
    for (const auto& a : p)
        out << a.m << " ";
    out << "\n        </DataArray>\n";

    out << "      </PointData>\n";

    out << "    </Piece>\n";
    out << "  </UnstructuredGrid>\n";
    out << "</VTKFile>\n";
}