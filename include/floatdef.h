#pragma once
#if defined(NEXT_FP64)
    using real = double;
#elif defined(NEXT_FP32)
    using real = float;
#else
    #error "You must define NEXT_FP64 or NEXT_FP32 to proceed."
#endif