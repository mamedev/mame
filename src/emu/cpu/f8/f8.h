/*****************************************************************************
 *
 *   f8.h
 *   Portable Fairchild F8 emulator interface
 *
 *   Copyright (c) 2000 Juergen Buchmueller, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/

#ifndef _F8_H
#define _F8_H

#include "cpuintrf.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	F8_PC0=1, F8_PC1, F8_DC0, F8_DC1, F8_W, F8_A, F8_IS,
        F8_J, F8_HU, F8_HL, F8_KU, F8_KL, F8_QU, F8_QL,

        F8_R0, F8_R1, F8_R2, F8_R3, F8_R4, F8_R5, F8_R6, F8_R7, F8_R8,
        F8_R16, F8_R17, F8_R18, F8_R19, F8_R20, F8_R21, F8_R22, F8_R23,
        F8_R24, F8_R25, F8_R26, F8_R27, F8_R28, F8_R29, F8_R30, F8_R31,
        F8_R32, F8_R33, F8_R34, F8_R35, F8_R36, F8_R37, F8_R38, F8_R39,
        F8_R40, F8_R41, F8_R42, F8_R43, F8_R44, F8_R45, F8_R46, F8_R47,
        F8_R48, F8_R49, F8_R50, F8_R51, F8_R52, F8_R53, F8_R54, F8_R55,
        F8_R56, F8_R57, F8_R58, F8_R59, F8_R60, F8_R61, F8_R62, F8_R63
};

#define F8_INT_NONE  0
#define F8_INT_INTR  1

void f8_get_info(UINT32 state, cpuinfo *info);

#ifdef __cplusplus
}
#endif

#endif /* _F8_H */


