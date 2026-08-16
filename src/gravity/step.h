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
#include <omp.h>

#include <algorithm>
#include <memory>

#include "floatdef.h"
#include "nfw.h"
#include "octree.h"
#include "struct/particle.h"
#ifdef NEXT_ENABLE_CUDA
#include "cuda/cuda_gravity.h"
#endif
#ifdef NEXT_MPI
#include <mpi.h>
#endif
#include <chrono>
#include <fstream>

#include "cosmology.h"
#include "pm_solver.h"

inline void Step(ParticleSystem& ps, real dt, bool use_treepm = false, int pm_grid = 128, real r_split_param = real(0.0), next::cosmology::Cosmology* cosmo = nullptr) {
    if (ps.size() == 0) return;

    // Cosmological scale factors (a = 1.0 for non-cosmological runs)
    real a_scale = (cosmo && cosmo->enabled) ? static_cast<real>(cosmo->a) : real(1.0);
    real drift_factor = (cosmo && cosmo->enabled) ? (dt / (a_scale * a_scale)) : dt;
    real kick_factor = (cosmo && cosmo->enabled) ? ((dt * real(0.5)) / a_scale) : (dt * real(0.5));

#ifdef NEXT_ENABLE_CUDA
#ifdef NEXT_BENCHMARK
    auto t_start = std::chrono::high_resolution_clock::now();
#endif

    const int N = static_cast<int>(ps.size());
    const real half = kick_factor;
    std::vector<float> ax, ay, az;
    static thread_local OctreePool cuda_pool;
    static next::gravity::PMSolver pm_solver;
    static bool pm_inited = false;

    real minx = 1e30, miny = 1e30, minz = 1e30;
    real maxx = -1e30, maxy = -1e30, maxz = -1e30;
    for (int i = 0; i < N; ++i) {
        minx = std::min(minx, ps.x[i]); miny = std::min(miny, ps.y[i]); minz = std::min(minz, ps.z[i]);
        maxx = std::max(maxx, ps.x[i]); maxy = std::max(maxy, ps.y[i]); maxz = std::max(maxz, ps.z[i]);
    }
    const real cx = (minx + maxx) * real(0.5);
    const real cy = (miny + maxy) * real(0.5);
    const real cz = (minz + maxz) * real(0.5);
    real size = std::max({maxx - minx, maxy - miny, maxz - minz}) * real(0.5);
    if (size <= real(0)) size = real(1.0);

    real box_size = size * real(2.5);
    real r_split = (r_split_param > real(0.0)) ? r_split_param : (box_size / pm_grid * real(1.25));

    if (use_treepm && (!pm_inited || pm_solver.get_grid_dim() != pm_grid)) {
        pm_solver.init(pm_grid, box_size, r_split);
        pm_inited = true;
    }

    auto buildTreeGPU = [&]() -> Octree* {
        cuda_pool.reset();
        Octree* root = cuda_pool.createNode(cx, cy, cz, size);
        for (int i = 0; i < N; ++i) root->insert(i, ps, cuda_pool);
        root->computeMass(ps);
        return root;
    };

    static std::vector<float> cached_ax, cached_ay, cached_az;
    static bool has_cached_accel = false;

    // If first step, compute initial acceleration
    if (!has_cached_accel || cached_ax.size() != static_cast<size_t>(N)) {
        auto root = buildTreeGPU();
        float r_cut_cuda = use_treepm ? static_cast<float>(r_split) : 0.0f;
        next::cuda::compute_gravity_cuda(root, ps, 1.0f, 0.05f, 0.5f, cached_ax, cached_ay, cached_az, r_cut_cuda);

        if (use_treepm) {
            std::vector<real> ax_pm, ay_pm, az_pm;
            pm_solver.compute_long_range_forces(ps, ax_pm, ay_pm, az_pm);
            for (int i = 0; i < N; ++i) {
                cached_ax[i] += static_cast<float>(ax_pm[i]);
                cached_ay[i] += static_cast<float>(ay_pm[i]);
                cached_az[i] += static_cast<float>(az_pm[i]);
            }
        }
        has_cached_accel = true;
    }

    // 1. Kick (half step using cached acceleration) + Drift (full step)
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; ++i) {
        ps.vx[i] += cached_ax[i] * half;
        ps.vy[i] += cached_ay[i] * half;
        ps.vz[i] += cached_az[i] * half;
        ps.x[i] += ps.vx[i] * drift_factor;
        ps.y[i] += ps.vy[i] * drift_factor;
        ps.z[i] += ps.vz[i] * drift_factor;
    }

    // 2. Build Tree ONCE for new particle positions & Compute New Acceleration
    {
        auto root = buildTreeGPU();
        float r_cut_cuda = use_treepm ? static_cast<float>(r_split) : 0.0f;
        next::cuda::compute_gravity_cuda(root, ps, 1.0f, 0.05f, 0.5f, cached_ax, cached_ay, cached_az, r_cut_cuda);

        if (use_treepm) {
            std::vector<real> ax_pm, ay_pm, az_pm;
            pm_solver.compute_long_range_forces(ps, ax_pm, ay_pm, az_pm);
            for (int i = 0; i < N; ++i) {
                cached_ax[i] += static_cast<float>(ax_pm[i]);
                cached_ay[i] += static_cast<float>(ay_pm[i]);
                cached_az[i] += static_cast<float>(az_pm[i]);
            }
        }
    }

    // 3. Final Kick (half step using new acceleration)
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; ++i) {
        ps.vx[i] += cached_ax[i] * half;
        ps.vy[i] += cached_ay[i] * half;
        ps.vz[i] += cached_az[i] * half;
        ps.ax[i] = static_cast<real>(cached_ax[i]);
        ps.ay[i] = static_cast<real>(cached_ay[i]);
        ps.az[i] = static_cast<real>(cached_az[i]);
    }

