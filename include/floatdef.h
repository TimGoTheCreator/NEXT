#pragma once
#include <immintrin.h>

// =======================
// real type selection
// =======================
#if defined(NEXT_FP64)
    using real = double;

#elif defined(NEXT_FP32)
    using real = float;

#elif defined(NEXT_SIMD64)
    using real = __m128d;

#elif defined(NEXT_SIMD32)
    using real = __m128;

#else
    #error "You must define NEXT_FP64, NEXT_FP32, NEXT_SIMD32 or NEXT_SIMD64 to proceed."
#endif



// =======================
// operator overloads
// =======================

#if defined(NEXT_SIMD32)

// float SIMD (__m128)
inline real operator+(real a, real b) { return _mm_add_ps(a, b); }
inline real operator-(real a, real b) { return _mm_sub_ps(a, b); }
inline real operator*(real a, real b) { return _mm_mul_ps(a, b); }
inline real operator/(real a, real b) { return _mm_div_ps(a, b); }

#elif defined(NEXT_SIMD64)

// double SIMD (__m128d)
inline real operator+(real a, real b) { return _mm_add_pd(a, b); }
inline real operator-(real a, real b) { return _mm_sub_pd(a, b); }
inline real operator*(real a, real b) { return _mm_mul_pd(a, b); }
inline real operator/(real a, real b) { return _mm_div_pd(a, b); }

#endif