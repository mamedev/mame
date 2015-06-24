// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    rspcp2.h

    Interface file for Reality Signal Processor (RSP) vector extensions.

***************************************************************************/

#pragma once

#ifndef __RSPCP2_H__
#define __RSPCP2_H__

#include "cpu/drcuml.h"
#include "rsp.h"

union VECTOR_REG
{
	UINT64 d[2];
	UINT32 l[4];
	INT16 s[8];
	UINT8 b[16];
};

union ACCUMULATOR_REG
{
	UINT64 q;
	UINT32 l[2];
	UINT16 w[4];
};

struct compiler_state;

class rsp_cop2
{
	friend class rsp_device;

protected:
	rsp_cop2(rsp_device &rsp, running_machine &machine);

	virtual void init();
	virtual void start();

	virtual int generate_cop2(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc) { return TRUE; }
	virtual int generate_lwc2(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc) { return TRUE; }
	virtual int generate_swc2(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc) { return TRUE; }

	virtual void state_string_export(const int index, std::string &str);

public:
	virtual ~rsp_cop2();

	virtual void lbv() { }
	virtual void lsv() { }
	virtual void llv() { }
	virtual void ldv() { }
	virtual void lqv() { }
	virtual void lrv() { }
	virtual void lpv() { }
	virtual void luv() { }
	virtual void lhv() { }
	virtual void lfv() { }
	virtual void lwv() { }
	virtual void ltv() { }
	virtual void sbv() { }
	virtual void ssv() { }
	virtual void slv() { }
	virtual void sdv() { }
	virtual void sqv() { }
	virtual void srv() { }
	virtual void spv() { }
	virtual void suv() { }
	virtual void shv() { }
	virtual void sfv() { }
	virtual void swv() { }
	virtual void stv() { }
	virtual void vmulf() { }
	virtual void vmulu() { }
	virtual void vmudl() { }
	virtual void vmudm() { }
	virtual void vmudn() { }
	virtual void vmudh() { }
	virtual void vmacf() { }
	virtual void vmacu() { }
	virtual void vmadl() { }
	virtual void vmadm() { }
	virtual void vmadn() { }
	virtual void vmadh() { }
	virtual void vadd() { }
	virtual void vsub() { }
	virtual void vabs() { }
	virtual void vaddc() { }
	virtual void vsubc() { }
	virtual void vaddb() { }
	virtual void vsaw() { }
	virtual void vlt() { }
	virtual void veq() { }
	virtual void vne() { }
	virtual void vge() { }
	virtual void vcl() { }
	virtual void vch() { }
	virtual void vcr() { }
	virtual void vmrg() { }
	virtual void vand() { }
	virtual void vnand() { }
	virtual void vor() { }
	virtual void vnor() { }
	virtual void vxor() { }
	virtual void vnxor() { }
	virtual void vrcp() { }
	virtual void vrcpl() { }
	virtual void vrcph() { }
	virtual void vmov() { }
	virtual void vrsql() { }
	virtual void vrsqh() { }
	virtual void vrsq() { }
	virtual void mfc2();
	virtual void cfc2();
	virtual void mtc2();
	virtual void ctc2();

	virtual void    handle_cop2(UINT32 op);

	void            log_instruction_execution();
	virtual void    cfunc_unimplemented_opcode() { }

protected:
	virtual int     generate_vector_opcode(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc) { return TRUE; }

	UINT16          SATURATE_ACCUM(int accum, int slice, UINT16 negative, UINT16 positive);
	UINT16          SATURATE_ACCUM1(int accum, UINT16 negative, UINT16 positive);

	// Data that needs to be stored close to the generated DRC code
	struct internal_rspcop2_state
	{
		UINT32      op;
	};

	internal_rspcop2_state *m_rspcop2_state;
	rsp_device&     m_rsp;
	running_machine& m_machine;
	UINT32          m_vres[8];          /* used for temporary vector results */

	VECTOR_REG      m_v[32];

	ACCUMULATOR_REG m_accum[8];
	UINT16          m_vflag[6][8];

	INT32           m_reciprocal_res;
	UINT32          m_reciprocal_high;
	INT32           m_dp_allowed;

private:
	void            handle_lwc2(UINT32 op);
	void            handle_swc2(UINT32 op);
	void            handle_vector_ops(UINT32 op);
};

#endif /* __RSPCP2_H__ */
