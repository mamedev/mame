// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    dsppfe.h

    Front-end for DSPP recompiler

***************************************************************************/

#ifndef MAME_CPU_DSPP_DSPPFE_H
#define MAME_CPU_DSPP_DSPPFE_H

#pragma once

#include "dspp.h"
#include "cpu/drcfe.h"


//**************************************************************************
//  MACROS
//**************************************************************************

// register flags 0
#define REGFLAG_R(n)                    (1 << (n))
#define REGFLAG_RZ(n)                   (((n) == 0) ? 0 : REGFLAG_R(n))

// register flags 1
#define REGFLAG_FR(n)                   (1 << (n))

// register flags 2
#define REGFLAG_CR(n)                   (0xf0000000 >> (4 * (n)))
#define REGFLAG_CR_BIT(n)               (0x80000000 >> (n))

// register flags 3
#define REGFLAG_XER_CA                  (1 << 0)
#define REGFLAG_XER_OV                  (1 << 1)
#define REGFLAG_XER_SO                  (1 << 2)
#define REGFLAG_XER_COUNT               (1 << 3)
#define REGFLAG_CTR                     (1 << 4)
#define REGFLAG_LR                      (1 << 5)
#define REGFLAG_FPSCR(n)                (1 << (6 + (n)))



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class dspp_frontend : public drc_frontend
{
public:
	// construction/destruction
	dspp_frontend(dspp_device *dspp, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);

protected:
	// required overrides
	virtual bool describe(opcode_desc &desc, const opcode_desc *prev) override;

private:
	// inlines

	// internal helpers
	void describe_special(uint16_t op, opcode_desc &desc);
	void describe_branch(uint16_t op, opcode_desc &desc);
	void describe_complex_branch(uint16_t op, opcode_desc &desc);
	void describe_arithmetic(uint16_t op, opcode_desc &desc);
	void parse_operands(uint16_t op, opcode_desc &desc, uint32_t numops);

	// internal state
	dspp_device *m_dspp;
};

#endif // MAME_CPU_DSPP_DSPPFE_H
