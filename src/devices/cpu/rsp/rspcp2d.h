// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    rspcp2d.h

    Interface file for Reality Signal Processor (RSP) vector extensions
    using Universal Machine Language (UML) dynamic recompilation.

***************************************************************************/

#pragma once

#ifndef __RSPCP2D_H__
#define __RSPCP2D_H__

#include "cpu/drcuml.h"
#include "rsp.h"
#include "rspcp2.h"

class rsp_cop2_drc : public rsp_cop2
{
	friend class rsp_device;

	rsp_cop2_drc(rsp_device &rsp, running_machine &machine) : rsp_cop2(rsp, machine) { }

	virtual int generate_cop2(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc);
	virtual int generate_lwc2(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc);
	virtual int generate_swc2(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc);

	virtual void state_string_export(const int index, std::string &str);

	void cfunc_unimplemented_opcode();

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

private:
	virtual int     generate_vector_opcode(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc);
};

#endif /* __RSPCP2D_H__ */
