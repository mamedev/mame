/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2026 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 1999 Dag Lem <resid@nimrod.no>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef RESIDFP_DEFS_H
#define RESIDFP_DEFS_H

/**
 * \file residfp_defs.h
 *
 * Public macros and definitions.
 */

#ifndef RESIDFP_EXTERN
/* DLL building support on win32 hosts */
#  ifdef DLL_EXPORT      /* defined by libtool (if required) */
#    define RESIDFP_EXTERN __declspec(dllexport)
#  endif
#  ifdef RESIDFP_DLL_IMPORT  /* define if linking with this dll */
#    define RESIDFP_EXTERN __declspec(dllimport)
#  endif
#  ifndef RESIDFP_EXTERN     /* static linking or !_WIN32 */
#    if (defined(__GNUC__) && (__GNUC__ >= 4)) || defined(__clang__)
#      define RESIDFP_EXTERN __attribute__ ((visibility("default")))
#    else
#      define RESIDFP_EXTERN
#    endif
#  endif
#endif

namespace reSIDfp
{
    /**
     *  The SID chip model, either MOS6581 or CSG8580
     */
    typedef enum {
        MOS6581,    ///< Old MOS6581
        CSG8580     ///< New CSG8580
    } ChipModel;

    /**
     * The combined waveforms strength
     */
    typedef enum { AVERAGE, WEAK, STRONG } CombinedWaveforms;

    /**
     * The resampling method
     */
    typedef enum {
        DECIMATE,   ///< Linear interpolation (fast but low quality)
        RESAMPLE,   ///< Sinc resampling (high quality but CPU intensive)
        NONE        ///< No resampling (raw 1MHz output)
    } SamplingMethod;
}

#endif