#ifdef NEXT_BENCHMARK
    auto t_end = std::chrono::high_resolution_clock::now();
    double elapsed_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    std::ofstream log("log.txt", std::ios::app);
    log << "[GPU Step Time] " << elapsed_ms << " ms" << std::endl;
#endif
    return;
#else

#ifdef NEXT_BENCHMARK
    auto t_start = std::chrono::high_resolution_clock::now();
#endif

    const real theta = real(0.5);
    const real half = dt * real(0.5);
    const int N = static_cast<int>(ps.size());

#ifdef NEXT_MPI
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Datatype MPI_REAL_T;
#ifdef NEXT_FP64
    MPI_REAL_T = MPI_DOUBLE;
#elif defined(NEXT_FP32)
    MPI_REAL_T = MPI_FLOAT;
#else
#error "Define NEXT_FP32 or NEXT_FP64 for 'real' type."
#endif
#else
    int rank = 0;
    int size = 1;
#endif

    const int start = (rank * N) / size;
    const int end = ((rank + 1) * N) / size;

#ifdef NEXT_MPI
    std::vector<int> counts(size), displs(size);
    for (int r = 0; r < size; ++r) {
        const int s = (r * N) / size;
        const int e = ((r + 1) * N) / size;
        counts[r] = e - s;
        displs[r] = s;
    }
#endif

    static thread_local OctreePool pool;

    auto buildTree = [&]() -> Octree* {
        pool.reset();

        struct BBox {
            real minx, miny, minz, maxx, maxy, maxz;
        };
        BBox local{real(1e30), real(1e30), real(1e30), real(-1e30), real(-1e30), real(-1e30)};

        for (int i = 0; i < N; ++i) {
            local.minx = std::min(local.minx, ps.x[i]);
            local.miny = std::min(local.miny, ps.y[i]);
            local.minz = std::min(local.minz, ps.z[i]);
            local.maxx = std::max(local.maxx, ps.x[i]);
            local.maxy = std::max(local.maxy, ps.y[i]);
            local.maxz = std::max(local.maxz, ps.z[i]);
        }

#ifdef NEXT_MPI
        real mins[3] = {local.minx, local.miny, local.minz};
        real maxs[3] = {local.maxx, local.maxy, local.maxz};

        MPI_Allreduce(MPI_IN_PLACE, mins, 3, MPI_REAL_T, MPI_MIN, MPI_COMM_WORLD);
        MPI_Allreduce(MPI_IN_PLACE, maxs, 3, MPI_REAL_T, MPI_MAX, MPI_COMM_WORLD);

        BBox global{mins[0], mins[1], mins[2], maxs[0], maxs[1], maxs[2]};
#else
        BBox global = local;
#endif

        const real cx = (global.minx + global.maxx) * real(0.5);
        const real cy = (global.miny + global.maxy) * real(0.5);
        const real cz = (global.minz + global.maxz) * real(0.5);
        real size = std::max({global.maxx - global.minx, global.maxy - global.miny,
                              global.maxz - global.minz}) *
                    real(0.5);

        if (size <= real(0)) size = real(1.0);

        Octree* root = pool.createNode(cx, cy, cz, size);

        for (int i = 0; i < N; ++i) root->insert(i, ps, pool);

        root->computeMass(ps);
        return root;
    };

    static next::gravity::PMSolver pm_solver_cpu;
    static bool pm_inited_cpu = false;

    real box_size = size * real(2.5);
    real r_split = (r_split_param > real(0.0)) ? r_split_param : (box_size / pm_grid * real(1.25));

    if (use_treepm && (!pm_inited_cpu || pm_solver_cpu.get_grid_dim() != pm_grid)) {
        pm_solver_cpu.init(pm_grid, box_size, r_split);
        pm_inited_cpu = true;
    }

    std::vector<real> ax_pm, ay_pm, az_pm;
    if (use_treepm) {
        pm_solver_cpu.compute_long_range_forces(ps, ax_pm, ay_pm, az_pm);
    }

    // FIRST KICK
    {
        auto root = buildTree();
        real r_cut_cpu = use_treepm ? r_split : real(0.0);

#pragma omp parallel for schedule(dynamic, 64)
        for (int i = start; i < end; ++i) {
            real ax = real(0), ay = real(0), az = real(0);
            bhAccel(root, i, ps, theta, ax, ay, az, r_cut_cpu);

            if (use_treepm) {
                ax += ax_pm[i];
                ay += ay_pm[i];
                az += az_pm[i];
            }

            ps.vx[i] += ax * half;
            ps.vy[i] += ay * half;
            ps.vz[i] += az * half;
        }

#ifdef NEXT_MPI
        MPI_Request reqs[3];
        MPI_Iallgatherv(MPI_IN_PLACE, 0, MPI_REAL_T, ps.vx.data(), counts.data(), displs.data(),
                        MPI_REAL_T, MPI_COMM_WORLD, &reqs[0]);
        MPI_Iallgatherv(MPI_IN_PLACE, 0, MPI_REAL_T, ps.vy.data(), counts.data(), displs.data(),
                        MPI_REAL_T, MPI_COMM_WORLD, &reqs[1]);
        MPI_Iallgatherv(MPI_IN_PLACE, 0, MPI_REAL_T, ps.vz.data(), counts.data(), displs.data(),
                        MPI_REAL_T, MPI_COMM_WORLD, &reqs[2]);
        MPI_Waitall(3, reqs, MPI_STATUSES_IGNORE);
#endif
    }

