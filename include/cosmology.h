// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#pragma once
#include <cmath>
#include "floatdef.h"

namespace next {
namespace cosmology {

/**
 * @brief Standard Lambda-CDM Cosmological Parameter State
 */
struct Cosmology {
    bool enabled = false;          // True if cosmological comoving integration is active
    double Omega_m = 0.3089;       // Total matter density parameter (baryonic + dark matter)
    double Omega_lambda = 0.6911;  // Dark energy / cosmological constant density
    double Omega_r = 0.0;          // Radiation density parameter
    double Omega_k = 0.0;          // Spatial curvature (0 for flat universe)
    double H0 = 1.0;               // Dimensionless Hubble parameter in internal simulation units
    double h = 0.6774;             // Dimensionless Hubble parameter (H0 / 100)

    double a = 1.0;                // Current scale factor (a = 1 / (1 + z))
    double z = 0.0;                // Current cosmological redshift
    double a_start = 0.02;         // Initial scale factor (e.g. z = 49 -> a = 0.02)
    double a_end = 1.0;            // Final scale factor (z = 0 -> a = 1.0)

    /**
     * @brief Evaluates dimensionless Hubble expansion rate E(a) = H(a) / H0
     * E(a) = sqrt(Omega_m * a^-3 + Omega_r * a^-4 + Omega_k * a^-2 + Omega_lambda)
     */
    inline double E_a(double scale_a) const {
        if (scale_a <= 0.001) scale_a = 0.001;
        double a_inv = 1.0 / scale_a;
        double a_inv2 = a_inv * a_inv;
        double a_inv3 = a_inv2 * a_inv;
        double a_inv4 = a_inv2 * a_inv2;

        double term = Omega_m * a_inv3 + Omega_r * a_inv4 + Omega_k * a_inv2 + Omega_lambda;
        return std::sqrt(std::max(0.0, term));
    }

    /**
     * @brief Computes physical Hubble parameter H(a) = H0 * E(a)
     */
    inline double H_a(double scale_a) const {
        return H0 * E_a(scale_a);
    }

    /**
     * @brief Computes dt / da = 1 / (a * H(a))
     */
    inline double dt_da(double scale_a) const {
        double h_val = H_a(scale_a);
        if (h_val <= 0.0) return 0.0;
        return 1.0 / (scale_a * h_val);
    }

    /**
     * @brief Updates scale factor 'a' by a physical or cosmological time step dt
     */
    inline void advance_scale_factor(double dt) {
        if (!enabled) return;
        // In cosmological time integration, da = a * H(a) * dt
        double da = a * H_a(a) * dt * 0.05; // Smooth scale factor advancement over simulation run
        a += da;
        if (a > 1.0) a = 1.0;
        if (a < 0.001) a = 0.001;
        z = (1.0 / a) - 1.0;
    }

    /**
     * @brief Sets redshift z and updates scale factor a accordingly
     */
    inline void set_redshift(double new_z) {
        z = new_z;
        a = 1.0 / (1.0 + z);
    }
};

} // namespace cosmology
} // namespace next
