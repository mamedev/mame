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
public:
	rsp_cop2_drc(rsp_device &rsp, running_machine &machine) : rsp_cop2(rsp, machine) { }
private:
	virtual int generate_cop2(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc) override;
	virtual int generate_lwc2(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc) override;
	virtual int generate_swc2(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc) override;

	virtual void state_string_export(const int index, std::string &str) override;

	void cfunc_unimplemented_opcode() override;

public:
	virtual void lbv() override;
	virtual void lsv() override;
	virtual void llv() override;
	virtual void ldv() override;
	virtual void lqv() override;
	virtual void lrv() override;
	virtual void lpv() override;
	virtual void luv() override;
	virtual void lhv() override;
	virtual void lfv() override;
	virtual void lwv() override;
	virtual void ltv() override;
	virtual void sbv() override;
	virtual void ssv() override;
	virtual void slv() override;
	virtual void sdv() override;
	virtual void sqv() override;
	virtual void srv() override;
	virtual void spv() override;
	virtual void suv() override;
	virtual void shv() override;
	virtual void sfv() override;
	virtual void swv() override;
	virtual void stv() override;
	virtual void vmulf() override;
	virtual void vmulu() override;
	virtual void vmudl() override;
	virtual void vmudm() override;
	virtual void vmudn() override;
	virtual void vmudh() override;
	virtual void vmacf() override;
	virtual void vmacu() override;
	virtual void vmadl() override;
	virtual void vmadm() override;
	virtual void vmadn() override;
	virtual void vmadh() override;
	virtual void vadd() override;
	virtual void vsub() override;
	virtual void vabs() override;
	virtual void vaddc() override;
	virtual void vsubc() override;
	virtual void vaddb() override;
	virtual void vsaw() override;
	virtual void vlt() override;
	virtual void veq() override;
	virtual void vne() override;
	virtual void vge() override;
	virtual void vcl() override;
	virtual void vch() override;
	virtual void vcr() override;
	virtual void vmrg() override;
	virtual void vand() override;
	virtual void vnand() override;
	virtual void vor() override;
	virtual void vnor() override;
	virtual void vxor() override;
	virtual void vnxor() override;
	virtual void vrcp() override;
	virtual void vrcpl() override;
	virtual void vrcph() override;
	virtual void vmov() override;
	virtual void vrsql() override;
	virtual void vrsqh() override;
	virtual void vrsq() override;
	virtual void mfc2() override;
	virtual void cfc2() override;
	virtual void mtc2() override;
	virtual void ctc2() override;

private:
	virtual int     generate_vector_opcode(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc) override;
};

#endif /* __RSPCP2D_H__ */
