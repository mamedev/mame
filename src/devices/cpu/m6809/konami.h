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

	// instructions
	void lmul();
	void divx();

	// miscellaneous
	void set_lines(uint8_t data);
	void execute_one();

	uint8_t m_bcount;
	uint8_t m_temp_im;
};

#define KONAMI_IRQ_LINE  M6809_IRQ_LINE   /* 0 - IRQ line number */
#define KONAMI_FIRQ_LINE M6809_FIRQ_LINE  /* 1 - FIRQ line number */

#endif // MAME_CPU_M6809_KONAMI_H
