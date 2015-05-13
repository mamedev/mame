// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    rspcp2s.h

    Interface file for Reality Signal Processor (RSP) vector extensions
    using SSSE3 SIMD acceleration.

***************************************************************************/

#pragma once

#ifndef __RSPCP2S_H__
#define __RSPCP2S_H__

#include "cpu/drcuml.h"
#include "rsp.h"
#include "rspcp2.h"

#include <tmmintrin.h>

class rsp_cop2_simd : public rsp_cop2_drc
{
	friend class rsp_device;

	rsp_cop2_simd(rsp_device &rsp, running_machine &machine);

	virtual int generate_cop2(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc);
	virtual int generate_lwc2(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc);
	virtual int generate_swc2(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc);

	virtual void state_string_export(const int index, std::string &str);

public:
	virtual void lbv();
	virtual void lsv();
	virtual void llv();
	virtual void ldv();
	virtual void lqv();
	virtual void lrv();
	virtual void lpv();
	virtual void luv();
	virtual void lhv();
	virtual void lfv();
	virtual void lwv();
	virtual void ltv();
	virtual void sbv();
	virtual void ssv();
	virtual void slv();
	virtual void sdv();
	virtual void sqv();
	virtual void srv();
	virtual void spv();
	virtual void suv();
	virtual void shv();
	virtual void sfv();
	virtual void swv();
	virtual void stv();
	virtual void vmulf();
	virtual void vmulu();
	virtual void vmudl();
	virtual void vmudm();
	virtual void vmudn();
	virtual void vmudh();
	virtual void vmacf();
	virtual void vmacu();
	virtual void vmadl();
	virtual void vmadm();
	virtual void vmadn();
	virtual void vmadh();
	virtual void vadd();
	virtual void vsub();
	virtual void vabs();
	virtual void vaddc();
	virtual void vsubc();
	virtual void vaddb();
	virtual void vsaw();
	virtual void vlt();
	virtual void veq();
	virtual void vne();
	virtual void vge();
	virtual void vcl();
	virtual void vch();
	virtual void vcr();
	virtual void vmrg();
	virtual void vand();
	virtual void vnand();
	virtual void vor();
	virtual void vnor();
	virtual void vxor();
	virtual void vnxor();
	virtual void vrcp();
	virtual void vrcpl();
	virtual void vrcph();
	virtual void vmov();
	virtual void vrsql();
	virtual void vrsqh();
	virtual void vrsq();
	virtual void mfc2();
	virtual void cfc2();
	virtual void mtc2();
	virtual void ctc2();

#if SIMUL_SIMD
	void backup_regs();
	void restore_regs();
	void verify_regs();
#endif

private:
	virtual int     generate_vector_opcode(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc);

	UINT16          ACCUM_H(int x);
	UINT16          ACCUM_M(int x);
	UINT16          ACCUM_L(int x);
	UINT16          ACCUM_LL(int x);
	UINT16          CARRY_FLAG(const int x);
	UINT16          COMPARE_FLAG(const int x);
	UINT16          CLIP1_FLAG(const int x);
	UINT16          ZERO_FLAG(const int x);
	UINT16          CLIP2_FLAG(const int x);
	UINT16          SATURATE_ACCUM(int accum, int slice, UINT16 negative, UINT16 positive);

	__m128i         m_accum_h;
	__m128i         m_accum_m;
	__m128i         m_accum_l;
	__m128i         m_accum_ll;

	// Mirror of v[] for now, to be used in parallel as
	// more vector ops are transitioned over
	__m128i         m_xv[32];
	__m128i         m_xvflag[6];

#if SIMUL_SIMD
	UINT32          m_old_r[35];
	UINT8           m_old_dmem[4096];

	UINT32          m_scalar_r[35];
	UINT8           m_scalar_dmem[4096];

	INT32           m_old_reciprocal_res;
	UINT32          m_old_reciprocal_high;
	INT32           m_old_dp_allowed;

	INT32           m_scalar_reciprocal_res;
	UINT32          m_scalar_reciprocal_high;
	INT32           m_scalar_dp_allowed;

	INT32           m_simd_reciprocal_res;
	UINT32          m_simd_reciprocal_high;
	INT32           m_simd_dp_allowed;
#endif
};

#endif /* __RSPCP2S_H__ */
