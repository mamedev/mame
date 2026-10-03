// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    Portable Konami CPU emulator

**********************************************************************/

#ifndef MAME_CPU_M6809_KONAMI_H
#define MAME_CPU_M6809_KONAMI_H

#pragma once

#include "m6809.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(KONAMI, konami_cpu_device)

// ======================> konami_cpu_device

class konami_cpu_device : public m6809_base_device
{
public:
	// construction/destruction
	konami_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	auto line() { return m_set_lines.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual void execute_run() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual bool hd6309_native_mode() override { return true; }

private:
	typedef m6809_base_device super;

	// incidentals
	devcb_write8 m_set_lines;

	// konami-specific addressing modes
	uint16_t &ireg();
	uint8_t read_operand();
	uint8_t read_operand(int ordinal);
	void write_operand(uint8_t data);
	void write_operand(int ordinal, uint8_t data);
	uint16_t read_exgtfr_register(uint8_t reg);
	void write_exgtfr_register(uint8_t reg, uint16_t value);
	uint8_t read_konami_opcode();

	// instructions
	void lmul();
	void divx();
	void daa_konami();

	// miscellaneous
	void set_lines(uint8_t data);
	void begin_instruction_timing(uint8_t opcode);
	void execute_one();
	void finish_instruction_timing(int cycle_count);
	int documented_instruction_cycles() const;

	struct timing_scope
	{
		konami_cpu_device &cpu;
		int const cycle_count;
		timing_scope(konami_cpu_device &device) : cpu(device), cycle_count(device.m_icount) { }
		~timing_scope() { cpu.finish_instruction_timing(cycle_count); }
	};

	uint8_t m_bcount;
	uint8_t m_temp_im;
	uint8_t m_dp_exgtfr_low;
	uint8_t m_pc_transfer_opcode;
	bool m_pc_transfer_opcode_valid;
	bool m_timing_active;
	uint8_t m_timing_opcode;
	uint8_t m_timing_postbyte;
	uint8_t m_timing_operand1;
	uint8_t m_timing_cc;
	uint8_t m_timing_a;
	uint8_t m_timing_b;
	uint16_t m_timing_u;
	uint16_t m_timing_x;
	int m_timing_elapsed;
};

#define KONAMI_IRQ_LINE  M6809_IRQ_LINE   /* 0 - IRQ line number */
#define KONAMI_FIRQ_LINE M6809_FIRQ_LINE  /* 1 - FIRQ line number */

#endif // MAME_CPU_M6809_KONAMI_H
