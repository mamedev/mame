/***************************************************************************

    rgbutil.h

    Utility definitions for RGB manipulation. Allows RGB handling to be
    performed in an abstracted fashion and optimized with SIMD.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef __RGBUTIL__
#define __RGBUTIL__

/* use SSE on 64-bit implementations, where it can be assumed */
#if (defined(__SSE2__) && defined(PTR64))
#include "rgbsse.h"
#elif defined(__ALTIVEC__)
#include "rgbvmx.h"
#else
#include "rgbgen.h"
#endif

#endif /* __RGBUTIL__ */
