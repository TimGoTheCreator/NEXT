#pragma once
#include<immintrin.h>
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

#if defined(NEXT_SIMD32)

inline real add(real a, real b) { return _mm_add_ps(a, b); }
inline real sub(real a, real b) { return _mm_sub_ps(a, b); }
inline real mul(real a, real b) { return _mm_mul_ps(a, b); }
inline real div(real a, real b) { return _mm_div_ps(a, b); }

#elif defined(NEXT_SIMD64)

inline real add(real a, real b) { return _mm_add_pd(a, b); }
inline real sub(real a, real b) { return _mm_sub_pd(a, b); }
inline real mul(real a, real b) { return _mm_mul_pd(a, b); }
inline real div(real a, real b) { return _mm_div_pd(a, b); }

#else

// scalar fallback
inline real add(real a, real b) { return a + b; }
inline real sub(real a, real b) { return a - b; }
inline real mul(real a, real b) { return a * b; }
inline real div(real a, real b) { return a / b; }

#endif