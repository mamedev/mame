// license:BSD-3-Clause
// copyright-holders:Ville Linde

/******************************************************************************

    Front-end for SHARC recompiler

******************************************************************************/

#pragma once

#include "sharc.h"
#include "cpu/drcfe.h"

#ifndef __SHARCFE_H__
#define __SHARCFE_H__

class sharc_frontend : public drc_frontend
{
public:
	sharc_frontend(adsp21062_device *sharc, UINT32 window_start, UINT32 window_end, UINT32 max_sequence);
	void flush();

	enum UREG_ACCESS
	{
		UREG_READ,
		UREG_WRITE
	};

	enum LOOP_TYPE
	{
		LOOP_TYPE_COUNTER,
		LOOP_TYPE_CONDITIONAL
	};

	enum LOOP_ENTRY_TYPE
	{
		LOOP_ENTRY_START = 0x1,
		LOOP_ENTRY_EVALUATION = 0x2,
		LOOP_ENTRY_ASTAT_CHECK = 0x4,
	};

	struct LOOP_ENTRY
	{
		UINT16 entrytype;
		UINT8 looptype;
		UINT8 condition;
		UINT32 start_pc;
	};

	struct LOOP_DESCRIPTOR
	{
		UINT32 start_pc;
		UINT32 end_pc;
		UINT32 astat_check_pc;
		LOOP_TYPE type;
		int condition;
	};

protected:
	// required overrides
	virtual bool describe(opcode_desc &desc, const opcode_desc *prev) override;

private:
	bool describe_compute(opcode_desc &desc, UINT64 opcode);
	bool describe_ureg_access(opcode_desc &desc, int reg, UREG_ACCESS access);
	bool describe_shiftop_imm(opcode_desc &desc, int shiftop, int rn, int rx);
	void describe_if_condition(opcode_desc &desc, int condition);

	void insert_loop(const LOOP_DESCRIPTOR &loopdesc);
	void add_loop_entry(UINT32 pc, UINT8 type, UINT32 start_pc, UINT8 looptype, UINT8 condition);
	bool is_loop_evaluation(UINT32 pc);
	bool is_loop_start(UINT32 pc);
	bool is_astat_delay_check(UINT32 pc);

	adsp21062_device *m_sharc;

	std::unique_ptr<LOOP_ENTRY[]> m_loopmap;
};

#endif /* __SHARCFE_H__ */