// DRIFT
#pragma omp parallel for schedule(static)
    for (int i = start; i < end; ++i) {
        ps.x[i] += ps.vx[i] * drift_factor;
        ps.y[i] += ps.vy[i] * drift_factor;
        ps.z[i] += ps.vz[i] * drift_factor;
    }

#ifdef NEXT_MPI
    MPI_Request reqs3[3];
    MPI_Iallgatherv(MPI_IN_PLACE, 0, MPI_REAL_T, ps.x.data(), counts.data(), displs.data(),
                    MPI_REAL_T, MPI_COMM_WORLD, &reqs3[0]);
    MPI_Iallgatherv(MPI_IN_PLACE, 0, MPI_REAL_T, ps.y.data(), counts.data(), displs.data(),
                    MPI_REAL_T, MPI_COMM_WORLD, &reqs3[1]);
    MPI_Iallgatherv(MPI_IN_PLACE, 0, MPI_REAL_T, ps.z.data(), counts.data(), displs.data(),
                    MPI_REAL_T, MPI_COMM_WORLD, &reqs3[2]);
    MPI_Waitall(3, reqs3, MPI_STATUSES_IGNORE);
#endif

    // SECOND KICK
    {
        if (use_treepm) {
            pm_solver_cpu.compute_long_range_forces(ps, ax_pm, ay_pm, az_pm);
        }

        auto root = buildTree();
        real r_cut_cpu = use_treepm ? r_split : real(0.0);

#pragma omp parallel for schedule(dynamic, 64)
        for (int i = start; i < end; ++i) {
            real ax = real(0), ay = real(0), az = real(0);
            bhAccel(root, i, ps, theta, ax, ay, az, r_cut_cpu);

            if (use_treepm) {
                ax += ax_pm[i];
                ay += ay_pm[i];
                az += az_pm[i];
            }

            ps.vx[i] += ax * half;
            ps.vy[i] += ay * half;
            ps.vz[i] += az * half;
        }

#ifdef NEXT_MPI
        MPI_Request reqs4[3];
        MPI_Iallgatherv(MPI_IN_PLACE, 0, MPI_REAL_T, ps.vx.data(), counts.data(), displs.data(),
                        MPI_REAL_T, MPI_COMM_WORLD, &reqs4[0]);
        MPI_Iallgatherv(MPI_IN_PLACE, 0, MPI_REAL_T, ps.vy.data(), counts.data(), displs.data(),
                        MPI_REAL_T, MPI_COMM_WORLD, &reqs4[1]);
        MPI_Iallgatherv(MPI_IN_PLACE, 0, MPI_REAL_T, ps.vz.data(), counts.data(), displs.data(),
                        MPI_REAL_T, MPI_COMM_WORLD, &reqs4[2]);
        MPI_Waitall(3, reqs4, MPI_STATUSES_IGNORE);
#endif
    }

#ifdef NEXT_BENCHMARK
    auto t_end = std::chrono::high_resolution_clock::now();
    double elapsed_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();

    if (rank == 0) {
        std::ofstream log("log.txt", std::ios::app);
        log << "Step time: " << elapsed_ms << " ms" << std::endl;
    }
#endif
#endif
}
