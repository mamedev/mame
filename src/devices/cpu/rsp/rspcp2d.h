// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    rspcp2d.h

    Interface file for Reality Signal Processor (RSP) vector extensions
    using Universal Machine Language (UML) dynamic recompilation.

***************************************************************************/
#ifndef MAME_CPU_RSP_RSPCP2D_H
#define MAME_CPU_RSP_RSPCP2D_H

#pragma once

#include "rsp.h"
#include "rspcp2.h"
#include "cpu/drcuml.h"


class rsp_device::cop2_drc : public rsp_device::cop2
{
	friend class rsp_device;
public:
	cop2_drc(rsp_device &rsp, running_machine &machine) : cop2(rsp, machine) { }
private:
	virtual bool generate_cop2(drcuml_block &block, rsp_device::compiler_state &compiler, const opcode_desc *desc) override;
	virtual bool generate_lwc2(drcuml_block &block, rsp_device::compiler_state &compiler, const opcode_desc *desc) override;
	virtual bool generate_swc2(drcuml_block &block, rsp_device::compiler_state &compiler, const opcode_desc *desc) override;

	virtual void state_string_export(const int index, std::string &str) const override;

	void cfunc_unimplemented_opcode() override;

	static void unimplemented_opcode(void *param) { ((cop2 *)param)->cfunc_unimplemented_opcode(); }
	static void cfunc_lbv(void *param) { ((cop2 *)param)->lbv(); }
	static void cfunc_lsv(void *param) { ((cop2 *)param)->lsv(); }
	static void cfunc_llv(void *param) { ((cop2 *)param)->llv(); }
	static void cfunc_ldv(void *param) { ((cop2 *)param)->ldv(); }
	static void cfunc_lqv(void *param) { ((cop2 *)param)->lqv(); }
	static void cfunc_lrv(void *param) { ((cop2 *)param)->lrv(); }
	static void cfunc_lpv(void *param) { ((cop2 *)param)->lpv(); }
	static void cfunc_luv(void *param) { ((cop2 *)param)->luv(); }
	static void cfunc_lhv(void *param) { ((cop2 *)param)->lhv(); }
	static void cfunc_lfv(void *param) { ((cop2 *)param)->lfv(); }
	static void cfunc_lwv(void *param) { ((cop2 *)param)->lwv(); }
	static void cfunc_ltv(void *param) { ((cop2 *)param)->ltv(); }
	static void cfunc_sbv(void *param) { ((cop2 *)param)->sbv(); }
	static void cfunc_ssv(void *param) { ((cop2 *)param)->ssv(); }
	static void cfunc_slv(void *param) { ((cop2 *)param)->slv(); }
	static void cfunc_sdv(void *param) { ((cop2 *)param)->sdv(); }
	static void cfunc_sqv(void *param) { ((cop2 *)param)->sqv(); }
	static void cfunc_srv(void *param) { ((cop2 *)param)->srv(); }
	static void cfunc_spv(void *param) { ((cop2 *)param)->spv(); }
	static void cfunc_suv(void *param) { ((cop2 *)param)->suv(); }
	static void cfunc_shv(void *param) { ((cop2 *)param)->shv(); }
	static void cfunc_sfv(void *param) { ((cop2 *)param)->sfv(); }
	static void cfunc_swv(void *param) { ((cop2 *)param)->swv(); }
	static void cfunc_stv(void *param) { ((cop2 *)param)->stv(); }
	static void cfunc_vmulf(void *param) { ((cop2 *)param)->vmulf(); }
	static void cfunc_vmulu(void *param) { ((cop2 *)param)->vmulu(); }
	static void cfunc_vmudl(void *param) { ((cop2 *)param)->vmudl(); }
	static void cfunc_vmudm(void *param) { ((cop2 *)param)->vmudm(); }
	static void cfunc_vmudn(void *param) { ((cop2 *)param)->vmudn(); }
	static void cfunc_vmudh(void *param) { ((cop2 *)param)->vmudh(); }
	static void cfunc_vmacf(void *param) { ((cop2 *)param)->vmacf(); }
	static void cfunc_vmacu(void *param) { ((cop2 *)param)->vmacu(); }
	static void cfunc_vmadl(void *param) { ((cop2 *)param)->vmadl(); }
	static void cfunc_vmadm(void *param) { ((cop2 *)param)->vmadm(); }
	static void cfunc_vmadn(void *param) { ((cop2 *)param)->vmadn(); }
	static void cfunc_vmadh(void *param) { ((cop2 *)param)->vmadh(); }
	static void cfunc_vadd(void *param) { ((cop2 *)param)->vadd(); }
	static void cfunc_vsub(void *param) { ((cop2 *)param)->vsub(); }
	static void cfunc_vabs(void *param) { ((cop2 *)param)->vabs(); }
	static void cfunc_vaddc(void *param) { ((cop2 *)param)->vaddc(); }
	static void cfunc_vsubc(void *param) { ((cop2 *)param)->vsubc(); }
	static void cfunc_vaddb(void *param) { ((cop2 *)param)->vaddb(); }
	static void cfunc_vsaw(void *param) { ((cop2 *)param)->vsaw(); }
	static void cfunc_vlt(void *param) { ((cop2 *)param)->vlt(); }
	static void cfunc_veq(void *param) { ((cop2 *)param)->veq(); }
	static void cfunc_vne(void *param) { ((cop2 *)param)->vne(); }
	static void cfunc_vge(void *param) { ((cop2 *)param)->vge(); }
	static void cfunc_vcl(void *param) { ((cop2 *)param)->vcl(); }
	static void cfunc_vch(void *param) { ((cop2 *)param)->vch(); }
	static void cfunc_vcr(void *param) { ((cop2 *)param)->vcr(); }
	static void cfunc_vmrg(void *param) { ((cop2 *)param)->vmrg(); }
	static void cfunc_vand(void *param) { ((cop2 *)param)->vand(); }
	static void cfunc_vnand(void *param) { ((cop2 *)param)->vnand(); }
	static void cfunc_vor(void *param) { ((cop2 *)param)->vor(); }
	static void cfunc_vnor(void *param) { ((cop2 *)param)->vnor(); }
	static void cfunc_vxor(void *param) { ((cop2 *)param)->vxor(); }
	static void cfunc_vnxor(void *param) { ((cop2 *)param)->vnxor(); }
	static void cfunc_vrcp(void *param) { ((cop2 *)param)->vrcp(); }
	static void cfunc_vrcpl(void *param) { ((cop2 *)param)->vrcpl(); }
	static void cfunc_vrcph(void *param) { ((cop2 *)param)->vrcph(); }
	static void cfunc_vmov(void *param) { ((cop2 *)param)->vmov(); }
	static void cfunc_vrsq(void *param) { ((cop2 *)param)->vrsq(); }
	static void cfunc_vrsql(void *param) { ((cop2 *)param)->vrsql(); }
	static void cfunc_vrsqh(void *param) { ((cop2 *)param)->vrsqh(); }
	static void cfunc_mfc2(void *param) { ((cop2 *)param)->mfc2(); }
	static void cfunc_cfc2(void *param) { ((cop2 *)param)->cfc2(); }
	static void cfunc_mtc2(void *param) { ((cop2 *)param)->mtc2(); }
	static void cfunc_ctc2(void *param) { ((cop2 *)param)->ctc2(); }

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
	virtual bool generate_vector_opcode(drcuml_block &block, rsp_device::compiler_state &compiler, const opcode_desc *desc) override;
};

#endif // MAME_CPU_RSP_RSPCP2D_H
