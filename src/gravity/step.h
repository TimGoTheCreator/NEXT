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
#include "floatdef.h"
#include "octree.h"
#include "struct/particle.h"
#include <memory>
#include <algorithm>
#include <omp.h>
#ifdef NEXT_MPI
    #include <mpi.h>
#endif

/**
 * @brief Performs a complete Leapfrog Step (Kick-Drift-Kick) using SoA data.
 */
inline void Step(ParticleSystem &ps, real dt) {
    if (ps.size() == 0) return;

    const real theta = real(0.5);
    const real half  = dt * real(0.5);
    const int  N     = static_cast<int>(ps.size());

    // ---------------------------------------------------------
    // MPI SETUP
    // ---------------------------------------------------------
#ifdef NEXT_MPI
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Select MPI datatype matching
    MPI_Datatype MPI_REAL_T;
#  ifdef NEXT_FP64
    MPI_REAL_T = MPI_DOUBLE;
#  elif defined(NEXT_FP32)
    MPI_REAL_T = MPI_FLOAT;
#  else
#    error "Define NEXT_FP32 or NEXT_FP64 for 'real' type."
#  endif
#else
    int rank = 0;
    int size = 1;
#endif

    // ---------------------------------------------------------
    // DOMAIN DECOMPOSITION
    // ---------------------------------------------------------
    const int start = (rank * N) / size;
    const int end   = ((rank + 1) * N) / size;

#ifdef NEXT_MPI
    // Precompute counts and displacements for Allgatherv
    std::vector<int> counts(size), displs(size);
    for (int r = 0; r < size; ++r) {
        const int s = (r * N) / size;
        const int e = ((r + 1) * N) / size;
        counts[r]   = e - s;
        displs[r]   = s;
    }
#endif

    // ---------------------------------------------------------
    // TREE BUILDER
    // ---------------------------------------------------------
    auto buildTree = [&]() -> std::unique_ptr<Octree> {
        struct BBox { real minx, miny, minz, maxx, maxy, maxz; };
        BBox local{ real(1e30), real(1e30), real(1e30),
                    real(-1e30), real(-1e30), real(-1e30) };

        for (int i = 0; i < N; ++i) {
            local.minx = std::min(local.minx, ps.x[i]);
            local.miny = std::min(local.miny, ps.y[i]);
            local.minz = std::min(local.minz, ps.z[i]);
            local.maxx = std::max(local.maxx, ps.x[i]);
            local.maxy = std::max(local.maxy, ps.y[i]);
            local.maxz = std::max(local.maxz, ps.z[i]);
        }

#ifdef NEXT_MPI
        BBox global;
        real mins[3] = {local.minx, local.miny, local.minz};
        real maxs[3] = {local.maxx, local.maxy, local.maxz};

        MPI_Allreduce(MPI_IN_PLACE, mins, 3, MPI_REAL_T, MPI_MIN, MPI_COMM_WORLD);
        MPI_Allreduce(MPI_IN_PLACE, maxs, 3, MPI_REAL_T, MPI_MAX, MPI_COMM_WORLD);

        global.minx = mins[0]; global.miny = mins[1]; global.minz = mins[2];
        global.maxx = maxs[0]; global.maxy = maxs[1]; global.maxz = maxs[2];

#else
        BBox global = local;
#endif

        const real cx   = (global.minx + global.maxx) * real(0.5);
        const real cy   = (global.miny + global.maxy) * real(0.5);
        const real cz   = (global.minz + global.maxz) * real(0.5);
        real       size = std::max({global.maxx - global.minx,
                                    global.maxy - global.miny,
                                    global.maxz - global.minz}) * real(0.5);

        if (size <= real(0)) size = real(1.0);

        auto root = std::make_unique<Octree>(cx, cy, cz, size);

        for (int i = 0; i < N; ++i)
            root->insert(i, ps);

        root->computeMass(ps);
        return root;
    };

    // ---------------------------------------------------------
    // FIRST KICK
    // ---------------------------------------------------------
    {
        auto root = buildTree();

        #pragma omp parallel for schedule(dynamic, 64)
        for (int i = start; i < end; ++i) {
            real ax = real(0), ay = real(0), az = real(0);
            bhAccel(root.get(), i, ps, theta, ax, ay, az);

            ps.vx[i] += ax * half;
            ps.vy[i] += ay * half;
            ps.vz[i] += az * half;
        }

#ifdef NEXT_MPI
        MPI_Request reqs[3];
        MPI_Iallgatherv(ps.vx.data() + start, end - start, MPI_REAL_T,
                        ps.vx.data(), counts.data(), displs.data(), MPI_REAL_T,
                        MPI_COMM_WORLD, &reqs[0]);
        MPI_Iallgatherv(ps.vy.data() + start, end - start, MPI_REAL_T,
                        ps.vy.data(), counts.data(), displs.data(), MPI_REAL_T,
                        MPI_COMM_WORLD, &reqs[1]);
        MPI_Iallgatherv(ps.vz.data() + start, end - start, MPI_REAL_T,
                        ps.vz.data(), counts.data(), displs.data(), MPI_REAL_T,
                        MPI_COMM_WORLD, &reqs[2]);
        MPI_Waitall(3, reqs, MPI_STATUSES_IGNORE);
#endif
    }

    // ---------------------------------------------------------
    // DRIFT
    // ---------------------------------------------------------
    #pragma omp parallel for schedule(static)
    for (int i = start; i < end; ++i) {
        ps.x[i] += ps.vx[i] * dt;
        ps.y[i] += ps.vy[i] * dt;
        ps.z[i] += ps.vz[i] * dt;
    }

#ifdef NEXT_MPI
    MPI_Request reqs[3];
    MPI_Iallgatherv(ps.x.data() + start, end - start, MPI_REAL_T,
                    ps.x.data(), counts.data(), displs.data(), MPI_REAL_T,
                    MPI_COMM_WORLD, &reqs[0]);
    MPI_Iallgatherv(ps.y.data() + start, end - start, MPI_REAL_T,
                    ps.y.data(), counts.data(), displs.data(), MPI_REAL_T,
                    MPI_COMM_WORLD, &reqs[1]);
    MPI_Iallgatherv(ps.z.data() + start, end - start, MPI_REAL_T,
                    ps.z.data(), counts.data(), displs.data(), MPI_REAL_T,
                    MPI_COMM_WORLD, &reqs[2]);
    MPI_Waitall(3, reqs, MPI_STATUSES_IGNORE);
#endif

    // ---------------------------------------------------------
    // SECOND KICK
    // ---------------------------------------------------------
    {
        auto root = buildTree();

        #pragma omp parallel for schedule(dynamic, 64)
        for (int i = start; i < end; ++i) {
            real ax = real(0), ay = real(0), az = real(0);
            bhAccel(root.get(), i, ps, theta, ax, ay, az);

            ps.vx[i] += ax * half;
            ps.vy[i] += ay * half;
            ps.vz[i] += az * half;
        }

#ifdef NEXT_MPI
        MPI_Request reqs2[3];
        MPI_Iallgatherv(ps.vx.data() + start, end - start, MPI_REAL_T,
                        ps.vx.data(), counts.data(), displs.data(), MPI_REAL_T,
                        MPI_COMM_WORLD, &reqs2[0]);
        MPI_Iallgatherv(ps.vy.data() + start, end - start, MPI_REAL_T,
                        ps.vy.data(), counts.data(), displs.data(), MPI_REAL_T,
                        MPI_COMM_WORLD, &reqs2[1]);
        MPI_Iallgatherv(ps.vz.data() + start, end - start, MPI_REAL_T,
                        ps.vz.data(), counts.data(), displs.data(), MPI_REAL_T,
                        MPI_COMM_WORLD, &reqs2[2]);
        MPI_Waitall(3, reqs2, MPI_STATUSES_IGNORE);
#endif
    }
}
