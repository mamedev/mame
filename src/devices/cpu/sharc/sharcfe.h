// license:BSD-3-Clause
// copyright-holders:Ville Linde

/******************************************************************************

    Front-end for SHARC recompiler

******************************************************************************/
#ifndef MAME_CPU_SHARC_SHARCFE_H
#define MAME_CPU_SHARC_SHARCFE_H

#pragma once

#include "sharc.h"
#include "cpu/drcfe.h"


class sharc_frontend : public drc_frontend
{
public:
	sharc_frontend(adsp21062_device *sharc, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);
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
		uint16_t entrytype;
		uint32_t userflags;
	};

	struct LOOP_DESCRIPTOR
	{
		uint32_t start_pc;
		uint32_t end_pc;
		uint32_t astat_check_pc;
		LOOP_TYPE type;
		int condition;
	};

protected:
	// required overrides
	virtual bool describe(opcode_desc &desc, const opcode_desc *prev) override;

private:
	bool describe_compute(opcode_desc &desc, uint64_t opcode);
	bool describe_ureg_access(opcode_desc &desc, int reg, UREG_ACCESS access);
	bool describe_shiftop_imm(opcode_desc &desc, int shiftop, int rn, int rx);
	void describe_if_condition(opcode_desc &desc, int condition);

	void insert_loop(const LOOP_DESCRIPTOR &loopdesc);
	void add_loop_entry(uint32_t pc, uint8_t type, uint32_t userflags);
	bool is_loop_evaluation(uint32_t pc);
	bool is_loop_start(uint32_t pc);
	bool is_astat_delay_check(uint32_t pc);

	adsp21062_device *m_sharc;

	std::unique_ptr<LOOP_ENTRY[]> m_loopmap;
};

#endif // MAME_CPU_SHARC_SHARCFE_H
