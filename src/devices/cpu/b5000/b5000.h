// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell B5000 MCU core implementation

*/

#ifndef MAME_CPU_B5000_B5000_H
#define MAME_CPU_B5000_B5000_H

#pragma once

#include "b5000base.h"

// pinout reference

/*

*/

class b5000_cpu_device : public b5000_base_device
{
public:
	b5000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface overrides
	virtual void execute_one() override;

	void data_map(address_map &map);
	void program_map(address_map &map);

	// opcode helpers
	u8 ram_r();
	void ram_w(u8 data);
	void pop_pc();
	void push_pc();

	// opcode handlers

};


DECLARE_DEVICE_TYPE(B5000, b5000_cpu_device)

#endif // MAME_CPU_B5000_B5000_H
