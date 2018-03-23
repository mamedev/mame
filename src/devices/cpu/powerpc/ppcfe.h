// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ppcfe.h

    Front-end for PowerPC recompiler

***************************************************************************/
#ifndef MAME_CPU_POWERPC_PPCFE_H
#define MAME_CPU_POWERPC_PPCFE_H

#pragma once

#include "ppc.h"

#include "cpu/drcfe.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ppc_device::frontend : public drc_frontend
{
public:
	// register flags 0
	static constexpr u32 REGFLAG_R(unsigned n)      { return 1U << n; }
	static constexpr u32 REGFLAG_RZ(unsigned n)     { return !n ? 0U : REGFLAG_R(n); }

	// register flags 1
	static constexpr u32 REGFLAG_FR(unsigned n)     { return 1U << n; }

	// register flags 2
	static constexpr u32 REGFLAG_CR(unsigned n)     { return 0xf0000000U >> (4 * n); }
	static constexpr u32 REGFLAG_CR_BIT(unsigned n) { return 0x80000000U >> n; }

	// register flags 3
	static constexpr u32 REGFLAG_XER_CA             = 1U << 0;
	static constexpr u32 REGFLAG_XER_OV             = 1U << 1;
	static constexpr u32 REGFLAG_XER_SO             = 1U << 2;
	static constexpr u32 REGFLAG_XER_COUNT          = 1U << 3;
	static constexpr u32 REGFLAG_CTR                = 1U << 4;
	static constexpr u32 REGFLAG_LR                 = 1U << 5;
	static constexpr u32 REGFLAG_FPSCR(unsigned n)  { return 1U << (6 + n); }

	// construction/destruction
	frontend(ppc_device &ppc, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);

protected:
	// required overrides
	virtual bool describe(opcode_desc &desc, const opcode_desc *prev) override;

private:
	// inlines
	uint32_t compute_spr(uint32_t spr) const { return ((spr >> 5) | (spr << 5)) & 0x3ff; }
	bool is_403_class() const { return (m_ppc.m_flavor == ppc_device::PPC_MODEL_403GA || m_ppc.m_flavor == ppc_device::PPC_MODEL_403GB || m_ppc.m_flavor == ppc_device::PPC_MODEL_403GC || m_ppc.m_flavor == ppc_device::PPC_MODEL_403GCX || m_ppc.m_flavor == ppc_device::PPC_MODEL_405GP); }
	bool is_601_class() const { return (m_ppc.m_flavor == ppc_device::PPC_MODEL_601); }
	bool is_602_class() const { return (m_ppc.m_flavor == ppc_device::PPC_MODEL_602); }
	bool is_603_class() const { return (m_ppc.m_flavor == ppc_device::PPC_MODEL_603 || m_ppc.m_flavor == ppc_device::PPC_MODEL_603E || m_ppc.m_flavor == ppc_device::PPC_MODEL_603EV || m_ppc.m_flavor == ppc_device::PPC_MODEL_603R); }

	// internal helpers
	bool describe_13(uint32_t op, opcode_desc &desc, const opcode_desc *prev);
	bool describe_1f(uint32_t op, opcode_desc &desc, const opcode_desc *prev);
	bool describe_3b(uint32_t op, opcode_desc &desc, const opcode_desc *prev);
	bool describe_3f(uint32_t op, opcode_desc &desc, const opcode_desc *prev);

	// internal state
	ppc_device &m_ppc;
};


#endif // MAME_CPU_POWERPC_PPCFE_H
