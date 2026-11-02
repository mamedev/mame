// license:MIT
// copyright-holders:Stefanos Kornilios Mitsis Poiitidis, R. Belmont
/*****************************************************************************
 *
 *   sh4fpu.h
 *
 *   SH-4 vector and approximation FPU instructions: FIPR, FTRV, FSCA, FSRRA
 *
 *****************************************************************************/

#ifndef MAME_CPU_SH_SH4FPU_H
#define MAME_CPU_SH_SH4FPU_H

#pragma once

namespace sh4fpu {

// exceptions an instruction raises, in the bit order of the FPSCR cause field
enum : u32
{
	CAUSE_I = 0x01,     // inexact
	CAUSE_U = 0x02,     // underflow
	CAUSE_O = 0x04,     // overflow
	CAUSE_Z = 0x08,     // division by zero
	CAUSE_V = 0x10,     // invalid operation
	CAUSE_E = 0x20      // FPU error
};

// All values are IEEE-754 single precision bit patterns.
// rz is true when FPSCR.RM selects round to zero, dn is FPSCR.DN.

// inner product of FVm and FVn, which goes to the last element of FVn
u32 fipr(const u32 *fvm, const u32 *fvn, bool rz, bool dn, u32 &cause);

// fv = xmtrx * fv, xmtrx being XF0-XF15 (column major); returns the exceptions raised
u32 ftrv(const u32 *xmtrx, u32 *fv, bool rz, bool dn);

// reciprocal of the square root; there is no result when an FPU error is raised
u32 fsrra(u32 x, bool rz, bool dn, u32 &cause);

// sine of angle * 2 * pi / 65536 in units of 2^-26, the cosine is the sine of angle + 0x4000
s32 fsca(u32 angle);

// what FSCA makes of that value
u32 fsca_round(s32 value, bool rz);

} // namespace sh4fpu

#endif // MAME_CPU_SH_SH4FPU_H
