// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_CLIPPER_COMMON_H
#define MAME_CPU_CLIPPER_COMMON_H

#pragma once

enum addressing_modes : u8
{
	ADDR_MODE_PC32  = 0x10, // pc relative with 32 bit displacement
	ADDR_MODE_ABS32 = 0x30, // 32 bit absolute
	ADDR_MODE_REL32 = 0x60, // relative with 32 bit displacement
	ADDR_MODE_PC16  = 0x90, // pc relative with 16 bit displacement
	ADDR_MODE_REL12 = 0xa0, // relative with 12 bit displacement
	ADDR_MODE_ABS16 = 0xb0, // 16 bit absolute
	ADDR_MODE_PCX   = 0xd0, // pc indexed
	ADDR_MODE_RELX  = 0xe0  // relative indexed
};

enum ssw_mask : u32
{
	SSW_IN  = 0x0000000f, // interrupt number (4 bits)
	SSW_IL  = 0x000000f0, // interrupt level (4 bits)
	SSW_EI  = 0x00000100, // enable interrupts
	SSW_ID  = 0x0001fe00, // cpu rev # and type (8 bits)
							// unused (5 bits)
	SSW_FRD = 0x00400000, // floating registers dirty
	SSW_TP  = 0x00800000, // trace trap pending
	SSW_ECM = 0x01000000, // enable corrected memory error
	SSW_DF  = 0x02000000, // fpu disabled
	SSW_M   = 0x04000000, // mapped mode
	SSW_KU  = 0x08000000, // user protect key
	SSW_UU  = 0x10000000, // user data mode
	SSW_K   = 0x20000000, // protect key
	SSW_U   = 0x40000000, // user mode
	SSW_P   = 0x80000000, // previous mode
};

enum exception_vector : u16
{
	// data memory trap group
	EXCEPTION_D_CORRECTED_MEMORY_ERROR     = 0x108,
	EXCEPTION_D_UNCORRECTABLE_MEMORY_ERROR = 0x110,
	EXCEPTION_D_ALIGNMENT_FAULT            = 0x120,
	EXCEPTION_D_PAGE_FAULT                 = 0x128,
	EXCEPTION_D_READ_PROTECT_FAULT         = 0x130,
	EXCEPTION_D_WRITE_PROTECT_FAULT        = 0x138,

	// floating-point arithmetic trap group
	EXCEPTION_FLOATING_INEXACT             = 0x180,
	EXCEPTION_FLOATING_UNDERFLOW           = 0x188,
	EXCEPTION_FLOATING_DIVIDE_BY_ZERO      = 0x190,
	EXCEPTION_FLOATING_OVERFLOW            = 0x1a0,
	EXCEPTION_FLOATING_INVALID_OPERATION   = 0x1c0,

	// integer arithmetic trap group
	EXCEPTION_INTEGER_DIVIDE_BY_ZERO       = 0x208,

	// instruction memory trap group
	EXCEPTION_I_CORRECTED_MEMORY_ERROR     = 0x288,
	EXCEPTION_I_UNCORRECTABLE_MEMORY_ERROR = 0x290,
	EXCEPTION_I_ALIGNMENT_FAULT            = 0x2a0,
	EXCEPTION_I_PAGE_FAULT                 = 0x2a8,
	EXCEPTION_I_EXECUTE_PROTECT_FAULT      = 0x2b0,

	// illegal operation trap group
	EXCEPTION_ILLEGAL_OPERATION            = 0x300,
	EXCEPTION_PRIVILEGED_INSTRUCTION       = 0x308,

	// diagnostic trap group
	EXCEPTION_TRACE                        = 0x380,

	// supervisor calls (0x400-0x7f8)
	EXCEPTION_SUPERVISOR_CALL_BASE         = 0x400,

	// prioritized interrupts (0x800-0xff8)
	EXCEPTION_INTERRUPT_BASE               = 0x800
};

#endif // MAME_CPU_CLIPPER_COMMON_H
