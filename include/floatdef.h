// This file is licensed under the LGPL3+, in case you didnt get the Lesser GNU General public license in this project, check
// <https://www.gnu.org/licenses/licenses.html>
#pragma once

//---------------------------------------------
// Precision selection 
//---------------------------------------------
#if defined(NEXT_FP64)

    using real = double;

#elif defined(NEXT_FP32)

    using real = float;

#else
    #error "Define one of: NEXT_FP32 or NEXT_FP64."
#endif
