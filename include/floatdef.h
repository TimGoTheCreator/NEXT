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