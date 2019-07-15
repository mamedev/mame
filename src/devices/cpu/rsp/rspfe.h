// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    rspfe.h

    Front-end for RSP recompiler

***************************************************************************/
#ifndef MAME_CPU_RSP_RSPFE_H
#define MAME_CPU_RSP_RSPFE_H

#pragma once

#include "rsp.h"
#include "cpu/drcfe.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// register flags 0
#define REGFLAG_R(n)                    (((n) == 0) ? 0 : (1 << (n)))



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class rsp_device::frontend : public drc_frontend
{
public:
	// construction/destruction
	frontend(rsp_device &rsp, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);

protected:
	// required overrides
	virtual bool describe(opcode_desc &desc, const opcode_desc *prev) override;

private:
	// internal helpers
	bool describe_special(uint32_t op, opcode_desc &desc);
	bool describe_regimm(uint32_t op, opcode_desc &desc);
	bool describe_cop0(uint32_t op, opcode_desc &desc);
	bool describe_cop2(uint32_t op, opcode_desc &desc);

	// internal state
	rsp_device &m_rsp;
};



#endif // MAME_CPU_RSP_RSPFE_H
