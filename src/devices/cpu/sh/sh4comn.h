// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*****************************************************************************
 *
 *   sh4comn.h
 *
 *   SH-4 non-specific components
 *
 *****************************************************************************/

#ifndef MAME_CPU_SH_SH4COMN_H
#define MAME_CPU_SH_SH4COMN_H

#pragma once

#include "sh.h"

#define VERBOSE (0)
#include "logmacro.h"

#define EXPPRI(pl, po, p, n)    (((4 - (pl)) << 24) | ((15 - (po)) << 16) | ((p) << 8) | (255 - (n)))
#define NMIPRI()                EXPPRI(3, 0, 16, SH4_INTC_NMI)
#define INTPRI(p, n)            EXPPRI(4, 2, p, n)

#define FP_RS(r) m_sh2_state->m_fr[(r)] // binary representation of single precision floating point register r
#define FP_RFS(r) *( (float  *)(m_sh2_state->m_fr + (r)) ) // single precision floating point register r
#define FP_RFD(r) *( (double *)(m_sh2_state->m_fr + (r)) ) // double precision floating point register r
#define FP_XS(r) m_sh2_state->m_xf[(r)] // binary representation of extended single precision floating point register r
#define FP_XFS(r) *( (float  *)(m_sh2_state->m_xf + (r)) ) // single precision extended floating point register r
#define FP_XFD(r) *( (double *)(m_sh2_state->m_xf + (r)) ) // double precision extended floating point register r
#ifdef LSB_FIRST
#define FP_RS2(r) m_sh2_state->m_fr[(r) ^ m_sh2_state->m_fpu_pr]
#define FP_RFS2(r) *( (float  *)(m_sh2_state->m_fr + ((r) ^ m_sh2_state->m_fpu_pr)) )
#define FP_XS2(r) m_sh2_state->m_xf[(r) ^ m_sh2_state->m_fpu_pr]
#define FP_XFS2(r) *( (float  *)(m_sh2_state->m_xf + ((r) ^ m_sh2_state->m_fpu_pr)) )
#endif

#define FPSCR           mem(&m_sh2_state->m_fpscr)
#define FPS32(reg)      m_fs_regmap[reg]
#define FPD32(reg)      m_fd_regmap[reg & 14]
enum
{
	ICF  = 0x00800000,
	OCFA = 0x00080000,
	OCFB = 0x00040000,
	OVF  = 0x00020000
};

#define FD  0x00008000
#define BL  0x10000000
#define sRB 0x20000000
#define MD  0x40000000

/* 29 bits */
#define SH34_AM  0x1fffffff

#define SH34_FLAGS   (MD | sRB | BL | FD | SH_M | SH_Q | SH_I | SH_S | SH_T)

/* Bits in FPSCR */
#define RM  0x00000003
#define DN  0x00040000
#define PR  0x00080000
#define SZ  0x00100000
#define FR  0x00200000

#define REGFLAG_R(n)                    (1 << (n))

/* additional register flags 1 */
#define REGFLAG_SGR                     (1 << 6)
#define REGFLAG_FPUL                    (1 << 7)
#define REGFLAG_FPSCR                   (1 << 8)
#define REGFLAG_DBR                     (1 << 9)
#define REGFLAG_SSR                     (1 << 10)
#define REGFLAG_SPC                     (1 << 11)

#endif // MAME_CPU_SH_SH4COMN_H
