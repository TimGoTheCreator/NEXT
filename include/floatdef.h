#pragma once
#include <immintrin.h>

//---------------------------------------------
// Precision / SIMD mode selection
//---------------------------------------------
#if defined(NEXT_FP64)

    using real = double;

#elif defined(NEXT_FP32)

    using real = float;

#elif defined(NEXT_SIMD64)

    using real = __m128d;    // 2-wide double (SSE)

#elif defined(NEXT_SIMD32)

    using real = __m128;     // 4-wide float (SSE)

#elif defined(NEXT_AVX512_64)

    using real = __m512d;    // 8-wide double (AVX-512)

#elif defined(NEXT_AVX512_32)

    using real = __m512;     // 16-wide float (AVX-512)

#else
    #error "Define one of: NEXT_FP64, NEXT_FP32, NEXT_SIMD32, NEXT_SIMD64, NEXT_AVX512_32, NEXT_AVX512_64."
#endif


//---------------------------------------------
// SIMD operator overloads
//---------------------------------------------
#if defined(NEXT_SIMD32)

inline real operator+(real a, real b) { return _mm_add_ps(a, b); }
inline real operator-(real a, real b) { return _mm_sub_ps(a, b); }
inline real operator*(real a, real b) { return _mm_mul_ps(a, b); }
inline real operator/(real a, real b) { return _mm_div_ps(a, b); }

#elif defined(NEXT_SIMD64)

inline real operator+(real a, real b) { return _mm_add_pd(a, b); }
inline real operator-(real a, real b) { return _mm_sub_pd(a, b); }
inline real operator*(real a, real b) { return _mm_mul_pd(a, b); }
inline real operator/(real a, real b) { return _mm_div_pd(a, b); }

#elif defined(NEXT_AVX512_32)

inline real operator+(real a, real b) { return _mm512_add_ps(a, b); }
inline real operator-(real a, real b) { return _mm512_sub_ps(a, b); }
inline real operator*(real a, real b) { return _mm512_mul_ps(a, b); }
inline real operator/(real a, real b) { return _mm512_div_ps(a, b); }

#elif defined(NEXT_AVX512_64)

inline real operator+(real a, real b) { return _mm512_add_pd(a, b); }
inline real operator-(real a, real b) { return _mm512_sub_pd(a, b); }
inline real operator*(real a, real b) { return _mm512_mul_pd(a, b); }
inline real operator/(real a, real b) { return _mm512_div_pd(a, b); }

#endif