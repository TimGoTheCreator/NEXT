#pragma once

//---------------------------------------------
// Precision selection (scalar only)
//---------------------------------------------
#if defined(NEXT_FP64)

    using real = double;

#elif defined(NEXT_FP32)

    using real = float;

#else
    #error "Define one of: NEXT_FP32 or NEXT_FP64."
#endif
