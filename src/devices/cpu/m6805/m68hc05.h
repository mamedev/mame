// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_CPU_M6805_M68HC05_H
#define MAME_CPU_M6805_M68HC05_H

#pragma once

#include "m6805.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern device_type const M68HC05C4;


//**************************************************************************
//  TYPE DECLARATIONS
//**************************************************************************

// ======================> m68hc05_device

class m68hc05_device : public m6805_base_device
{
protected:
	m68hc05_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock,
			device_type type,
			char const *name,
			address_map_delegate internal_map,
			char const *shortname,
			char const *source);

	virtual void execute_set_input(int inputnum, int state) override;
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const override;
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const override;

	virtual offs_t disasm_disassemble(
			std::ostream &stream,
			offs_t pc,
			const uint8_t *oprom,
			const uint8_t *opram,
			uint32_t options) override;
};


// ======================> m68hc05c4_device

class m68hc05c4_device : public m68hc05_device
{
public:
	m68hc05c4_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	DECLARE_ADDRESS_MAP(c4_map, 8);

	virtual offs_t disasm_disassemble(
			std::ostream &stream,
			offs_t pc,
			const uint8_t *oprom,
			const uint8_t *opram,
			uint32_t options) override;
};

#endif // MAME_CPU_M6805_M68HC05_H
